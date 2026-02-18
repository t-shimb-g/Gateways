#pragma once

#include <utility>
#include "vec.h"
#include "graphics.h"
#include "physics.h"

class World;
class FSM;

class GameObject {
public:
    GameObject(const Vec<float>& position, const Vec<int>& size, World& world, FSM* fsm, Color color);
    ~GameObject();

    void input(World& world);
    void update(World& world, float dt);

    std::pair<Vec<float>, Color> get_sprite() const;

    // GameObject data
    Physics physics;
    Vec<int> size;
    FSM* fsm;
    Color color;
};