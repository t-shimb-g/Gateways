#include "game_object.h"
#include "fsm.h"
#include "action.h"
#include "input.h"

GameObject::GameObject(const Vec<int>& size, World& world, FSM* fsm, Input* input, Color color)
    : size{size}, fsm{fsm}, input{input}, color{color} {}

GameObject::~GameObject() {
    delete fsm;
    delete input;
}

void GameObject::update(World& world, float dt) {
    fsm->current_state->update(world, *this, dt);
    sprites[sprite_name].update(dt);
    bool flipped = false;
    if (physics.velocity.x > 0) flipped = false;
    else if (physics.velocity.x < 0) flipped = true;
    sprites[sprite_name].flip(flipped);
    set_sprite(sprite_name);
}

std::pair<Vec<float>, Color> GameObject::get_sprite() const {
    return {physics.position, color};
}

void GameObject::set_sprite(const std::string& next_sprite) {
    // if the next sprite is a new sprite, reset current sprite
    if (next_sprite != sprite_name) {
        sprites[sprite_name].reset();

        auto itr = sprites.find(next_sprite);
        if (itr != sprites.end()) {
            sprite_name = next_sprite;
        }
        else {
            sprite_name = "idle"; // assume every GameObject has an idle sprite
        }
    }

    sprite = sprites[sprite_name].get_sprite();
}
