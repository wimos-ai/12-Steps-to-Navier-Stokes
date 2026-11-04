#include "scenes/Scene0.hpp"
#include "utility.hpp"
#include <algorithm>

Scene1::Scene1()
{
    auto x_values = linspacev<double>(0, 10, 100);
    std::vector<double> y_values(x_values.size());
    std::transform(x_values.begin(), x_values.end(), y_values.begin(), FN_FWD(std::sin));
    figure.plot(2, -0.5);
    figure.plot(x_values, y_values);
    figure.set_title("Scene0");
    figure.set_x_label("x");
    figure.set_y_label("sin(x)");
}

void Scene1::render(SDL_Renderer *renderer)
{
    figure.render(renderer);
}

void Scene1::handle_event(const SDL_Event &)
{
}
