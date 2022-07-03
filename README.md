This is an archive of an abandoned, but interesting project.

# SLiM
A nesC clone for C++ applications.  
SLiM transpiles a system of linked modules to C++ using metaprogramming.  
This is a useful programming paradigm in low level embedded systems, where dynamic memory allocation is a risky endeavour and multiple processes have to work together without the guarantees of an operating system.

## Short description of SLiM programming
Given interfaces (.if) and modules (.h/.hpp) that are providing interfaces, an interlinked network of modules can be created. Modules are attached through the interfaces as defined in a configuration file (.slim). Configurations define components (instances of modules), routing between the component interfaces (wiring) and provides interfaces to the outside world (component interfaces can be routed for external access).

<TODO add figure to depict the SLiM interface-module-component system>
