#include "events.h"

#include "audio.h"
#include "world.h"

#include <iostream>

void NextLevel::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return; // only player can trigger next_level
    world.end_level = true;
}

void SendToBlue::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        if (world.active_blue_facing == world.active_orange_facing) {
            Facing facing = world.active_blue_facing;
            if (facing == Facing::Up || facing == Facing::Down) {
                obj.physics.velocity.y = obj.physics.velocity.y * -1.05;
            }
            else if (facing == Facing::Left || facing == Facing::Right) {
                obj.physics.velocity.x = obj.physics.velocity.x * -1;
                obj.physics.acceleration.x = obj.physics.acceleration.x * -1;
            }
        }
        obj.physics.position = world.active_blue_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.2;
        world.audio->play_sound("portal-use");
    }
}

void SendToOrange::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        if (world.active_blue_facing == world.active_orange_facing) {
            Facing facing = world.active_blue_facing;
            if (facing == Facing::Up || facing == Facing::Down) {
                obj.physics.velocity.y = obj.physics.velocity.y * -1.05; // always gives a slight boost because otherwise player loses net velocity
            }
            else if (facing == Facing::Left || facing == Facing::Right) {
                obj.physics.velocity.x = obj.physics.velocity.x * -1;
                obj.physics.acceleration.x = obj.physics.acceleration.x * -1;
            }
        }
        obj.physics.position = world.active_orange_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.2;
        world.audio->play_sound("portal-use");
    }
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