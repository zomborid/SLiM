#include <iostream>

#define CAT(A, B) A ## B
#define UCAT(A, B) A ## _ ## B
#define UCAT3(A, B, C) A ## _ ## B ## _ ## C

#define UseInterface(module,name)  \
typedef typename Parent::UCAT3(module,name,t) UCAT(name,t); \
UCAT(name,t)& name;

#define ProvideInterface(module,name)  \
struct name {   \
    module& p;  \
    name(module& p) : p(p) {}



template<typename Parent>
struct Trigger{
    typedef Trigger<Parent> This;

    UseInterface(Trigger,SimplePrintEvent)

    Trigger(SimplePrintEvent_t& SimplePrintEvent) :
        SimplePrintEvent(SimplePrintEvent)
    {}

    void run(){
        SimplePrintEvent.signal();
    }

};

template<typename Parent>
struct Printer{

    void simplePrint(){
        printf("this is printer");
    }

    ProvideInterface(Printer,SimplePrintEvent)
        void signal(){
            p.simplePrint();
        }
    };
};

template<typename Parent>
struct Tee{
    typedef Tee<Parent> This;

    UseInterface(Tee,ForwardedEvent)

    ProvideInterface(This,ToForwardEvent)
        void signal(){
            printf("this is tee");
            p.ForwardedEvent.signal();
        }
    };

    Tee(ForwardedEvent_t& ForwardedEvent) :
        ForwardedEvent(ForwardedEvent)
    {}
};



// ----- Generated based on connectivity graph -----

template<typename Parent>
struct SubApp{
	typedef SubApp<Parent> This;

    // input interface definitions
    typedef typename Parent::SubApp_SimplePrintEvent_t SimplePrintEvent_t;
    SimplePrintEvent_t& SimplePrintEvent;

    // inner link definitions
    typedef SimplePrintEvent_t Tee_ForwardedEvent_t;
    Tee_ForwardedEvent_t& Tee_ForwardedEvent_v;

    // output interface definitions
    typedef typename Tee<This>::ToForwardEvent SimplePrintEventOut;
    SimplePrintEventOut SimplePrintEventOut_v;

    // hidden objects
    Tee<This> Tee_v;

    SubApp(SimplePrintEvent_t& SimplePrintEvent_v) :
        SimplePrintEvent(SimplePrintEvent_v), // receive input interface object
        Tee_ForwardedEvent_v(SimplePrintEvent),
        SimplePrintEventOut_v(SimplePrintEventOut(Tee_v)), // link output interface object
        Tee_v(
            Tee_ForwardedEvent_v
            )
    {}
};

struct App {
	typedef App This;
	
	// Link objects
	typedef typename Printer<This>::SimplePrintEvent SubApp_SimplePrintEvent_t;
	SubApp_SimplePrintEvent_t SubApp_SimplePrintEvent_v;
	
	typedef typename SubApp<This>::SimplePrintEventOut Trigger_SimplePrintEvent_t;
	Trigger_SimplePrintEvent_t Trigger_SimplePrintEvent_v;
	
	// Modules
	Trigger<This> Trigger_v;
	SubApp<This> SubApp_v;
	Printer<This> Printer_v;
	
	// Object binding
	App() :
		SubApp_SimplePrintEvent_v(SubApp_SimplePrintEvent_t(Printer_v)),
		Trigger_SimplePrintEvent_v(Trigger_SimplePrintEvent_t(SubApp_v.SimplePrintEventOut_v)),
		Trigger_v(
			Trigger_SimplePrintEvent_v
			),
		SubApp_v(
			SubApp_SimplePrintEvent_v
			),
		Printer_v(

			)
	{}
};


// ----- End of generated -----


App app;

int main()
{
    app.Trigger_v.run();
    return 0;
}