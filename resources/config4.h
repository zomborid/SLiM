#pragma once

void slim_start();

namespace slim_ap {
    int get_secret();
    void preboot(int foo);
}

