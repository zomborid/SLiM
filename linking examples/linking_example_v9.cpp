

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// ES definitions

#include <stdint.h>
#include <array>

using tid_t = uint8_t;
static void _fun_PostTask(const tid_t id);

#define Module(type) \
template<typename Parent>   \
struct _m_##type

#define UseInterface(iface) \
using iface=typename Parent::iface##_as_##iface

#define UseInterfaceAs(iface,name) \
using name=typename Parent::iface##_as_##name

#define ProvideInterface(iface) \
using iface=typename Parent::iface##_as_##iface

#define ProvideInterfaceAs(iface,name) \
using name=typename Parent::iface##_as_##name

#define Command(iface,name) iface##_com_##name
#define Signal(iface,name) iface##_sig_##name

#define Task(name) \
void name()

#define PostTask(name) _fun_PostTask(Parent::_task_##name##_id)

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// include Bootable.h

Module(Bootable){
// interface uses and provides, tasks

	UseInterface(Boot);

	int secret;

	void Signal(Boot, booted)(int time) {
		printf("booted\n");
		PostTask(BootTask);
	}

	Task(BootTask){
		printf("run BootTask\n");
	}

	void run() {
		Boot::enable();
	}
};

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// include Startup.h

Module(Startup){
    // interface uses and provides, tasks

    ProvideInterfaceAs(Boot,B);

    bool enabled = false;

    void Command(B,enable)(){
        enabled = true;
    }

    void run(){
        if (enabled) B::booted(0);
    }

};



// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// eswiring.cpp

struct _slim_app{

    struct _m3_t; // bootable
    struct _m4_t; // bootable
    struct _m6_t; // startup

    struct _m3_t{
        static constexpr tid_t _task_BootTask_id = 0;

        struct Boot_as_Boot{
            static void enable(){
                _m6.Command(B,enable)();
            }
        };
    };
    static _m_Bootable<_m3_t> _m3;

    struct _m4_t{
        static constexpr tid_t _task_BootTask_id = 1;
        
        struct Boot_as_Boot{
            static void enable(){_m6.Command(B,enable)();}
        };
    };
    static _m_Bootable<_m4_t> _m4;

    struct _m6_t{
        struct Boot_as_B{
            static void booted(int time){
                _m3.Signal(Boot,booted)(time);
                _m4.Signal(Boot,booted)(time);
            }
        };
    };
    static _m_Startup<_m6_t> _m6;
};

_m_Bootable<_slim_app::_m3_t> _slim_app::_m3;
_m_Bootable<_slim_app::_m4_t> _slim_app::_m4;
_m_Startup<_slim_app::_m6_t> _slim_app::_m6;

static void _task0(){
    _slim_app::_m3.BootTask();
}

static void _task1(){
    _slim_app::_m4.BootTask();
}



static std::array<void(*)(), 2> taskTable = {
    _task0,
    _task1
};

// generation end


static tid_t taskid;
static void _fun_PostTask(const tid_t id){
    taskid = id;
    taskTable[taskid]();
}

static void launchES(){
	// in the real application calls a 
    _slim_app::_m3.run();
    _slim_app::_m6.run();
}










// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// main.cpp
//#include "eswiring.h"

int main(){
    launchES();
    return 0;
}



















