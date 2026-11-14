#pragma once
#include <QPainter>

enum class Mood { Wander, Sleep, Eat, Pet, Play, Look };

class SnailRenderer {
public:
    static void draw(QPainter &p, int width, int height, Mood mood,
                     double phase, double vx, bool blinking);
};
