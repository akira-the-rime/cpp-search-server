#pragma once

#include <deque>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : server_link(search_server) { }
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        std::vector<Document> is_found = server_link.FindTopDocuments(raw_query, document_predicate);
        if (is_found.empty()) {
            QueryResult created_result;
            created_result.raw_query_ = raw_query;
            requests_.push_back(created_result);
        }
        for (auto it = requests_.begin(); it != requests_.end() && requests_.size(); ++it) {
            if (++it->shelf_life == min_in_day_) {
                requests_.pop_front();
                it = requests_.begin();
            }
        }
        return is_found;
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus stowed_status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        int shelf_life = 0;
        std::string raw_query_;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& server_link;
};