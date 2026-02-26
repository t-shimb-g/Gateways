#pragma once
#include "fsm.h"

class Standing : public State {
public:
    void on_enter(World&, GameObject&) override;
    Action* input(World&, GameObject&, ActionType) override;
    void update(World&, GameObject&, double dt) override;
};

class InAir : public State {
public:
    void on_enter(World&, GameObject&) override;
    void update(World&, GameObject&, double dt) override;

    const double cooldown = 0.1; // 1 10th of second
    double elapsed = 0;
};

class Running : public State {
public:
    void on_enter(World&, GameObject&) override;
    Action* input(World&, GameObject&, ActionType) override;
    void update(World&, GameObject&, double dt) override;
};

class Crouching : public State {
public:
    void on_enter(World&, GameObject&) override;
    Action* input(World&, GameObject&, ActionType) override;
    // void update(World& world, GameObject& obj, double dt) override;
};

class Crawling : public State {
public:
    void on_enter(World&, GameObject&) override;
    Action* input(World&, GameObject&, ActionType) override;
    // void update(World& world, GameObject& obj, double dt) override;
};
