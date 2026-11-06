#include "scenes/Scene1.hpp"
#include "utility.hpp"
#include <algorithm>

Scene1::Scene1()
{
    auto x_values = linspacev<double>(0, 5, 1000);
    std::vector<double> y_values(x_values.size());
    std::transform(x_values.begin(), x_values.end(), y_values.begin(), FN_FWD(std::cos));
    figure.plot(x_values, y_values);
    figure.set_title("Scene1");
    figure.set_x_label("x");
    figure.set_y_label("cos(x)");
}

void Scene1::render(SDL_Renderer *renderer)
{
    figure.render(renderer);
}

void Scene1::handle_event(const SDL_Event &)
{
}
