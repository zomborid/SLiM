#include <cstdint>
#include <array>

// tid_t or task id type results in uint8_t or uint16_t according to the number of tasks
using tid_t = uint8_t;

using _task_return_t = bool;
using _task_t = _task_return_t(*)();

// a task should not be posted from both regular
// and interrupt threads
static bool _fun_PostTask(const tid_t id);
static bool _fun_PostUrgentTask(const tid_t id);

// call only when the thread can not be interrupted
// ie.: from non-preemptive interrupt thread
// if interrupts are disabled, or in an environment
// where interrupts do not access the scheduler
// use PostTask
static bool _fun_PostTaskFromInterrupt(const tid_t id);
static bool _fun_PostUrgentTaskFromInterrupt(const tid_t id);

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

#define TypeParameter(name) \
using name=typename Parent::param_##name

#define IntParameter(name) \
static const int name=Parent::param_##name

#define Command(iface,name) iface##_com_##name
#define Signal(iface,name) iface##_sig_##name

// returns if the task should be re-posted immedietely
#define Task(name) \
bool name()

#define PostTask(name) _fun_PostTask(Parent::_task_##name##_id)
#define PostTaskFromInterrupt(name) _fun_PostTaskFromInterrupt(Parent::_task_##name##_id)
#define PostUrgentTask(name) _fun_PostUrgentTask(Parent::_task_##name##_id)
#define PostUrgentTaskFromInterrupt(name) _fun_PostUrgentTaskFromInterrupt(Parent::_task_##name##_id)



// includes start

#include "D:\projects\slim_dev\slim_packer\build\system\System.h"
#include ".\TimedP.h"
#include ".\TimerP.h"

// includes end



// wiring start

struct _slim_app {
    
    struct _m1; // System
    struct _m2; // TimedP
    struct _m3; // TimerP
    
    struct _m1_t{
        
        struct Boot_as_Boot{ // provided interface
            static void booted(){
                _m2.Signal(Boot,booted)();
                _m3.Signal(Boot,booted)();
            }
        };
        struct Scheduler_as_Scheduler{ // provided interface
            static void startup(){
                _m2.Signal(Scheduler,startup)();
            }
            static void idle(){
                _m2.Signal(Scheduler,idle)();
            }
        };
    };
    static _m_System<_m1_t> _m1;
    
    struct _m2_t{
        static constexpr tid_t _task_TimeoutTask_id = 0;
        
        struct Boot_as_Boot{ // used interface
        };
        struct Scheduler_as_Scheduler{ // used interface
            static void exit(){_m1.Command(Scheduler,exit)();}
        };
        struct Time_as_Time{ // used interface
            static void set_timeout(int time, bool flag){_m3.Command(Time,set_timeout)(time, flag);}
            static bool start(){return _m3.Command(Time,start)();}
            static void stop(){_m3.Command(Time,stop)();}
        };
    };
    static _m_TimedP<_m2_t> _m2;
    
    struct _m3_t{
        static constexpr tid_t _task_TimeoutTask_id = 1;
        
        struct Boot_as_Boot{ // used interface
        };
        struct Time_as_Time{ // provided interface
            static void timeout(int time){
                _m2.Signal(Time,timeout)(time);
            }
        };
    };
    static _m_TimerP<_m3_t> _m3;
    
};

_m_System<_slim_app::_m1_t> _slim_app::_m1;
_m_TimedP<_slim_app::_m2_t> _slim_app::_m2;
_m_TimerP<_slim_app::_m3_t> _slim_app::_m3;

static _task_return_t _task0(){return _slim_app::_m2.TimeoutTask();}
static _task_return_t _task1(){return _slim_app::_m3.TimeoutTask();}

static constexpr int _task_count = 2;
static std::array<_task_t, _task_count> _task_table = {
    _task0,
    _task1
};



// wiring end


// System module is always module #1, and should always be present
#define System _slim_app::_m1
	
