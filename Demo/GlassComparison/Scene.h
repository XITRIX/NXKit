// Shared point geometry for the native UIKit and NXKit material captures.
#pragma once

enum { GlassSceneWidth = 720, GlassSceneHeight = 440 };
typedef struct {
    float x, y, width, height, radius;
    int clear;
} GlassSceneElement;

static const GlassSceneElement GlassSceneElements[] = {
    {80, 210, 120, 60, 30, 1},
    {300, 130, 300, 160, 52, 0},
    {100, 345, 520, 56, 28, 0},
};

static inline GlassSceneElement GlassElementAtSize(unsigned index, float size) {
    GlassSceneElement element = GlassSceneElements[index];
    if (index == 1) {
        element.width = 120 + 180 * size;
        element.height = 64 + 96 * size;
        element.x = 450 - element.width * 0.5f;
        element.y = 210 - element.height * 0.5f;
    }
    return element;
}
