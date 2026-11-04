#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <cstdlib>
#include <stdexcept>

#include "app.hpp"
#include "scenes/Scene0.hpp"

void App::backend_init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        std::exit(-1);
    }

    if (!TTF_Init())
    {
        SDL_Log("TTF_Init Error: %s", SDL_GetError());
        SDL_Quit();
        std::exit(-1);
    }

    // --------------------------------------------------------
    // Initialize Dear ImGui
    // --------------------------------------------------------

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
}

App::App() : window(nullptr), renderer(nullptr), scene(nullptr)
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

    // Hook IMGUI up to SDL

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

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    this->scene = std::make_unique<Scene1>();
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

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        this->scene->render(this->renderer);

        ImGui::Render();
        SDL_RenderPresent(renderer);

        // Update and Render additional Platform Windows
        // See https://github.com/ocornut/imgui/wiki/Multi-Viewports
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    // --------------------------------------------------------
    // Cleanup
    // --------------------------------------------------------

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Init();
    SDL_Quit();
}
