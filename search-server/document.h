#pragma once

struct Document {
    int id = 0;
    double relevance = 0;
    int rating = 0;
    Document() = default;
    Document(int got_id, double got_relevance, int got_rating) : id(got_id), relevance(got_relevance), rating(got_rating) { }
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

void PrintDocument(const Document &document);