#pragma once

#include "SnakePosition.hpp"

namespace Snake
{

class ICollisionDetector
{
public:
    virtual ~ICollisionDetector() = default;

    virtual bool isCollision(Position position) const = 0;
};

} // namespace Snake
