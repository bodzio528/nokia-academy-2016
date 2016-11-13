#pragma once

#include <functional>

#include "SnakeIWalkable.hpp"
#include "SnakeDimension.hpp"

class IPort;

namespace Snake
{
class ICollisionDetector;

class World : public IWalkable
{
public:
    World(IPort& foodPort, IPort& displayPort, Dimension dimension, Position food);

    bool canWalk(Position position) const override;
    bool canEat(Position position) const override;

    void updateFoodPosition(Position position, ICollisionDetector const& segments);
    void placeFood(Position position, ICollisionDetector const& segments);

private:
    IPort& m_displayPort;
    IPort& m_foodPort;

    Position m_foodPosition;
    Dimension m_dimension;

    void sendPlaceNewFood(Position position);
    void sendClearOldFood();
    void updateFoodPositionWithCleanPolicy(Position, ICollisionDetector const&, std::function<void()>);
};

} // namespace Snake
