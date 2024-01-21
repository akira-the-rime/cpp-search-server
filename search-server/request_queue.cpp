#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query, DocumentStatus stowed_status) {
    return AddFindRequest(raw_query, [stowed_status](int document_id, DocumentStatus status, int rating) {
        return status == stowed_status;
        });
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}
int RequestQueue::GetNoResultRequests() const {
    return requests_.size();
}