struct _static_task_list{
	// the list stores task id + 1 to use zero initialization for
	// the pointers, since task ids are starting from zero
	// the public methods expect and return task ids starting from 1,
	// therefore on append and return 1 should be added and removed
	std::array<tid_t, _task_count + 1> list;
	tid_t head = 0;
	tid_t tail = 0;
	bool blocking = false;
	uint8_t iswitch = 0;
	bool interrupt_appended = false;
	std::array<tid_t, 2> ihead;
	std::array<tid_t, 2> itail;
	
private:
	// merges a list into the main list
	// interrupt unsafe
	void append_list(const tid_t h, const tid_t t){
		if (h != 0){
			if (head == 0){
				head = h;
				tail = t;
			} else {
				list[tail] = h;
				tail = t;
			}
		}
	}
	
	void resolve_concurrency(){
		blocking = false;
		while(interrupt_appended){
			blocking = true;
			interrupt_appended = false;
			iswitch = 1-iswitch;
			append_list(ihead[1-iswitch], itail[1-iswitch]);
			ihead[1-iswitch] = 0;
			itail[1-iswitch] = 0;
			blocking = false;
		}
	}
	
	bool append_to(tid_t& h, tid_t& t, const tid_t tid){
		if (t == 0){
			h = tid;
			t = tid;
		} else {
			if (list[tid] != 0 || t == tid) return true;
			list[t] = tid;
			t = tid;
		}
		return false;
	}
	
	bool append_urgent_to(tid_t& h, tid_t& t, const tid_t tid){
		if (h == 0){
			h = tid;
			t = tid;
		} else {
			if (list[tid] != 0 || t == tid) return true;
			list[tid] = h;
			h = tid;
		}
		return false;
	}
	
	tid_t pop_from_list(tid_t& h, tid_t& t){
		tid_t pop = h;
		if (pop != 0){
			h = list[pop];
			list[pop] = 0;
			if (h == 0){
				t = 0;
			}
		}
		return pop;
	}

public:
	tid_t remove_in_regular(){
		blocking = true;
		tid_t pop = pop_from_list(head, tail);
		resolve_concurrency();
		return pop;
	}
	
	bool append_in_regular(const tid_t tid){
		blocking = true;
		bool already_present = append_to(head, tail, tid);
		resolve_concurrency();
		return already_present;
	}
	
	bool append_in_interrupt(const tid_t tid){
		bool already_present;
		if (blocking){
			interrupt_appended = true;
			already_present = append_to(ihead[iswitch], itail[iswitch], tid);
		} else {
			already_present = append_to(head, tail, tid);
		}
		return already_present;
	}
	
	bool append_urgent_in_regular(const tid_t tid){
		blocking = true;
		bool already_present = append_urgent_to(head, tail, tid);
		resolve_concurrency();
		return already_present;
	}
	
	bool append_urgent_in_interrupt(const tid_t tid){
		bool already_present;
		if (blocking){
			interrupt_appended = true;
			already_present = append_urgent_to(ihead[iswitch], itail[iswitch], tid);
		} else {
			already_present = append_urgent_to(head, tail, tid);
		}
		return already_present;
	}
};


// task function pointers are stored in: static std::array<void(*)(), _task_count> task_table;
_static_task_list _stl;

// returns if the task is already present
static bool _fun_PostTask(const tid_t id){
	return _stl.append_in_regular(id + 1);
}

// returns if the task is already present
static bool _fun_PostTaskFromInterrupt(const tid_t id){
	return _stl.append_in_interrupt(id + 1);
}

// returns if the task is already present
static bool _fun_PostUrgentTask(const tid_t id){
	return _stl.append_urgent_in_regular(id + 1);
}

// returns if the task is already present
static bool _fun_PostUrgentTaskFromInterrupt(const tid_t id){
	return _stl.append_urgent_in_interrupt(id + 1);
}

static void _start_scheduler(){
	tid_t task_id;
	bool run = true;
	while(run){
		task_id = _stl.remove_in_regular();
		if (task_id != 0){
			bool loop = _task_table[task_id - 1]();
			if (loop){
				_stl.append_in_regular(task_id);
			}
		} else {
			System.scheduler_idle();
			run = !System.scheduler_exit_flag;
		}
	}
}

void slim_start(){
    System.boot();
	System.scheduler_startup();
	_start_scheduler();
}
