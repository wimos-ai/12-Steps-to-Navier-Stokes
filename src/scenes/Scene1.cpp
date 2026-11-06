#include "scenes/Scene1.hpp"
#include "utility.hpp"
#include <algorithm>

Scene1::Scene1() : xvalues(linspacev<double>(0, 5, nx)), yvalues(nt)
{
    // Allocate memory
    std::fill(yvalues.begin(), yvalues.end(), std::vector<double>(nx));

    // Setup Initial Conditions
    for (size_t i = 0; i < nx; i++)
    {
        if (xvalues[i] > 0.5 && xvalues[i] < 1)
        {
            yvalues[0][i] = 1;
        }
        else
        {
            yvalues[0][i] = 0;
        }
    }

    double c{1.0};
    double dx{xvalues[1]};
    double courant_number = (c*dt)/xvalues[1];
    SDL_Log("Step 1 courant_number: %f", courant_number);
    // Forward Propagation
    for (size_t n = 1; n < nt; n++)
    {
        for (size_t i = 1; i < nx; i++)
        {
            yvalues[n][i] = yvalues[n - 1][i] - c * (dt/dx) * (yvalues[n-1][i] - yvalues[n-1][i-1]);
        }
    }
}

void Scene1::render(SDL_Renderer *renderer)
{
    auto elapsed_time_s = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() / 1000.0;
    size_t idx = elapsed_time_s / dt;
    if (idx >= std::size(yvalues))
    {
        t0 = std::chrono::steady_clock::now();
        idx = 0;
    }

    figure.clear();
    figure.plot(xvalues, yvalues[idx]);
    figure.set_title("Scene1");
    figure.set_x_label("x");
    figure.set_y_label("u(x,t)");
    figure.set_graph_area(0,5,0,1);

    figure.render(renderer);
}

void Scene1::handle_event(const SDL_Event &)
{
}
