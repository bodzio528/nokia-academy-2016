#pragma once

#include <list>

#include "SnakeInterface.hpp"
#include "SnakeICollisionDetector.hpp"

class IPort;

namespace Snake
{
class IWalkable;

class Segments : public ICollisionDetector
{
public:
    Segments(IPort& displayPort, IPort& scorePort, Direction direction);

    void nextStep(IWalkable const& world);

    void addSegment(Position position);
    void updateDirection(Direction newDirection);

    bool isCollision(Position position) const override;

private:
    IPort& m_displayPort;
    IPort& m_scorePort;

    void addHead(Position position);
    Position removeTail();
    Position nextHead() const;

    Direction m_headDirection;
    std::list<Position> m_segments;

    void removeTailSegment();
    void addHeadSegment(Position position);
    void removeTailSegmentIfNotScored(Position position, IWalkable const& world);
    void updateSegments(Position position, IWalkable const& world);
};

} // namespace Snake

