#pragma once

#include <SDL3/SDL.h>

class IScene
{
public:
    virtual void render(SDL_Renderer *) = 0;
    virtual void handle_event(const SDL_Event& event) = 0;
    virtual ~IScene() = default;
};