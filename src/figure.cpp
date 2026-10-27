#include "figure.hpp"

#include <cstring>
#include <ranges>
#include <algorithm>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

namespace
{
    void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius)
    {
        // See: https://stackoverflow.com/questions/38334081/how-to-draw-circles-arcs-and-vector-graphics-in-sdl
        const int32_t diameter = (radius * 2);

        int32_t x = (radius - 1);
        int32_t y = 0;
        int32_t tx = 1;
        int32_t ty = 1;
        int32_t error = (tx - diameter);

        while (x >= y)
        {
            //  Each of the following renders an octant of the circle
            SDL_RenderPoint(renderer, centreX + x, centreY - y);
            SDL_RenderPoint(renderer, centreX + x, centreY + y);
            SDL_RenderPoint(renderer, centreX - x, centreY - y);
            SDL_RenderPoint(renderer, centreX - x, centreY + y);
            SDL_RenderPoint(renderer, centreX + y, centreY - x);
            SDL_RenderPoint(renderer, centreX + y, centreY + x);
            SDL_RenderPoint(renderer, centreX - y, centreY - x);
            SDL_RenderPoint(renderer, centreX - y, centreY + x);

            if (error <= 0)
            {
                ++y;
                error += ty;
                ty += 2;
            }

            if (error > 0)
            {
                --x;
                tx += 2;
                error += (tx - diameter);
            }
        }
    }

    void DrawFilledCircle(SDL_Renderer *renderer, double x, double y, double radius)
    {
        // See: https://stackoverflow.com/questions/65723827/sdl2-function-to-draw-a-filled-circle
        for (int h = 0; h < radius; h++)
        {
            for (int w = 0; w < radius; w++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx * dx + dy * dy) < (radius * radius))
                {
                    SDL_RenderLine(renderer,x+dx,y+dy,x-dx, y+dy);
                    SDL_RenderLine(renderer,x+dx,y-dy,x-dx, y-dy);
                    goto drawn_line;
                }
            }
            drawn_line:
        }
    }

    void drawHTextInZone(SDL_Renderer *renderer, const SDL_FRect &zone, const std::string &txt)
    {
        // Draws horizontal text in zone
        // TODO
        SDL_RenderFillRect(renderer, &zone);
    }

    void drawVTextInZone(SDL_Renderer *renderer, const SDL_FRect &zone, const std::string &txt)
    {
        // Draws vertical text in zone
        // TODO
        SDL_RenderFillRect(renderer, &zone);
    }

    auto linspace(double start, double end, size_t num)
    {
        double step = (num > 1) ? (end - start) / (num - 1) : 0.0;
        return std::views::iota(std::size_t{0}, num) | std::views::transform([=](std::size_t i)
                                                                             { return start + i * step; });
    }

}

void Figure::set_graph_area(double x0, double xf, double y0, double yf)
{
    this->graph_area = Rect(x0, y0, xf, yf);
}

void Figure::set_title(std::string title)
{
    this->title = title;
}

void Figure::set_x_label(std::string label)
{
    this->x_label = label;
}

void Figure::set_y_label(std::string label)
{
    this->y_label = label;
}

void Figure::plot(std::span<double> x, std::span<double> y)
{
    this->draw_data.emplace_back(std::in_place_type<Figure::DrawableFunction>, x, y);
}

void Figure::plot(double x, double y)
{
    this->draw_data.emplace_back(std::in_place_type<Figure::DrawablePoint>, x, y);
}

void Figure::clear()
{
    this->draw_data.clear();
}

auto determine_graph_transform(const Figure::Rect &r1, const SDL_FRect &r2)
{
    /// Determines the affine transform from graph coordinates to SDL canvas coordinates
    // TODO

    return [=](double x_in, double y_in) -> SDL_FPoint
    {
        double x_percent = (x_in - r1.x0) / (r1.xf - r1.x0);
        double x_out = (r2.w * x_percent) + r2.x;

        double y_percent = (y_in - r1.y0) / (r1.yf - r1.y0);
        double y_out = (r2.h * (1.0 - y_percent)) + r2.y;

        return {static_cast<float>(x_out), static_cast<float>(y_out)};
    };
}

