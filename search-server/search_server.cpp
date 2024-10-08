#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

#include "document.h"
#include "search_server.h"
#include "string_processing.h"

void SearchServer::AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings) {
    if (document_id < 0 || documents_.count(document_id) != 0) {
        throw std::invalid_argument("The document was not added to the server because of an invalid document ID.");
    }

    document_ids_.push_back(document_id);
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();

    for (const std::string &word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }

    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, DocumentStatus status_stowed) const {
    return FindTopDocuments(raw_query, [status_stowed](int document_id, DocumentStatus status, int rating) {
        return status == status_stowed;
    });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string &raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;

    for (const std::string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    for (const std::string &word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

int SearchServer::GetDocumentId(int index) const {
    if (index < 0 || index >= GetDocumentCount()) {
        throw std::out_of_range("You are out of range.");
    }

    return document_ids_.at(index);
}

bool SearchServer::IsStopWord(const std::string &word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string &text) const {
    std::vector<std::string> words;
    for (const std::string &word : SplitIntoWords(text)) {
        if (CheckOnSpecialCharactersExistence(word)) {
            throw std::invalid_argument(word);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }

    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int> &ratings) {
    if (ratings.empty()) {
        return 0;
    }

    const int rating_sum = accumulate(ratings.begin(), ratings.end(), 0, [](const int &first, const int &second) {
        return first + second;
    });

    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    if (!CheckOnRightUsageOfMinuses(text)) {
        throw std::invalid_argument("Wrong query has been inputed.");
    }

    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }

    return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string &text) const {
    Query query;
    for (const std::string &word : SplitIntoWords(text)) {
        if (CheckOnSpecialCharactersExistence(word)) {
            throw std::invalid_argument(word);
        }

        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            }
            else {
                query.plus_words.insert(query_word.data);
            }
        }
    }

    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string &word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

bool SearchServer::CheckCharacter(const char &c, int &count, char &previous) {
    if (c == ' ' && previous == '-') {
        return false;
    }
    else if (c == '-') {
        if (++count == 2) {
            return false;
        }

        previous = c;
        return true;
    }
    count = 0;
    previous = c;
    return true;
}

bool SearchServer::CheckOnRightUsageOfMinuses(const std::string &text) {
    int count = 0;
    char previous = ' ';
    if (!all_of(text.begin(), text.end(), [&count, &previous](const char &got) {
        return CheckCharacter(got, count, previous);
    })) {
        return false;
    }

    if (previous == '-') {
        return false;
    }

    return true;
}

bool SearchServer::CheckOnSpecialCharactersExistence(const std::string &text) {
    return !none_of(text.begin(), text.end(), [](const char &got) {
        return got >= '\0' && got < ' ';
    });
}