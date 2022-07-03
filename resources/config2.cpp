#include <stdint.h>

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



// includes start

#include "D:\personal\CNC\es\dev\slim_packer\build\system\System.h"
#include "..\resources\Bootable.h"

// includes end



// wiring start

struct _slim_app {
    
    struct _m1; // System
    struct _m2; // Bootable
    struct _m3; // Bootable
    
    struct _m1_t{
        
        struct Boot_as_Boot{ // provided interface
            static void booted(){
                _m2.Signal(Boot,booted)();
                _m3.Signal(Boot,booted)();
            }
        };
    };
    static _m_System<_m1_t> _m1;
    
    struct _m2_t{
        static constexpr tid_t _task_BootTask_id = 0;
        
        struct Boot_as_Boot{ // used interface
        };
    };
    static _m_Bootable<_m2_t> _m2;
    
    struct _m3_t{
        static constexpr tid_t _task_BootTask_id = 1;
        
        struct Boot_as_Boot{ // used interface
        };
    };
    static _m_Bootable<_m3_t> _m3;
    
};

_m_System<_slim_app::_m1_t> _slim_app::_m1
_m_Bootable<_slim_app::_m2_t> _slim_app::_m2
_m_Bootable<_slim_app::_m3_t> _slim_app::_m3

static void _task0(){_slim_app::_m2.BootTask();}
static void _task1(){_slim_app::_m3.BootTask();}

static std::array<void(*)(), 2> task_table = {
    _task0,
    _task1
};



// wiring end


#include <array>

// System module is always module #1, and should always be present
#define System _slim_app::_m1

/*
// post task and scheduler implementation

static tid_t taskid;
static void _fun_PostTask(const tid_t id){
    taskid = id;
    taskTable[taskid]();
}
*/

void slim_start(){
    System.run();
}
