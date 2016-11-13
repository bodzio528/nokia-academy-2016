#include "SnakeWorld.hpp"

#include "IPort.hpp"
#include "EventT.hpp"

#include "SnakeInterface.hpp"
#include "SnakeICollisionDetector.hpp"

namespace Snake
{

World::World(IPort& displayPort, IPort& foodPort, Dimension dimension, Position food)
    : m_displayPort(displayPort),
      m_foodPort(foodPort),
      m_foodPosition(food),
      m_dimension(dimension)
{}

bool World::canWalk(Position position) const
{
    return m_dimension.isInside(position);
}

bool World::canEat(Position position) const
{
    bool eaten = (m_foodPosition == position);
    if (eaten) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    }
    return eaten;
}

void World::updateFoodPosition(Position position, ICollisionDetector const& segments)
{
    updateFoodPositionWithCleanPolicy(position, segments, std::bind(&World::sendClearOldFood, this));
}

void World::placeFood(Position position, ICollisionDetector const& segments)
{
    static auto noCleanPolicy = []{};
    updateFoodPositionWithCleanPolicy(position, segments, noCleanPolicy);
}

void World::sendPlaceNewFood(Position position)
{
    m_foodPosition = position;

    DisplayInd placeNewFood;
    placeNewFood.x = position.x;
    placeNewFood.y = position.y;
    placeNewFood.value = Cell_FOOD;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewFood));
}

void World::sendClearOldFood()
{
    DisplayInd clearOldFood;
    clearOldFood.x = m_foodPosition.x;
    clearOldFood.y = m_foodPosition.y;
    clearOldFood.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearOldFood));
}

void World::updateFoodPositionWithCleanPolicy(Position position, ICollisionDetector const& segments, std::function<void ()> clearPolicy)
{
    if (segments.isCollision(position)) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        return;
    }

    clearPolicy();
    sendPlaceNewFood(position);
}
} // namespace Snake
