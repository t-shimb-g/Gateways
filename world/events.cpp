#include "events.h"

#include "audio.h"
#include "world.h"

#include <iostream>

void NextLevel::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return; // only player can trigger next_level
    world.end_level = true;
}

bool is_vertical(Facing f) {
    return f == Facing::Up || f == Facing::Down;
}

bool same_axis(Facing a, Facing b) {
    return is_vertical(a) == is_vertical(b);
}

int axis_sign(Facing f) {
    switch (f) {
        case Facing::Up:    return  1; // positive Y
        case Facing::Down:  return -1; // negative Y
        case Facing::Left:  return -1; // negative X
        case Facing::Right: return  1; // positive X
    }
    return 0;
}

void SendToBlue::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (!obj.able_to_portal) return;

    Facing entry = world.active_orange_facing;
    Facing exit  = world.active_blue_facing;

    float vx = obj.physics.velocity.x;
    float vy = obj.physics.velocity.y;

    if (!same_axis(entry, exit)) {
        // different axes = swap velocities
        std::swap(vx, vy);
        obj.physics.velocity.x = abs(vx) * axis_sign(exit);
        obj.physics.velocity.y = abs(vy) * axis_sign(exit);
    }
    else {
        // same axis = flip if facing the same direction
        if (entry == exit) {
            if (is_vertical(entry)) {
                obj.physics.velocity.y = -vy;
            } else {
                obj.physics.velocity.x = -vx;
            }
        }
        // opposite directions = keep velocity unchanged
    }
    obj.physics.position = world.active_blue_pos;
    obj.able_to_portal = false;
    obj.able_to_portal_timer = 0.3;
    world.audio->play_sound("portal-use");
}

void SendToOrange::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (!obj.able_to_portal) return;

    Facing entry = world.active_blue_facing;
    Facing exit  = world.active_orange_facing;

    float vx = obj.physics.velocity.x;
    float vy = obj.physics.velocity.y;

    if (!same_axis(entry, exit)) {
        // different axes = swap velocities
        std::swap(vx, vy);
        obj.physics.velocity.x = abs(vx) * axis_sign(exit);
        obj.physics.velocity.y = abs(vy) * axis_sign(exit);
    }
    else {
        // same axis = flip if facing the same direction
        if (entry == exit) {
            if (is_vertical(entry)) {
                obj.physics.velocity.y = -vy;
            } else {
                obj.physics.velocity.x = -vx;
            }
        }
        // opposite directions = keep velocity unchanged
    }
    obj.physics.position = world.active_orange_pos;
    obj.able_to_portal = false;
    obj.able_to_portal_timer = 0.3;
    world.audio->play_sound("portal-use");
}

void ToggleBlue::perform(World& world, GameObject& obj) {
    world.blue_portal_state = !world.blue_portal_state;
    world.active_blue_pos = world.blue_portal_state ? world.blue_portal_pos_a : world.blue_portal_pos_b;
    world.audio->play_sound("portal-swap");
}

void ToggleOrange::perform(World& world, GameObject& obj) {
    world.orange_portal_state = !world.orange_portal_state;
    world.active_orange_pos = world.orange_portal_state ? world.orange_portal_pos_a : world.orange_portal_pos_b;
    world.audio->play_sound("portal-swap");
}

void WinButton::perform(World& world, GameObject& obj) {
    world.win = true;
    world.end_game = true;
}