auto determine_layout(const SDL_Rect &render_area)
{
    // TODO
    struct Layout
    {
        SDL_FRect outer_graph_zone;
        SDL_FRect inner_graph_zone;
        SDL_FRect title_zone;
        SDL_FRect xlabel_zone;
        SDL_FRect ylabel_zone;
    };

    Layout p{};

    // Determine graph area

    constexpr double H_PAD = 0.1;
    constexpr double W_PAD = 0.1;
    constexpr int BOARDER_WIDTH = 5;

    p.outer_graph_zone.x = render_area.x + W_PAD * render_area.w;
    p.outer_graph_zone.h = (1.0 - (2 * H_PAD)) * render_area.h;
    p.outer_graph_zone.y = render_area.y + H_PAD * render_area.h;
    p.outer_graph_zone.w = (1.0 - (2 * W_PAD)) * render_area.w;

    p.inner_graph_zone.x = p.outer_graph_zone.x + BOARDER_WIDTH;
    p.inner_graph_zone.h = p.outer_graph_zone.h - (2 * BOARDER_WIDTH);
    p.inner_graph_zone.y = p.outer_graph_zone.y + BOARDER_WIDTH;
    p.inner_graph_zone.w = p.outer_graph_zone.w - (2 * BOARDER_WIDTH);

    // Title Zone
    p.title_zone.x = render_area.x;
    p.title_zone.h = render_area.h * 0.1;
    p.title_zone.y = render_area.y;
    p.title_zone.w = render_area.w;

    // YLabel Zone
    p.ylabel_zone.x = render_area.x;
    p.ylabel_zone.w = W_PAD * render_area.w;
    p.ylabel_zone.y = p.inner_graph_zone.y;
    p.ylabel_zone.h = p.inner_graph_zone.h;

    // XLabel Zone
    p.xlabel_zone.x = p.inner_graph_zone.x;
    p.xlabel_zone.w = p.inner_graph_zone.w;
    p.xlabel_zone.y = p.inner_graph_zone.y + p.inner_graph_zone.h + BOARDER_WIDTH;
    p.xlabel_zone.h = p.title_zone.h;

    return p;
}

void Figure::render(SDL_Renderer *renderer, std::optional<std::reference_wrapper<SDL_Rect>> render_area_)
{
    SDL_Rect render_area = render_area_.has_value() ? render_area_.value() : [renderer]()
    {
        SDL_Rect r;
        if (!SDL_GetRenderSafeArea(renderer, &r))
        {
            std::string err = "SDL_Error: ";
            err += SDL_GetError();
            throw std::runtime_error(err);
        }
        return r;
    }();

    // Determine transform values
    auto positioning = determine_layout(render_area);

    auto transform = determine_graph_transform(this->determine_graph_area(), positioning.inner_graph_zone);

    // Render blank graph
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &positioning.outer_graph_zone);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &positioning.inner_graph_zone);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
    drawHTextInZone(renderer, positioning.title_zone, this->title.value_or(""));

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);
    drawHTextInZone(renderer, positioning.xlabel_zone, this->x_label.value_or(""));

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
    drawVTextInZone(renderer, positioning.ylabel_zone, this->y_label.value_or(""));

    // Render blank graph background grid

    // Render graph objects
    // TODO: Fix draw order so circles plotted at the edge of the graph
    // Don't exscape onto the general canvas
    auto visitor = [renderer, transform, positioning](const auto &i)
    {
        i.render(renderer, transform, positioning.inner_graph_zone);
    };

    for (const auto &i : this->draw_data)
    {
        std::visit(visitor, i);
    }
}

