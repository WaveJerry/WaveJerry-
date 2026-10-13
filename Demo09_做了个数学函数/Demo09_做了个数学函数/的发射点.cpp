#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
float F(float m) {
	m = 1 + 1 / (1 + 1 / (1 + 1 / (1 + 1 / (1 + 1 / m))));
	return m;

}


int main() {
	float x, y;
	scanf("%f", &x);
	y = F(x);
	printf("%.3f", y);
	return 0;
}