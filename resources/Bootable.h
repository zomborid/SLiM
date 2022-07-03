#pragma once

Module(Bootable){
// interface uses and provides, tasks

	UseInterface(Boot);

	int secret;

	void Signal(Boot, booted)() {
		printf("booted");
		PostTask(BootTask);
	}

	Task(BootTask){
		printf("run BootTask");
	}
};