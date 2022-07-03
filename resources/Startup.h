#pragma once

Module(Startup){
    // interface uses and provides, tasks

    ProvideInterfaceAs(Boot,B);

    bool enabled = false;

    void Command(B,enable)(){
        enabled = true;
    }

    void run(){
        if (enabled) B::booted(0);
    }

};