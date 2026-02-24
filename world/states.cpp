#include "states.h"
#include "action.h"
#include "game_object.h"
#include "world.h"

// Helper function
bool on_platform(World& world, GameObject& obj) {
    constexpr float epsilon = 1e-4;
    Vec<float> left_foot{obj.physics.position.x + epsilon, obj.physics.position.y - epsilon};
    Vec<float> right_foot{obj.physics.position.x + obj.size.x - epsilon, obj.physics.position.y - epsilon};
    return world.collides(left_foot) || world.collides(right_foot);
}

///////////////
// Standing
///////////////
void Standing::on_enter(World&, GameObject& obj) {
    obj.color = {255, 0, 255, 255};
    obj.physics.acceleration.x = 0;
}

Action* Standing::input(World& world, GameObject& obj, ActionType action_type) {
    if (action_type == ActionType::Jump) {
        obj.fsm->transition(Transition::Jump, world, obj);
        return new Jump();
    }
    else if (action_type == ActionType::MoveRight) {
        obj.fsm->transition(Transition::Move, world, obj);
        return new MoveRight();
    }
    else if (action_type == ActionType::MoveLeft) {
        obj.fsm->transition(Transition::Move, world, obj);
        return new MoveLeft();
    }
    else if (action_type == ActionType::Crouch) {
        obj.fsm->transition(Transition::Crouch, world, obj);
        return new Crouch();
    }
    return nullptr;
}

void Standing::update(World& world, GameObject& obj, double dt) {
    if (!on_platform(world, obj)) {
        obj.fsm->transition(Transition::Jump, world, obj);
    }
}

///////////////
// InAir
///////////////
void InAir::on_enter(World& world, GameObject& obj) {
    elapsed = cooldown;
    obj.color = {0, 0, 255, 255};
}

void InAir::update(World& world, GameObject& obj, double dt) {
    elapsed -= dt;
    if (elapsed <= 0 && on_platform(world, obj)) {
        obj.fsm->transition(Transition::Stop, world, obj);
    }
}

///////////////
// Running
///////////////
void Running::on_enter(World&, GameObject& obj) {
    obj.color = {255, 255, 0, 255};
}

Action* Running::input(World& world, GameObject& obj, ActionType action_type) {
    if (action_type == ActionType::None) {
        obj.fsm->transition(Transition::Stop, world, obj);
    }
    else if (action_type == ActionType::Jump) {
        obj.fsm->transition(Transition::Jump, world, obj);
        return new Jump();
    }
    else if (action_type == ActionType::Crouch) {
        obj.fsm->transition(Transition::Crouch, world, obj);
        return new Crouch();
    }
    return nullptr;
}

void Running::update(World& world, GameObject& obj, double dt) {
    if (!on_platform(world, obj)) {
        obj.fsm->transition(Transition::Jump, world, obj);
    }
}

///////////////
// Crouching
///////////////
void Crouching::on_enter(World&, GameObject& obj) {
    obj.color = {127, 0, 255, 255};
    obj.physics.acceleration.x = 0;
}

Action* Crouching::input(World& world, GameObject& obj, ActionType action_type) {
    if (action_type == ActionType::MoveRight) {
        obj.fsm->transition(Transition::Move, world, obj);
        return new MoveRight();
    }
    else if (action_type == ActionType::MoveLeft) {
        obj.fsm->transition(Transition::Move, world, obj);
        return new MoveLeft();
    }
    else if (action_type == ActionType::Crouch) {
        obj.fsm->transition(Transition::Crouch, world, obj);
        return new Crouch();
    }
    return nullptr;
}

void Crouching::update(World& world, GameObject& obj, double dt) {
    if (!on_platform(world, obj)) {
        obj.fsm->transition(Transition::Crouch, world, obj);
        obj.fsm->transition(Transition::Jump, world, obj);
    }
}

///////////////
// Crawling
///////////////
void Crawling::on_enter(World&, GameObject& obj) {
    obj.color = {0, 255, 255, 255};
}

Action* Crawling::input(World& world, GameObject& obj, ActionType action_type) {
    if (action_type == ActionType::None) {
        obj.fsm->transition(Transition::Stop, world, obj);
    }
    else if (action_type == ActionType::Crouch) {
        obj.fsm->transition(Transition::Crouch, world, obj);
        return new Crouch();
    }
    return nullptr;
}

void Crawling::update(World& world, GameObject& obj, double dt) {
    if (!on_platform(world, obj)) {
        obj.fsm->transition(Transition::Stop, world, obj);
        obj.fsm->transition(Transition::Crouch, world, obj);
        obj.fsm->transition(Transition::Jump, world, obj);
    }
}