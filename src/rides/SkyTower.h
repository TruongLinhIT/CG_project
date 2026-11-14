#ifndef SKY_TOWER_H
#define SKY_TOWER_H

/**
 * @file SkyTower.h
 * @brief Chapter 4 & 5: Simulation of the Golden Sky Tower (Tháp Rơi Tự Do 47m Asia Park)
 *
 * Implements:
 *  - Real Golden Sky Tower specifications:
 *      * Height H = 47m, Base coordinate: (76.0, 0.0, -22.0) on the East Adventure Plaza
 *      * Tower column: Parametric cylinder with vertical steel guide tracks & top machinery cap
 *      * Aviation warning strobe beacon at tower peak
 *  - Chapter 4 Hierarchical Modeling & Kinematic Physics Cycle:
 *      * Level 1: Static central structural column & octagonal foundation pedestal
 *      * Level 2: Passenger Gondola Ring (Toroid + 16 outward-facing bucket seats)
 *        undergoing multi-phase cyclic vertical translation Y(t) and azimuth rotation:
 *          1. Slow Winch Ascent   (0.0s -> 13.0s): Clings upward at ~3 m/s with 360-deg panoramic yaw
 *          2. Apex Thrill Hold    (13.0s -> 15.5s): Suspended at 42m height (anticipation pause)
 *          3. Free Fall Plunge    (15.5s -> 18.0s): Gravity drop under downward acceleration
 *          4. Magnetic Braking    (18.0s -> 20.0s): Smooth eddy-current deceleration
 *          5. Boarding Pause      (20.0s -> 22.0s): Rest at platform before next cycle
 *  - Chapter 5 Parametric Surfaces:
 *      * Central tower column: ParametricShapes::drawCylinderY
 *      * Gondola structural collar: ParametricShapes::drawToroidY
 *      * Machinery pinnacle & canopy: ParametricShapes::drawConeY
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"

namespace AsiaParkRides {

class SkyTower {
public:
    MathCG::Vec3 position       = MathCG::Vec3(76.0f, 0.0f, -22.0f);
    float        totalHeight    = 47.0f;
    float        towerRadius    = 1.65f;
    float        gondolaRadius  = 3.35f;
    int          numSeats       = 16;
    float        cycleTime      = 0.0f;  // Internal animation clock
    float        currentY       = 3.5f;  // Gondola height
    float        currentYawDeg  = 0.0f;  // Gondola rotation angle

    SkyTower() = default;

    /**
     * @brief Physics update: Drives the multi-phase freefall drop cycle
     */
    void update(float dt) {
        cycleTime = std::fmod(cycleTime + dt, 22.0f);

        const float minY = 3.5f;
        const float maxY = 42.0f;
        const float liftDuration = 13.0f;
        const float holdDuration = 2.5f;
        const float dropDuration = 2.5f;
        const float brakeDuration = 2.0f;

        float t = cycleTime;

        if (t < liftDuration) {
            // Phase 1: Slow Winch Ascent with panoramic 360-degree rotation
            float u = t / liftDuration;
            // Smooth-step ease-in / ease-out
            float ease = u * u * (3.0f - 2.0f * u);
            currentY = minY + (maxY - minY) * ease;
            currentYawDeg = u * 360.0f * 1.5f;
        }
        else if (t < liftDuration + holdDuration) {
            // Phase 2: Apex Thrill Hold (suspended high in the sky)
            currentY = maxY;
            // Gentle idle sway
            float holdT = t - liftDuration;
            currentYawDeg += holdT * 4.0f * dt;
        }
        else if (t < liftDuration + holdDuration + dropDuration) {
            // Phase 3: Free Fall Plunge under gravity: y(t) = maxY - 0.5 * g_eff * dt^2
            float dropT = t - (liftDuration + holdDuration);
            float u = dropT / dropDuration;
            // Quadratic downward acceleration
            float fallDist = (maxY - (minY + 6.0f)) * (u * u);
            currentY = maxY - fallDist;
        }
        else if (t < liftDuration + holdDuration + dropDuration + brakeDuration) {
            // Phase 4: Magnetic Eddy-Current Braking cushion
            float brakeT = t - (liftDuration + holdDuration + dropDuration);
            float u = brakeT / brakeDuration;
            float ease = 1.0f - (1.0f - u) * (1.0f - u);
            currentY = (minY + 6.0f) - 6.0f * ease;
        }
        else {
            // Phase 5: Boarding Pause at platform
            currentY = minY;
        }
    }

    /**
     * @brief Get passenger seat world coordinates (for ride camera)
     */
    MathCG::Vec3 getPassengerWorldPosition(int seatIndex = 0) const {
        float seatAngleDeg = currentYawDeg + (360.0f / numSeats) * seatIndex;
        float rad = seatAngleDeg * MathCG::DEG2RAD;
        return MathCG::Vec3(
            position.x + gondolaRadius * std::cos(rad),
            position.y + currentY + 0.95f,
            position.z + gondolaRadius * std::sin(rad)
        );
    }

    /**
     * @brief Render the Golden Sky Tower
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ====================================================================
        // 1. Static Foundation Base & Queue Platform
        // ====================================================================
        // Concrete octagonal podium
        glColor3f(0.42f, 0.44f, 0.48f);
        ParametricShapes::drawCylinderY(7.2f, 1.2f, 8, 2, true, showNormals);
        glColor3f(0.85f, 0.70f, 0.18f); // Golden perimeter trim ring
        ParametricShapes::drawToroidY(7.25f, 0.15f, 16, 8, showNormals);

        // Boarding deck stairs & perimeter railing pillars
        glColor3f(0.35f, 0.38f, 0.42f);
        for (int i = 0; i < 8; ++i) {
            float u = (static_cast<float>(i) / 8) * MathCG::TWO_PI;
            float px = 7.0f * std::cos(u);
            float pz = 7.0f * std::sin(u);
            ParametricShapes::drawCylinderY(0.12f, 2.2f, 8, 2, true, false);
            glPushMatrix();
            glTranslatef(px, 0.0f, pz);
            ParametricShapes::drawCylinderY(0.12f, 2.2f, 8, 2, true, false);
            glPopMatrix();
        }

        // ====================================================================
        // 2. Central Structural Tower Column (H = 47m)
        // ====================================================================
        // Main white-and-gold column body (Parametric cylinder)
        LightingSystem::setMaterial(0.60f, 48.0f);
        glColor3f(0.92f, 0.94f, 0.96f);
        ParametricShapes::drawCylinderY(towerRadius, totalHeight, 24, 16, true, showNormals);

        // 4 Vertical Steel Guide Rails running full height
        glColor3f(0.24f, 0.26f, 0.30f);
        for (int i = 0; i < 4; ++i) {
            float u = (static_cast<float>(i) / 4) * MathCG::TWO_PI;
            float rx = std::cos(u);
            float rz = std::sin(u);
            MathCG::Vec3 railBase(rx * (towerRadius + 0.12f), 1.2f, rz * (towerRadius + 0.12f));
            MathCG::Vec3 railTop(rx * (towerRadius + 0.12f), totalHeight, rz * (towerRadius + 0.12f));
            ParametricShapes::drawCylinderBetween(railBase, railTop, 0.14f, 0.14f, 6, false);
        }

        // Decorative golden spiral rings around the tower
        glColor3f(0.95f, 0.76f, 0.12f);
        const float ringY[] = { 10.0f, 20.0f, 30.0f, 40.0f };
        for (float y : ringY) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(towerRadius + 0.08f, 0.10f, 24, 8, false);
            glPopMatrix();
        }

        // ====================================================================
        // 3. Top Crown Machinery Cap & Aviation Warning Strobe
        // ====================================================================
        glPushMatrix();
        glTranslatef(0.0f, totalHeight, 0.0f);

        // Machinery housing collar
        glColor3f(0.88f, 0.72f, 0.16f); // Golden crown
        ParametricShapes::drawCylinderY(towerRadius + 0.45f, 2.4f, 16, 2, true, showNormals);

        // Conical spire top cap
        glTranslatef(0.0f, 2.4f, 0.0f);
        glColor3f(0.95f, 0.32f, 0.14f);
        ParametricShapes::drawConeY(towerRadius + 0.45f, 0.0f, 3.2f, 16, 4, true, showNormals);

        // Aviation Strobe Light Beacon at apex
        glTranslatef(0.0f, 3.35f, 0.0f);
        if (isNightOrSunset) {
            // Pulsing red strobe flash
            float strobe = std::fmod(cycleTime * 3.0f, 1.0f);
            float intensity = (strobe < 0.25f) ? 1.0f : 0.15f;
            GLfloat emit[4] = { 1.0f * intensity, 0.1f * intensity, 0.1f * intensity, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(1.0f * intensity, 0.1f * intensity, 0.1f * intensity);
        } else {
            glColor3f(0.90f, 0.15f, 0.15f);
        }
        ParametricShapes::drawSphere(0.45f, 12, 10, false);
        if (isNightOrSunset) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
        glPopMatrix();

        // ====================================================================
        // 4. Hierarchical Kinematic Passenger Gondola Carriage
        // ====================================================================
        glPushMatrix();
        // Step 1: Vertical translation Y(t)
        glTranslatef(0.0f, currentY, 0.0f);
        // Step 2: Azimuth rotation Yaw(t)
        glRotatef(currentYawDeg, 0.0f, 1.0f, 0.0f);

        drawGondolaRing(isNightOrSunset, showNormals);

        glPopMatrix();

        glPopMatrix();
    }

private:
    /**
     * @brief Gondola ring: Toroid collar, 16 outward passenger seats, sunshade canopy
     */
    void drawGondolaRing(bool isNightOrSunset, bool showNormals) const {
        // 1. Structural inner collar sleeve sliding along the tower
        glColor3f(0.18f, 0.20f, 0.24f);
        ParametricShapes::drawCylinderY(towerRadius + 0.22f, 2.1f, 20, 2, true, false);

        // 2. Main passenger floor ring (Parametric Toroid)
        LightingSystem::setMaterial(0.70f, 48.0f);
        glColor3f(0.96f, 0.78f, 0.12f); // Vibrant Golden Ring
        glPushMatrix();
        glTranslatef(0.0f, 0.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius, 0.32f, 32, 10, showNormals);
        glPopMatrix();

        // 3. Overhead circular sunshade canopy ring
        glColor3f(0.92f, 0.28f, 0.14f); // Asian vermilion canopy
        glPushMatrix();
        glTranslatef(0.0f, 2.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius + 0.15f, 0.22f, 32, 8, false);
        glPopMatrix();

        // 4. 16 Outward-Facing Passenger Seats around the ring
        for (int i = 0; i < numSeats; ++i) {
            float angleDeg = (360.0f / numSeats) * i;
            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, gondolaRadius);

            // Seat support bracket connecting to inner collar
            glColor3f(0.35f, 0.38f, 0.42f);
            ParametricShapes::drawCylinderBetween(
                MathCG::Vec3(0.0f, 0.35f, -gondolaRadius + towerRadius + 0.25f),
                MathCG::Vec3(0.0f, 0.35f, 0.0f),
                0.08f, 0.08f, 6, false
            );

            // Molded bucket seat cushion (Crimson)
            glColor3f(0.85f, 0.14f, 0.12f);
            // Seat base
            drawLocalBox(0.0f, 0.55f, 0.0f, 0.65f, 0.14f, 0.55f);
            // Seat back
            drawLocalBox(0.0f, 1.05f, -0.24f, 0.62f, 0.90f, 0.12f);
            // Headrest
            drawLocalBox(0.0f, 1.62f, -0.22f, 0.42f, 0.28f, 0.14f);

            // Over-the-shoulder safety harness (Yellow)
            glColor3f(0.98f, 0.82f, 0.15f);
            drawLocalBox(-0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);
            drawLocalBox( 0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);

            // Footrest bar dangling below
            glColor3f(0.50f, 0.52f, 0.56f);
            drawLocalBox(0.0f, 0.05f, 0.12f, 0.50f, 0.06f, 0.20f);

            glPopMatrix();
        }

        // Night under-glow LED lights beneath the gondola ring
        if (isNightOrSunset) {
            GLfloat emit[4] = { 0.2f, 0.8f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(0.2f, 0.8f, 1.0f);
            glPushMatrix();
            glTranslatef(0.0f, 0.05f, 0.0f);
            ParametricShapes::drawToroidY(gondolaRadius, 0.08f, 24, 6, false);
            glPopMatrix();
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }

        LightingSystem::resetMaterial();
    }

    static void drawLocalBox(float cx, float cy, float cz, float sx, float sy, float sz) {
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);

        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glEnd();
    }
};

} // namespace AsiaParkRides

#endif // SKY_TOWER_H
