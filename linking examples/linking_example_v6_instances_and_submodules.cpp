#include <iostream>
#include <type_traits>

using namespace std;

// IEvent.h
template<typename Module, typename Name>
struct IEvent{
    static void signal();
};


// A.h

template<typename Parent>
struct A{
    using This = A<Parent>;

    using Printer = IEvent<typename Parent::A_uses_Printer_source,typename Parent::A_uses_Printer_name>;

    using StopPrinter = IEvent<typename Parent::A_uses_StopPrinter_source,typename Parent::A_uses_StopPrinter_name>;

    static void run(){
        Printer::signal();
        StopPrinter::signal();
    }
};

struct A_provides_Printer{};
template<typename Parent>
struct IEvent<A<Parent>, A_provides_Printer>{
    using This = A<Parent>;

    static void signal(){
        printf("this is A");
    }
};

// B.h
template<typename Parent>
struct B{
    using This = B<Parent>;

    //using Printer = IEvent<typename Parent::B_uses_Printer_source,typename Parent::B_uses_Printer_name>;

    static void run(){
        //Printer::signal();
    }

};

struct B_provides_Printer{};
template<typename Parent>
struct IEvent<B<Parent>, B_provides_Printer>{
    using This = B<Parent>;

    static void signal(){
        printf("this is B");
        //This::Printer::signal();
    }
};

struct B_provides_StopPrinter{};
template<typename Parent>
struct IEvent<B<Parent>, B_provides_StopPrinter>{
    using This = B<Parent>;

    static void signal(){
        printf("this is stop B");
    }
};

#define Module(name)    \
template<typename Parent>   \
struct name{    \
    using This = name<Parent>;  \

#define UseInterface(module,iface,name) \
using name = iface<typename Parent::module##_uses_##name##_source,typename Parent::module##_uses_##name##_name>;

#define ImplementInterface(module,iface,name) \
struct module##_provides_##name{};  \
template<typename Parent>   \
struct iface<module<Parent>,module##_provides_##name>{ \
    using This = module<Parent>;   \

// Tee.h

Module(Tee)
    UseInterface(Tee,IEvent,EventIn) 
    //static constexpr char* secret = (char*)"tee secret";
};

ImplementInterface(Tee,IEvent,EventOut)
    static void signal(){
        printf("this is Tee");
        This::EventIn::signal();
    }
};


// Wiring.h

















template<typename Parent>
struct SubApp1 {
	using This = SubApp1<Parent>;
	
	// component declarations
	struct Tee_as_Tee;
	
	
	// config use declarations
	using uses_PrinterOut_source = typename Parent::uses_PrinterOut_source;
	using uses_PrinterOut_name = typename Parent::uses_PrinterOut_name;
	
	
	// subconfig use declarations
	
	// config provide declarations
	using provides_PrinterIn_source = Tee<Tee_as_Tee>;
	using provides_PrinterIn_name = Tee_provides_EventOut;
	
	
	struct Tee_as_Tee {
		using Tee_uses_EventIn_source = uses_PrinterOut_source;
		using Tee_uses_EventIn_name = uses_PrinterOut_name;
		
	};
	
};


template<typename Parent>
struct SubApp4 {
	using This = SubApp4<Parent>;
	
	// component declarations
	
	
	// config use declarations
	using uses_Cloned_source = typename Parent::uses_Cloned_source;
	using uses_Cloned_name = typename Parent::uses_Cloned_name;
	
	
	// subconfig use declarations
	
	// config provide declarations
	using provides_PrinterIn_source = uses_Cloned_source;
	using provides_PrinterIn_name = uses_Cloned_name;
	
	
};


template<typename Parent>
struct SubApp2 {
	using This = SubApp2<Parent>;
	
	// component declarations
	struct SubApp4_in_SubApp2;
	
	
	// config use declarations
	using uses_PrinterOut_source = typename Parent::uses_PrinterOut_source;
	using uses_PrinterOut_name = typename Parent::uses_PrinterOut_name;
	
	
	// subconfig use declarations
	struct SubApp4_in_SubApp2 {
		using uses_Cloned_source = uses_PrinterOut_source;
		using uses_Cloned_name = uses_PrinterOut_name;
		
	};
	
	// config provide declarations
	using provides_PrinterIn_source = typename SubApp4<SubApp4_in_SubApp2>::provides_PrinterIn_source;
	using provides_PrinterIn_name = typename SubApp4<SubApp4_in_SubApp2>::provides_PrinterIn_name;
	
	
};


template<typename Root>
struct MainConfig {
	using This = MainConfig<Root>;
	// component declarations
	struct A_as_A;
	struct B_as_Sink;
	struct Tee_as_Tee;
	struct SubApp1_in_MainConfig;
	struct SubApp2_in_MainConfig;
	
	
	struct SubApp1_in_MainConfig {
		using uses_PrinterOut_source = typename SubApp2<SubApp2_in_MainConfig>::provides_PrinterIn_source;
		using uses_PrinterOut_name = typename SubApp2<SubApp2_in_MainConfig>::provides_PrinterIn_name;
		
	};
	struct SubApp2_in_MainConfig {
		using uses_PrinterOut_source = B<B_as_Sink>;
		using uses_PrinterOut_name = B_provides_Printer;
		
	};
	struct A_as_A {
		using A_uses_Printer_source = typename SubApp1<SubApp1_in_MainConfig>::provides_PrinterIn_source;
		using A_uses_Printer_name = typename SubApp1<SubApp1_in_MainConfig>::provides_PrinterIn_name;
		
		using A_uses_StopPrinter_source = Tee<Tee_as_Tee>;
		using A_uses_StopPrinter_name = Tee_provides_EventOut;
		
	};
	
	struct B_as_Sink {
	};
	
	struct Tee_as_Tee {
		using Tee_uses_EventIn_source = B<B_as_Sink>;
		using Tee_uses_EventIn_name = B_provides_StopPrinter;
		
	};
	
};
struct Root {};
using App = MainConfig<Root>;
















static void run(){
    A<App::A_as_A>::run();
}


// main.cpp

int main(){
    run();
    return 0;
}