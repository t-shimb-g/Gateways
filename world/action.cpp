#include "action.h"
#include "game_object.h"
#include "world.h"

void Jump::perform(World&, GameObject& obj) {
    obj.physics.velocity.y = obj.physics.jump_velocity;
}

void MoveRight::perform(World&, GameObject& obj) {
    obj.physics.acceleration.x = obj.physics.walk_acceleration;
}

void MoveLeft::perform(World&, GameObject& obj) {
    obj.physics.acceleration.x = -obj.physics.walk_acceleration;
}

void Crouch::perform(World&, GameObject& obj) {
    // static bool is made and "remembered" between perform() calls
    static bool crouched = true; // initially true since player starts standing
    obj.physics.walk_acceleration = !crouched ? 40.0f : 10.0f;
    obj.physics.terminal_velocity = !crouched ? 40.0f : 15.0f;
    crouched = !crouched;
}