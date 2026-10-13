#include <iostream>
#include <windows.h>             //////
using namespace std;
int main1() {

	SetConsoleOutputCP(65001);//////解决中文问题，要加windows.h和SetConsoleOutputCP(65001)，
	                          ///////以及utf-8编码格式和项目属性写上/utf-8   ！！

	int arr[10] = { 7,56,6,9,70,8,5,34,56,1 };
	cout << "arr[] 是 :";
	for (int i = 0;i < 10;i++) {
		cout << arr[i] << " ";
	}
	cout << endl;

	int temp;
	for (int i = 0;i < 10;i++ ) {
		for (int j = 0;j < 10 - i-1;j++) {
			if (arr[j] > arr[j + 1]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;

			}
		}
		cout << i + 1 <<"冒泡又一轮，排序是  " << endl;
		for (int i = 0;i < 10;i++) {
			cout <<arr[i]<< " ";
		}
		cout << endl;
	}

	cout<<"冒泡结果是 ";

 	for (int i = 0;i < 10;i++) {
		cout << arr[i] << " ";
	}

	return 0;
}