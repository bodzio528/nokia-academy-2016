#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "EventT.hpp"
#include "IPort.hpp"

#include "SnakeSegments.hpp"
#include "SnakeWorld.hpp"

namespace Snake
{

ConfigurationError::ConfigurationError()
    : std::logic_error("Bad configuration of Snake::Controller.")
{}

UnexpectedEventException::UnexpectedEventException()
    : std::runtime_error("Unexpected event received!")
{}

Controller::Controller(IPort& displayPort, IPort& foodPort, IPort& scorePort, std::string const& initialConfiguration)
{
    std::istringstream istr(initialConfiguration);
    char w, f, s, d;

    istr >> w;
    if (w == 'W') {
        Dimension worldDimension;
        istr  >> worldDimension.width >> worldDimension.height;

        istr >> f;
        if (f == 'F') {
            Position foodPosition;
            istr >> foodPosition.x >> foodPosition.y;

            m_world = std::make_unique<World>(displayPort, foodPort, worldDimension, foodPosition);
        } else {
            throw ConfigurationError();
        }
    } else {
        throw ConfigurationError();
    }

    istr >> s;
    if (s == 'S') {
        Direction startDirection;
        istr >> d;
        switch (d) {
            case 'U':
                startDirection = Direction_UP;
                break;
            case 'D':
                startDirection = Direction_DOWN;
                break;
            case 'L':
                startDirection = Direction_LEFT;
                break;
            case 'R':
                startDirection = Direction_RIGHT;
                break;
            default:
                throw ConfigurationError();
        }
        m_segmentss = std::make_unique<Segments>(displayPort, scorePort, startDirection);

        int length;
        istr >> length;

        while (length--) {
            Position position;
            istr >> position.x >> position.y;
            m_segmentss->addSegment(position);
        }
    } else {
        throw ConfigurationError();
    }
}

Controller::~Controller()
{}

void Controller::handleTimeoutInd()
{
    m_segmentss->nextStep(*m_world);
}

void Controller::handleDirectionInd(std::unique_ptr<Event> e)
{
    m_segmentss->updateDirection(payload<DirectionInd>(*e).direction);
}

void Controller::handleFoodInd(std::unique_ptr<Event> e)
{
    auto newFood = payload<FoodInd>(*e);
    auto newFoodPosition = Position{newFood.x, newFood.y};

    m_world->updateFoodPosition(newFoodPosition, *m_segmentss);
}

void Controller::handleFoodResp(std::unique_ptr<Event> e)
{
    auto newFood = payload<FoodResp>(*e);
    auto newFoodPosition = Position{newFood.x, newFood.y};

    m_world->placeFood(newFoodPosition, *m_segmentss);
}

void Controller::receive(std::unique_ptr<Event> e)
{
    switch (e->getMessageId()) {
        case TimeoutInd::MESSAGE_ID:
            return handleTimeoutInd();
        case DirectionInd::MESSAGE_ID:
            return handleDirectionInd(std::move(e));
        case FoodInd::MESSAGE_ID:
            return handleFoodInd(std::move(e));
        case FoodResp::MESSAGE_ID:
            return handleFoodResp(std::move(e));
        default:
            throw UnexpectedEventException();
    }
}

} // namespace Snake
