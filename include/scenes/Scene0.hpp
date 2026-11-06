#pragma once
#include "IScene.hpp"

#include "figure.hpp"

class Scene0 : public IScene
{
public:
    Scene0();

    void render(SDL_Renderer *) override;
    void handle_event(const SDL_Event& event) override;
    ~Scene0() override = default;

private:
    Figure figure;
};