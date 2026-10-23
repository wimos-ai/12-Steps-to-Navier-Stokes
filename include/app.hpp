#pragma once

#include <memory>

#include "IScene.hpp"

class App
{
public:
    static void backend_init();

public:
    App();

    void run();

private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    uint64_t previous_time;
    std::unique_ptr<IScene> scene;
};