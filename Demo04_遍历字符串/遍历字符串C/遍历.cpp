#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main() {
	char s[] = "Hello World";
	for (char* p = s;*p != '\0';p++) {
		putchar(*p);
	}

	return 0;
}