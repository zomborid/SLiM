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

struct App{
    using This = App;


    struct Tee_as_TeePrinter;
    struct Tee_as_TeeStopPrinter;
    struct A_as_A;
    struct B_as_B;

    struct Tee_as_TeePrinter{
        using Tee_uses_EventIn_source = B<This::B_as_B>;
        using Tee_uses_EventIn_name = B_provides_Printer;
    };

    struct Tee_as_TeeStopPrinter{
        using Tee_uses_EventIn_source = B<This::B_as_B>;
        using Tee_uses_EventIn_name = B_provides_StopPrinter;
    };

    struct A_as_A{
        using A_uses_Printer_source = Tee<This::Tee_as_TeePrinter>;
        using A_uses_Printer_name = Tee_provides_EventOut;

        using A_uses_StopPrinter_source = Tee<This::Tee_as_TeeStopPrinter>;
        using A_uses_StopPrinter_name = Tee_provides_EventOut;
    };

    struct B_as_B{
    };


    static void run(){
        // init modules, start scheduler
        A<A_as_A>::run();
    }
};


// main.cpp

int main(){
    App::run();
    return 0;
}