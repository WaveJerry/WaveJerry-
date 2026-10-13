#include <stdio.h>
//typedef：给结构体起别名 Stu
typedef struct student {
	int id;
	char name[40];
	int score;
}Stu;

int main()
{
	Stu s[3] = {
		{1,"Joe",130},
		{2,"Ruey",150},
		{3,"Cow",97}
	};
	Stu* p = s;
	int len = sizeof(s) / sizeof(s[0]);
	for (int i = 0;i < len;i++)
	{
		printf("%d %s %d\n", p->id, p->name, p->score);
		p++;
	}
	return 0;
}