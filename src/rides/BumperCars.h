#ifndef BUMPER_CARS_H
#define BUMPER_CARS_H

/**
 * @file BumperCars.h
 * @brief Mini Bumper-Car Arena (Xe Va Chạm) - Chapter 4 Hierarchical Animation
 *
 * Arena: circular track, rubber-bumper railing wall.
 * Cars: 6 small box cars with dome tops, each moves along its own circular orbit.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class BumperCars {
public:
    MathCG::Vec3 position{-45.0f, 0.0f, 25.0f};
    float arenaRadius = 10.0f;

    struct Car {
        float orbitRadius;
        float orbitSpeed;
        float orbitPhase;
        float facingOff;
        MathCG::ColorRGB bodyColor;
        MathCG::ColorRGB domeColor;
    };

    std::vector<Car> cars;

    BumperCars() {
        using C = MathCG::ColorRGB;
        cars = {
            { 4.5f, 1.4f,  0.00f,  0.0f, C(0.9f,0.1f,0.1f), C(1.0f,0.5f,0.5f) },
            { 6.0f, 0.9f,  1.05f, 25.0f, C(0.1f,0.4f,0.9f), C(0.5f,0.7f,1.0f) },
            { 3.5f, 1.8f,  2.10f,-15.0f, C(0.1f,0.8f,0.2f), C(0.5f,1.0f,0.5f) },
            { 7.0f, 0.7f,  3.14f, 10.0f, C(0.9f,0.6f,0.0f), C(1.0f,0.9f,0.4f) },
            { 5.5f, 1.1f,  4.20f,-20.0f, C(0.7f,0.1f,0.8f), C(0.9f,0.5f,1.0f) },
            { 2.8f, 2.0f,  5.25f,  5.0f, C(0.9f,0.4f,0.1f), C(1.0f,0.8f,0.6f) },
        };
    }

    void update(float dt) {
        for (auto& c : cars)
            c.orbitPhase = std::fmod(c.orbitPhase + c.orbitSpeed * dt, MathCG::TWO_PI);
    }

    void draw(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        float n = isNightOrSunset ? 0.6f : 1.0f;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ── ARENA FLOOR ───────────────────────────────────────────────
        glColor3f(0.35f*n, 0.35f*n, 0.38f*n);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.01f, 0.0f);
        for (int i = 0; i <= 36; ++i) {
            float a = (float)i / 36.0f * MathCG::TWO_PI;
            glVertex3f(arenaRadius * std::cos(a), 0.01f, arenaRadius * std::sin(a));
        }
        glEnd();

        // ── RAILING WALL ──────────────────────────────────────────────
        glColor3f(0.8f*n, 0.15f*n, 0.15f*n);
        ParametricShapes::drawCylinderY(arenaRadius, 0.9f, 48, 2, false, false);

        // ── TOP/BOTTOM TORUS RINGS ────────────────────────────────────
        glColor3f(0.6f*n, 0.6f*n, 0.6f*n);
        ParametricShapes::drawToroidY(arenaRadius, 0.18f, 48, 8, false);
        glPushMatrix();
        glTranslatef(0.0f, 0.9f, 0.0f);
        ParametricShapes::drawToroidY(arenaRadius, 0.18f, 48, 8, false);
        glPopMatrix();

        // ── CARS ──────────────────────────────────────────────────────
        for (const auto& c : cars) {
            float px = c.orbitRadius * std::cos(c.orbitPhase);
            float pz = c.orbitRadius * std::sin(c.orbitPhase);
            float headingDeg = c.orbitPhase * MathCG::RAD2DEG + 90.0f + c.facingOff;

            glPushMatrix();
            glTranslatef(px, 0.0f, pz);
            glRotatef(headingDeg, 0.0f, 1.0f, 0.0f);

            // Body box
            {
                MathCG::ColorRGB bc(c.bodyColor.r*n, c.bodyColor.g*n, c.bodyColor.b*n);
                auto body = Geometry::PolygonMesh::createBox(0.0f, 0.28f, 0.0f, 1.0f, 0.42f, 1.6f, bc);
                glColor3f(bc.r, bc.g, bc.b);
                body.draw(false, false);
            }
            // Dome cab (sphere scaled flat)
            {
                glColor3f(c.domeColor.r*n, c.domeColor.g*n, c.domeColor.b*n);
                glPushMatrix();
                glTranslatef(0.0f, 0.55f, 0.0f);
                glScalef(0.9f, 0.5f, 1.0f);
                ParametricShapes::drawSphere(0.52f, 10, 6, false);
                glPopMatrix();
            }
            // Bumper rubber ring
            {
                glColor3f(0.9f*n, 0.9f*n, 0.9f*n);
                glPushMatrix();
                glTranslatef(0.0f, 0.2f, 0.0f);
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                ParametricShapes::drawToroidY(0.65f, 0.09f, 24, 8, false);
                glPopMatrix();
            }
            // Pole / antenna
            {
                glColor3f(0.8f*n, 0.8f*n, 0.2f*n);
                glPushMatrix();
                glTranslatef(0.0f, 0.65f, -0.6f);
                ParametricShapes::drawCylinderY(0.025f, 0.9f, 6, 1, true, false);
                glPopMatrix();
            }

            glPopMatrix(); // car
        }

        glPopMatrix(); // arena
    }
};

} // namespace AsiaParkRides

#endif // BUMPER_CARS_H
