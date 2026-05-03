#include "events.h"
#include "world.h"

void NextLevel::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return; // only player can trigger next_level
    world.end_level = true;
}

void SendToBlue::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        obj.physics.position = world.blue_portal_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.5;
    }
}

void SendToOrange::perform(World& world, GameObject& obj) {
    if (&obj != world.player) return;
    if (obj.able_to_portal) {
        obj.physics.position = world.orange_portal_pos;
        obj.able_to_portal = false;
        obj.able_to_portal_timer = 0.5;
    }
}