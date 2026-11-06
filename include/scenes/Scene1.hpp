#pragma once
#include "IScene.hpp"

#include "figure.hpp"

#include <vector>
#include <chrono>

class Scene1 : public IScene
{
public:
    Scene1();

    void render(SDL_Renderer *renderer) override;
    void handle_event(const SDL_Event &event) override;
    ~Scene1() override = default;

private:
    Figure figure;

    double dt{1 / 60.0};
    std::size_t nt{200};
    std::size_t nx{100};
    std::size_t idx{0};
    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
    std::vector<double> xvalues;
    std::vector<std::vector<double>> yvalues;
};
