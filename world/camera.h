#pragma once

#include "tilemap.h"
#include "vec.h"
#include "toggle.h"
#include "physics.h"

class Graphics;
class Color;

class Camera {
public:
    Camera(Graphics& graphics, float tilesize);

    void handle_input();
    void update(const Vec<float>& new_location, float dt);
    Vec<float> world_to_screen(const Vec<float>& world_position) const;
    void set_location(const Vec<float>& new_location);

    void render(const Vec<float>& position, const Color& color, bool filled=true) const;
    void render(const Tilemap& tilemap) const;

private:
    Graphics& graphics;
    float tilesize;
    Physics physics;
    Toggle grid_toggle;

    void calculate_visible_tiles();
    Vec<int> visible_min, visible_max;
    Vec<float> goal;
};