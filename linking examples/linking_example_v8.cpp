#include <iostream>
#include <array>

using namespace std;

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// ES definitions

using tid_t = uint8_t;
static void _PostTask(const tid_t id);

#define Module(name) \
template<typename Parent>   \
struct _##name{   \
    using This = _##name<Parent>;  \

#define Interface(name) \
template<typename Parent, typename Name>   \
struct name##_if{   \
    using This = name##_if<Parent,Name>;  \

#define UseInterface(iface) \
using iface=typename Parent::iface##_as_##iface;    \

#define UseInterfaceAs(iface,name) \
using name=typename Parent::iface##_as_##name;    \

#define ProvideInterface(iface) \
using iface=typename Parent::iface##_as_##iface;    \

#define ProvideInterfaceAs(iface,name) \
using name=typename Parent::iface##_as_##name;    \

#define Command(iface,name) iface##_com_##name
#define Signal(iface,name) iface##_sig_##name

#define Task(name) \
void name(){    \

#define PostTask(name) _PostTask(Parent::name##_id);

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// Boot.if
#if 1 == 0
// not cpp code, prevent error
interface Boot;
signal{
	std::vector<int *> * * booted(int a, int *&*b, std::array<list<int>,2>& b);
}
command{
	void booted();
}

#endif

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// eswiring.h



// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// Bootable.h


Module(Bootable){
    // interface uses and provides, tasks

    UseInterface(Boot);

    int secret;

    void Signal(Boot,booted)(){
        printf("booted");
        PostTask(BootTask)
    }

    Task(BootTask){
        printf("run BootTask");
    }

    void run(){
        Boot::enable();
    }
};

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// Startup.h


Module(Startup)
    // interface uses and provides, tasks

    ProvideInterface(Boot)

    bool enabled = false;

    void Command(Boot,enable)(){
        enabled = true;
    }

    void run(){
        if (enabled) Boot::booted();
    }

};

// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------
// eswiring.cpp


struct App{

    struct Startup_as_Startup;
    struct Bootable_as_B1;
    struct Bootable_as_B2;

    struct Startup_as_Startup{

        struct Boot_as_Boot{
            static void booted(){
                B1.Signal(Boot,booted)();
                B2.Signal(Boot,booted)();
            }
        };
    };
    static _Startup<Startup_as_Startup> Startup;

    struct Bootable_as_B1{
        static constexpr tid_t BootTask_id = 0;
        
        struct Boot_as_Boot{
            static void enable(){
                Startup.Command(Boot,enable)();
            }
        };
    };
    static _Bootable<Bootable_as_B1> B1;

    struct Bootable_as_B2{
        static constexpr tid_t BootTask_id = 1;
        
        struct Boot_as_Boot{
            static void enable(){
                Startup.Command(Boot,enable)();
            }
        };
    };
    static _Bootable<Bootable_as_B2> B2;
};

_Startup<App::Startup_as_Startup> App::Startup;
_Bootable<App::Bootable_as_B1> App::B1;
_Bootable<App::Bootable_as_B2> App::B2;

static void _task0(){
    App::B1.BootTask();
}

static void _task1(){
    App::B2.BootTask();
}



static std::array<void(*)(), 2> taskTable = {
    _task0,
    _task1
};




static tid_t taskid;
static void _PostTask(const tid_t id){
    taskid = id;
    taskTable[taskid]();
}




static void launchES(){
    App::B1.run();
    App::Startup.run();
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

 