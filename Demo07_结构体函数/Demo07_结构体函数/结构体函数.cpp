#include <stdio.h>
struct stu {
	int id;
	int num;
};
void kipp(struct stu* p, int n) {
	for (int i = 0;i < n;i++) {
		printf("%d %d \n", p[i].id, p[i].num);
	}
}
int main() {
	struct stu s[3] = {
		{10,100},{11,95},{12,100}
	};
	int len = sizeof(s) / sizeof(s[0]);
	kipp(s, len);
	return 0;
}

//p[i].id   ← 完全合法！
////等价于
//(* (p + i)).id --------优先级 . > *
// //等价于
//s[i].id
////等价于
//(p + i)->id