#ifndef SUN_WHEEL_H
#define SUN_WHEEL_H

/**
 * @file SunWheel.h
 * @brief Chapter 1, 4 & 5: Simulation of the 115m Da Nang Sun Wheel (Vòng quay Mặt Trời)
 *
 * Implements:
 *  - Real Da Nang Asia Park specifications:
 *      * Diameter = 100m (Radius R = 50m), Hub Height H_center = 65m (Total Height = 115m)
 *      * Center coordinates: I(0, 65, -38)
 *      * 32 radial LED spokes, 4 concentric rainbow LED rings, twin outer toroidal rims
 *      * 64 passenger gondola cabins (delta = 5.625 deg)
 *  - Chapter 4 Hierarchical Modeling & Coordinate Transformations:
 *      * Main wheel rotation around center I:
 *        M_wheel = T(x0, y0, z0) * R_z(alpha) * T(-x0, -y0, -z0)
 *      * Hierarchical cabin suspension & upright maintenance:
 *        At spoke angle beta_k = k * (360 / N), pivot position is at distance R.
 *        Local counter-rotation: R_z(-(beta_k + alpha)) cancels net angular velocity,
 *        guaranteeing the cabin floor stays strictly horizontal and upright at all positions!
 *      * First-Person Cabin POV Camera World Coordinate query for Camera.h
 *  - Chapter 1 HSV to RGB Color Model:
 *      * Dynamic cascading LED animations across 32 spokes & 5 rings via HSVtoRGB
 *  - Chapter 5 Parametric Surfaces:
 *      * Outer toroidal rims & hub collars via ParametricShapes::drawToroid
 *      * Segmented rainbow LED rings via ParametricShapes::drawSegmentedLedToroid
 *      * Structural tubular trusses via ParametricShapes::drawCylinderBetween
 */

#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include <vector>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class SunWheel {
public:
    MathCG::Vec3 center           = MathCG::Vec3(0.0f, 65.0f, -38.0f);
    float        radius           = 50.0f;  // R = 50m (Diameter 100m)
    int          numSpokes        = 32;     // 32 structural radial spokes
    int          numCabins        = 64;     // 64 passenger gondola cabins (5.625 deg interval)
    float        rotationSpeed    = 12.0f;  // Rotation speed in deg/s
    float        currentAngleDeg  = 0.0f;   // Current wheel rotation angle alpha

    SunWheel() = default;

    /**
     * @brief Advance simulation timer
     */
    void update(float dt) {
        currentAngleDeg = std::fmod(currentAngleDeg + rotationSpeed * dt, 360.0f);
    }

    /**
     * @brief Chapter 4: Compute exact 3D World Position of cabin #index
     *        P_cabin = I + R * (cos(beta_k + alpha), sin(beta_k + alpha), 0) + offset
     *        Used for First-Person POV Ride Camera in Camera.h
     */
    MathCG::Vec3 getCabinWorldPosition(int cabinIndex = 0) const {
        if (numCabins <= 0) return center;
        float cabinAngleDeg = (360.0f / numCabins) * cabinIndex;
        float totalAngleRad = (cabinAngleDeg + currentAngleDeg) * MathCG::DEG2RAD;
        float localX        = radius * std::cos(totalAngleRad);
        float localY        = radius * std::sin(totalAngleRad);
        return MathCG::Vec3(
            center.x + localX,
            center.y + localY - 1.75f,
            center.z + 0.65f
        );
    }

    /**
     * @brief Render the complete Sun Wheel assembly
     * @param ledHueDeg Hue angle H in [0, 360) for HSV to RGB color transitions
     * @param isNightOrSunset Controls nighttime LED glow and neon lighting
     * @param showNormals Debug visualization of surface normals
     */
    void draw(float ledHueDeg, bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();

        // 1. Static Space-Truss A-Frame Tower, Pedestals & "SUN WHEEL" Sign
        drawSupportTower(isNightOrSunset);
        drawNeonSign(isNightOrSunset);

        // 2. Hierarchical Rotating Wheel Assembly: M = T(I) * R_z(alpha) * T(-I)
        MathCG::Mat4 wheelRot = MathCG::Mat4::rotateZAroundPoint(center, currentAngleDeg);
        glMultMatrixf(wheelRot.m);
        glTranslatef(center.x, center.y, center.z);

        drawWheelStructure(ledHueDeg, isNightOrSunset, showNormals);
        drawCabins(currentAngleDeg, isNightOrSunset, showNormals);

        glPopMatrix();
    }

