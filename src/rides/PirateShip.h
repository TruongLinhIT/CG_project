#ifndef PIRATE_SHIP_H
#define PIRATE_SHIP_H

/**
 * @file PirateShip.h
 * @brief Swinging Pirate Ship Ride (Thuyền Cướp Biển) - Chapter 4 Hierarchical Animation
 *
 * A large pendulum "viking/pirate boat" swinging on a central arm.
 * Structure:
 *  - Two A-frame support towers
 *  - Suspension axle (horizontal cylinder)
 *  - Swinging boat hull (box sections)
 *  - Seated passenger silhouettes (cylinders + spheres)
 * Animation: SHM pendulum with configurable amplitude and period.
 */

#include <GL/freeglut.h>
#include <cmath>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class PirateShip {
public:
    MathCG::Vec3 position = ParkLayout::PIRATE_SHIP_TARGET_POS;
    float swingPhase     = 0.0f;
    float swingAmplitude = 55.0f; // Max degrees
    float swingPeriod    = 6.0f;  // Seconds
    float pendulumAngle  = 0.0f;  // Current angle (computed in update)

    void update(float dt) {
        swingPhase   = std::fmod(swingPhase + (MathCG::TWO_PI / swingPeriod) * dt, MathCG::TWO_PI);
        pendulumAngle = swingAmplitude * std::sin(swingPhase);
    }

    void draw(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        float n = isNightOrSunset ? 0.6f : 1.0f;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ── SUPPORT TOWERS ──────────────────────────────────────────────
        for (int side : {-1, 1}) {
            glPushMatrix();
            glTranslatef(side * 7.5f, 0.0f, 0.0f);
            // Two angled legs
            for (int leg : {-1, 1}) {
                glPushMatrix();
                glTranslatef(leg * 1.2f, 0.0f, 0.0f);
                glRotatef(-leg * 12.0f, 0.0f, 0.0f, 1.0f);
                {
                    auto post = Geometry::PolygonMesh::createBox(0.0f, 5.5f, 0.0f,
                                    0.3f, 11.0f, 0.3f,
                                    MathCG::ColorRGB(0.55f*n, 0.57f*n, 0.60f*n));
                    glColor3f(0.55f*n, 0.57f*n, 0.60f*n);
                    post.draw(false, false);
                }
                glPopMatrix();
            }
            // Cross brace
            {
                auto brace = Geometry::PolygonMesh::createBox(0.0f, 4.5f, 0.0f,
                                 2.4f, 0.2f, 0.2f,
                                 MathCG::ColorRGB(0.55f*n, 0.57f*n, 0.60f*n));
                glColor3f(0.55f*n, 0.57f*n, 0.60f*n);
                brace.draw(false, false);
            }
            glPopMatrix();
        }

        // ── AXLE ──────────────────────────────────────────────────────
        {
            glColor3f(0.55f*n, 0.57f*n, 0.60f*n);
            glPushMatrix();
            glTranslatef(-7.5f, 10.5f, 0.0f);
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            ParametricShapes::drawCylinderY(0.22f, 15.0f, 16, 2, true, false);
            glPopMatrix();
        }

        // ── PENDULUM GROUP (swings around axle at y=10.5) ────────────
        glPushMatrix();
        glTranslatef(0.0f, 10.5f, 0.0f);
        glRotatef(pendulumAngle, 1.0f, 0.0f, 0.0f);

        // Suspension arms
        for (int s : {-1, 1}) {
            glColor3f(0.42f*n, 0.42f*n, 0.45f*n);
            glPushMatrix();
            glTranslatef(s * 5.5f, 0.0f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            ParametricShapes::drawCylinderY(0.15f, 7.5f, 10, 2, true, false);
            glPopMatrix();
        }

        // ── BOAT HULL ───────────────────────────────────────────────────
        glTranslatef(0.0f, -7.5f, 0.0f);

        // Main mid section
        {
            auto mid = Geometry::PolygonMesh::createBox(0.0f, 0.5f, 0.0f,
                           11.0f, 1.2f, 2.0f,
                           MathCG::ColorRGB(0.48f*n, 0.25f*n, 0.08f*n));
            glColor3f(0.48f*n, 0.25f*n, 0.08f*n);
            mid.draw(false, false);
        }
        // Bow taper
        {
            auto bow = Geometry::PolygonMesh::createTaperedPrism(
                5.5f,  0.5f, 2.0f, 1.2f,
                8.0f,  0.5f, 0.3f, 0.9f,
                MathCG::ColorRGB(0.48f*n, 0.25f*n, 0.08f*n));
            glColor3f(0.48f*n, 0.25f*n, 0.08f*n);
            bow.draw(false, false);
        }
        // Stern taper
        {
            auto stern = Geometry::PolygonMesh::createTaperedPrism(
                -8.0f, 0.5f, 0.3f, 0.9f,
                -5.5f, 0.5f, 2.0f, 1.2f,
                MathCG::ColorRGB(0.48f*n, 0.25f*n, 0.08f*n));
            glColor3f(0.48f*n, 0.25f*n, 0.08f*n);
            stern.draw(false, false);
        }
        // Gold rail strips
        for (int s : {-1, 1}) {
            auto rail = Geometry::PolygonMesh::createBox(0.0f, 1.1f, s * 0.98f,
                            11.0f, 0.12f, 0.1f,
                            MathCG::ColorRGB(0.80f*n, 0.68f*n, 0.18f*n));
            glColor3f(0.80f*n, 0.68f*n, 0.18f*n);
            rail.draw(false, false);
        }
        // Figurehead
        {
            auto fig = Geometry::PolygonMesh::createPyramid(7.5f, 0.5f, 0.0f,
                            0.6f, 0.6f, 1.2f,
                            MathCG::ColorRGB(0.80f*n, 0.68f*n, 0.18f*n));
            glColor3f(0.80f*n, 0.68f*n, 0.18f*n);
            fig.draw(false, false);
        }
        // Mast
        {
            auto mast = Geometry::PolygonMesh::createBox(0.0f, 1.2f, 0.0f,
                            0.18f, 2.5f, 0.18f,
                            MathCG::ColorRGB(0.42f*n, 0.42f*n, 0.45f*n));
            glColor3f(0.42f*n, 0.42f*n, 0.45f*n);
            mast.draw(false, false);
        }

        // Seated passengers (cylinder torso + sphere head)
        {
            static const float passengerX[] = { -4.5f, -2.5f, -0.5f, 1.5f, 3.5f };
            for (float px : passengerX) {
                glPushMatrix();
                glTranslatef(px, 1.1f, 0.0f);
                glColor3f(0.3f*n, 0.3f*n, 0.35f*n);
                ParametricShapes::drawCylinderY(0.18f, 0.65f, 8, 1, true, false);
                glTranslatef(0.0f, 0.75f, 0.0f);
                ParametricShapes::drawSphere(0.17f, 8, 6, false);
                glPopMatrix();
            }
        }

        glPopMatrix(); // pendulum group
        glPopMatrix(); // position
    }
};

} // namespace AsiaParkRides

#endif // PIRATE_SHIP_H
