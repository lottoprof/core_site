#pragma once

#include <string>
#include <sstream>
#include <vector>

template <typename T>
std::string array_to_string(T* array, int size, bool simple = false) {
    std::stringstream sstream;

    if (!simple)
        sstream << "[";

    for (int i = 0; i < size; i++) {
        sstream << array[i];

        if (i != size - 1) {
            sstream << ",";

            if (!simple)
                sstream << ' ';
        }
    }

    if (!simple)
        sstream << "]";

    return sstream.str();
}

template <typename T>
std::string array_to_string(const std::vector<T>& array, bool simple = false) {
    return array_to_string<T>((T*) array.data(), array.size(), simple);
}

std::string fmt(const char *fmt, ...);
