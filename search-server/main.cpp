#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine();
int ReadLineWithNumber();
vector<string> SplitIntoWords(const string &text);

struct Document {
    int id = 0;
    double relevance = 0;
    int rating = 0;
    Document() = default;
    Document(int got_id, double got_relevance, int got_rating) : id(got_id), relevance(got_relevance), rating(got_rating) { }
};

template <class StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
    set<string> non_empty_strings;
    for (const string &str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:

    explicit SearchServer(const string &text) : SearchServer(SplitIntoWords(text)) { }

    template <class StringContainer>
    explicit SearchServer(const StringContainer &stop_words) {
        for (const string &word : MakeUniqueNonEmptyStrings(stop_words)) {
            if (CheckOnSpecialCharactersExistence(word)) {
                throw invalid_argument("Wrong character has been inputed. Object was not made.");
            }
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string &document, DocumentStatus status, const vector<int> &ratings);

    template <class Type>
    vector<Document> FindTopDocuments(const string &raw_query, Type predicate) const;

    vector<Document> FindTopDocuments(const string &raw_query, DocumentStatus predicate) const;

    vector<Document> FindTopDocuments(const string &raw_query) const;

    int GetDocumentCount() const;

    tuple<vector<string>, DocumentStatus> MatchDocument(const string &raw_query, int document_id) const;

    int GetDocumentId(int index) const;

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;          // library
    map<int, DocumentData> documents_;                              // documents and their ratings and status
    vector<int> document_ids_;                                      // document IDs

    bool IsStopWord(const string &word) const;

    vector<string> SplitIntoWordsNoStop(const string &text) const;

    static int ComputeAverageRating(const vector<int> &ratings);

    QueryWord ParseQueryWord(string text) const;

    Query ParseQuery(const string &text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const string &word) const;

    template <class Type>
    vector<Document> FindAllDocuments(const Query &query, Type predicate) const;

    static bool CheckCharacter(const char &c, int &count, char &previous);

    static bool CheckOnRightUsageOfMinuses(const string &text);

    static bool CheckOnSpecialCharactersExistence(const string &text);
};

void SearchServer::AddDocument(int document_id, const string &document, DocumentStatus status, const vector<int> &ratings) {
    if (document_id < 0 || documents_.count(document_id) != 0) {
        throw invalid_argument("The document was not added to the server because of an invalid document ID.");
    }
    document_ids_.push_back(document_id);
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string &word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
}

template <class Type>
vector<Document> SearchServer::FindTopDocuments(const string &raw_query, Type predicate) const {
    if (!CheckOnRightUsageOfMinuses(raw_query)) {
        throw invalid_argument("Wrong query has been inputed.");
    }
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, predicate);
    const double EPSILON = 1e-6;
    sort(matched_documents.begin(), matched_documents.end(), [EPSILON](const Document &lhs, const Document &rhs) {
        return abs(lhs.relevance - rhs.relevance) < EPSILON ? lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query, DocumentStatus status_stowed) const {
    return FindTopDocuments(raw_query, [status_stowed](int document_id, DocumentStatus status, int rating) {
        return status == status_stowed;
    });
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string &raw_query, int document_id) const {
    if (!CheckOnRightUsageOfMinuses(raw_query)) {
        throw invalid_argument("Wrong query has been inputed.");
    }
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string &word : query.minus_words) {
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
        throw out_of_range("You are out of range.");
    }
    return document_ids_[index];
}

bool SearchServer::IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
        if (CheckOnSpecialCharactersExistence(word)) {
            throw invalid_argument(word);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings) {
    if (ratings.empty()) {
        return 0;
    }
    const int rating_sum = accumulate(ratings.begin(), ratings.end(), 0, [](const int &first, const int &second) {
        return first + second;
    });
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const string &text) const {
    Query query;
    for (const string &word : SplitIntoWords(text)) {
        if (CheckOnSpecialCharactersExistence(word)) {
            throw invalid_argument(word);
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

double SearchServer::ComputeWordInverseDocumentFreq(const string &word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

template <class Type>
vector<Document> SearchServer::FindAllDocuments(const Query &query, Type predicate) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
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
    for (const string &word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto &word : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(word.first);
        }
    }
    vector<Document> matched_documents;
    for (const auto &document : document_to_relevance) {
        matched_documents.push_back({ document.first, document.second, documents_.at(document.first).rating });
    }
    return matched_documents;
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

bool SearchServer::CheckOnRightUsageOfMinuses(const string &text) {
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

bool SearchServer::CheckOnSpecialCharactersExistence(const string &text) {
    return !none_of(text.begin(), text.end(), [](const char &got) {
        return got >= '\0' && got < ' ';
    });
}

// Я не совсем понял, какие два метода сделать, как один. 
// В прошлый раз я прислал метод IsValid(), который проверял символ и на спецсимволы, и на минусы. 
// Но Вы сказали, что их надо разделить.

// ========================= для примера =========================

void PrintDocument(const Document &document) {
    cout << "{ "s << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}

int main() {
    setlocale(LC_ALL, "ru");
    try {
        SearchServer search_server("и в на"s);
        search_server.AddDocument(4, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(2, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(3, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        search_server.AddDocument(1, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
        cout << "ACTUAL by default:"s << endl;
        for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
            PrintDocument(document);
        }
        cout << "BANNED:"s << endl;
        for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
            PrintDocument(document);
        }
        cout << "Even ids:"s << endl;
        for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) {
            return document_id % 2 == 0;
        })) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument &ex) {
        cerr << "An error is: " << endl;
        cerr << ex.what() << endl;
    }
    catch (const out_of_range &ex) {
        cerr << ex.what() << endl;
    }
    return 0;
}

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
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