#pragma once
#include <string>
#include <map>

class World;
class GameObject;
class Event;

using Events = std::map<std::string, Event*>;

class Event {
public:
    virtual ~Event() = default;
    virtual void perform(World& world, GameObject& obj) = 0;
};

class NextLevel : public Event {
public:
    void perform(World& world, GameObject& obj) override;
};

class SendToBlue : public Event {
public:
    void perform(World& world, GameObject& obj) override;
};

class SendToOrange : public Event {
public:
    void perform(World& world, GameObject& obj) override;
};

class ToggleBlue : public Event {
public:
    void perform(World& world, GameObject& obj) override;
};

class ToggleOrange : public Event {
public:
    void perform(World& world, GameObject& obj) override;
};