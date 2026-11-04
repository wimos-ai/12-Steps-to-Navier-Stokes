#include <cmath>
#include <cstdio>
#include <vector>

#include "app.hpp"
#include "figure.hpp"

int main()
{
    App::backend_init();
    Figure::FigureInit();
    {
        App app;
        app.run();
    }
    Figure::FigureQuit();
    return 0;
}
