#include <iostream>


#define ProvideInterface(module,name)  \
struct name {   \
    module& p;  \
    name(module& p) : p(p) {}

template<class IFaces>
struct A{
    typedef A<IFaces> This;

    ProvideInterface(This,Printer)
        void print(){
            printf("this is A");
        }
    };
};

template<class IFaces>
struct B{
    typedef B<IFaces> This;
    IFaces& ifaces;
    B(IFaces& ifaces) : ifaces(ifaces) {}

    ProvideInterface(This,Printer)
        void print(){
            printf("this is B");
        }
    };

    void run(){
        ifaces.Printer.print();
    }
};

struct App{

    struct IFaces_of_A{
        
    };
    A<IFaces_of_A> A_v;
    A<IFaces_of_A>::Printer Printer_of_A;


    struct IFaces_of_B{
        IFaces_of_B(A<IFaces_of_A>::Printer& Printer) :
            Printer(Printer)
        {}

        A<IFaces_of_A>::Printer& Printer;
    };
    IFaces_of_B IFaces_of_B_v;
    B<IFaces_of_B> B_v;

    App() :
        A_v(),
        B_v(IFaces_of_B_v),
        Printer_of_A(A_v),
        IFaces_of_B_v(
            Printer_of_A
        )
    {}
};

App app;

int main()
{
    app.B_v.run();
    return 0;
}