#include <iostream>

namespace {


#define DefineInterface(module,iface,name)   \
template<typename Parent> struct module##_provides_##name{};  \
template<typename Parent> struct iface<module##_provides_##name<Parent>>

#define DefineInterfaceFunction(module,iface,return_type,name)   \
template<>  \
template<typename Parent>   \
return_type iface< module##_provides_##name<Parent> >  

#define Interface(module,iface,name) using name = iface<typename Parent::module##_uses_##name>;

// Interface declaration

template<typename Provider>
struct IEvent{
    static void signal();
};

template<typename Parent>
struct A{

    Interface(A,IEvent,Printer)

    static void run(){
        Printer::signal();
    }
};


// B

template<typename Parent>
struct B{
    static const int secret = 10;

};

DefineInterface(B,IEvent,Print){
    static void signal();
};

DefineInterfaceFunction(B,IEvent,void,Print)::signal(){
    printf("%i", B<Parent>::secret);
}

// End B

// ------- Wiring

struct App{
    using This = App;

    using A_uses_Printer = B_provides_Print<This>;
};

// ------- End wiring
}

int main()
{
    A<App>::run();
    return 0;
}