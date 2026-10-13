#include <iostream>
using namespace std;


int swap(int* a, int* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
	return 8;
}

int main() {

	int x = 9, y = 10;
	int z=swap(&x, &y);
	cout << x<<" "<<y << " "<<z << endl;
	system("pause");
	return 0;
}