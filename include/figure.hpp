#pragma once

#include <span>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <memory>

#include <SDL3/SDL.h>

class SDL_Renderer;

class Figure
{

public:
    Figure() = default;

    void set_graph_area(double x0, double xf, double y0, double yf);

    void set_title(std::string title);

    void set_x_label(std::string label);

    void set_y_label(std::string label);

    void plot(std::span<double> x, std::span<double> y);

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

    friend auto determine_graph_transform(Figure::Rect r1, SDL_FRect r2);

private:
    class DrawableFunction
    {
    public:
        friend class Figure;
        DrawableFunction(std::span<double> x, std::span<double> y);

        // No copy
        DrawableFunction(DrawableFunction &) = delete;
        DrawableFunction &operator=(const DrawableFunction &other) = delete;

        // Moveable
        inline DrawableFunction(DrawableFunction &&other) noexcept
        {
            this->x_values = other.x_values;
            this->y_values = other.y_values;
            this->num_values = other.num_values;
        }

        inline DrawableFunction &operator=(const DrawableFunction &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            this->~DrawableFunction(); // clean up our data

            this->x_values = other.x_values;
            this->y_values = other.y_values;
            this->num_values = other.num_values;
        }

        ~DrawableFunction(); // TODO: Free x and y values

    private:
        void render(SDL_Renderer *renderer, auto transform, const SDL_FRect& graph_area) const;

    public:
        Rect get_render_area() const;

    private:
        double *x_values{nullptr};
        double *y_values{nullptr};
        std::size_t num_values{0};
    };

    class DrawablePoint
    {
    public:
        friend class Figure;
        inline DrawablePoint(double x_, double y_) : x(x_), y(y_) {};
        inline Rect get_render_area() const
        {
            return Rect{x, y, x, y};
        };

    private:
        void render(SDL_Renderer *renderer, auto transform, const SDL_FRect& graph_area) const;

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

    std::optional<Rect> draw_area;

    std::vector<UDrawable> draw_data;
};