#include "render_graph.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <stdexcept>
#include <memory>
#include <array>

#include <Eigen/Dense>
#include <Eigen/Geometry>

template <typename It1, typename It2>
auto deref_pair(const std::pair<It1, It2> &it_pair)
{
    return std::make_pair(*it_pair.first, *it_pair.second);
}

void render_graph(SDL_Renderer *renderer,
                  std::span<double> x,
                  std::span<double> y,
                  std::optional<std::pair<double, double>> xspan_,
                  std::optional<std::pair<double, double>> yspan_)
{
    if (std::size(x) < 1 || std::size(y) < 1)
    {
        throw std::runtime_error("Invalid sizes");
    }

    if (std::size(x) != std::size(y))
    {
        throw std::runtime_error("Mismatched sizes");
    }

    if (xspan_.has_value())
    {
        if (xspan_.value().first > xspan_.value().second)
        {
            throw std::runtime_error("xspan must be ordered");
        }
    }

    if (yspan_.has_value())
    {
        if (yspan_.value().first > yspan_.value().second)
        {
            throw std::runtime_error("xspan must be ordered");
        }
    }

    double x_min;
    double x_max;
    double y_min;
    double y_max;

    {
        std::pair<double, double> xspan = xspan_.value_or(deref_pair(std::minmax_element(x.begin(), x.end())));
        std::pair<double, double> yspan = yspan_.value_or(deref_pair(std::minmax_element(y.begin(), y.end())));
        x_min = xspan.first;
        x_max = xspan.second;
        y_min = yspan.first;
        y_max = yspan.second;
    }
    SDL_Rect render_area;

    if (!SDL_GetRenderViewport(renderer, &render_area))
    {
        std::string err = "SDL_GetRenderViewport: ";
        err += SDL_GetError();
        throw std::runtime_error(err);
    }

    constexpr double BORDER_SZ = 10.0;

    const double width = render_area.w - 2.0 * BORDER_SZ;
    const double height = render_area.h - 2.0 * BORDER_SZ;

    Eigen::Matrix2d graph_to_sdl_transform;
    graph_to_sdl_transform << width / (x_max - x_min), 0.0,
        0.0, -height / (y_max - y_min);

    Eigen::Vector2d offset{
        BORDER_SZ - x_min * width / (x_max - x_min),
        BORDER_SZ + y_max * height / (y_max - y_min)};

    SDL_FPoint *points = new SDL_FPoint[std::size(x)];

    for (size_t i = 0; i < std::size(x); ++i)
    {
        Eigen::Vector2d graph_space{x[i], y[i]};

        Eigen::Vector2d screen_space =
            graph_to_sdl_transform * graph_space + offset;

        points[i].x = static_cast<float>(screen_space.x());
        points[i].y = static_cast<float>(screen_space.y());
    }

    SDL_RenderLines(renderer, points, static_cast<int>(std::size(x)));

    delete[] points;
}