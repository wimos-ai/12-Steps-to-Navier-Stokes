#include "testScene.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <cmath>

TestScene::TestScene() : waveform(256)
{
}

void TestScene::render(SDL_Renderer *renderer)
{
    ImGuiIO &io = ImGui::GetIO();

    // Calculate frame time.
    auto current_time = std::chrono::steady_clock::now();

    auto clock_delta = current_time - previous_time;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(clock_delta).count() / 1000.0;

    previous_time = current_time;

    elapsed_time += dt;

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
        5.0f);

    ImGui::SliderFloat(
        "Amplitude",
        &amplitude,
        0.1f,
        2.0f);

    // Generate waveform samples.
    constexpr float TWO_PI = 6.28318530718f;

    for (std::size_t i = 0; i < waveform.size(); ++i)
    {
        const float x =
            static_cast<float>(i) /
            static_cast<float>(waveform.size() - 1);

        waveform[i] =
            amplitude * std::sin(
                            TWO_PI * frequency * x + elapsed_time);
    }

    ImGui::PlotLines(
        "##sine",
        waveform.data(),
        static_cast<int>(waveform.size()),
        0,
        nullptr,
        -2.0f,
        2.0f,
        ImVec2(0.0f, 180.0f));

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
        renderer);

    SDL_RenderPresent(renderer);
}