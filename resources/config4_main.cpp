#include "config4.h"

#include <iostream>

int main(){
	
	std::cout << "Secret: " << slim_ap::get_secret() << std::endl;
	int foo = 3;
	slim_ap::preboot(foo);
	
	slim_start();
	
	return 0;
}