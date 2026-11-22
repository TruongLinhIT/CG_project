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
    // Lộ trình đi bộ đã cập nhật theo bố cục Đợt 6: đi dạo trên Quảng trường Trung tâm,
    // sân đón Cổng Ngọ Môn, Sân bông hoa khu Tây-Nam và trục lối đi chính khu Tây
    // (0% đi xuống hồ nước hoặc xuyên qua các công trình).
    std::vector<HumanConfig> configs = {
        // 1. Quảng trường phía Nam Sân khấu Sun Wheel (tâm (0, 0, 4.0), R = 5.5m -> Z thuộc [-1.5, +9.5], cách sân khấu Z=-11.4 là 10m)
        { MathCG::Vec3(   0.0f, 0.0f,   4.0f), 5.5f, 0.00f, 0.40f, C(0.9f,0.2f,0.2f), C(0.2f,0.3f,0.7f), C(1.0f,0.85f,0.72f) },
        { MathCG::Vec3(   0.0f, 0.0f,   4.0f), 5.5f, 2.09f, 0.35f, C(0.2f,0.6f,0.9f), C(0.4f,0.4f,0.4f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(   0.0f, 0.0f,   4.0f), 5.5f, 4.18f, 0.45f, C(0.3f,0.7f,0.3f), C(0.5f,0.3f,0.2f), C(1.0f,0.85f,0.72f) },
        // 2. Quảng trường Trung tâm phía Bắc & Đông Tháp Đồng Hồ (tâm (28.0, 0, 34.0) và (-6.0, 0, 38.0))
        { MathCG::Vec3(  28.0f, 0.0f,  34.0f), 6.5f, 0.80f, 0.50f, C(0.9f,0.6f,0.1f), C(0.2f,0.4f,0.2f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(  28.0f, 0.0f,  34.0f), 6.5f, 3.00f, 0.42f, C(0.7f,0.1f,0.6f), C(0.3f,0.3f,0.3f), C(1.0f,0.85f,0.72f) },
        { MathCG::Vec3(  -6.0f, 0.0f,  38.0f), 6.0f, 1.57f, 0.38f, C(1.0f,0.5f,0.0f), C(0.2f,0.2f,0.6f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(  -6.0f, 0.0f,  38.0f), 6.0f, 4.70f, 0.55f, C(0.2f,0.8f,0.8f), C(0.5f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
        // 3. Quảng trường Trung tâm phía Nam Tháp Đồng Hồ (tâm (16.1, 0, 92.0), R = 7.5m)
        { MathCG::Vec3(  16.1f, 0.0f,  92.0f), 7.5f, 0.30f, 0.30f, C(0.9f,0.9f,0.2f), C(0.3f,0.5f,0.3f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(  16.1f, 0.0f,  92.0f), 7.5f, 3.40f, 0.33f, C(0.6f,0.3f,0.9f), C(0.4f,0.4f,0.4f), C(1.0f,0.85f,0.72f) },
        // 4. Đường dẫn phía Nam Cổng Ngọ Môn giữa 2 Hồ sen (tâm (16.1, 0, 156.0), R = 4.0m)
        { MathCG::Vec3(  16.1f, 0.0f, 156.0f), 4.0f, 1.00f, 0.28f, C(0.8f,0.2f,0.4f), C(0.2f,0.3f,0.5f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(  16.1f, 0.0f, 156.0f), 4.0f, 4.00f, 0.32f, C(0.3f,0.4f,0.8f), C(0.6f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
        // 5. Sân lát Bông hoa 8 cánh khu Tây-Nam (tâm (-108.0, 0, 76.0), R = 6.5m)
        { MathCG::Vec3(-108.0f, 0.0f,  76.0f), 6.5f, 0.60f, 0.45f, C(0.9f,0.5f,0.5f), C(0.3f,0.3f,0.3f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-108.0f, 0.0f,  76.0f), 6.5f, 3.70f, 0.42f, C(0.4f,0.9f,0.5f), C(0.3f,0.2f,0.6f), C(1.0f,0.85f,0.72f) },
        // 6. Ngã ba lối đi khu Tây giữa Port of Sky Treasure & Pirate Ship (tâm (-188.0, 0, 38.0), R = 4.5m)
        { MathCG::Vec3(-188.0f, 0.0f,  38.0f), 4.5f, 1.20f, 0.38f, C(0.8f,0.6f,0.1f), C(0.2f,0.3f,0.6f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-188.0f, 0.0f,  38.0f), 4.5f, 4.50f, 0.40f, C(0.5f,0.2f,0.8f), C(0.4f,0.4f,0.2f), C(1.0f,0.85f,0.72f) },
        // 7. Sân phía Đông Sky Tower & Hành lang khu giữa (tâm (-238.0, 0, -54.6) và (-120.0, 0, -108.0))
        { MathCG::Vec3(-238.0f, 0.0f, -54.6f), 4.2f, 0.50f, 0.42f, C(0.9f,0.3f,0.2f), C(0.2f,0.3f,0.6f), C(1.0f,0.85f,0.72f) },
        { MathCG::Vec3(-120.0f, 0.0f,-108.0f), 5.5f, 2.20f, 0.36f, C(0.2f,0.7f,0.8f), C(0.3f,0.3f,0.4f), C(0.9f,0.76f,0.60f) },
        { MathCG::Vec3(-212.0f, 0.0f,-274.0f), 4.2f, 3.80f, 0.38f, C(0.9f,0.7f,0.2f), C(0.3f,0.4f,0.3f), C(1.0f,0.85f,0.72f) },
    };
    std::vector<Human> crowd;
    crowd.reserve(configs.size());
    for (const auto& cfg : configs) crowd.emplace_back(cfg);
    return crowd;
}

} // namespace AsiaParkActors

#endif // HUMAN_H
