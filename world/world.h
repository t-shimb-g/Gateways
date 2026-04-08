#pragma once

#include "tilemap.h"
#include "vec.h"
#include "events.h"

class GameObject;
class Level;
class Audio;

class World {
public:
    World(const Level& level, Audio& audio, GameObject* player, Events events);

    void add_platform(float x, float y, float width, float height);
    bool collides(const Vec<float>& position) const;
    void move_to(Vec<float>& position, const Vec<int>& size, Vec<float>& velocity);
    void update(float dt);

    void load_level(const Level& level);

    Tilemap tilemap;
    bool end_level{false};

    GameObject* player;
    Audio* audio;
    std::vector<GameObject> game_objects;
    Events events;

    void touch_tiles(GameObject& obj);
};