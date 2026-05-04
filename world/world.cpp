#include "world.h"

#include <algorithm>

#include "game_object.h"
#include "vec.h"
#include "physics.h"
#include "fsm.h"
#include "keyboard_input.h"
#include "level.h"
#include "audio.h"

World::World(const Level& level, Audio& audio, GameObject* player, Events events)
    : tilemap{level.width, level.height}, audio{&audio}, player{player}, events{events},
    quadtree{
        AABB{
                {level.width / 2.0f, level.height / 2.0f},
                {level.width / 2.0f, level.height / 2.0f}
        }
    } {
    load_level(level);
}

World::~World() {
    for (auto obj : game_objects) {
        if (obj == player) continue;
        delete obj;
    }
}

void World::add_platform(float x, float y, float width, float height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            tilemap(x+j, y+i) = Tile{};
        }
    }
}

bool World::collides(const Vec<float>& position) const {
    int x = std::floor(position.x);
    int y = std::floor(position.y);
    return tilemap(x, y).blocking;
}

void World::update(double dt) {
    for (auto& obj : game_objects) {
        update_object(obj, dt);

        touch_tiles(*obj);
    }

    // update projectiles
    for (auto& projectile : projectiles) {
        update_object(projectile, dt);
    }

    // check for collisions with the player
    build_quadtree();
    std::vector<GameObject*> collides_with = quadtree.query_range(player->get_bounding_box());
    for (auto& obj : collides_with) {
        if (obj == player) continue;
        player->take_damage(obj->damage);
    }

    // check for collision with projectile and enemy
    for (auto& projectile : projectiles) {
        std::vector<GameObject*> p_collides_with = quadtree.query_range(projectile->get_bounding_box());
        for (auto& obj : p_collides_with) {
            if (obj == player) continue;
            obj->take_damage(projectile->damage);
            projectile->elapsed += projectile->lifetime;
        }
    }

    // std::partition puts objects that return TRUE at the beginning.
    // So we flip the logic: Keep alive objects at the front.
    auto itr = std::stable_partition(game_objects.begin(), game_objects.end(),
        [](GameObject* obj) { return obj->is_alive; }
    );

    // Now [itr, end) contains the original pointers to the dead objects
    std::for_each(itr, game_objects.end(), [](GameObject* p) { delete p; });
    game_objects.erase(itr, game_objects.end());

    // same for projectiles
    auto p_itr = std::stable_partition(projectiles.begin(), projectiles.end(),
        [](Projectile* projectile) { return projectile->elapsed <= projectile->lifetime; }   );

    std::for_each(p_itr, projectiles.end(), [](Projectile* p) { delete p; });
    projectiles.erase(p_itr, projectiles.end());

    // check for player death
    if (!player->is_alive) {
        end_game = true;
        return;
    }
}

void World::update_object(GameObject* obj, double dt) {
    obj->update(*this, dt);
    auto position = obj->physics.position;
    auto velocity = obj->physics.velocity;
    auto acceleration = obj->physics.acceleration;

    velocity += 0.5f * acceleration * static_cast<float>(dt);
    position += velocity * static_cast<float>(dt);
    velocity += 0.5f * acceleration * static_cast<float>(dt);
    velocity.x *= obj->physics.damping;

    velocity.x = std::clamp(velocity.x, -obj->physics.terminal_velocity, obj->physics.terminal_velocity);
    velocity.y = std::clamp(velocity.y, -obj->physics.terminal_velocity, obj->physics.terminal_velocity);

    // check for x collisions
    // Check for collisions with the world - x direction
    Vec<float> future_position{position.x, obj->physics.position.y};
    Vec<float> future_velocity{velocity.x, 0};
    move_to(future_position, obj->size, future_velocity);
    if (future_velocity.x == 0) obj->physics.acceleration.x = 0;

    // y direction attempt after (maybe) moving in x
    future_velocity.y = velocity.y;
    future_position.y = position.y;
    move_to(future_position, obj->size, future_velocity);

    // update obj
    obj->physics.position = future_position;
    obj->physics.velocity = future_velocity;
}

