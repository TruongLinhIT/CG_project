#ifndef PLAYGROUND_H
#define PLAYGROUND_H

/**
 * @file Playground.h
 * @brief Animated children's playground: Swing Set + Slide + Seesaw + Sandbox
 *        (Chapter 4 Hierarchical Animation)
 *
 * Uses PolygonMesh::createBox, createPyramid, createPrismY and
 * ParametricShapes::drawCylinderY for all geometry. No external assets.
 */

#include <GL/freeglut.h>
#include <cmath>
#include "core/MathCG.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class Playground {
public:
    MathCG::Vec3 position{40.0f, 0.0f, -50.0f};

    float swingPhase   = 0.0f;
    float seesawPhase  = 0.0f;
    float swingAngle   = 0.0f;  // degrees
    float seesawAngle  = 0.0f;  // degrees

    void update(float dt) {
        swingPhase   = std::fmod(swingPhase  + 1.2f * dt, MathCG::TWO_PI);
        seesawPhase  = std::fmod(seesawPhase + 0.8f * dt, MathCG::TWO_PI);
        swingAngle   = 35.0f * std::sin(swingPhase);
        seesawAngle  = 15.0f * std::sin(seesawPhase);
    }

    void draw(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        float n = isNightOrSunset ? 0.6f : 1.0f;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        _drawSwingSet(n);

        glPushMatrix();
        glTranslatef(10.0f, 0.0f, 0.0f);
        _drawSlide(n);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-10.0f, 0.0f, 2.0f);
        _drawSeesaw(n);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 8.0f);
        _drawSandbox(n);
        glPopMatrix();

        glPopMatrix();
    }

private:
    // Helper: draw a coloured box mesh
    void _box(float cx, float cy, float cz,
               float sx, float sy, float sz,
               float r, float g, float b) const {
        auto m = Geometry::PolygonMesh::createBox(cx, cy, cz, sx, sy, sz,
                     MathCG::ColorRGB(r, g, b));
        glColor3f(r, g, b);
        m.draw(false, false);
    }

    void _drawSwingSet(float n) const {
        // Frame steel colour
        float fr = 0.5f*n, fg = 0.5f*n, fb = 0.52f*n;

        // Top horizontal bar
        _box(0.0f, 5.0f, 0.0f, 7.0f, 0.14f, 0.14f, fr, fg, fb);

        // Two A-frame uprights (left side, right side)
        for (int s : {-1, 1}) {
            float ox = s * 3.5f;
            // Leg A
            glPushMatrix();
            glTranslatef(ox - 0.6f*s, 0.0f, 0.0f);
            glRotatef(10.0f * s, 0.0f, 0.0f, 1.0f);
            _box(0.0f, 2.5f, 0.0f, 0.12f, 5.0f, 0.12f, fr, fg, fb);
            glPopMatrix();
            // Leg B
            glPushMatrix();
            glTranslatef(ox + 0.6f*s, 0.0f, 0.0f);
            glRotatef(-10.0f * s, 0.0f, 0.0f, 1.0f);
            _box(0.0f, 2.5f, 0.0f, 0.12f, 5.0f, 0.12f, fr, fg, fb);
            glPopMatrix();
        }

        // Two hanging seats (swing from pivot on top bar)
        for (int s : {-1, 1}) {
            glPushMatrix();
            glTranslatef(s * 1.5f, 5.0f, 0.0f);
            glRotatef(swingAngle * s, 1.0f, 0.0f, 0.0f);
            // Rope
            _box(0.0f, -1.8f, 0.0f, 0.04f, 3.5f, 0.04f, 0.85f*n, 0.80f*n, 0.60f*n);
            // Seat plank
            _box(0.0f, -3.65f, 0.0f, 0.60f, 0.10f, 0.30f, 0.75f*n, 0.35f*n, 0.15f*n);
            glPopMatrix();
        }
    }

    void _drawSlide(float n) const {
        // Central column
        {
            auto col = Geometry::PolygonMesh::createPrismY(0.0f, 0.0f, 0.0f, 0.4f, 4.0f, 12,
                           MathCG::ColorRGB(0.55f*n, 0.55f*n, 0.55f*n));
            glColor3f(0.55f*n, 0.55f*n, 0.55f*n);
            col.draw(false, false);
        }
        // Platform deck
        _box(0.0f, 4.1f, 0.0f, 1.4f, 0.15f, 1.4f, 0.5f*n, 0.8f*n, 0.5f*n);
        // Slide chute (tilted)
        glPushMatrix();
        glTranslatef(0.0f, 4.0f, 0.7f);
        glRotatef(-38.0f, 1.0f, 0.0f, 0.0f);
        _box(0.0f, -1.5f, 0.0f, 0.90f, 0.12f, 3.5f, 0.9f*n, 0.6f*n, 0.1f*n);
        glPopMatrix();
        // Ladder rungs
        for (int r = 0; r < 4; ++r)
            _box(0.0f, 0.8f + r * 0.85f, -0.6f, 0.80f, 0.08f, 0.08f, 0.55f*n, 0.55f*n, 0.55f*n);
    }

    void _drawSeesaw(float n) const {
        // Pivot post
        _box(0.0f, 0.7f, 0.0f, 0.15f, 1.4f, 0.15f, 0.55f*n, 0.55f*n, 0.55f*n);
        // Pivot cap
        {
            auto cap = Geometry::PolygonMesh::createPrismY(0.0f, 1.4f, 0.0f, 0.18f, 0.12f, 8,
                           MathCG::ColorRGB(0.55f*n, 0.55f*n, 0.55f*n));
            glColor3f(0.55f*n, 0.55f*n, 0.55f*n);
            cap.draw(false, false);
        }
        // Beam
        glPushMatrix();
        glTranslatef(0.0f, 1.46f, 0.0f);
        glRotatef(seesawAngle, 0.0f, 0.0f, 1.0f);
        _box(0.0f, 0.0f, 0.0f, 5.0f, 0.12f, 0.28f, 0.6f*n, 0.35f*n, 0.15f*n);
        // Handle bars
        for (int e : {-1, 1})
            _box(e * 2.3f, 0.35f, 0.0f, 0.08f, 0.6f, 0.08f, 0.4f*n, 0.6f*n, 0.9f*n);
        glPopMatrix();
    }

    void _drawSandbox(float n) const {
        // Rim walls
        for (int s = 0; s < 4; ++s) {
            glPushMatrix();
            glRotatef(s * 90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 2.6f);
            _box(0.0f, 0.2f, 0.0f, 5.2f, 0.4f, 0.2f, 0.6f*n, 0.42f*n, 0.20f*n);
            glPopMatrix();
        }
        // Sand floor
        _box(0.0f, 0.05f, 0.0f, 5.0f, 0.1f, 5.0f, 0.90f*n, 0.82f*n, 0.60f*n);
    }
};

} // namespace AsiaParkRides

#endif // PLAYGROUND_H
