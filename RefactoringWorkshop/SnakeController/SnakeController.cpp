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
    : m_displayPort(displayPort),
      m_foodPort(foodPort),
      m_scorePort(scorePort)
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

            m_world = std::make_unique<World>(worldDimension, foodPosition);
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
        m_segmentss = std::make_unique<Segments>(startDirection);

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

void Controller::sendPlaceNewFood(Position position)
{
    m_world->setFoodPosition(position);

    DisplayInd placeNewFood;
    placeNewFood.x = position.x;
    placeNewFood.y = position.y;
    placeNewFood.value = Cell_FOOD;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewFood));
}

void Controller::sendClearOldFood()
{
    auto foodPosition = m_world->getFoodPosition();

    DisplayInd clearOldFood;
    clearOldFood.x = foodPosition.x;
    clearOldFood.y = foodPosition.y;
    clearOldFood.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearOldFood));
}

void Controller::removeTailSegment()
{
    auto tail = m_segmentss->removeTail();

    DisplayInd clearTail;
    clearTail.x = tail.x;
    clearTail.y = tail.y;
    clearTail.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearTail));
}

void Controller::addHeadSegment(Position position)
{
    m_segmentss->addHead(position);

    DisplayInd placeNewHead;
    placeNewHead.x = position.x;
    placeNewHead.y = position.y;
    placeNewHead.value = Cell_SNAKE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewHead));
}

void Controller::removeTailSegmentIfNotScored(Position position)
{
    if (position == m_world->getFoodPosition()) {
        m_scorePort.send(std::make_unique<EventT<ScoreInd>>());
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        removeTailSegment();
    }
}

void Controller::updateSegmentsIfSuccessfullMove(Position position)
{
    if (m_segmentss->isCollision(position) or not m_world->contains(position)) {
        m_scorePort.send(std::make_unique<EventT<LooseInd>>());
    } else {
        addHeadSegment(position);
        removeTailSegmentIfNotScored(position);
    }
}

void Controller::handleTimeoutInd()
{
    auto newHead = m_segmentss->nextHead();
    updateSegmentsIfSuccessfullMove(newHead);
}

void Controller::handleDirectionInd(std::unique_ptr<Event> e)
{
    m_segmentss->updateDirection(payload<DirectionInd>(*e).direction);
}

void Controller::updateFoodPosition(Position position, std::function<void()> clearPolicy)
{
    if (m_segmentss->isCollision(position)) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        return;
    }

    clearPolicy();
    sendPlaceNewFood(position);
}

void Controller::handleFoodInd(std::unique_ptr<Event> e)
{
    auto newFood = payload<FoodInd>(*e);
    auto newFoodPosition = Position{newFood.x, newFood.y};

    updateFoodPosition(newFoodPosition, std::bind(&Controller::sendClearOldFood, this));
}

void Controller::handleFoodResp(std::unique_ptr<Event> e)
{
    static auto noCleanPolicy = []{};
    auto newFood = payload<FoodResp>(*e);
    auto newFoodPosition = Position{newFood.x, newFood.y};

    updateFoodPosition(newFoodPosition, noCleanPolicy);
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
