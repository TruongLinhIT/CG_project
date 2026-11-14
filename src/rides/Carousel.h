#ifndef CAROUSEL_H
#define CAROUSEL_H

/**
 * @file Carousel.h
 * @brief Chapter 4 & 5: Simulation of the Festival Carousel (Đu Quay Ngựa Gỗ Asia Park)
 *
 * Implements:
 *  - Real Festival Carousel specifications:
 *      * Radius R = 8.5m, Height H = 8.8m
 *      * Location: (45.0, 0.0, 48.0) on the Southeast Festive Plaza
 *      * 16 galloping carousel horses arranged in 2 concentric rings (Outer R=6.2m, Inner R=4.2m)
 *  - Chapter 4 Hierarchical Modeling (2-Level Transformation Hierarchy):
 *      * Level 1 (Parent): Rotating wooden circular deck, central mirrored core,
 *        and conical festive canopy roof rotating around world Y by angle gamma = carouselAngleDeg:
 *        M_parent = T(pos) * R_y(gamma)
 *      * Level 2 (Child): 16 brass support poles and horses inheriting parent rotation gamma,
 *        plus independent vertical harmonic oscillation along each pole:
 *        y_i(t) = y_base + A * sin(omega * t + phi_i)
 *        with staggered phase phi_i = i * (2*pi / N) creating an authentic galloping wave!
 *  - Chapter 5 Parametric Surfaces:
 *      * Grand conical canopy roof: ParametricShapes::drawConeY
 *      * Brass vertical poles: ParametricShapes::drawCylinderY
 *      * Golden rim collar: ParametricShapes::drawToroidY
 *      * Crown decorative finial sphere: ParametricShapes::drawSphere
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class Carousel {
public:
    MathCG::Vec3 position       = MathCG::Vec3(45.0f, 0.0f, 48.0f);
    float        platformRadius = 8.5f;
    float        canopyHeight   = 8.8f;
    float        rotationSpeed  = 35.0f; // deg/s
    float        currentAngleDeg= 0.0f;  // gamma: platform rotation angle
    float        oscillationPhase= 0.0f; // omega * t: horse bounce phase
    int          numHorses      = 12;    // 12 galloping horses

    Carousel() = default;

    /**
     * @brief Advance simulation timers for both rotation and horse galloping
     */
    void update(float dt) {
        currentAngleDeg  = std::fmod(currentAngleDeg + rotationSpeed * dt, 360.0f);
        oscillationPhase = std::fmod(oscillationPhase + 3.2f * dt, MathCG::TWO_PI);
    }

    /**
     * @brief Get saddle world position for first-person horse ride camera
     */
    MathCG::Vec3 getHorseWorldPosition(int horseIndex = 0) const {
        if (numHorses <= 0) return position;
        float horseAngleDeg = currentAngleDeg + (360.0f / numHorses) * horseIndex;
        float rad = horseAngleDeg * MathCG::DEG2RAD;
        float ringR = (horseIndex % 2 == 0) ? 6.2f : 4.2f;
        float phaseOffset = (MathCG::TWO_PI / numHorses) * horseIndex;
        float horseY = 1.35f + 0.45f * std::sin(oscillationPhase + phaseOffset);

        return MathCG::Vec3(
            position.x + ringR * std::cos(rad),
            position.y + horseY + 0.85f,
            position.z + ringR * std::sin(rad)
        );
    }

    /**
     * @brief Render the complete Festival Carousel
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ====================================================================
        // 1. Static Circular Concrete Ground Base
        // ====================================================================
        glColor3f(0.48f, 0.50f, 0.54f);
        ParametricShapes::drawCylinderY(platformRadius + 0.8f, 0.35f, 28, 2, true, false);
        glColor3f(0.85f, 0.70f, 0.18f); // Outer gold ring
        ParametricShapes::drawToroidY(platformRadius + 0.82f, 0.10f, 32, 6, false);

        // ====================================================================
        // 2. LEVEL 1 (PARENT): Rotating Deck, Center Column & Conical Roof
        // ====================================================================
        glPushMatrix();
        glRotatef(currentAngleDeg, 0.0f, 1.0f, 0.0f); // Spin by angle gamma

        // Rotating polished wooden deck
        glColor3f(0.68f, 0.42f, 0.22f); // Teak wood
        ParametricShapes::drawCylinderY(platformRadius, 0.55f, 28, 2, true, showNormals);
        glColor3f(0.95f, 0.80f, 0.15f); // Gold platform rim
        ParametricShapes::drawToroidY(platformRadius + 0.05f, 0.12f, 32, 8, false);

        // Ornate central core column with golden moldings
        glColor3f(0.88f, 0.85f, 0.80f); // Cream marble
        ParametricShapes::drawCylinderY(2.1f, canopyHeight - 2.5f, 16, 4, true, showNormals);
        glColor3f(0.92f, 0.75f, 0.16f); // Central gold bands
        for (float y = 1.5f; y < canopyHeight - 2.5f; y += 1.8f) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(2.15f, 0.12f, 20, 6, false);
            glPopMatrix();
        }

        // ====================================================================
        // 3. Conical Pavilion Roof (ParametricShapes::drawConeY)
        // ====================================================================
        glPushMatrix();
        glTranslatef(0.0f, canopyHeight - 2.5f, 0.0f);

        // Eaves valance band
        glColor3f(0.95f, 0.75f, 0.15f);
        ParametricShapes::drawCylinderY(platformRadius + 0.45f, 0.65f, 28, 2, true, false);

        // Main conical roof (Striped Crimson and Ivory Asian canopy)
        glColor3f(0.88f, 0.18f, 0.14f); // Asian red
        ParametricShapes::drawConeY(platformRadius + 0.45f, 0.0f, 3.2f, 28, 4, true, showNormals);

        // Decorative crown pinnacle & golden sphere at peak
        glTranslatef(0.0f, 3.2f, 0.0f);
        glColor3f(0.98f, 0.82f, 0.18f); // Pure gold
        ParametricShapes::drawConeY(0.9f, 0.0f, 1.2f, 16, 2, true, false);
        glTranslatef(0.0f, 1.35f, 0.0f);
        ParametricShapes::drawSphere(0.48f, 16, 12, false);

        glPopMatrix();

        // Perimeter warm fairy lights around canopy eaves at night
        if (isNightOrSunset) {
            GLfloat emit[4] = { 1.0f, 0.90f, 0.45f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(1.0f, 0.92f, 0.50f);
            for (int i = 0; i < 28; ++i) {
                float u = (static_cast<float>(i) / 28) * MathCG::TWO_PI;
                float px = (platformRadius + 0.52f) * std::cos(u);
                float pz = (platformRadius + 0.52f) * std::sin(u);
                glPushMatrix();
                glTranslatef(px, canopyHeight - 2.5f, pz);
                ParametricShapes::drawSphere(0.12f, 8, 6, false);
                glPopMatrix();
            }
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            LightingSystem::resetMaterial();
        }

        // ====================================================================
        // 4. LEVEL 2 (CHILDREN): 12 Galloping Horses + Vertical Brass Poles
        // ====================================================================
        for (int i = 0; i < numHorses; ++i) {
            float angleDeg = (360.0f / numHorses) * i;
            float ringR    = (i % 2 == 0) ? 6.2f : 4.2f; // Alternating 2 rings
            float phaseOffset = (MathCG::TWO_PI / numHorses) * i;

            // Harmonic vertical bounce: y(t) = y_base + A * sin(omega*t + phi_i)
            float horseY = 1.35f + 0.45f * std::sin(oscillationPhase + phaseOffset);

            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(ringR, 0.0f, 0.0f);

            // Level 2.1: Brass mounting pole fixed between floor and ceiling
            glColor3f(0.92f, 0.78f, 0.22f); // Polished brass
            ParametricShapes::drawCylinderY(0.065f, canopyHeight - 2.5f, 10, 4, true, false);

            // Level 2.2: Galloping Carousel Horse sliding vertically along pole
            glTranslatef(0.0f, horseY, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Face tangential forward along carousel rotation

            drawCarouselHorse(i, isNightOrSunset);

            glPopMatrix();
        }

        glPopMatrix(); // End Level 1 parent

        glPopMatrix();
    }

private:
    /**
     * @brief Detailed 3D Carousel Horse with ornate saddle, mane, and galloping posture
     */
    void drawCarouselHorse(int index, bool isNightOrSunset) const {
        (void)isNightOrSunset;

        // Horse coat colors: alternate between Snow White, Chestnut Bay, and Midnight Black
        MathCG::ColorRGB coatCol;
        if (index % 3 == 0)      coatCol = MathCG::ColorRGB(0.95f, 0.95f, 0.97f); // White
        else if (index % 3 == 1) coatCol = MathCG::ColorRGB(0.55f, 0.28f, 0.16f); // Chestnut
        else                     coatCol = MathCG::ColorRGB(0.18f, 0.18f, 0.20f); // Black

        LightingSystem::setMaterial(0.55f, 32.0f);

        // 1. Torso barrel body
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        drawBox(0.0f, 0.0f, 0.0f, 0.55f, 0.45f, 1.35f);

        // 2. Rump / hindquarters
        drawBox(0.0f, 0.06f, -0.65f, 0.50f, 0.42f, 0.45f);

        // 3. Chest & shoulders
        drawBox(0.0f, 0.08f, 0.62f, 0.52f, 0.48f, 0.45f);

        // 4. Arched Neck
        glPushMatrix();
        glTranslatef(0.0f, 0.48f, 0.75f);
        glRotatef(38.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.32f, 0.65f, 0.38f);
        glPopMatrix();

        // 5. Horse Head & Muzzle
        glPushMatrix();
        glTranslatef(0.0f, 0.85f, 0.98f);
        glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.30f, 0.32f, 0.42f); // Head
        // Tapered muzzle
        glTranslatef(0.0f, -0.10f, 0.26f);
        drawBox(0.0f, 0.0f, 0.0f, 0.22f, 0.22f, 0.28f);
        // Ears
        glColor3f(coatCol.r * 0.9f, coatCol.g * 0.9f, coatCol.b * 0.9f);
        drawBox(-0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        drawBox( 0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        glPopMatrix();

        // 6. Flowing Golden Mane
        glColor3f(0.96f, 0.78f, 0.15f); // Gold mane
        glPushMatrix();
        glTranslatef(0.0f, 0.62f, 0.65f);
        glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.18f, -0.12f, 0.12f, 0.55f, 0.18f);
        glPopMatrix();

        // 7. Golden Saddle & Crimson Saddle Blanket
        // Crimson blanket
        glColor3f(0.85f, 0.12f, 0.14f);
        drawBox(0.0f, 0.18f, 0.0f, 0.58f, 0.14f, 0.75f);
        // Golden saddle
        glColor3f(0.96f, 0.80f, 0.18f);
        drawBox(0.0f, 0.26f, 0.02f, 0.48f, 0.12f, 0.52f);
        // Cantle & pommel
        drawBox(0.0f, 0.35f, -0.20f, 0.42f, 0.16f, 0.10f);
        drawBox(0.0f, 0.35f,  0.22f, 0.38f, 0.14f, 0.09f);

        // 8. Galloping Forelegs (extended forward in stride)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.20f, -0.18f, 0.60f);
            glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // Reach forward
            drawBox(0.0f, -0.22f, 0.0f, 0.12f, 0.48f, 0.14f);
            glTranslatef(0.0f, -0.42f, 0.0f);
            glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.10f, 0.42f, 0.11f);
            // Golden hoof
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.12f, 0.10f, 0.14f);
            glPopMatrix();
        }

        // 9. Galloping Hindlegs (trailing back in leap)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.19f, -0.16f, -0.62f);
            glRotatef(-35.0f, 1.0f, 0.0f, 0.0f); // Trail backward
            drawBox(0.0f, -0.22f, 0.0f, 0.14f, 0.50f, 0.16f);
            glTranslatef(0.0f, -0.46f, 0.0f);
            glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.11f, 0.42f, 0.12f);
            // Golden hoof
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.13f, 0.10f, 0.15f);
            glPopMatrix();
        }

        // 10. Flowing Golden Tail
        glColor3f(0.96f, 0.78f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, 0.10f, -0.85f);
        glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, -0.25f, 0.0f, 0.12f, 0.55f, 0.14f);
        glPopMatrix();

        LightingSystem::resetMaterial();
    }

    static void drawBox(float cx, float cy, float cz, float sx, float sy, float sz) {
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

#endif // CAROUSEL_H
