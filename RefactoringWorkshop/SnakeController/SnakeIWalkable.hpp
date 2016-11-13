#pragma once

#include "SnakePosition.hpp"

namespace Snake
{

class IWalkable
{
public:
    virtual ~IWalkable() = default;

    virtual bool canWalk(Position position) const = 0;
    virtual bool canEat(Position position) const = 0;
};

} // namespace Snake
