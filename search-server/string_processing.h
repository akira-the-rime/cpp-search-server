#pragma once

#include <set>
#include <string>
#include <vector>

template <class StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string &str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

std::vector<std::string> SplitIntoWords(const std::string &text);