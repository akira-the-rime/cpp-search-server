// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:

#include <iostream>
using namespace std;

int main() {
	int num = 1000;
	int count = 0;
	do {
		int temp = num;
		while (temp > 0) {
			if (temp % 10 == 3) {
				count++;
				break;
			}
			temp /= 10;
		}
		--num;
	} while (num >= 3);
	cout << count << endl;
	return 0;
}

//
// Закомитьте изменения и отправьте их в свой репозиторий.