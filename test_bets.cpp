#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <cstddef>
#include <set>

#include "orakel_bets.hpp"

__asm(
    ".global bbbb_file\n"
    ".global _bbbb_file\n"
    "bbbb_file:\n"
    "_bbbb_file:\n"
    ".incbin \"../key.key\"\n"
    ".global bbbb_file_len\n"
    ".global _bbbb_file_len\n"
    "bbbb_file_len:\n"
    "_bbbb_file_len:\n"
    ".int .-bbbb_file \n"
    ".global cert\n"
    "cert:\n"
    ".incbin \"../key.crt\"\n"
    ".global certlen\n"
    "certlen:\n"
    ".int .-cert \n"
);

extern void* bbbb_file;
extern void* bbbb_file_len;

static u_char* data    = (u_char*) &bbbb_file;
static    int* datalen = (int*) &bbbb_file_len;

extern void* cert;
extern void* certlen;

const static std::set<OrakelBets::Game> GAMES = {
    {2115, 5}, {2115, 10}, {2115, 15}, {2115, 20}, {2115, 50}, {2115, 100}, {2115, 250},
    {2115, 500},

    {2113, 4}, {2113, 10}, {2113, 20}, {2113, 50}, {2113, 100}, {2113, 200}, {2113, 300},
    {2113, 500}, {2113, 1000},

    {2112, 4}, {2112, 10}, {2112, 20}, {2112, 50}, {2112, 100}, {2112, 200}, {2112, 500},
    {2112, 1000}, {2112, 2000},

    {2111, 20}, {2111, 40}, {2111, 100}, {2111, 200}, {2111, 500}, {2111, 1000},

    {2110, 20}, {2110, 40}, {2110, 100}, {2110, 200}, {2110, 500}, {2110, 1000},
};

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

int main(int argc, char* argv[]) {
    Orakel orakel((char*) data, *datalen);
    OrakelBets orakel_bets(&orakel, GAMES);

    std::cout << ">>> Welcome to test_bets test utility!" << std::endl;

    while (true) {
        try {
            std::cout << "------------------------------------------------\n>>> Enter command: ";

            char command_raw[256];

            std::cin.getline(command_raw, 256);

            std::cout << "------------------------------------------------" << std::endl;

            auto command = std::string(command_raw);

            //line = "load_bets 2112";

            if (command == "exit" || command == "quit") {
                break;
            }

            if (command.empty()) {
                continue;
            }

            auto splitted = split(command, ' ');

            if (splitted[0] == "load_bets" && splitted.size() == 2) {
                orakel_bets.load_bets(std::stoi(splitted[1]));
            }

            else if (splitted[0] == "get_bets" && splitted.size() == 4) {
                auto bets = orakel_bets.get_bets(
                    std::stoi(splitted[1]),
                    std::stoi(splitted[2]),
                    std::stoi(splitted[3])
                );

                std::cout << "------------------------------------------------\n>>> Result:"
                          << std::endl;

                for (const auto& bet: bets) {
                    std::cout << "game_id: " << bet.game_id << std::endl;
                    std::cout << "number: " << bet.number << std::endl;
                    std::cout << "price: " << bet.price << std::endl;
                    std::cout << "win: " << bet.win << std::endl;

                    std::cout << "mask: ";

                    for (const auto& num: bet.mask) {
                        std::cout << num << ", ";
                    }

                    std::cout << std::endl << std::endl;
                }
            }

            else {
                std::cout << "------------------------------------------------\n>>> Unknown command."
                          << std::endl;
            }
        } catch (const std::ios_base::failure& e) {
            std::cout << "Caught an ios_base::failure.\n"
                      << "Explanatory string: " << e.what() << '\n'
                      << "Error code: " << e.code() << '\n';
        }
    }

    return 0;
}
