#include "SnakeWorld.hpp"

namespace Snake
{

World::World(Dimension dimension, Position food)
    : m_dimension(dimension),
      m_foodPosition(food)
{}

void World::setFoodPosition(Position position)
{
    m_foodPosition = position;
}

Position World::getFoodPosition() const
{
    return m_foodPosition;
}

bool World::contains(Position position) const
{
    return m_dimension.isInside(position);
}
} // namespace Snake
