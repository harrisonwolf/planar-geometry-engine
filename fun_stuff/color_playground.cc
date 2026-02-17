#include <iostream>

using namespace std;

int main(){
	cout << "\033[30;1mBlack\033[0m\n";
	cout << "\033[32;1mGreen\033[0m\n";
	cout << "\033[36;1mCyan\033[0m\n";
	cout << "\033[33;1mYellow\033[0m\n";
	cout << "\033[30;1;7mBlack\033[0m\n";
	cout << "\033[32;1;7mGreen\033[0m\n";
	cout << "\033[36;1;7mCyan\033[0m\n";
	cout << "\033[33;1;7mYellow\033[0m\n";

	cout << "\033[35;1;4mMagenta Underline\033[0m\n";
	cout << "\033[35;1;7;4mMagenta Underline\033[0m\n";
}
