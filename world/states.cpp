#include "states.h"
#include "action.h"
#include "game_object.h"

// Standing
void Standing::on_enter(World&, GameObject& obj) {
    obj.color = {255, 0, 255, 255};
}

Action* Standing::input(World& world, GameObject& obj, ActionType action_type) {
    if (action_type == ActionType::Jump) {
        obj.fsm->transition(Transition::Jump, world, obj);
        return new Jump();
    }
    return nullptr;
}

// InAir
void InAir::on_enter(World& world, GameObject& obj) {
    elapsed = cooldown;
    obj.color = {0, 0, 255, 255};
}

void InAir::update(World& world, GameObject& obj, double dt) {
    elapsed -= dt;
    if (elapsed <= 0) {
        obj.fsm->transition(Transition::Stop, world, obj);
    }
}
