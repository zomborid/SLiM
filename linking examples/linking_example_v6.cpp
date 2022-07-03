#include <iostream>
#include <type_traits>

using namespace std;

// IEvent.h
template<typename M>
struct IEvent{
    static void signal();
};


// A.h

template<typename Parent>
struct A{
    using This = A<Parent>;

    using Printer = IEvent<typename Parent::A_Printer>;

    static void run(){
        Printer::signal();
    }
};

template<typename Parent>
struct IEvent<A<Parent>>{
    using This = A<Parent>;

    static void signal(){
        printf("this is A");
    }
};

// B.h
template<typename Parent>
struct B{
    using This = B<Parent>;

    using Printer = IEvent<typename Parent::B_Printer>;

};

template<typename Parent>
struct IEvent<B<Parent>>{
    using This = B<Parent>;

    static void signal(){
        printf("this is B");
        This::Printer::signal();
    }
};

#define Module(name)    \
template<typename Parent>   \
struct name{    \
    using This = name<Parent>;  \

#define UseInterface(module,iface,name) \
using name = iface<typename Parent::module##_##name>;

#define ImplementInterface(module,iface) \
template<typename Parent>   \
struct iface<module<Parent>>{ \
    using This = module<Parent>;   \

// Tee.h

Module(Tee)
    using Printer = IEvent<typename Parent::Tee_Printer>;

};

ImplementInterface(Tee,IEvent)
    static void signal(){
        printf("this is Tee");
        This::Printer::signal();
    }
};


// Wiring.h

template<typename Parent>
struct SubApp1{
    using This = SubApp1<Parent>;

    using Printer = Tee<This>;
    using Tee_Printer = B<Parent>;
};

struct App{
    using This = App;

    using A_Printer = SubApp1<This>::Printer;
    using B_Printer = A<This>;
};

// main.cpp

int main(){
    A<App>::run();
    return 0;
}