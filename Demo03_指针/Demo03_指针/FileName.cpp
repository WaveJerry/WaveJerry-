#include <stdio.h>

// 1. 指针做函数参数：交换两个整数（地址传递，指针核心考点）
void swap(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

// 2. 指针遍历数组，求最大值
int getMax(int* p, int len)
{
    int max = *p;
    for (int i = 1; i < len; i++)
    {
        if (*(p + i) > max)
        {
            max = *(p + i);
        }
    }
    return max;
}

// 3. 字符指针遍历字符串
void printString(char* str)
{
    while (*str != '\0')
    {
        printf("%c ", *str);
        str++;
    }
    printf("\n");
}

int main1(void)
{
    // ========== ① 基础指针 & 取地址、解引用 ==========
    int a = 100;
    int* p = &a;
    printf("a value = %d\n", a);
    printf("a address = %p\n", &a);
    printf("pointer p store address = %p\n", p);
    printf("*p value = %d\n", *p);
    *p = 200;
    printf("after *p modify, a = %d\n\n", a);

    // ========== ② 指针+数组、指针偏移 ==========
    int arr[] = { 11,22,33,44,55 };
    int len = sizeof(arr) / sizeof(arr[0]);
    int* p_arr = arr; // arr等价 &arr[0]
    printf("array by pointer: ");
    for (int i = 0; i < len; i++)
    {
        printf("%d ", *(p_arr + i));
    }
    printf("\narray max = %d\n\n", getMax(arr, len));

    // ========== ③ swap 指针交换 ==========
    int num1 = 6, num2 = 8;
    printf("before swap: num1=%d, num2=%d\n", num1, num2);
    swap(&num1, &num2);
    printf("after swap: num1=%d, num2=%d\n\n", num1, num2);

    // ========== ④ char指针 / 字符串 ==========
    char str[] = "PointerDemo";
    printf("string traverse: ");
    printString(str);

    return 0;
}