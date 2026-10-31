#include "figure.hpp"

#include <cstring>
#include <ranges>
#include <algorithm>
#include <cstdint>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

TTF_Font *Figure::font{nullptr};

namespace
{
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
                    SDL_RenderLine(renderer, x + dx, y + dy, x - dx, y + dy);
                    SDL_RenderLine(renderer, x + dx, y - dy, x - dx, y - dy);
                    break;
                }
            }
        }
    }

    SDL_FRect determine_h_rectangle(const SDL_FRect &zone, int w, int h)
    {
        SDL_FRect out;
        float y_scale = zone.h / h;
        float x_scale = zone.w / w;

        if (x_scale < y_scale)
        {
            out.w = x_scale * w;
            out.h = x_scale * h;
            out.x = zone.x;
            out.y = zone.y + ((zone.h - out.h) / 2);
        }
        else
        {
            out.w = y_scale * w;
            out.h = y_scale * h;
            out.x = zone.x + ((zone.w - out.w) / 2);
            out.y = zone.y;
        }
        return out;
    }

    void drawHTextInZone(SDL_Renderer *renderer, const SDL_FRect &zone, SDL_Surface *surface)
    {
        // Draws horizontal text in zone
        if (surface)
        {
            SDL_Texture *tx = SDL_CreateTextureFromSurface(renderer, surface);

            if (!tx)
            {
                SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
                std::exit(-1);
            }

            auto dst = determine_h_rectangle(zone, tx->w, tx->h);

            SDL_RenderTexture(renderer, tx, NULL, &dst);

            SDL_DestroyTexture(tx);
        }
    }

    SDL_FRect rotate_90(const SDL_FRect &rect)
    {
        SDL_FRect out;
        out.w = rect.h;
        out.h = rect.w;
        out.x = rect.x + ((rect.w - rect.h) / 2.0);
        out.y = rect.y + ((rect.h - rect.w) / 2.0);
        return out;
    }

    void drawVTextInZone(SDL_Renderer *renderer, const SDL_FRect &zone, SDL_Surface *surface)
    {
        // Draws vertical text in zone
        if (surface)
        {
            SDL_Texture *tx = SDL_CreateTextureFromSurface(renderer, surface);

            if (!tx)
            {
                SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
                std::exit(-1);
            }

            auto dst = determine_h_rectangle(zone, tx->h, tx->w);

            // We have to rotate the dst rectangle because the call to RenderTexture Rotated
            // rotates the RECTANGLE and not the texture.
            dst = rotate_90(dst);

            SDL_RenderTextureRotated(renderer, tx, NULL, &dst, 270, NULL, SDL_FLIP_NONE);

            SDL_DestroyTexture(tx);
        }
    }

}

void Figure::set_graph_area(double x0, double xf, double y0, double yf)
{
    this->cache.reset();
    this->graph_area = Rect(x0, y0, xf, yf);
}

void Figure::set_title(std::string title)
{
    this->cache.reset();
    this->title = title;
}

void Figure::set_x_label(std::string label)
{
    this->cache.reset();
    this->x_label = label;
}

void Figure::set_y_label(std::string label)
{
    this->cache.reset();
    this->y_label = label;
}

void Figure::plot(std::span<const double> x, std::span<const double> y)
{
    if (std::size(x) != std::size(y))
    {
        throw std::invalid_argument("Size of x must equal size of y");
    }
    if (std::size(x) == 0)
    {
        return;
    }
    if (std::size(x) == 1)
    {
        this->plot(x[0], y[0]);
        return;
    }

    this->cache.reset();
    this->draw_data.emplace_back(std::in_place_type<Figure::DrawableFunction>, x, y);
}

void Figure::plot(double x, double y)
{
    this->cache.reset();
    this->draw_data.emplace_back(std::in_place_type<Figure::DrawablePoint>, x, y);
}

void Figure::clear()
{
    this->cache.reset();
    this->draw_data.clear();
}

auto determine_graph_transform(const Figure::Rect &r1, const SDL_FRect &r2)
{
    /// Determines the affine transform from graph coordinates to SDL canvas coordinates

    return [=](double x_in, double y_in) -> SDL_FPoint
    {
        double x_percent = (x_in - r1.x0) / (r1.xf - r1.x0);
        double x_out = (r2.w * x_percent) + r2.x;

        double y_percent = (y_in - r1.y0) / (r1.yf - r1.y0);
        double y_out = (r2.h * (1.0 - y_percent)) + r2.y;

        return {static_cast<float>(x_out), static_cast<float>(y_out)};
    };
}

Figure::Layout Figure::determine_layout(const SDL_Rect &render_area)
{
    Figure::Layout p{};

    // Determine graph area

    constexpr double H_PAD = 0.05;
    constexpr double W_PAD = 0.05;
    constexpr int BORDER_WIDTH = 5;

    p.outer_graph_zone.x = render_area.x + W_PAD * render_area.w;
    p.outer_graph_zone.h = (1.0 - (2 * H_PAD)) * render_area.h;
    p.outer_graph_zone.y = render_area.y + H_PAD * render_area.h;
    p.outer_graph_zone.w = (1.0 - (2 * W_PAD)) * render_area.w;

    p.inner_graph_zone.x = p.outer_graph_zone.x + BORDER_WIDTH;
    p.inner_graph_zone.h = p.outer_graph_zone.h - (2 * BORDER_WIDTH);
    p.inner_graph_zone.y = p.outer_graph_zone.y + BORDER_WIDTH;
    p.inner_graph_zone.w = p.outer_graph_zone.w - (2 * BORDER_WIDTH);

    // Title Zone
    p.title_zone.x = render_area.x;
    p.title_zone.h = render_area.h * H_PAD;
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
    p.xlabel_zone.y = p.inner_graph_zone.y + p.inner_graph_zone.h + BORDER_WIDTH;
    p.xlabel_zone.h = p.title_zone.h;

    return p;
}

