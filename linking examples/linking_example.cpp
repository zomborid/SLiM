#include <iostream>
#include <cstdint>

#include "..\..\..\system\es_base.h"

template<typename Assembly>
struct MyModule {

	typedef typename Assembly::MyModuleACKEvent ACKEvent;
	ACKEvent ackEvent;

	typedef typename Assembly::MyModuleTimeoutEvent TimeoutEvent;
	TimeoutEvent timeoutEvent;

	MyModule(ACKEvent& ackEvent, TimeoutEvent& timeoutEvent) :
		ackEvent(ackEvent),
		timeoutEvent(timeoutEvent)
	{}

	inline void run() {
		printf("this is mymodule\n");
		ackEvent.signal();
		timeoutEvent.signal();
	}
};


template<typename Assembly>
struct IFModule {
	Assembly& a;
	IFModule(Assembly& a) : a(a), ackEvent(*this), timeoutEvent(*this) {}


	struct ACKEvent {

		inline void signal() {
			m.whatever();
		}

		IFModule& m;
		ACKEvent(IFModule& m) : m(m) {}
	} ackEvent;

	struct TimeoutEvent {

		inline void signal() {
			m.whateverelse();
		}

		IFModule& m;
		TimeoutEvent(IFModule& m) : m(m) {}
	} timeoutEvent;

	inline void whatever() {
		printf("this is whatever\n");
	}

	inline void whateverelse() {
		printf("this is whateverelse\n");
	}
};

struct App {
	typedef Event<IFModule<App>::ACKEvent> MyModuleACKEvent;
	MyModuleACKEvent mymodule_ackevent;

	typedef Event<IFModule<App>::TimeoutEvent> MyModuleTimeoutEvent;
	MyModuleTimeoutEvent mymodule_timeoutevent;

	MyModule<App> mymodule;
	IFModule<App> ifmodule;


	App() :
		mymodule(mymodule_ackevent, mymodule_timeoutevent),
		ifmodule(IFModule<App>(*this)),
		mymodule_ackevent(MyModuleACKEvent(ifmodule.ackEvent)),
		mymodule_timeoutevent(MyModuleTimeoutEvent(ifmodule.timeoutEvent))
	{}
};

App app;

int main() {

	app.mymodule.run();
	getchar();

	return 0;
}