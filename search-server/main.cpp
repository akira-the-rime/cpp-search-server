#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine();
int ReadLineWithNumber();
vector<string> SplitIntoWords(const string &text);

struct Document {
    int id;
    double relevance;
};

struct WordsInfo {
    set<string> all_words;
    set<string> minus_words;
};

class SearchServer {
public:
    void SetStopWords(const string &text);
    void AddDocument(int document_id, const string &document);
    vector<Document> FindTopDocuments(const string &raw_query) const;
private:
    map<string, map<int, double>> library_;
    set<string> stop_words_;
    int document_count_ = 0;
    bool IsStopWord(const string &word) const;
    vector<string> SplitIntoWordsNoStop(const string &text) const;
    WordsInfo ParseQuery(const string &text) const;
    double CountInverseDocumentFrequency(const string &word) const;
    vector<Document> FindAllDocuments(const WordsInfo &query_words) const;
};

void SearchServer::SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

void SearchServer::AddDocument(int document_id, const string &document) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    map<string, double> term_frequency;
    for (const string &word : words) {
        ++term_frequency[word];
    }
    for (const pair<string, double> &term : term_frequency) {
        term_frequency[term.first] = term_frequency[term.first] / static_cast<double>(words.size());
    }
    for (const string &word : words) {
        library_[word].insert({ document_id, term_frequency[word] });
    }
    document_count_++;
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query) const {
    const WordsInfo query_words = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query_words);
    sort(matched_documents.begin(), matched_documents.end(), [](const Document &lhs, const Document &rhs) {
        return lhs.relevance > rhs.relevance;
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

bool SearchServer::IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

WordsInfo SearchServer::ParseQuery(const string &text) const {
    WordsInfo query_words;
    for (const string &word : SplitIntoWordsNoStop(text)) {
        string minus_word;
        if (word[0] == '-') {
            string minus_word = word;
            minus_word.erase(0, 1);
            if (!query_words.minus_words.count(minus_word)) {
                query_words.minus_words.insert(minus_word);
                if (query_words.all_words.count(minus_word)) {
                    query_words.all_words.erase(minus_word);
                }
            }
            continue;
        }
        if (!query_words.all_words.count(word) && !query_words.minus_words.count(word)) {
            query_words.all_words.insert(word);
        }
    }
    return query_words;
}

double SearchServer::CountInverseDocumentFrequency(const string &word) const {
    return log(static_cast<double>(document_count_) / library_.at(word).size());
}

vector<Document> SearchServer::FindAllDocuments(const WordsInfo &query_words) const {
    vector<Document> matched_documents;
    map<int, double> sorted_results;
    for (const string &word : query_words.all_words) {
        if (library_.count(word) != 0) {
            double inverse_document_frequency = CountInverseDocumentFrequency(word);
            for (const pair<int, double> &document : library_.at(word)) {
                sorted_results[document.first] += inverse_document_frequency * document.second;
            }
        }
    }
    for (const string &word : query_words.minus_words) {
        if (library_.count(word) != 0) {
            for (const pair<int, double> document : library_.at(word)) {
                sorted_results.erase(document.first);
            }
        }
    }
    for (const pair<int, double> &result : sorted_results) {
        matched_documents.push_back({ result.first, result.second });
    }
    return matched_documents;
}

SearchServer CreateSearchServer();

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto &got : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << got.id << ", " << "relevance = "s << got.relevance << " }"s << endl;
    }
}

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string &text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}