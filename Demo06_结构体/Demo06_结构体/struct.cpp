#include <stdio.h>
int main1() {
	struct teacher {
		int id;
		char name[40];
		int score;
	}Mr_Chow = {5,"Chow",150};
		printf("Teacher:\n%d\n%s\n%d\n", Mr_Chow.id,Mr_Chow.name,Mr_Chow.score);
		struct student {int id;char name[40];int score;} s[3] = 
		{
			{1,"Joe",130},
			{ 2, "Ruey", 150 },
			{3,"Cow",97}
		};
		printf("students:\n");
		for (int i = 0;i < sizeof(s) / sizeof(s[0]);i++) {
			printf("%d  %s  %d  \n", s[i].id, s[i].name, s[i].score);


		}
	

	return 0;
}