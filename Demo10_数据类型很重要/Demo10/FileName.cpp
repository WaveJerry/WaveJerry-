#include <stdio.h>
int main() {
	long double x, s, h;
	int n;///s总路程，h第n次跳起高度
	scanf("%Lf%d", &x, &n);
	s = x;
	////第一次落地
	int j = 1;
	for (long double i = x / 2;i > 1e-5;i /= 2, j++) {
		s = s + (2 * i);
		if (n == j) {
			h = i;

		}

	}
	printf("%.6Lf %.6Lf", h, s);

	return 0;
}