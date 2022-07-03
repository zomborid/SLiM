#include <iostream>

namespace {


#define DefineInterface(module,iface,name)   \
template<typename Parent> struct module##_provides_##name{};  \
template<typename Parent> struct iface<module##_provides_##name<Parent>>

#define DefineInterfaceFunction(module,iface,return_type,name)   \
template<>  \
template<typename Parent>   \
return_type iface< module##_provides_##name<Parent> >

#define InterfaceThis(module) using This = module<Parent>;

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
    static const int secret = 13;

};

DefineInterface(B,IEvent,Print){
    static void signal();
};

DefineInterfaceFunction(B,IEvent,void,Print)::signal(){
    printf("%i", B<Parent>::secret);
}

// End B



// Tee

template<typename Parent>
struct Tee{
    Interface(Tee,IEvent,PrinterOut)
};

DefineInterface(Tee,IEvent,PrinterIn){
    static void signal();
};

DefineInterfaceFunction(Tee,IEvent,void,PrinterIn)::signal(){
    InterfaceThis(Tee)
    printf("this is tee");
    This::PrinterOut::signal();
}

// End Tee

// ------- Wiring

template<typename Parent>
struct SubApp{
    using This = SubApp<Parent>;

    using _provides_PrinterIn = Tee_provides_PrinterIn<This>;
    using Tee_uses_PrinterOut = typename Parent::SubApp_uses_PrinterOut;
};

struct App{
    using This = App;

    using SubApp_uses_PrinterOut = B_provides_Print<This>;
    using A_uses_Printer = SubApp<This>::_provides_PrinterIn;
};

// ------- End wiring
}

int main()
{
    A<App>::run();
    return 0;
}