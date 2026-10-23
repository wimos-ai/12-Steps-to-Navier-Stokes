#include <cmath>
#include <cstdio>
#include <vector>

#include "app.hpp"

int main()
{
    App::backend_init();
    {
        App app;
        app.run();
    }
    return 0;
}