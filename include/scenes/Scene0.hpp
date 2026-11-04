#pragma once
#include "IScene.hpp"

#include "figure.hpp"

class Scene1 : public IScene
{
public:
    Scene1();

    void render(SDL_Renderer *) override;
    void handle_event(const SDL_Event& event) override;
    ~Scene1() override = default;

private:
    Figure figure;
};