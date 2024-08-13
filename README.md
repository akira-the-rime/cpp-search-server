ENG:
Search Server. 
This program is designed to test my recently-acquired C++ programming skills. 
It requires the C++17 language standard to run.

1) To use this program, you need to add a document to the Search Server with the AddDocument() method, which takes 4 parameters: document ID, the document itself, document status (ACTUAL, IRRELEVANT, BANNED, REMOVED), and document users' ratings (the program computes an average rating).
2) Retrieve the top-N documents (the number is set by the global variable MAX_RESULT_DOCUMENT_COUNT) with the FindTopDocuments() method.
3) Paginate the results with the Paginate() function.

RU:
Поисковой сервер. 
Эта программа создана для того, чтобы проверить мои недавно приобретённые знания по C++. 
Для запуска программы требуется стандарт языка C++17.

1) Чтобы использовать эту программу, добавьте документ в поисковой сервер с помощью метода AddDocument(), который принимает 4 параметра: ID документа, сам документ, статус документа (ACTUAL - актуальный, IRRELEVANT - нерелевантный, BANNED - заблокированный, REMOVED - удалённый) и пользовательские рейтинги документа (программа вычисляет средний рейтинг).
2) Получите топ-N документов (их количество задаётся глобальной переменной MAX_RESULT_DOCUMENT_COUNT) с помощью метода FindTopDocuments().
3) Разбейте результаты на страницы с помощью функции Paginate().
