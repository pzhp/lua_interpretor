#include <iostream>
#include <string>
#include <fstream>
#include "scanner.h"

using namespace lua_in;

int main()
{
	std::string str1= "in.txt";
	Scanner sc(str1);
	sc.RunScanner();
	
	return 0;
}