Figure::Rect Figure::determine_graph_area()
{
    Figure::Rect r = [this]()
    {
        if (this->graph_area.has_value())
        {
            return this->graph_area.value();
        }

        if (this->draw_data.empty())
        {
            return Rect(-10, -10, 10, 10);
        }

        auto get_draw_area_visitor = [](const auto &d) -> Figure::Rect
        {
            return d.get_render_area();
        };

        Rect r = std::visit(get_draw_area_visitor, this->draw_data[0]);

        for (const auto &i : draw_data)
        {
            auto area = std::visit(get_draw_area_visitor, i);
            r = r.get_bounding_volume(area);
        }
        return r;
    }();

    if (r.x0 == r.xf)
    {
        r.x0 -= 10;
        r.xf += 10;
    }

    if (r.y0 == r.yf)
    {
        r.y0 -= 10;
        r.yf += 10;
    }

    return r;
}

Figure::DrawableFunction::DrawableFunction(std::span<double> x, std::span<double> y)
{
    if (std::size(x) != std::size(y))
    {
        throw std::runtime_error("Sizes of x and y must match");
    }

    this->num_values = std::size(x);
    this->x_values = new double[this->num_values];
    this->y_values = new double[this->num_values];
    std::memcpy(this->x_values, x.data(), std::size(x) * sizeof(double));
    std::memcpy(this->y_values, y.data(), std::size(y) * sizeof(double));
}

Figure::DrawableFunction::~DrawableFunction()
{
    delete[] x_values;
    delete[] y_values;
#ifndef NDEBUG
    // Future jacob will thank me for the crashes
    this->x_values = nullptr;
    this->y_values = nullptr;
    this->num_values = 0;
#endif
}

void Figure::DrawableFunction::render(SDL_Renderer *renderer, auto transform, const SDL_FRect &graph_area) const
{
    if (this->num_values == 0)
    {
        return;
    }

    if (this->num_values == 1)
    {
        Figure::DrawablePoint pt(this->x_values[0], this->y_values[0]);
        pt.render(renderer, transform, graph_area);
        return;
    }

    SDL_FPoint *points = new SDL_FPoint[this->num_values];

    for (size_t i = 0; i < this->num_values; ++i)
    {
        points[i] = transform(this->x_values[i], this->y_values[i]);
    }

    SDL_RenderLines(renderer, points, static_cast<int>(this->num_values));

    delete[] points;
}

Figure::Rect Figure::DrawableFunction::get_render_area() const
{
    auto xs = std::minmax_element(this->x_values, &this->x_values[this->num_values]);
    auto ys = std::minmax_element(this->y_values, &this->y_values[this->num_values]);
    return Rect(*xs.first, *ys.first, *xs.second, *ys.second);
}

void Figure::Rect::normalize()
{
    auto min_x = std::min(this->x0, this->xf);
    auto min_y = std::min(this->y0, this->yf);
    auto max_x = std::max(this->x0, this->xf);
    auto max_y = std::max(this->y0, this->yf);

    this->x0 = min_x;
    this->xf = max_x;
    this->y0 = min_y;
    this->yf = max_y;
}

Figure::Rect Figure::Rect::get_bounding_volume(const Rect &other)
{
    Rect a = *this;
    a.normalize();
    Rect b = other;
    b.normalize();

    auto min_x0 = std::min(a.x0, b.x0);
    auto min_y0 = std::min(a.y0, b.y0);
    auto max_xf = std::max(a.xf, b.xf);
    auto max_yf = std::max(a.yf, b.yf);
    return Rect(min_x0, min_y0, max_xf, max_yf);
}

double Figure::Rect::area() const
{
    return std::abs((xf - x0) * (yf - y0));
}

void Figure::DrawablePoint::render(SDL_Renderer *renderer, auto transform, const SDL_FRect &graph_area) const
{
    auto ss_point = transform(this->x, this->y);
    auto radius = std::min(graph_area.h, graph_area.w) * 0.05;
    DrawFilledCircle(renderer, ss_point.x, ss_point.y, radius);
}
