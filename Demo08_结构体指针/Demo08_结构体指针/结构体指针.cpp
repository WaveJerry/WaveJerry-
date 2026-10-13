#include <stdio.h>
struct Student
{
    int id;
};
void show(struct Student* p)
{
    printf("%d", p->id);
}
int main(void) {
    struct Student s = { 1001 };
    struct Student* p = &s;

    p->id = 2002;
    show(p);

    return 0;
}