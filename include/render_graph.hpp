#pragma once

struct SDL_Renderer;

#include <span>
#include <utility>
#include <optional>

void render_graph(SDL_Renderer *renderer, std::span<double> x, std::span<double> y, std::optional<std::pair<double, double>> xspan = std::nullopt, std::optional<std::pair<double, double>> yspan = std::nullopt);