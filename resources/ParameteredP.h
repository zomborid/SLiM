#pragma once

Module(ParameteredP){
// interface uses and provides, tasks

	UseInterface(Boot);
	
	IntParameter(I1);
	IntParameter(I2);
	IntParameter(IM3);
	TypeParameter(T1);
	TypeParameter(T2);

	void Signal(Boot, booted)() {
		printf("I1: %i\n", I1);
		printf("I2: %i\n", I2);
		printf("IM3: %i\n", IM3);
		printf("T1: %i\n", sizeof(T1));
		printf("T2: %i\n", sizeof(T2));
	}
};