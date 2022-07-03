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



// ----- Generated based on connectivity graph -----

struct App {
	typedef App This;
	
	// Link objects
	typedef Printer<This>::SimplePrintEvent Trigger_SimplePrintEvent_t;
	Trigger_SimplePrintEvent_t Trigger_SimplePrintEvent_v;
	
	// Modules
	Trigger<This> Trigger_v;
	Printer<This> Printer_v;
	
	// Object binding
	App() :
		Trigger_SimplePrintEvent_v(Trigger_SimplePrintEvent_t(Printer_v)),
		Trigger_v(
			Trigger_SimplePrintEvent_v
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