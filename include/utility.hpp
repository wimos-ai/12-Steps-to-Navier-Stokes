#pragma once

#include <ranges>
#include <concepts>
#include <cstddef>
#include <vector>

template <std::floating_point T>
constexpr auto linspace(T start, T end, std::size_t n)
{
    const T step = n > 1
                       ? (end - start) / static_cast<T>(n - 1)
                       : T{};

    auto transform = [=](std::size_t i) -> T
    {
        if (n <= 1)
            return start;

        return i == n - 1
                   ? end
                   : start + static_cast<T>(i) * step;
    };

    return std::views::iota(std::size_t{0}, n) | std::views::transform(transform);
}

template <std::floating_point T>
constexpr std::vector<T> linspacev(T start, T end, std::size_t n)
{
    auto range = linspace(start, end, n);

    std::vector<T> result(n);
    std::ranges::copy(range, result.begin());

    return result;
}

#ifndef FN_FWD
#define FN_FWD(x) [](auto &&...args) -> decltype(auto) { return x(std::forward<decltype(args)>(args)...); }
#endif