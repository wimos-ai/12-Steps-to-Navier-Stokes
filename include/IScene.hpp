#pragma once

struct SDL_Window;
struct SDL_Renderer;

class IScene
{
public:
    virtual void render(SDL_Renderer *) = 0;
    virtual ~IScene() = default;
};