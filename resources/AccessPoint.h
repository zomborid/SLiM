#pragma once

#include <iostream>
#define Expose(name) name

Module(AccessPoint){

	ProvideInterface(Boot);
	
	int secret = 3;
	
	void Expose(preboot)(int foo){
		std::cout << "AccessPoint preboot\n";
		Boot::booted();
		std::cout << "AccessPoint preboot end\n";
	}
	
	int Expose(get_secret)(){
		return secret;
	}
};