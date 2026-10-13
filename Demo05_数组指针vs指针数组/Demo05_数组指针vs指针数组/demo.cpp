#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
int main() {
	const char* ch[] = { "Mon.","Tue.","Wed.","Thu.","Fri.","Sat.","Sun." };
	int k = sizeof(ch) / sizeof(ch[0]); /////每个指针大小相同，定义时存入指针数组的自动就是字符串的首项的地址
		for (int i = 0;i < k;i++) {  ///////////指针数组，数组里存的是指针

			printf("%s ", ch[i]);
	}
		
	return 0;
}
//int* p
//声明： *= 标记 p 是指针
//表达式： * p = 解引用，取地址里的值
//int arr[5]
//arr → 大多情况 == &arr[0]          arr取数组首项地址   
//例外：sizeof(arr)求整个数组大小； & arr取整个数组地址
//arr + 1：跳 1 个 int； & arr + 1：跳整个数组
//int* p1[5]={&a,&b,&c,&d,&e};👉指针数组：数组，存指针
//int (*p2)[5]=&arr;👉数组指针：指针，指向数组
//char str[] = "abc" ✅可修改内容（栈）
//char* str = "abc" ❌不可修改（常量区）
//函数形参 int arr[] ≡ int* arr
//函数内sizeof(arr)得到指针大小，不是数组长
//arr[i] == *(arr + i)
//[] 只是语法糖
// arr[i]  <==> *(arr + i) <==> *(p + i) <==> p[i]