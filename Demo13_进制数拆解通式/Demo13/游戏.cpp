#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>

char* convertToTitle(int columnNumber) {
    char ch[10];
    for (int i = 0;i < 10;i++)
        ch[i] = 64;
    int n = columnNumber;

    for (int i = 0;n != 0; i++) {
        n = n - 1;
        ch[i] = 'A' + n % 26;
        n = n / 26;
    }

    // 先数有多少个有效字母
    int len = 0;
    for (int i = 9;i >= 0;i--) {
        if (ch[i] >= 65) len++;
    }
    // 申请字符串空间 + 末尾'\0'
    char* res = (char*)malloc(len + 1);
    int p = 0;
    for (int i = 9;i >= 0;i--) {
        if (ch[i] >= 65) {
            res[p++] = ch[i];
        }
    }
    res[p] = '\0';
    return res;
}


int main() {
    int n = 0;
    scanf("%d", &n);
    char* ans = convertToTitle(n);
    printf("%s", ans);
    return 0;
}