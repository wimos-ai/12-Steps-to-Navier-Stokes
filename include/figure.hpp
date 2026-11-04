#pragma once

#include <span>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <utility>
#include <cmath>

#include <SDL3/SDL.h>

#include "utility.hpp"

struct TTF_Font;

class Figure
{

public:
    Figure() = default;

    void set_graph_area(double x0, double xf, double y0, double yf);

    void set_title(std::string title);

    void set_x_label(std::string label);

    void set_y_label(std::string label);

    void plot(std::span<const double> x, std::span<const double> y);

    void plot(double x, double y);

    void clear();

    void render(SDL_Renderer *renderer, std::optional<std::reference_wrapper<SDL_Rect>> render_area = std::nullopt);

private:
    class Rect
    {
    public:
        void normalize();

        Rect get_bounding_volume(const Rect &other);

        inline Rect(double x0_, double y0_, double xf_, double yf_) : x0(x0_), y0(y0_), xf(xf_), yf(yf_) { this->normalize(); }

        double area() const;

        double x0, y0, xf, yf;
    };

    friend auto determine_graph_transform(const Figure::Rect &r1, const SDL_FRect &r2);

private:
    class DrawableFunction
    {
    public:
        DrawableFunction(std::span<const double> x, std::span<const double> y);

    public:
        void render(SDL_Renderer *renderer, auto transform, const SDL_FRect &graph_area) const;

    public:
        Rect get_render_area() const;

    private:
        std::unique_ptr<double[]> x_values;
        std::unique_ptr<double[]> y_values;
        std::size_t num_values{0};
    };

    class DrawablePoint
    {
    public:
        inline DrawablePoint(double x_, double y_) : x(x_), y(y_) {};
        inline Rect get_render_area() const
        {
            return Rect{x, y, x, y};
        };

    public:
        void render(SDL_Renderer *renderer, auto transform, const SDL_FRect &graph_area) const;

    private:
        double x, y;
    };

    using UDrawable = std::variant<DrawableFunction, DrawablePoint>;

protected:
    Rect determine_graph_area();

private:
    std::optional<std::string> title;
    std::optional<std::string> x_label;
    std::optional<std::string> y_label;

    std::optional<Rect> graph_area;

    std::vector<UDrawable> draw_data;

private:
    class Axis
    {
    public:
        inline Axis(double min_, double max_) : min(min_), max(max_)
        {
            if (min >= max)
            {
                throw std::invalid_argument("max must be strictly greater than min");
            }
            double distance = max - min;
            step = std::pow(10, std::floor(std::log10(distance) - 0.5));
            start = std::floor(min / step) * step;
            end = std::ceil(max / step) * step;
            count = static_cast<size_t>((end - start) / step) + 1;
        };

        inline size_t size() const
        {
            return count;
        }

        auto ticks() const
        {
            return linspace(start, end, count);
        }

    private:
        double min;
        double max;
        double start;
        double end;
        double step;
        size_t count;
    };

    class Layout
    {
    public:
        Layout() = default;

        Layout(const SDL_Rect &render_area);

    public:
        SDL_FRect outer_graph_zone{};
        SDL_FRect inner_graph_zone{};
        SDL_FRect title_zone{};
        SDL_FRect xlabel_zone{};
        SDL_FRect ylabel_zone{};
        SDL_FRect x_axis_zone{};
        SDL_FRect y_axis_zone{};
    };

    static void render_layout_dbg(SDL_Renderer *renderer, const Figure::Layout &layout);

    struct RenderCache
    {
    public:
        RenderCache() = default;

        RenderCache(const SDL_Rect &render_area, const std::optional<std::string> &title, const std::optional<std::string> &xlabel, const std::optional<std::string> &ylabel);

    private:
        class SDLSurfaceDeleter
        {
        public:
            inline void operator()(SDL_Surface *ptr) const
            {
                if (ptr)
                {
                    SDL_DestroySurface(ptr);
                }
            }
        };

        using SDL_RAII_Surface = std::unique_ptr<SDL_Surface, SDLSurfaceDeleter>;

    public:
        Layout layout{};
        SDL_RAII_Surface title{nullptr};
        SDL_RAII_Surface xlabel{nullptr};
        SDL_RAII_Surface ylabel{nullptr};
        SDL_Rect area{};
    };

    std::optional<RenderCache> cache;

public:
    static void FigureInit();
    static void FigureInit(TTF_Font *font);

    static void FigureQuit();
    static TTF_Font *font;
};