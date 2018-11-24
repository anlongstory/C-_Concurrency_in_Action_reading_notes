#include<iostream>
#include<thread> // 支持多线程的头文件
using namespace std;

void hello()
{
	cout << " Hello Concurrent World\n" << endl;
}

int main()
{
	// 每个线程必须有一个初始函数，所以这里把打印单独成一个函数；
	// 新线程的执行从这里开始
	thread t(hello); // 新线程名字为 t ，有 hello（）作为初始函数
	t.join();
	system("pause");
	return 0;
}