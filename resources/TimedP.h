#pragma once

Module(TimedP){

	UseInterface(Time);
	UseInterface(Boot);
	UseInterface(Scheduler);

	int secret;

	void Signal(Boot, booted)() {
		printf("TimedP booted\n");
		secret = 5;
	}

	void Signal(Scheduler, startup)() {
		printf("TimedP startup\n");
		Time::set_timeout(secret, true);
		Time::start();
	}

	void Signal(Scheduler, idle)() {
		printf("TimedP idle\n");
		Scheduler::exit();
	}

	void Signal(Time, timeout)(int time) {
		printf("timeout, time = %i\n", time);
		PostTask(TimeoutTask);
	}

	Task(TimeoutTask){
		printf("run TimeoutTask\n");
		return false;
	}
};