Figure::RenderCache &Figure::get_render_info(const SDL_Rect &render_area)
{
    if (this->cache.has_value())
    {
        const SDL_Rect &cached_area = this->cache.value().area;
        if (cached_area.h == render_area.h &&
            cached_area.w == render_area.w &&
            cached_area.x == render_area.x &&
            cached_area.y == render_area.y)
        {
            return this->cache.value();
        }
    }
    this->cache.reset();
    this->cache.emplace();
    cache.value().layout = determine_layout(render_area);
    cache.value().area = render_area;

    SDL_Color black{0, 0, 0, SDL_ALPHA_OPAQUE};

    if (this->title.has_value())
    {
        const auto &title = this->title.value();
        auto surface = TTF_RenderText_Solid(Figure::font, title.data(), title.size(), black);
        if (!surface)
        {
            SDL_Log("TTF_RenderText_Solid Error: %s", SDL_GetError());
            exit(-1);
        }
        cache.value().title = surface;
    }
    else
    {
        cache.value().title = nullptr;
    }

    if (this->x_label.has_value())
    {
        const auto &x_label = this->x_label.value();
        auto surface = TTF_RenderText_Solid(Figure::font, x_label.data(), x_label.size(), black);
        if (!surface)
        {
            SDL_Log("TTF_RenderText_Solid Error: %s", SDL_GetError());
            exit(-1);
        }
        cache.value().xlabel = surface;
    }
    else
    {
        cache.value().xlabel = nullptr;
    }

    if (this->y_label.has_value())
    {
        const auto &y_label = this->y_label.value();
        auto surface = TTF_RenderText_Solid(Figure::font, y_label.data(), y_label.size(), black);
        if (!surface)
        {
            SDL_Log("TTF_RenderText_Solid Error: %s", SDL_GetError());
            exit(-1);
        }
        cache.value().ylabel = surface;
    }
    else
    {
        cache.value().ylabel = nullptr;
    }

    return this->cache.value();
}

#include "monalisa_font.h"

void Figure::FigureInit()
{
    if (Figure::font != nullptr)
    {
        TTF_CloseFont(Figure::font);
    }

    SDL_IOStream *io = SDL_IOFromConstMem(MonoLisaRegular, std::size(MonoLisaRegular));

    if (!io)
    {
        SDL_Log("SDL_IOFromConstMem Error: %s", SDL_GetError());
        exit(-1);
    }

    Figure::font = TTF_OpenFontIO(io, true, 72);

    if (!Figure::font)
    {
        SDL_Log("TTF_OpenFontIO Error: %s", SDL_GetError());
        exit(-1);
    }
}

void Figure::FigureInit(TTF_Font *font)
{
    if (font == nullptr)
    {
        throw std::invalid_argument("font cannot be null");
    }

    if (Figure::font != nullptr)
    {
        TTF_CloseFont(Figure::font);
    }

    Figure::font = TTF_CopyFont(font);
}

void Figure::FigureQuit()
{
    TTF_CloseFont(Figure::font);
    Figure::font = nullptr;
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
    auto &render_info = get_render_info(render_area);

    auto transform = determine_graph_transform(this->determine_graph_area(), render_info.layout.inner_graph_zone);

    // Render blank graph
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &render_info.layout.outer_graph_zone);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &render_info.layout.inner_graph_zone);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
    drawHTextInZone(renderer, render_info.layout.title_zone, render_info.title);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);
    drawHTextInZone(renderer, render_info.layout.xlabel_zone, render_info.xlabel);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
    drawVTextInZone(renderer, render_info.layout.ylabel_zone, render_info.ylabel);

    // Render blank graph background grid

    // Render graph objects
    SDL_Rect graph_area_i{
        .x = static_cast<int>(render_info.layout.inner_graph_zone.x),
        .y = static_cast<int>(render_info.layout.inner_graph_zone.y),
        .w = static_cast<int>(render_info.layout.inner_graph_zone.w),
        .h = static_cast<int>(render_info.layout.inner_graph_zone.h),
    };

    SDL_SetRenderClipRect(renderer, &graph_area_i);

    auto visitor = [renderer, transform, &render_info](const auto &i)
    {
        i.render(renderer, transform, render_info.layout.inner_graph_zone);
    };

    for (const auto &i : this->draw_data)
    {
        std::visit(visitor, i);
    }
    SDL_SetRenderClipRect(renderer, nullptr);
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

Figure::DrawableFunction::DrawableFunction(std::span<const double> x, std::span<const double> y)
{
    if (std::size(x) != std::size(y))
    {
        throw std::runtime_error("Sizes of x and y must match");
    }
    if (std::size(x) < 2)
    {
        throw std::invalid_argument("Number of points must be at least two");
    }

    this->num_values = std::size(x);

    std::unique_ptr<double[]> xs{new double[this->num_values]};
    std::unique_ptr<double[]> ys{new double[this->num_values]};

    this->x_values = xs.release();
    this->y_values = ys.release();

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

Figure::RenderCache::~RenderCache()
{
    SDL_DestroySurface(this->title);
    SDL_DestroySurface(this->xlabel);
    SDL_DestroySurface(this->ylabel);
}
