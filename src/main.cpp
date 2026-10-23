#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <cmath>
#include <cstdio>
#include <vector>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // --------------------------------------------------------
    // Initialize SDL3
    // --------------------------------------------------------

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 + Dear ImGui Test",
        1280,
        800,
        SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Enable VSync when supported.
    SDL_SetRenderVSync(renderer, 1);

    // --------------------------------------------------------
    // Initialize Dear ImGui
    // --------------------------------------------------------

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
    {
        SDL_Log("ImGui SDL3 backend initialization failed.");

        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer))
    {
        SDL_Log("ImGui SDL renderer backend initialization failed.");

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // --------------------------------------------------------
    // Application state
    // --------------------------------------------------------

    bool running = true;
    bool show_demo = true;
    int counter = 0;

    float frequency = 1.0f;
    float amplitude = 1.0f;

    float elapsed_time = 0.0f;
    Uint64 previous_time = SDL_GetPerformanceCounter();

    std::vector<float> waveform(256);

    // --------------------------------------------------------
    // Main loop
    // --------------------------------------------------------

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

        // Calculate frame time.
        const Uint64 current_time = SDL_GetPerformanceCounter();

        const double delta_time =
            static_cast<double>(current_time - previous_time) /
            static_cast<double>(SDL_GetPerformanceFrequency());

        previous_time = current_time;

        elapsed_time += static_cast<float>(delta_time);

        // Start a new ImGui frame.
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // ----------------------------------------------------
        // Custom application window
        // ----------------------------------------------------

        ImGui::Begin("SDL3 + ImGui Test");

        ImGui::Text("SDL3 and Dear ImGui are working!");
        ImGui::Separator();

        ImGui::Text("Frame rate: %.1f FPS", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f * io.DeltaTime);

        ImGui::Spacing();

        if (ImGui::Button("Click me"))
        {
            ++counter;
        }

        ImGui::SameLine();
        ImGui::Text("Button clicked %d times", counter);

        ImGui::Checkbox("Show ImGui demo", &show_demo);

        ImGui::Spacing();
        ImGui::Separator();

        // Sine wave controls.
        ImGui::Text("Sine Wave");

        ImGui::SliderFloat(
            "Frequency",
            &frequency,
            0.1f,
            5.0f
        );

        ImGui::SliderFloat(
            "Amplitude",
            &amplitude,
            0.1f,
            2.0f
        );

        // Generate waveform samples.
        constexpr float TWO_PI = 6.28318530718f;

        for (std::size_t i = 0; i < waveform.size(); ++i)
        {
            const float x =
                static_cast<float>(i) /
                static_cast<float>(waveform.size() - 1);

            waveform[i] =
                amplitude * std::sin(
                    TWO_PI * frequency * x + elapsed_time
                );
        }

        ImGui::PlotLines(
            "##sine",
            waveform.data(),
            static_cast<int>(waveform.size()),
            0,
            nullptr,
            -2.0f,
            2.0f,
            ImVec2(0.0f, 180.0f)
        );

        ImGui::End();

        // Show the built-in ImGui demonstration.
        if (show_demo)
        {
            ImGui::ShowDemoWindow(&show_demo);
        }

        // ----------------------------------------------------
        // Render
        // ----------------------------------------------------

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            renderer
        );

        SDL_RenderPresent(renderer);
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

    return 0;
}