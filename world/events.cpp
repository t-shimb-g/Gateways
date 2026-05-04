#include "events.h"
#include "world.h"

#include <iostream>

void NextLevel::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return; // only player can trigger next_level
    world.end_level = true;
}

void SendToBlue::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        if (world.orange_portal_facing == world.blue_portal_facing) {
            Facing facing = world.orange_portal_facing;
            if (facing == Facing::Up || facing == Facing::Down) {
                std::cout << "Entering velocity: " << obj.physics.velocity.y << '\n';
                obj.physics.velocity.y = obj.physics.velocity.y * -1.05;
            }
            else if (facing == Facing::Left || facing == Facing::Right) {
                obj.physics.velocity.x = obj.physics.velocity.x * -1;
                obj.physics.acceleration.x = obj.physics.acceleration.x * -1;
            }
        }
        std::cout << "Exiting velocity: " << obj.physics.velocity.y << '\n';
        obj.physics.position = world.blue_portal_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.2;
    }
}

void SendToOrange::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        if (world.orange_portal_facing == world.blue_portal_facing) {
            Facing facing = world.orange_portal_facing;
            if (facing == Facing::Up || facing == Facing::Down) {
                std::cout << "Entering velocity: " << obj.physics.velocity.y << '\n';
                obj.physics.velocity.y = obj.physics.velocity.y * -1.05; // always gives a slight boost
            }
            else if (facing == Facing::Left || facing == Facing::Right) {
                obj.physics.velocity.x = obj.physics.velocity.x * -1;
                obj.physics.acceleration.x = obj.physics.acceleration.x * -1;
            }
        }
        std::cout << "Exiting velocity: " << obj.physics.velocity.y << '\n';
        obj.physics.position = world.orange_portal_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.2;
    }
}