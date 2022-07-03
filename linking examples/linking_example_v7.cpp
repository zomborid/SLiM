#include <iostream>
#include <type_traits>

using namespace std;



template<typename Module, typename Name>
struct SignalMultiplexer{};

#define Module(name)    \
template<typename Parent>   \
struct name{    \
    using This = name<Parent>;  \

#define UseInterface(module,iface,name) \
using name = iface##_commands<typename Parent::module##_uses_##name##_source,typename Parent::module##_uses_##name##_name>;

#define ProvideInterface(module,iface,name) \
using name = typename Parent::module##_signals_##name;

#define ImplementInterface(module,iface,name) \
struct module##_provides_##name{};  \
template<typename Parent>   \
struct iface##_commands<module<Parent>,module##_provides_##name>{ \
    using This = module<Parent>;   \

#define ImplementSignal(module,iface,name) \
struct module##_hears_##name{};  \
template<typename Parent>   \
struct iface##_signals<module<Parent>,module##_hears_##name>{ \
    using This = module<Parent>;   \



// IBoot.h
template<typename Module, typename Name>
struct IBoot_commands{

};
template<typename Module, typename Name>
struct IBoot_signals{
    static void booted();
};






// Boot.h

Module(BootSource)
    ProvideInterface(BootSource,IBoot,Boot)

    static void run(){
        Boot::booted();
    }
};

ImplementInterface(BootSource,IBoot,Boot)

};


// Bootable.h

Module(Bootable)
    UseInterface(Bootable,IBoot,Boot)
};

ImplementSignal(Bootable,IBoot,Boot)
    static void booted(){
        printf("bootable");
    }
};


// StaticBootable.h

Module(StaticBootable)
    UseInterface(Bootable,IBoot,Boot)
};

ImplementSignal(StaticBootable,IBoot,Boot)
    static void booted(){
        printf("static bootable");
    }
};

// Wiring.h


static void BootSource_signals_Boot_booted();

// -----------------------------------------------------------------------

struct _GC {
	
    struct BootSource_singleton;
    struct StaticBootable_singleton;

	struct BootSource_singleton {
        using BootSource_signals_Boot = SignalMultiplexer<BootSource<_GC::BootSource_singleton>, BootSource_provides_Boot>;
	};
	struct StaticBootable_singleton {
		using Bootable_uses_Boot_source = BootSource<_GC::BootSource_singleton>;
		using Bootable_uses_Boot_name = BootSource_provides_Boot;
		
	};

    static void BootSource_signals_Boot_booted(){
        IBoot_signals<StaticBootable<StaticBootable_singleton>, StaticBootable_hears_Boot>::booted();
    }
};

// -----------------------------------------------------------------------

template<>
struct SignalMultiplexer<BootSource<_GC::BootSource_singleton>, BootSource_provides_Boot>{
    static void booted(){
        BootSource_signals_Boot_booted();
    }
};

// -----------------------------------------------------------------------

template<typename Parent>
struct SubApp1 {
	using This = SubApp1<Parent>;
	
	// component declarations
	struct Bootable_new_B1;

	
	// config use declarations
	
	
	// subconfig use declarations

	
	// config provide declarations
	

	
	struct Bootable_new_B1 {
		using Bootable_uses_Boot_source = BootSource<_GC::BootSource_singleton>;
		using Bootable_uses_Boot_name = BootSource_provides_Boot;
		
	};
	
    static void BootSource_signals_Boot_booted(){
        IBoot_signals<Bootable<Bootable_new_B1>, Bootable_hears_Boot>::booted();
    }
	
};


template<typename Root>
struct MainConfig {
	using This = MainConfig<Root>;

	// component declarations
	struct Bootable_new_B1;
	struct Bootable_new_B2;
	struct SubApp1_new_SubApp1;
	
	
	struct Bootable_new_B1 {
		using Bootable_uses_Boot_source = BootSource<_GC::BootSource_singleton>;
		using Bootable_uses_Boot_name = BootSource_provides_Boot;
		
	};
	struct Bootable_new_B2 {
		using Bootable_uses_Boot_source = BootSource<_GC::BootSource_singleton>;
		using Bootable_uses_Boot_name = BootSource_provides_Boot;
		
	};
	struct SubApp1_new_SubApp1 {
	};
	
    static void BootSource_signals_Boot_booted(){
        IBoot_signals<Bootable<Bootable_new_B1>, Bootable_hears_Boot>::booted();
        IBoot_signals<Bootable<Bootable_new_B2>, Bootable_hears_Boot>::booted();
        SubApp1<SubApp1_new_SubApp1>::BootSource_signals_Boot_booted();
    }
};

// -----------------------------------------------------------------------

struct Root {};
using App = MainConfig<Root>;

// -----------------------------------------------------------------------

static void BootSource_signals_Boot_booted(){
    _GC::BootSource_signals_Boot_booted();
    App::BootSource_signals_Boot_booted();
}

// -----------------------------------------------------------------------

static void run(){
    BootSource<_GC::BootSource_singleton>::run();
}

// main.cpp

int main(){
    run();
    return 0;
}