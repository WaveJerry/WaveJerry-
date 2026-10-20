#include <stdio.h>
#include <stdlib.h>


int main() {
	system("chcp 65001");

	for (int i = 0;i < 400;i++) {
		printf("ascll码为%d的字符：", i);
		putchar(i);
		system("pause");
	}

	return 0;
}