void World::move_to(Vec<float>& position, const Vec<int>& size, Vec<float>& velocity) {
    float epsilon = 0.001f;

    // cache corner collisions
    const bool bottom_left_collides = collides(position);
    const bool bottom_right_collides = collides(Vec{position.x + size.x - epsilon, position.y});
    const bool top_left_collides = collides(Vec{position.x, position.y + size.y - epsilon});
    const bool top_right_collides = collides(Vec{position.x + size.x - epsilon, position.y + size.y - epsilon});

    // test for collisions on the bottom or top first
    if (bottom_left_collides && bottom_right_collides) { // bottom
        position.y = std::ceil(position.y);
        velocity.y = 0;
    }
    else if (top_left_collides && top_right_collides) { // top
        position.y = std::floor(position.y);
        velocity.y = 0;
    }

    // then test for collisions on the left and right sides
    if (bottom_left_collides && top_left_collides) { // left
        position.x = std::ceil(position.x);
        velocity.x = 0;
    }
    else if (bottom_right_collides && top_right_collides) { // right
        position.x = std::floor(position.x);
        velocity.x = 0;
    }

    // cache offsets
    const auto dx_left = std::ceil(position.x) - position.x;
    const auto dx_right = position.x - std::floor(position.x);
    const auto dy_bottom = std::ceil(position.y) - position.y;
    const auto dy_top = position.y - std::floor(position.y);

    // now test each corner
    if (bottom_left_collides) {
        // if it's shorter/easier to move in the y-axis compared to the x-axis
        if (dx_left > dy_bottom) {
            // move in y
            position.y = std::ceil(position.y);
            velocity.y = 0;
        }
        else {
            // move in x
            position.x = std::ceil(position.x);
            velocity.x = 0;
        }
    }
    else if (bottom_right_collides) {
        if (dx_right > dy_bottom) {
            position.y = std::ceil(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::floor(position.x);
            velocity.x = 0;
        }
    }
    else if (top_left_collides) {
        if (dx_left > dy_top) {
            position.y = std::floor(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::ceil(position.x);
            velocity.x = 0;
        }
    }
    else if (top_right_collides) {
        if (dx_right > dy_top) {
            position.y = std::floor(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::floor(position.x);
            velocity.x = 0;
        }
    }
}

Facing get_facing_at(const Level& level, const Vec<float>& pos) {
    return level.tile_facings.at({
        static_cast<int>(pos.x),
        static_cast<int>(pos.y)
    });
}

void World::load_level(const Level& level) {
    audio->load_sounds(level.sounds);
    // get the backgrounds
    backgrounds = level.backgrounds;
    for (const auto& [pos, tile_id] : level.tile_locations) {
        tilemap(pos.x, pos.y) = level.tile_types.at(tile_id);
    }
    get_portal_details(level.tile_facings);
    active_blue_pos = blue_portal_state ? blue_portal_pos_a : blue_portal_pos_b;
    active_orange_pos = orange_portal_state ? orange_portal_pos_b : orange_portal_pos_a;
    active_blue_facing = get_facing_at(level, active_blue_pos);
    active_orange_facing = get_facing_at(level, active_orange_pos);

    // get all enemies
    for (const auto& [pos, enemy_name] : level.enemy_locations) {
        auto enemy = new GameObject{enemy_name, nullptr, nullptr, {255, 255, 0, 255}};
        enemy->physics.position = pos;
        game_objects.push_back(enemy);
    }

    game_objects.push_back(player);
}

void World::get_portal_details(TileFacings tile_facings) {
    bool first_blue = true;
    bool first_orange = true;

    for (int y = 0; y < tilemap.height; ++y) {
        for (int x = 0; x < tilemap.width; ++x) {
            auto tile = tilemap(x, y);
            if (tile.event_name == "send_to_blue") {
                if (first_orange) {
                    orange_portal_pos_a = {static_cast<float>(x), static_cast<float>(y)};
                    orange_portal_facing_a = tile_facings[{x, y}];
                    first_orange = false;
                    orange_portal_pos_b = orange_portal_pos_a; // in case only 1 orange
                }
                else { // second orange portal
                    orange_portal_pos_b = {static_cast<float>(x), static_cast<float>(y)};
                    orange_portal_facing_b = tile_facings[{x, y}];
                }
            }
            else if (tile.event_name == "send_to_orange") {
                if (first_blue) {
                    blue_portal_pos_a = {static_cast<float>(x), static_cast<float>(y)};
                    blue_portal_facing_a = tile_facings[{x, y}];
                    first_blue = false;
                    blue_portal_pos_b = blue_portal_pos_a; // in case only 1 blue
                }
                else { // second blue portal
                    blue_portal_pos_b = {static_cast<float>(x), static_cast<float>(y)};
                    blue_portal_facing_b = tile_facings[{x, y}];
                }
            }
        }
    }
}

void World::touch_tiles(GameObject& obj, bool use) {
    float epsilon = 0.001f;

    const std::vector<Vec<float>> tiles {
                {obj.physics.position.x - epsilon, obj.physics.position.y},
                {obj.physics.position.x, obj.physics.position.y + obj.size.y + epsilon},
                {obj.physics.position.x + obj.size.x + epsilon, obj.physics.position.y},
                {obj.physics.position.x, obj.physics.position.y - epsilon}
    };

    for (const auto& p : tiles) {
        int x = static_cast<int>(std::floor(p.x));
        int y = static_cast<int>(std::floor(p.y));
        Tile& tile = tilemap(x, y);
        if (!tile.event_name.empty()) {
            auto itr = events.find(tile.event_name);
            if (itr == events.end()) {
                throw std::runtime_error("Cannot find event: " + tile.event_name);
            }

            // toggle events
            if (tile.event_name == "toggle_blue" || tile.event_name == "toggle_orange") {
                if (use) {
                    itr->second->perform(*this, obj);
                }
                continue;
            }

            // portal events
            if (tile.event_name == "send_to_blue") {
                // only teleport if THIS tile is the active orange portal
                if (Vec<float>{float(x), float(y)} == active_orange_pos) {
                    itr->second->perform(*this, obj);
                }
                continue;
            }
            if (tile.event_name == "send_to_orange") {
                if (Vec<float>{float(x), float(y)} == active_blue_pos) {
                    itr->second->perform(*this, obj);
                }
                continue;
            }

            itr->second->perform(*this, obj);
        }
    }
}

void World::build_quadtree() {
    quadtree.clear();

    for (auto obj : game_objects) {
        quadtree.insert(obj);
    }
}
