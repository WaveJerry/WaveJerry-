#include <stdio.h>
int main() {
	int n;

	scanf_s("%d", &n);
	for (int j = 1;j <= n;j++) {
		
		for (int i = 0;i < n-j;i++) {
			printf("  ");
		}
		for (int i = 1; i <= j;i++) {
			if (i == 10) { printf(" 0"); }
			else { printf("%2d", i); }
		}
		for (int i = j-1;i >= 1;i--) {
			printf("%2d",i);
		}
		printf("\n"); 
	}
	return 0;
}