#pragma once
#include <functional>

#include <SDL3/SDL.h>

namespace std
{
    template <>

    struct equal_to<SDL_Rect>
    {
        bool operator()(const SDL_Rect &lhs, const SDL_Rect &rhs) const
        {
            // Define custom equality logic (e.g., users are equal if their IDs match)
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h;
        }
    };
}