#pragma once

#include "IScene.hpp"

#include <vector>
#include <cstdint>
#include <chrono>

class TestScene : public IScene
{
public:
    TestScene();
    void render(SDL_Renderer *) override;

private:
    bool show_demo = true;
    int counter = 0;

    float frequency = 1.0f;
    float amplitude = 1.0f;

    float elapsed_time = 0;

    std::chrono::time_point<std::chrono::steady_clock> previous_time = std::chrono::steady_clock::now();

    std::vector<float> waveform;
};