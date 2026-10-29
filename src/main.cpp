#include <cmath>
#include <cstdio>
#include <vector>

#include "app.hpp"

// int main()
// {
//     App::backend_init();
//     {
//         App app;
//         app.run();
//     }
//     return 0;
// }

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "render_graph.hpp"
#include "figure.hpp"

std::vector<double> linspace(double start, double end, size_t num)
{
    std::vector<double> result;

    if (num == 0)
        return result;
    if (num == 1)
    {
        result.push_back(start);
        return result;
    }

    result.reserve(num); // Optimize memory allocation
    double delta = (end - start) / (num - 1);

    for (size_t i = 0; i < num - 1; ++i)
    {
        result.push_back(start + delta * i);
    }

    // Explicitly add the final boundary to avoid floating-point drift
    result.push_back(end);

    return result;
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        std::exit(-1);
    }

    if (!TTF_Init())
    {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        std::exit(-1);
    }
    

    SDL_Window *window = SDL_CreateWindow(
        "SDL3 + Dear ImGui Test",
        1280,
        800,
        SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        std::exit(-1);
    }

    Figure::FigureInit();

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(-1);
    }

    // Enable VSync when supported.
    SDL_SetRenderVSync(renderer, 1);
    Figure fig;
    std::vector<double> x_values = linspace(0, 3.14159 * 2.0, 1000);
    std::vector<double> y_values(x_values.size());
    std::transform(
        x_values.begin(),
        x_values.end(),
        y_values.begin(),
        [](double x)
        { return std::sin(x); });
    fig.plot(2, -0.5);
    fig.plot(x_values, y_values);
    fig.set_title("Title");
    fig.set_x_label("XLabel");
    fig.set_y_label("YLabel");

    bool running{true};
    while (running)
    {
        // Process SDL events.
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {

            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        fig.render(renderer);

        SDL_RenderPresent(renderer);
    }

    return 0;
}