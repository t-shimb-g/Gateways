#pragma once

enum class ActionType {
    None,
    Jump,
    MoveLeft,
    MoveRight,
    Crouch,
};

class World;
class GameObject;

class Action {
public:
    virtual ~Action() = default;
    virtual void perform(World& world, GameObject& obj) = 0;
};

class Jump : public Action {
public:
    void perform(World&, GameObject& obj) override;
};

class MoveRight : public Action {
public:
    void perform(World&, GameObject& obj) override;
};

class MoveLeft : public Action {
public:
    void perform(World&, GameObject& obj) override;
};

class Crouch : public Action {
public:
    void perform(World&, GameObject& obj) override;
};