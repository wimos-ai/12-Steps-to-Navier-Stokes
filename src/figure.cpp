#include "figure.hpp"

#include <cstring>
#include <ranges>
#include <algorithm>
#include <cstdint>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "SDL_utils.hpp"

TTF_Font *Figure::font{nullptr};

namespace
{
    void DrawFilledCircle(SDL_Renderer *renderer, double x, double y, double radius)
    {
        // TODO: Fix >
        // See: https://stackoverflow.com/questions/65723827/sdl2-function-to-draw-a-filled-circle
        auto radius_sq = radius * radius;
        for (int h = 0; h < std::max(1, static_cast<int>(radius)); h++)
        {
            // w^2 + h^2 = r^2
            // w = sqrt(r^2-h^2);
            double w = std::sqrt(radius_sq - (h * h));

            SDL_RenderLine(renderer, x + w, y + h, x - w, y + h);
            SDL_RenderLine(renderer, x + w, y - h, x - w, y - h);
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
        out.x = rect.x + ((rect.w - rect.h) / 2.0f);
        out.y = rect.y + ((rect.h - rect.w) / 2.0f);
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

void Figure::set_title(std::string title_)
{
    this->cache.reset();
    this->title = std::move(title_);
}

void Figure::set_x_label(std::string label_)
{
    this->cache.reset();
    this->x_label = std::move(label_);
}

void Figure::set_y_label(std::string label_)
{
    this->cache.reset();
    this->y_label = std::move(label_);
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

void Figure::render_layout_dbg(SDL_Renderer *renderer, const Figure::Layout &layout)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &layout.title_zone);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &layout.xlabel_zone);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &layout.ylabel_zone);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &layout.outer_graph_zone);

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &layout.inner_graph_zone);
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

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_FRect render_area_f;
    render_area_f.x = render_area.x;
    render_area_f.y = render_area.y;
    render_area_f.h = render_area.h;
    render_area_f.w = render_area.w;
    SDL_RenderFillRect(renderer, &render_area_f);

    auto cmp = std::equal_to<SDL_Rect>();

    // Determine transform values
    if (!this->cache.has_value() || !cmp(cache.value().layout.render_area, render_area))
    {
        this->cache.emplace(render_area, this->title, this->x_label, this->y_label);
    }

    auto &render_info = this->cache.value();

    // render_layout_dbg(renderer, render_info.layout);
    // return;

    auto graph_area = this->determine_graph_area();

    auto transform = Figure::GraphTransform(graph_area, render_info.layout.inner_graph_zone);

    // Render blank graph
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &render_info.layout.outer_graph_zone);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &render_info.layout.inner_graph_zone);

    drawHTextInZone(renderer, render_info.layout.title_zone, render_info.title.get());

    drawHTextInZone(renderer, render_info.layout.xlabel_zone, render_info.xlabel.get());

    drawVTextInZone(renderer, render_info.layout.ylabel_zone, render_info.ylabel.get());

    // Constrain rendering to graph area
    SDL_Rect graph_area_i{
        .x = static_cast<int>(render_info.layout.inner_graph_zone.x),
        .y = static_cast<int>(render_info.layout.inner_graph_zone.y),
        .w = static_cast<int>(render_info.layout.inner_graph_zone.w),
        .h = static_cast<int>(render_info.layout.inner_graph_zone.h),
    };

    SDL_SetRenderClipRect(renderer, &graph_area_i);

    // Render blank graph background grid
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    Axis x_axis{graph_area.x0, graph_area.xf};
    for (auto tick_SDL_space : x_axis.sdl_x_ticks(transform))
    {
        SDL_FPoint p0, pf;
        p0.x = tick_SDL_space;
        p0.y = render_info.layout.inner_graph_zone.y;

        pf.x = tick_SDL_space;
        pf.y = render_info.layout.inner_graph_zone.y + render_info.layout.inner_graph_zone.h;

        SDL_RenderLine(renderer, p0.x, p0.y, pf.x, pf.y);
    }

    Axis y_axis{graph_area.y0, graph_area.yf};
    for (auto tick_SDL_space : y_axis.sdl_y_ticks(transform))
    {
        SDL_FPoint p0, pf;
        p0.x = render_info.layout.inner_graph_zone.x;
        p0.y = tick_SDL_space;

        pf.x = render_info.layout.inner_graph_zone.x + render_info.layout.inner_graph_zone.w;
        pf.y = tick_SDL_space;

        SDL_RenderLine(renderer, p0.x, p0.y, pf.x, pf.y);
    }

    SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
    // Render graph objects
    auto visitor = [renderer, transform, &render_info](const auto &i)
    {
        i.render(renderer, transform, render_info.layout.inner_graph_zone);
    };

    for (const auto &i : this->draw_data)
    {
        std::visit(visitor, i);
    }
    SDL_SetRenderClipRect(renderer, nullptr);

    // Render tick marks
    constexpr auto SCALING{0.20};

    auto texture_from_tick = [renderer, &transform](double tick)
    {
        char text[100];
        std::snprintf(text, sizeof(text), "%.1e", tick);
        SDL_Surface *txt_rendered = TTF_RenderText_LCD(Figure::font, text, 0, SDL_Color{.r = 0, .g = 0, .b = 0, .a = SDL_ALPHA_OPAQUE}, SDL_Color{.r = 255, .g = 255, .b = 255, .a = SDL_ALPHA_OPAQUE});
        if (!txt_rendered)
        {
            throw std::runtime_error("TTF_RenderText_Solid");
        }
        SDL_Texture *txture = SDL_CreateTextureFromSurface(renderer, txt_rendered);
        if (!txture)
        {
            SDL_DestroySurface(txt_rendered);
            throw std::runtime_error("SDL_CreateTextureFromSurface");
        }
        SDL_DestroySurface(txt_rendered);
        return txture;
    };

    for (auto tick : x_axis.ticks())
    {

        SDL_Texture *txture = texture_from_tick(tick);

        auto tick_SDL_space = transform(tick, 0).x;

        SDL_FRect render_area;
        render_area.w = txture->w * SCALING;
        render_area.h = txture->h * SCALING;
        render_area.x = tick_SDL_space - (render_area.w / 2.0f);
        render_area.y = (render_info.layout.inner_graph_zone.y + render_info.layout.inner_graph_zone.h) - render_area.h;

        SDL_RenderTexture(renderer, txture, NULL, &render_area);

        SDL_DestroyTexture(txture);
    }

    for (auto tick : y_axis.ticks())
    {
        SDL_Texture *txture = texture_from_tick(tick);

        auto tick_SDL_space = transform(0, tick).y;

        SDL_FRect render_area;
        render_area.w = txture->w * SCALING;
        render_area.h = txture->h * SCALING;
        render_area.x = render_info.layout.inner_graph_zone.x + render_info.layout.inner_graph_zone.w;
        render_area.y = tick_SDL_space - (render_area.h / 2.0);

        SDL_RenderTexture(renderer, txture, NULL, &render_area);

        SDL_DestroyTexture(txture);
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

        Rect rv = std::visit(get_draw_area_visitor, this->draw_data[0]);

        for (const auto &i : draw_data)
        {
            auto area = std::visit(get_draw_area_visitor, i);
            rv = rv.get_bounding_volume(area);
        }
        return rv;
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

Figure::DrawableFunction::DrawableFunction(std::span<const double> x, std::span<const double> y) : x_values(std::make_unique<double[]>(x.size())),
                                                                                                   y_values(std::make_unique<double[]>(y.size())),
                                                                                                   num_values(std::max(std::size(x), std::size(y)))
{
    if (std::size(x) != std::size(y))
    {
        throw std::runtime_error("Sizes of x and y must match");
    }
    if (std::size(x) < 2)
    {
        throw std::invalid_argument("Number of points must be at least two");
    }

    std::memcpy(this->x_values.get(), x.data(), std::size(x) * sizeof(double));
    std::memcpy(this->y_values.get(), y.data(), std::size(y) * sizeof(double));
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
    auto xs = std::minmax_element(this->x_values.get(), &this->x_values.get()[this->num_values]);
    auto ys = std::minmax_element(this->y_values.get(), &this->y_values.get()[this->num_values]);
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
    auto radius = std::min(graph_area.h, graph_area.w) * 0.005;
    DrawFilledCircle(renderer, ss_point.x, ss_point.y, radius);
}

Figure::RenderCache::RenderCache(const SDL_Rect &render_area, const std::optional<std::string> &title, const std::optional<std::string> &xlabel, const std::optional<std::string> &ylabel)
{

    this->layout = Layout(render_area);
    this->area = render_area;

    auto surface_from_optional = [](const std::optional<std::string> &str) -> Figure::RenderCache::SDL_RAII_Surface
    {
        constexpr SDL_Color black{0, 0, 0, SDL_ALPHA_OPAQUE};
        if (str.has_value())
        {
            const auto &str_ = str.value();
            SDL_Surface *surface = TTF_RenderText_Solid(Figure::font, str_.data(), str_.size(), black);
            if (!surface)
            {
                SDL_Log("TTF_RenderText_Solid Error: %s", SDL_GetError());
                exit(-1);
            }
            return Figure::RenderCache::SDL_RAII_Surface(surface);
        }
        else
        {
            return nullptr;
        }
    };

    this->title = surface_from_optional(title);
    this->xlabel = surface_from_optional(xlabel);
    this->ylabel = surface_from_optional(ylabel);
}

Figure::Layout::Layout(const SDL_Rect &render_area_) : render_area{render_area_}
{
    constexpr float H_PAD = 0.05f;
    constexpr float W_PAD = 0.05f;
    constexpr int BORDER_WIDTH = 5;

    this->outer_graph_zone.x = render_area_.x + W_PAD * render_area_.w;
    this->outer_graph_zone.h = (1.0f - (2.0f * H_PAD)) * render_area_.h;
    this->outer_graph_zone.y = render_area_.y + H_PAD * render_area_.h;
    this->outer_graph_zone.w = (1.0f - (2.0f * W_PAD)) * render_area_.w;

    this->inner_graph_zone.x = this->outer_graph_zone.x + BORDER_WIDTH;
    this->inner_graph_zone.h = this->outer_graph_zone.h - (2 * BORDER_WIDTH);
    this->inner_graph_zone.y = this->outer_graph_zone.y + BORDER_WIDTH;
    this->inner_graph_zone.w = this->outer_graph_zone.w - (2 * BORDER_WIDTH);

    // Title Zone
    this->title_zone.x = render_area_.x + (W_PAD * render_area_.w + BORDER_WIDTH);
    this->title_zone.h = render_area_.h * H_PAD;
    this->title_zone.y = render_area_.y;
    this->title_zone.w = render_area_.w - ((W_PAD * render_area_.w + BORDER_WIDTH) * 2.0f);

    // YLabel Zone
    this->ylabel_zone.x = render_area_.x;
    this->ylabel_zone.w = W_PAD * render_area_.w;
    this->ylabel_zone.y = this->inner_graph_zone.y;
    this->ylabel_zone.h = this->inner_graph_zone.h;

    // XLabel Zone
    this->xlabel_zone.x = this->inner_graph_zone.x;
    this->xlabel_zone.w = this->inner_graph_zone.w;
    this->xlabel_zone.y = this->inner_graph_zone.y + this->inner_graph_zone.h + BORDER_WIDTH;
    this->xlabel_zone.h = this->title_zone.h;
}

SDL_FPoint Figure::GraphTransform::operator()(double x, double y) const
{
    double x_percent = (x - r1.x0) / (r1.xf - r1.x0);
    double x_out = (r2.w * x_percent) + r2.x;

    double y_percent = (y - r1.y0) / (r1.yf - r1.y0);
    double y_out = (r2.h * (1.0 - y_percent)) + r2.y;

    return {static_cast<float>(x_out), static_cast<float>(y_out)};
}
