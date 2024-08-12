#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer final {
public:
    explicit SearchServer(const std::string &text) 
        : SearchServer(SplitIntoWords(text)) { 
    }

    template <class StringContainer>
    explicit SearchServer(const StringContainer &stop_words);

    void AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings);
    int GetDocumentCount() const;
    int GetDocumentId(int index) const;

    template <class Type>
    std::vector<Document> FindTopDocuments(const std::string &raw_query, Type predicate) const;
    std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentStatus predicate) const;
    std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const;

private:

    struct DocumentData final {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord final {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query final {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    bool IsStopWord(const std::string &word) const;
    Query ParseQuery(const std::string& text) const;
    QueryWord ParseQueryWord(std::string text) const;
    std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

    template <class Type>
    std::vector<Document> FindAllDocuments(const Query &query, Type predicate) const;

    static bool CheckCharacter(const char &c, int &count, char &previous);
    static bool CheckOnRightUsageOfMinuses(const std::string &text);
    static bool CheckOnSpecialCharactersExistence(const std::string &text);
    static int ComputeAverageRating(const std::vector<int>& ratings);
};

template <class StringContainer>
SearchServer::SearchServer(const StringContainer &stop_words) {
    for (const std::string &word : MakeUniqueNonEmptyStrings(stop_words)) {
        if (CheckOnSpecialCharactersExistence(word)) {
            throw std::invalid_argument("Wrong character has been inputed. Object was not made.");
        }
        stop_words_.insert(word);
    }
}

template <class Type>
std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, Type predicate) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, predicate);
    const double EPSILON = 1e-6;
    sort(matched_documents.begin(), matched_documents.end(), [EPSILON](const Document &lhs, const Document &rhs) {
        return std::abs(lhs.relevance - rhs.relevance) < EPSILON ? lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
    });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <class Type>
std::vector<Document> SearchServer::FindAllDocuments(const Query &query, Type predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto &document : word_to_document_freqs_.at(word)) {
            DocumentData rating_and_status = documents_.at(document.first);
            if (predicate(document.first, rating_and_status.status, rating_and_status.rating)) {
                document_to_relevance[document.first] += document.second * inverse_document_freq;
            }
        }
    }
    for (const std::string &word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto &word : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(word.first);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto &document : document_to_relevance) {
        matched_documents.push_back({ document.first, document.second, documents_.at(document.first).rating });
    }

    return matched_documents;
}