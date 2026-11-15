#ifndef HUMAN_H
#define HUMAN_H

/**
 * @file Human.h
 * @brief Animated park pedestrian figures (Chapter 4 Hierarchical Modeling)
 *
 * Each Human consists of:
 *  - Box torso  (PolygonMesh::createBox)
 *  - Sphere head (ParametricShapes::drawSphere)
 *  - Four cylinders for arms/legs (ParametricShapes::drawCylinderY)
 * Walking is driven by sinusoidal swing of the leg joints (limbPhase).
 * Each instance patrols a circular path around its spawn centre.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkActors {

struct HumanConfig {
    MathCG::Vec3     spawnCenter;
    float            patrolRadius  = 6.0f;
    float            phaseOffset   = 0.0f;
    float            walkSpeed     = 0.4f;    // rad/s patrol
    MathCG::ColorRGB torsoColor    = MathCG::ColorRGB(0.9f, 0.3f, 0.3f);
    MathCG::ColorRGB pantsColor    = MathCG::ColorRGB(0.3f, 0.3f, 0.7f);
    MathCG::ColorRGB skinColor     = MathCG::ColorRGB(1.0f, 0.85f, 0.72f);
};

class Human {
public:
    MathCG::Vec3 position{};
    float walkPhase = 0.0f;
    float limbPhase = 0.0f;
    HumanConfig cfg_{};  // declared before use in init-list

    Human() = default;
    explicit Human(const HumanConfig& cfg) : cfg_(cfg) {
        walkPhase = cfg_.phaseOffset;
        limbPhase = cfg_.phaseOffset * 2.0f;
        _updatePos();
    }

    void update(float dt) {
        walkPhase = std::fmod(walkPhase + cfg_.walkSpeed * dt, MathCG::TWO_PI);
        limbPhase = std::fmod(limbPhase + cfg_.walkSpeed * 4.0f * dt, MathCG::TWO_PI);
        _updatePos();
    }

    void draw(bool isNightOrSunset) const {
        float nightF     = isNightOrSunset ? 0.6f : 1.0f;
        float limbSwing  = std::sin(limbPhase) * 30.0f; // ±30°
        float facingDeg  = walkPhase * MathCG::RAD2DEG + 90.0f;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(facingDeg, 0.0f, 1.0f, 0.0f);

        // ── TORSO ─────────────────────────────────────────────
        {
            MathCG::ColorRGB c(cfg_.torsoColor.r * nightF, cfg_.torsoColor.g * nightF, cfg_.torsoColor.b * nightF);
            auto torso = Geometry::PolygonMesh::createBox(0.0f, 0.7f, 0.0f, 0.45f, 0.65f, 0.25f, c);
            torso.draw(false, false);
        }

        // ── HEAD ──────────────────────────────────────────────
        {
            MathCG::ColorRGB c = cfg_.skinColor;
            glColor3f(c.r * nightF, c.g * nightF, c.b * nightF);
            glPushMatrix();
            glTranslatef(0.0f, 1.2f, 0.0f);
            ParametricShapes::drawSphere(0.20f, 10, 8, false);
            glPopMatrix();
        }

        // ── LEFT LEG ──────────────────────────────────────────
        {
            MathCG::ColorRGB c = cfg_.pantsColor;
            glColor3f(c.r * nightF, c.g * nightF, c.b * nightF);
            glPushMatrix();
            glTranslatef(-0.12f, 0.4f, 0.0f);
            glRotatef(limbSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.4f, 0.0f);
            ParametricShapes::drawCylinderY(0.08f, 0.70f, 8, 2, true, false);
            glPopMatrix();
        }

        // ── RIGHT LEG ─────────────────────────────────────────
        {
            MathCG::ColorRGB c = cfg_.pantsColor;
            glColor3f(c.r * nightF, c.g * nightF, c.b * nightF);
            glPushMatrix();
            glTranslatef(0.12f, 0.4f, 0.0f);
            glRotatef(-limbSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.4f, 0.0f);
            ParametricShapes::drawCylinderY(0.08f, 0.70f, 8, 2, true, false);
            glPopMatrix();
        }

        // ── LEFT ARM ──────────────────────────────────────────
        {
            MathCG::ColorRGB c = cfg_.torsoColor;
            glColor3f(c.r * nightF, c.g * nightF, c.b * nightF);
            glPushMatrix();
            glTranslatef(-0.27f, 1.0f, 0.0f);
            glRotatef(-limbSwing * 0.6f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.3f, 0.0f);
            ParametricShapes::drawCylinderY(0.06f, 0.50f, 6, 1, true, false);
            glPopMatrix();
        }

        // ── RIGHT ARM ─────────────────────────────────────────
        {
            MathCG::ColorRGB c = cfg_.torsoColor;
            glColor3f(c.r * nightF, c.g * nightF, c.b * nightF);
            glPushMatrix();
            glTranslatef(0.27f, 1.0f, 0.0f);
            glRotatef(limbSwing * 0.6f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.3f, 0.0f);
            ParametricShapes::drawCylinderY(0.06f, 0.50f, 6, 1, true, false);
            glPopMatrix();
        }

        glPopMatrix();
    }

private:

    void _updatePos() {
        position.x = cfg_.spawnCenter.x + cfg_.patrolRadius * std::cos(walkPhase);
        position.y = cfg_.spawnCenter.y;
        position.z = cfg_.spawnCenter.z + cfg_.patrolRadius * std::sin(walkPhase);
    }
};

// ============================================================================
// PEDESTRIAN CROWD FACTORY
// ============================================================================
inline std::vector<Human> createDefaultCrowd() {
    using C = MathCG::ColorRGB;
    std::vector<HumanConfig> configs = {
        // Near Sun Wheel (around z=-15)
        { MathCG::Vec3( 0.0f, 0.0f,-15.0f), 8.0f, 0.00f, 0.40f, C(0.9f,0.2f,0.2f), C(0.2f,0.3f,0.7f), C(1.0f,0.85f,0.72f) },
        { MathCG::Vec3( 0.0f, 0.0f,-15.0f), 8.0f, 2.09f, 0.35f, C(0.2f,0.6f,0.9f), C(0.4f,0.4f,0.4f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3( 0.0f, 0.0f,-15.0f), 8.0f, 4.18f, 0.45f, C(0.3f,0.7f,0.3f), C(0.5f,0.3f,0.2f), C(1.0f,0.85f,0.72f) },
        // Near Carousel (approx 25, 0, 20)
        { MathCG::Vec3(25.0f, 0.0f, 20.0f), 7.0f, 0.80f, 0.50f, C(0.9f,0.6f,0.1f), C(0.2f,0.4f,0.2f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(25.0f, 0.0f, 20.0f), 7.0f, 3.00f, 0.42f, C(0.7f,0.1f,0.6f), C(0.3f,0.3f,0.3f), C(1.0f,0.85f,0.72f) },
        // Near SkyTower
        { MathCG::Vec3(-28.0f,0.0f,-28.0f), 6.0f, 1.57f, 0.38f, C(1.0f,0.5f,0.0f), C(0.2f,0.2f,0.6f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-28.0f,0.0f,-28.0f), 6.0f, 4.70f, 0.55f, C(0.2f,0.8f,0.8f), C(0.5f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
        // Near Pavilion
        { MathCG::Vec3( 0.0f, 0.0f, 22.0f), 9.0f, 0.30f, 0.30f, C(0.9f,0.9f,0.2f), C(0.3f,0.5f,0.3f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3( 0.0f, 0.0f, 22.0f), 9.0f, 3.40f, 0.33f, C(0.6f,0.3f,0.9f), C(0.4f,0.4f,0.4f), C(1.0f,0.85f,0.72f) },
        // Near NgoMon Gate
        { MathCG::Vec3(55.0f, 0.0f,  5.0f),10.0f, 1.00f, 0.28f, C(0.8f,0.2f,0.4f), C(0.2f,0.3f,0.5f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(55.0f, 0.0f,  5.0f),10.0f, 4.00f, 0.32f, C(0.3f,0.4f,0.8f), C(0.6f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
        // Near Playground / BumperCars area
        { MathCG::Vec3(40.0f, 0.0f,-45.0f), 5.0f, 0.60f, 0.60f, C(0.9f,0.5f,0.5f), C(0.3f,0.3f,0.3f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-45.0f,0.0f, 20.0f), 5.0f, 2.80f, 0.55f, C(0.4f,0.9f,0.5f), C(0.3f,0.2f,0.6f), C(1.0f,0.85f,0.72f) },
        // Near Pirate Ship
        { MathCG::Vec3(-20.0f,0.0f,-58.0f), 8.0f, 1.20f, 0.35f, C(0.8f,0.6f,0.1f), C(0.2f,0.3f,0.6f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-20.0f,0.0f,-58.0f), 8.0f, 4.50f, 0.40f, C(0.5f,0.2f,0.8f), C(0.4f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
    };
    std::vector<Human> crowd;
    crowd.reserve(configs.size());
    for (const auto& cfg : configs) crowd.emplace_back(cfg);
    return crowd;
}

} // namespace AsiaParkActors

#endif // HUMAN_H