private:
    /**
     * @brief Space-Truss A-frame support tower & concrete lakebed foundation piers
     */
    void drawSupportTower(bool isNightOrSunset) const {
        (void)isNightOrSunset;
        const float baseSpreadX = 27.0f;
        const float baseSpreadZ = 12.5f;
        const float topSpreadZ  = 6.2f;
        const float baseY       = 3.2f;

        auto legPointAtY = [&](int sideZ, int sideX, float y) {
            float frac = (center.y - y) / (center.y - baseY);
            return MathCG::Vec3(
                center.x + sideX * baseSpreadX * frac,
                y,
                center.z + sideZ * (topSpreadZ + (baseSpreadZ - topSpreadZ) * frac)
            );
        };

        // Reinforced concrete foundation piers
        LightingSystem::setMaterial(0.20f, 16.0f);
        glColor3f(0.48f, 0.50f, 0.54f);
        for (int sz = -1; sz <= 1; sz += 2) {
            for (int sx = -1; sx <= 1; sx += 2) {
                MathCG::Vec3 basePt = legPointAtY(sz, sx, baseY);
                drawBox(basePt.x, 1.35f, basePt.z, 5.4f, 3.8f, 5.4f);
                drawBox(basePt.x, 3.30f, basePt.z, 3.8f, 0.5f, 3.8f);
            }
        }

        // Space-truss steel A-frame legs & cross girders (front & back planes)
        LightingSystem::setMaterial(0.65f, 48.0f);
        glColor3f(0.84f, 0.86f, 0.90f);
        for (int sz = -1; sz <= 1; sz += 2) {
            MathCG::Vec3 hubEnd(center.x, center.y, center.z + sz * topSpreadZ);
            MathCG::Vec3 leftBase  = legPointAtY(sz, -1, baseY);
            MathCG::Vec3 rightBase = legPointAtY(sz,  1, baseY);

            // Main inclined legs
            ParametricShapes::drawCylinderBetween(leftBase,  hubEnd, 1.05f, 0.78f, 10, false);
            ParametricShapes::drawCylinderBetween(rightBase, hubEnd, 1.05f, 0.78f, 10, false);

            // Cross-girders & X-bracing bays
            const float tierY[] = { 12.0f, 25.0f, 38.0f, 51.0f };
            for (int t = 0; t < 4; ++t) {
                MathCG::Vec3 pL = legPointAtY(sz, -1, tierY[t]);
                MathCG::Vec3 pR = legPointAtY(sz,  1, tierY[t]);
                ParametricShapes::drawCylinderBetween(pL, pR, 0.42f, 0.42f, 8, false);

                if (t < 3) {
                    MathCG::Vec3 pLNext = legPointAtY(sz, -1, tierY[t + 1]);
                    MathCG::Vec3 pRNext = legPointAtY(sz,  1, tierY[t + 1]);
                    ParametricShapes::drawCylinderBetween(pL, pRNext, 0.28f, 0.28f, 6, false);
                    ParametricShapes::drawCylinderBetween(pR, pLNext, 0.28f, 0.28f, 6, false);
                }
            }
        }

        // Main steel central axle connecting front and back bearings
        ParametricShapes::drawCylinderBetween(
            MathCG::Vec3(center.x, center.y, center.z - topSpreadZ - 1.2f),
            MathCG::Vec3(center.x, center.y, center.z + topSpreadZ + 1.2f),
            1.45f, 1.45f, 14, true
        );
        LightingSystem::resetMaterial();
    }

    /**
     * @brief 3D Neon "SUN WHEEL" sign mounted on the front A-frame girder
     */
    void drawNeonSign(bool isNightOrSunset) const {
        const float signY = 39.2f;
        const float baseY = 3.2f;
        const float topSpreadZ = 6.2f;
        const float baseSpreadZ = 12.5f;
        float frac = (center.y - signY) / (center.y - baseY);
        float signZ = center.z + (topSpreadZ + (baseSpreadZ - topSpreadZ) * frac) + 1.25f;

        const char* title = "SUN WHEEL";
        const float charW = 1.75f;
        const float charH = 2.90f;
        const float stepX = 2.32f;
        const float startX = center.x - 10.155f;

        // Backing structural bar
        glColor3f(0.14f, 0.16f, 0.22f);
        drawBox(center.x, signY + charH * 0.5f, signZ - 0.32f, 22.8f, 0.55f, 0.35f);

        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.9f, 64.0f, 0.88f, 0.96f, 1.0f);
        }
        glColor3f(0.95f, 0.98f, 1.0f);
        for (int idx = 0; title[idx] != '\0'; ++idx) {
            if (title[idx] != ' ') {
                drawNeonChar3D(title[idx], startX + idx * stepX, signY, signZ, charW, charH, 0.34f, 0.38f);
            }
        }
        LightingSystem::resetMaterial();
    }

    /**
     * @brief Rotating wheel structure: Hub, Rims, Concentric LED rings, 32 Spokes
     */
    void drawWheelStructure(float ledHueDeg, bool isNightOrSunset, bool showNormals) const {
        // 1. Central illuminated hub discs
        if (isNightOrSunset) {
            float hr = 1.0f, hg = 1.0f, hb = 1.0f;
            MathCG::HSVtoRGB(ledHueDeg, 0.85f, 1.0f, hr, hg, hb);
            LightingSystem::setMaterial(0.8f, 48.0f, hr * 0.7f, hg * 0.7f, hb * 0.7f);
            glColor3f(hr, hg, hb);
        } else {
            glColor3f(0.88f, 0.90f, 0.94f);
        }
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, side * 2.5f);
            ParametricShapes::drawToroid(4.8f, 0.55f, 24, 8, false);
            glPopMatrix();
        }
        LightingSystem::resetMaterial();

        // 2. Twin outer structural rims (Radius = 49.45m, front & back z = +/- 2.1m)
        float ledR = 1.0f, ledG = 0.5f, ledB = 0.1f;
        MathCG::HSVtoRGB(ledHueDeg, 0.90f, 1.0f, ledR, ledG, ledB);
        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.85f, 64.0f, ledR * 0.55f, ledG * 0.55f, ledB * 0.55f);
        } else {
            LightingSystem::setMaterial(0.75f, 56.0f);
        }
        glColor3f(ledR, ledG, ledB);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, side * 2.1f);
            ParametricShapes::drawToroid(radius - 0.55f, 0.52f, 64, 8, showNormals);
            glPopMatrix();
        }
        LightingSystem::resetMaterial();

        // 3. Concentric Rainbow LED Structural Rings (Chapter 1 & 5)
        const float innerRingRadii[] = { 13.5f, 23.0f, 32.5f, 41.5f };
        for (int rIdx = 0; rIdx < 4; ++rIdx) {
            for (int side = -1; side <= 1; side += 2) {
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, side * 2.0f);
                ParametricShapes::drawSegmentedLedToroid(
                    innerRingRadii[rIdx],
                    0.22f,
                    numSpokes,
                    6,
                    ledHueDeg + static_cast<float>(rIdx) * 58.0f,
                    360.0f / numSpokes,
                    0.92f, 1.0f,
                    isNightOrSunset ? 0.85f : 0.0f
                );
                glPopMatrix();
            }
        }

        // 4. 32 Radial Spokes with cascading HSV LEDs
        const float spokeBands[] = { 4.8f, 13.5f, 23.0f, 32.5f, 41.5f, radius - 0.55f };
        for (int k = 0; k < numSpokes; ++k) {
            float spokeAngleDeg = (360.0f / numSpokes) * k;
            float rad = spokeAngleDeg * MathCG::DEG2RAD;
            float ca  = std::cos(rad);
            float sa  = std::sin(rad);

            for (int band = 0; band < 5; ++band) {
                float r0 = spokeBands[band];
                float r1 = spokeBands[band + 1];
                float hue = std::fmod(
                    ledHueDeg + k * (360.0f / numSpokes) + band * 42.0f,
                    360.0f
                );
                float sr = 1.0f, sg = 1.0f, sb = 1.0f;
                MathCG::HSVtoRGB(hue, 0.90f, 1.0f, sr, sg, sb);
                if (isNightOrSunset) {
                    LightingSystem::setMaterial(0.75f, 40.0f, sr * 0.78f, sg * 0.78f, sb * 0.78f);
                }
                glColor3f(sr, sg, sb);

                for (int side = -1; side <= 1; side += 2) {
                    ParametricShapes::drawCylinderBetween(
                        MathCG::Vec3(r0 * ca, r0 * sa, side * 2.3f),
                        MathCG::Vec3(r1 * ca, r1 * sa, side * 2.1f),
                        0.18f, 0.16f, 6, false
                    );
                }
            }

            // Cross-brace joining front & back rims at each spoke
            LightingSystem::resetMaterial();
            glColor3f(0.82f, 0.85f, 0.90f);
            ParametricShapes::drawCylinderBetween(
                MathCG::Vec3((radius - 0.55f) * ca, (radius - 0.55f) * sa, -2.1f),
                MathCG::Vec3((radius - 0.55f) * ca, (radius - 0.55f) * sa,  2.1f),
                0.18f, 0.18f, 6, false
            );
        }
    }

    /**
     * @brief 64 Gondola Cabins with Chapter 4 Hierarchical Counter-Rotation -(beta_k + alpha)
     */
    void drawCabins(float alpha, bool isNightOrSunset, bool showNormals) const {
        for (int k = 0; k < numCabins; ++k) {
            float cabinAngleDeg = (360.0f / numCabins) * k;

            glPushMatrix();
            // Step 1: Position cabin along spoke at distance R
            glRotatef(cabinAngleDeg, 0.0f, 0.0f, 1.0f);
            glTranslatef(radius, 0.0f, 0.0f);

            // Step 2: MANDATORY HIERARCHICAL COUNTER-ROTATION:
            // Cancel both cabin's spoke angle (beta_k) and wheel rotation (alpha)
            // so every cabin hangs perfectly vertical relative to world gravity!
            glRotatef(-(cabinAngleDeg + alpha), 0.0f, 0.0f, 1.0f);

            drawSingleCabin(isNightOrSunset, showNormals);

            glPopMatrix();
        }
    }

    /**
     * @brief Detailed 3D Gondola Passenger Cabin
     */
    void drawSingleCabin(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;

        // Suspension hanger bar & pivot pin
        glColor3f(0.78f, 0.80f, 0.85f);
        drawBox(0.0f, -0.65f, 0.0f, 0.14f, 1.30f, 0.14f);

        // Aerodynamic roof cap
        glColor3f(0.95f, 0.72f, 0.15f); // Golden trim
        drawBox(0.0f, -1.05f, 0.0f, 1.75f, 0.22f, 1.40f);

        // Main Crimson-red Gondola Cabin Body
        LightingSystem::setMaterial(0.55f, 40.0f);
        glColor3f(0.86f, 0.12f, 0.14f);
        drawBox(0.0f, -1.95f, 0.0f, 1.85f, 1.75f, 1.50f);

        // Panoramic Glass Windows (Warm amber glow at night)
        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.85f, 48.0f, 0.72f, 0.56f, 0.26f);
            glColor3f(1.0f, 0.86f, 0.52f);
        } else {
            glColor3f(0.14f, 0.24f, 0.36f);
        }
        drawBox(0.0f, -1.78f, 0.0f, 1.91f, 0.68f, 1.56f);

        LightingSystem::resetMaterial();
    }

    // ========================================================================
    // PROCEDURAL BOX & NEON CHARACTERS HELPERS
    // ========================================================================

    static void drawBox(float cx, float cy, float cz, float sx, float sy, float sz) {
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

        glBegin(GL_QUADS);
        // +Z Front
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);

        // -Z Back
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);

        // +Y Top
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        // -Y Bottom
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);

        // -X Left
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        // +X Right
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glEnd();
    }

    static void drawNeonStrokeBar(float x0, float y0, float x1, float y1, float z, float thick, float depth) {
        float dx = x1 - x0;
        float dy = y1 - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-4f) return;

        float angleDeg = std::atan2(dy, dx) * MathCG::RAD2DEG;
        glPushMatrix();
        glTranslatef((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, z);
        glRotatef(angleDeg, 0.0f, 0.0f, 1.0f);
        drawBox(0.0f, 0.0f, 0.0f, len + thick * 0.75f, thick, depth);
        glPopMatrix();
    }

    static void drawNeonChar3D(char ch, float x, float y, float z, float w, float h, float thick, float depth) {
        auto seg = [&](float ax, float ay, float bx, float by) {
            drawNeonStrokeBar(x + ax * w, y + ay * h, x + bx * w, y + by * h, z, thick, depth);
        };

        switch (ch) {
            case 'S':
                seg(1.0f, 1.0f, 0.0f, 1.0f);
                seg(0.0f, 1.0f, 0.0f, 0.5f);
                seg(0.0f, 0.5f, 1.0f, 0.5f);
                seg(1.0f, 0.5f, 1.0f, 0.0f);
                seg(1.0f, 0.0f, 0.0f, 0.0f);
                break;
            case 'U':
                seg(0.0f, 1.0f, 0.0f, 0.0f);
                seg(0.0f, 0.0f, 1.0f, 0.0f);
                seg(1.0f, 0.0f, 1.0f, 1.0f);
                break;
            case 'N':
                seg(0.0f, 0.0f, 0.0f, 1.0f);
                seg(0.0f, 1.0f, 1.0f, 0.0f);
                seg(1.0f, 0.0f, 1.0f, 1.0f);
                break;
            case 'W':
                seg(0.0f, 1.0f, 0.22f, 0.0f);
                seg(0.22f, 0.0f, 0.50f, 0.62f);
                seg(0.50f, 0.62f, 0.78f, 0.0f);
                seg(0.78f, 0.0f, 1.0f, 1.0f);
                break;
            case 'H':
                seg(0.0f, 0.0f, 0.0f, 1.0f);
                seg(0.0f, 0.5f, 1.0f, 0.5f);
                seg(1.0f, 0.0f, 1.0f, 1.0f);
                break;
            case 'E':
                seg(0.0f, 0.0f, 0.0f, 1.0f);
                seg(0.0f, 1.0f, 1.0f, 1.0f);
                seg(0.0f, 0.5f, 0.85f, 0.5f);
                seg(0.0f, 0.0f, 1.0f, 0.0f);
                break;
            case 'L':
                seg(0.0f, 0.0f, 0.0f, 1.0f);
                seg(0.0f, 0.0f, 1.0f, 0.0f);
                break;
            default:
                break;
        }
    }
};

} // namespace AsiaParkRides

#endif // SUN_WHEEL_H
