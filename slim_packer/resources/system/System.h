#pragma once

Module(System){
    ProvideInterfaceAs(Boot, Booting);
    ProvideInterfaceAs(Boot, Booted);
    ProvideInterface(Scheduler);
	
	bool scheduler_exit_flag = false;
	
	void Command(Scheduler, exit)(){
		scheduler_exit_flag = true;
	}

    void boot(){
        Booting::booted();
        Booted::booted();
    }

    void scheduler_startup(){
        Scheduler::startup();
    }

    void scheduler_idle(){
        Scheduler::idle();
    }

};