#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <cstdlib>
#include <stdexcept>

#include "app.hpp"
#include "testScene.hpp"

void App::backend_init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return std::exit(-1);
    }
}

App::App() : renderer(nullptr), window(nullptr), scene(nullptr)
{
    window = SDL_CreateWindow(
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

    renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(-1);
    }

    // Enable VSync when supported.
    SDL_SetRenderVSync(renderer, 1);

    // --------------------------------------------------------
    // Initialize Dear ImGui
    // --------------------------------------------------------

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
    {
        SDL_Log("ImGui SDL3 backend initialization failed.");

        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(-1);
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer))
    {
        SDL_Log("ImGui SDL renderer backend initialization failed.");

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(-1);
    }

    this->scene = std::make_unique<TestScene>();
}

void App::run()
{
    bool running{true};
    while (running)
    {
        // Process SDL events.
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

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

        this->scene->render(this->renderer);
    }

    // --------------------------------------------------------
    // Cleanup
    // --------------------------------------------------------

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}
