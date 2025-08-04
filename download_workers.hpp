#pragma once

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>
#include <chrono>
#include <utility>
#include <list>
#include <set>
#include <string>
#include <forward_list>
#include <stdexcept>
#include <shared_mutex>

using namespace std::chrono_literals;

constexpr static const auto thread_delay  = 25ms;
constexpr static const auto emplace_delay = 1ms;

template<typename T>
class DownloadWorkers {
    struct Payload {
    public:
        T inner;
        std::atomic_bool done;

        template<typename... Args>
        Payload(Args&&... args) : inner(std::forward<Args>(args)...), done(false) {}

        Payload(const Payload&) = delete;
        Payload(Payload&&)      = delete;

        bool operator<(const Payload& other) const {
            return inner < other.inner;
        }
    };

    class Worker {
    public:
        static const int MAX_PAYLOADS = 256;

        Worker(DownloadWorkers& workers) : joining(false) {
            thread = std::thread([=, this, &workers] {
                while (true) {
                    std::shared_lock<std::shared_timed_mutex> lock(works_mutex);

                    if (payloads.empty()) {
                        if (joining.load()) {
                            break;
                        } else {
                            lock.unlock();
                            std::this_thread::sleep_for(thread_delay);
                            continue;
                        }
                    }

                    auto* payload = payloads.front();
                    auto iter = payloads.cbegin();

                    lock.unlock();

                    workers.work(payload->inner);
                    payload->done.store(true);
                    workers.m_done_count++;

                    lock.lock();

                    payloads.erase(iter);
                }
            });
        }

        ~Worker() {
            if (!joining.load()) {
                join();
            }
        }

        Worker(const Worker&) = delete;
        Worker(Worker&&)      = delete;

        void join () {
            joining.store(true);
            thread.join();
        }

        void emplace(Payload* payload) {
            std::unique_lock<std::shared_timed_mutex> lock(works_mutex);

            for (; payloads.size() >= MAX_PAYLOADS; lock.unlock(),
                                                    std::this_thread::sleep_for(emplace_delay),
                                                    lock.lock());

            payloads.emplace_back(payload);
        }

    private:
        std::list<Payload*>     payloads;
        std::thread             thread;
        std::atomic_bool        joining;
        std::shared_timed_mutex works_mutex;
    };

public:
    DownloadWorkers(int workers_count, const std::function<void(T)>& work) :
        m_done_count(0),
        work(work) {

        for (int i = 0; i < workers_count; i++) {
            workers.emplace_front(*this);
        }

        worker_iter = workers.begin();
    }

    DownloadWorkers(const DownloadWorkers&) = delete;
    DownloadWorkers(DownloadWorkers&&)      = delete;

    void try_await() {
        std::list<typename std::set<Payload>::const_iterator> removing;

        for (auto it = payloads.begin(); it != payloads.end(); it++) {
            if ((*it).done.load()) {
                removing.push_back(it);
            }
        }

        for (auto it: removing) {
            payloads.erase(it);
        }
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        try_await();

        auto pair = payloads.emplace(args...);

        // Exit function if didn't succeed to emplace because the element already exists
        if (!pair.second) {
            return;
        }

        worker_iter->emplace(const_cast<Payload*>(&*pair.first));
        worker_iter++;

        if (worker_iter == workers.end()) {
            worker_iter = workers.begin();
        }
    }

    int done_count() const {
        return m_done_count;
    }

private:
    std::set<Payload>         payloads;
    std::forward_list<Worker> workers;
    std::atomic_int           m_done_count;

    typename std::forward_list<Worker>::iterator worker_iter;

    std::function<void(T)> work;
};
