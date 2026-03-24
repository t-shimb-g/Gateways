#pragma once

#include "tilemap.h"
#include "vec.h"

class GameObject;
class Level;
class Audio;

class World {
public:
    World(const Level& level, Audio& audio);

    void add_platform(float x, float y, float width, float height);
    bool collides(const Vec<float>& position);
    GameObject* create_player(const Level& level);
    void update(float dt);
    void move_to(Vec<float>& position, const Vec<int>& size, Vec<float>& velocity);

    void load_level(const Level& level);

    Tilemap tilemap;

private:
    GameObject* player;
    Audio* audio;
};
