#pragma once

#include <ctime>

Module(TimerP){

	ProvideInterface(Time);
	UseInterface(Boot);

	int timeout_time;
	time_t start_time;
	bool running;

	void Signal(Boot, booted)() {
		printf("TimerP booted\n");
		timeout_time = 1;
		running = false;
	}

	void Command(Time, set_timeout)(int time, bool flag) {
		timeout_time = time;
	}

	bool Command(Time, start)() {
		start_time = std::time(nullptr);
		bool prev = running;
		running = true;
		PostTask(TimeoutTask);
		return prev;
	}

	void Command(Time, stop)() {
		printf("timeout, time = %i\n", time);
	}

	Task(TimeoutTask){
		time_t now = std::time(nullptr);
		if (now - start_time - timeout_time >= 0){
			Time::timeout((int)(now - start_time));
			return false;
		} else {
			return true;
		}
	}
};