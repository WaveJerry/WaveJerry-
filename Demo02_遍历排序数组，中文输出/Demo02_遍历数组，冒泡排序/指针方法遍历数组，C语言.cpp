#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main() {
	int a[10] = { 1,2,3,4,5,6,7,8,9,10 };
	int* p;
	for (p = a;p <= a + 9;p++) {
		printf("%d\n", *p);

	}


	return 0;
}