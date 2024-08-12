#pragma once

#include <vector>

#include "document.h"

using namespace std::string_literals;

template <class Iter>
class Paginator final {
public:
    explicit Paginator(const Iter begin, const Iter end, const size_t page_size) {
        unsigned int count = 0;
        auto it = begin;
        std::vector<Document> temp;

        while (it != end) {
            while (count < page_size && it != end) {
                temp.push_back(*it);
                ++it;
                ++count;
            }

            paginated_.push_back(temp);
            temp.clear();
            count = 0;
        }
    }

    std::vector<std::vector<Document>>::const_iterator begin() const {
        return paginated_.begin();
    }

    std::vector<std::vector<Document>>::const_iterator end() const {
        return paginated_.end();
    }

private:
    std::vector<std::vector<Document>> paginated_;
};

template <typename Container>
auto Paginate(const Container &c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

std::ostream& operator<<(std::ostream &os, const std::vector<Document> &object) {
    for (const auto &document : object) {
        os << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
    }
    return os;
}