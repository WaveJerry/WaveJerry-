// #define _CRT_SECURE_NO_WARNINGS
// #include <stdio.h>
// int main1() {
// 	char s[] = "Hello World";
// 	for (char* p = &s[0];*p != '\0';p++) {
// 		putchar(*p);
// 	}

// 	return 0;
// }

#include <stdio.h>
int main() {

	char s[] = { "Issac Newton is the GOAT of science" };
	for (char* p = s;*p != '\0';p++) {
		_putchar_nolock(*p);
	}

	return 0;
}
