#pragma once

#include "tilemap.h"
#include "vec.h"
#include "events.h"
#include "quadtree.h"
#include "projectile.h"

using TileFacings = std::map<Vec<int>, Facing>;

class GameObject;
class Level;
class Audio;

class World {
public:
    World(const Level& level, Audio& audio, GameObject* player, Events events);
    ~World();

    void add_platform(float x, float y, float width, float height);
    bool collides(const Vec<float>& position) const;
    void move_to(Vec<float>& position, const Vec<int>& size, Vec<float>& velocity);
    void update(double dt);
    void update_object(GameObject* obj, double dt);

    void load_level(const Level& level);
    void get_portal_details(TileFacings tile_facings);

    void build_quadtree();

    Tilemap tilemap;
    bool end_level{false};
    bool win{false};
    bool end_game{false};

    GameObject* player;
    Audio* audio;
    std::vector<GameObject*> game_objects;
    std::vector<Projectile*> projectiles;

    Events events;

    // portal data
    Vec<float> active_blue_pos, active_orange_pos;
    Facing active_blue_facing, active_orange_facing;

    Vec<float> blue_portal_pos_a, orange_portal_pos_a;
    Facing blue_portal_facing_a, orange_portal_facing_a;

    Vec<float> blue_portal_pos_b, orange_portal_pos_b;
    Facing blue_portal_facing_b, orange_portal_facing_b;

    bool blue_portal_state = true;
    bool orange_portal_state = false;

    void touch_tiles(GameObject& obj, bool use=false);

    QuadTree quadtree;

    std::vector<Background> backgrounds;

    // items
    std::map<std::string, std::function<GameObject*()>> available_items;
};