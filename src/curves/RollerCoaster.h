#ifndef ROLLER_COASTER_H
#define ROLLER_COASTER_H

/**
 * @file RollerCoaster.h
 * @brief Chapters 4 & 5: Queen Cobra Roller Coaster & Asia Park Monorail Simulation
 *
 * Implements:
 *  - Multi-car articulated trains riding along closed C^1 Piecewise Cubic Bezier tracks
 *  - Exact arc-length car spacing so train cars maintain constant physical separation
 *  - Dynamic local 3D orientation (Yaw, Pitch, Roll) computed from Bezier tangent P'(t) & curvature
 *  - Energy-based speed profile for Queen Cobra (lift hill climb -> high-speed drop & airtime hills)
 *  - Custom procedural 3D meshes (wheel bogies, Cobra nose fairing, seats, monorail straddle skirts,
 *    boarding stations) with explicit outward normals (no glutSolid* primitives)
 */

#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "curves/BezierTrack.h"

enum class CoasterStyle {
    QUEEN_COBRA = 0,
    MONORAIL    = 1
};

namespace CoasterMesh {

inline void emitQuad(
    const MathCG::Vec3& v0,
    const MathCG::Vec3& v1,
    const MathCG::Vec3& v2,
    const MathCG::Vec3& v3
) {
    MathCG::Vec3 n = MathCG::computeFaceNormal(v0, v1, v2);
    glNormal3f(n.x, n.y, n.z);
    glVertex3f(v0.x, v0.y, v0.z);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
}

/**
 * @brief Draw an axis-aligned local box centered at (cx, cy, cz) with full size (sx, sy, sz)
 */
inline void drawBox(float cx, float cy, float cz, float sx, float sy, float sz) {
    float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

    MathCG::Vec3 b00(cx - hx, cy - hy, cz - hz);
    MathCG::Vec3 b10(cx + hx, cy - hy, cz - hz);
    MathCG::Vec3 b11(cx + hx, cy + hy, cz - hz);
    MathCG::Vec3 b01(cx - hx, cy + hy, cz - hz);

    MathCG::Vec3 f00(cx - hx, cy - hy, cz + hz);
    MathCG::Vec3 f10(cx + hx, cy - hy, cz + hz);
    MathCG::Vec3 f11(cx + hx, cy + hy, cz + hz);
    MathCG::Vec3 f01(cx - hx, cy + hy, cz + hz);

    glBegin(GL_QUADS);
    emitQuad(f00, f10, f11, f01); // +Z Front
    emitQuad(b10, b00, b01, b11); // -Z Back
    emitQuad(f10, b10, b11, f11); // +X Right
    emitQuad(b00, f00, f01, b01); // -X Left
    emitQuad(f01, f11, b11, b01); // +Y Top
    emitQuad(b00, b10, f10, f00); // -Y Bottom
    glEnd();
}

/**
 * @brief Draw a tapered aerodynamic prism between z0 (rear) and z1 (front, z1 > z0)
 */
inline void drawTaperedPrism(
    float z0, float cy0, float sx0, float sy0,
    float z1, float cy1, float sx1, float sy1
) {
    float rx0 = sx0 * 0.5f, ry0 = sy0 * 0.5f;
    float rx1 = sx1 * 0.5f, ry1 = sy1 * 0.5f;

    MathCG::Vec3 b00(-rx0, cy0 - ry0, z0);
    MathCG::Vec3 b10( rx0, cy0 - ry0, z0);
    MathCG::Vec3 b11( rx0, cy0 + ry0, z0);
    MathCG::Vec3 b01(-rx0, cy0 + ry0, z0);

    MathCG::Vec3 f00(-rx1, cy1 - ry1, z1);
    MathCG::Vec3 f10( rx1, cy1 - ry1, z1);
    MathCG::Vec3 f11( rx1, cy1 + ry1, z1);
    MathCG::Vec3 f01(-rx1, cy1 + ry1, z1);

    glBegin(GL_QUADS);
    emitQuad(f00, f10, f11, f01); // +Z Front
    emitQuad(b10, b00, b01, b11); // -Z Back
    emitQuad(f10, b10, b11, f11); // +X Right
    emitQuad(b00, f00, f01, b01); // -X Left
    emitQuad(f01, f11, b11, b01); // +Y Top
    emitQuad(b00, b10, f10, f00); // -Y Bottom
    glEnd();
}

/**
 * @brief Draw a custom wheel cylinder oriented along the local X-axis with outward caps
 */
inline void drawWheelX(float cx, float cy, float cz, float radius, float width, int slices = 12) {
    float x0 = cx - width * 0.5f;
    float x1 = cx + width * 0.5f;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float u  = (static_cast<float>(i % slices) / static_cast<float>(slices)) * MathCG::TWO_PI;
        float cu = std::cos(u);
        float su = std::sin(u);
        glNormal3f(0.0f, cu, su);
        glVertex3f(x1, cy + radius * cu, cz + radius * su);
        glVertex3f(x0, cy + radius * cu, cz + radius * su);
    }
    glEnd();

    // +X cap (increasing u is CCW when looking from +X)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x1, cy, cz);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i % slices) / static_cast<float>(slices)) * MathCG::TWO_PI;
        glVertex3f(x1, cy + radius * std::cos(u), cz + radius * std::sin(u));
    }
    glEnd();

    // -X cap (decreasing u is CCW when looking from -X)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x0, cy, cz);
    for (int i = slices; i >= 0; --i) {
        float u = (static_cast<float>(i % slices) / static_cast<float>(slices)) * MathCG::TWO_PI;
        glVertex3f(x0, cy + radius * std::cos(u), cz + radius * std::sin(u));
    }
    glEnd();
}

} // namespace CoasterMesh

class CoasterTrain {
public:
    CoasterStyle style              = CoasterStyle::QUEEN_COBRA;
    int          numCars            = 5;
    float        carSpacing         = 2.85f;
    float        paramT             = 0.0f;  // Lead car parameter t in [0, 1)
    float        currentDistMeters  = 0.0f;  // Lead car arc-length distance
    float        cruiseSpeedMps     = 16.0f;
    float        minSpeedMps        = 13.5f;
    float        maxSpeedMps        = 38.0f;
    bool         useGravityPhysics  = true;

    float        trackMaxY          = 47.6f;
    float        lastSpeedMps       = 16.0f;

    void init(const BezierTrack& track, CoasterStyle trainStyle, int cars, float spacing, float startT = 0.0f) {
        style       = trainStyle;
        numCars     = std::max(1, cars);
        carSpacing  = spacing;
        paramT      = BezierTrack::wrap01(startT);
        currentDistMeters = track.paramTToDistance(paramT);

        // Find peak track elevation for gravitational energy speed scaling
        trackMaxY = 10.0f;
        for (const auto& s : track.samples) {
            trackMaxY = std::max(trackMaxY, s.position.y);
        }

        if (style == CoasterStyle::MONORAIL) {
            useGravityPhysics = false;
            cruiseSpeedMps    = 17.5f;
            lastSpeedMps      = cruiseSpeedMps;
        } else {
            useGravityPhysics = true;
            cruiseSpeedMps    = 14.5f;
            minSpeedMps       = 14.5f;
            maxSpeedMps       = 38.0f;
        }
    }

    /**
     * @brief Advance train along BezierTrack by delta time dt (seconds).
     */
    void update(float dt, const BezierTrack& track) {
        float totalLen = track.getTotalLength();
        if (totalLen <= 1e-4f || dt <= 0.0f) return;

        float speed = cruiseSpeedMps;
        if (useGravityPhysics) {
            TrackFrame lead = track.evaluateFrameAtDistance(currentDistMeters);
            float dropH = std::max(0.0f, trackMaxY - lead.position.y);
            // Conservation of energy approximation: v = sqrt(v_min^2 + 2*g_eff*dh)
            speed = std::sqrt(minSpeedMps * minSpeedMps + 2.0f * 11.5f * dropH);
            speed = std::clamp(speed, minSpeedMps, maxSpeedMps);
        }
        lastSpeedMps = speed;

        currentDistMeters = track.wrapDistance(currentDistMeters + speed * dt);
        paramT = track.distanceToParamT(currentDistMeters);
    }

    TrackFrame getLeadFrame(const BezierTrack& track) const {
        return track.evaluateFrameAtDistance(currentDistMeters);
    }

    TrackFrame getCarFrame(int carIndex, const BezierTrack& track) const {
        float carDist = currentDistMeters - static_cast<float>(carIndex) * carSpacing;
        return track.evaluateFrameAtDistance(carDist);
    }

    /**
     * @brief Render all articulated train cars oriented via Yaw, Pitch, Roll along P'(t).
     */
    void draw(const BezierTrack& track, bool showDebugVectors, bool lightingEnabled, bool isNightOrSunset) const {
        if (track.samples.empty()) return;

        for (int i = 0; i < numCars; ++i) {
            TrackFrame f = getCarFrame(i, track);

            glPushMatrix();
            glTranslatef(f.position.x, f.position.y, f.position.z);
            glRotatef(f.yawDeg,   0.0f, 1.0f, 0.0f);
            glRotatef(f.pitchDeg, 1.0f, 0.0f, 0.0f);
            glRotatef(f.rollDeg,  0.0f, 0.0f, 1.0f);

            if (style == CoasterStyle::QUEEN_COBRA) {
                drawQueenCobraCar(i, track.config.trackGauge, track.config.railRadius, isNightOrSunset);
            } else {
                drawMonorailCar(i, isNightOrSunset);
            }

            if (i == 0 && showDebugVectors) {
                drawLocalOrientationAxes(lightingEnabled);
            }

            glPopMatrix();

            // Draw articulated coupler rod between car i and car i+1
            if (i + 1 < numCars) {
                TrackFrame nextF = getCarFrame(i + 1, track);
                drawCouplerBar(f.position, nextF.position, f.up);
            }
        }
    }

private:
    static void drawQueenCobraCar(int carIndex, float trackGauge, float railRadius, bool isNightOrSunset) {
        const float halfG = trackGauge * 0.5f;
        const float wheelR = 0.22f;
        const float wheelY = railRadius + wheelR - 0.04f;

        // 1. Steel bogie frame & 4 running wheels aligned to twin rails
        LightingSystem::setMaterial(0.75f, 56.0f);
        glColor3f(0.18f, 0.20f, 0.24f);
        CoasterMesh::drawBox(0.0f, wheelY + 0.06f,  0.75f, trackGauge + 0.12f, 0.14f, 0.26f);
        CoasterMesh::drawBox(0.0f, wheelY + 0.06f, -0.75f, trackGauge + 0.12f, 0.14f, 0.26f);
        CoasterMesh::drawBox(0.0f, wheelY + 0.16f,  0.00f, 0.90f, 0.18f, 1.85f);

        glColor3f(0.68f, 0.72f, 0.78f);
        CoasterMesh::drawWheelX(-halfG, wheelY,  0.75f, wheelR, 0.18f, 12);
        CoasterMesh::drawWheelX( halfG, wheelY,  0.75f, wheelR, 0.18f, 12);
        CoasterMesh::drawWheelX(-halfG, wheelY, -0.75f, wheelR, 0.18f, 12);
        CoasterMesh::drawWheelX( halfG, wheelY, -0.75f, wheelR, 0.18f, 12);

        // 2. Main Cobra Car Hull (Golden-Yellow with Crimson racing trim)
        const float baseY = wheelY + 0.26f;
        LightingSystem::setMaterial(0.60f, 48.0f);

        if (carIndex == 0) {
            // Lead Cobra Head Car: sleek tapered aerodynamic nose
            glColor3f(1.00f, 0.78f, 0.08f);
            CoasterMesh::drawTaperedPrism(
                -1.15f, baseY + 0.32f, 1.92f, 0.64f,
                 0.25f, baseY + 0.30f, 1.88f, 0.60f
            );
            // Tapered nose cone extending forward to z = +1.60
            glColor3f(0.95f, 0.34f, 0.08f);
            CoasterMesh::drawTaperedPrism(
                 0.25f, baseY + 0.30f, 1.88f, 0.60f,
                 1.62f, baseY + 0.12f, 0.78f, 0.26f
            );
            // Cobra hood crest / visor
            glColor3f(0.10f, 0.12f, 0.18f);
            CoasterMesh::drawTaperedPrism(
                -0.10f, baseY + 0.68f, 1.45f, 0.20f,
                 0.75f, baseY + 0.50f, 1.15f, 0.12f
            );

            // Glowing Cobra twin headlights
            if (isNightOrSunset) {
                LightingSystem::setMaterial(0.9f, 64.0f, 1.0f, 0.92f, 0.55f);
            }
            glColor3f(1.0f, 0.96f, 0.78f);
            CoasterMesh::drawBox(-0.46f, baseY + 0.24f, 1.25f, 0.24f, 0.14f, 0.22f);
            CoasterMesh::drawBox( 0.46f, baseY + 0.24f, 1.25f, 0.24f, 0.14f, 0.22f);
            LightingSystem::resetMaterial();
        } else {
            // Passenger car shell
            glColor3f(1.00f, 0.78f, 0.08f);
            CoasterMesh::drawBox(0.0f, baseY + 0.26f, 0.0f, 1.90f, 0.52f, 2.25f);

            // Side crimson cobra stripes
            glColor3f(0.86f, 0.16f, 0.12f);
            CoasterMesh::drawBox(-0.97f, baseY + 0.30f, 0.0f, 0.05f, 0.18f, 2.15f);
            CoasterMesh::drawBox( 0.97f, baseY + 0.30f, 0.0f, 0.05f, 0.18f, 2.15f);
        }

        // 3. Twin Passenger Seats + High Backrests + Safety Restraint Bar
        glColor3f(0.14f, 0.16f, 0.22f);
        CoasterMesh::drawBox(-0.44f, baseY + 0.78f, -0.58f, 0.62f, 0.66f, 0.22f);
        CoasterMesh::drawBox( 0.44f, baseY + 0.78f, -0.58f, 0.62f, 0.66f, 0.22f);
        // Headrest cushions
        glColor3f(0.85f, 0.18f, 0.14f);
        CoasterMesh::drawBox(-0.44f, baseY + 1.15f, -0.58f, 0.36f, 0.22f, 0.20f);
        CoasterMesh::drawBox( 0.44f, baseY + 1.15f, -0.58f, 0.36f, 0.22f, 0.20f);
        // Lap / shoulder safety grab bar
        glColor3f(0.75f, 0.78f, 0.84f);
        CoasterMesh::drawBox(0.0f, baseY + 0.74f, 0.08f, 1.56f, 0.09f, 0.09f);

        // Rear red tail lights
        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.8f, 32.0f, 0.95f, 0.10f, 0.08f);
        }
        glColor3f(0.95f, 0.12f, 0.10f);
        CoasterMesh::drawBox(-0.68f, baseY + 0.35f, -1.14f, 0.22f, 0.12f, 0.06f);
        CoasterMesh::drawBox( 0.68f, baseY + 0.35f, -1.14f, 0.22f, 0.12f, 0.06f);
        LightingSystem::resetMaterial();
    }

    void drawMonorailCar(int carIndex, bool isNightOrSunset) const {
        const bool isLead = (carIndex == 0);
        const bool isTail = (carIndex == numCars - 1);

        // 1. Straddle-beam side bogie skirts hugging the concrete box-girder beam
        LightingSystem::setMaterial(0.40f, 32.0f);
        glColor3f(0.15f, 0.20f, 0.26f);
        CoasterMesh::drawBox(-0.70f, -0.30f, 0.0f, 0.22f, 0.95f, 4.30f);
        CoasterMesh::drawBox( 0.70f, -0.30f, 0.0f, 0.22f, 0.95f, 4.30f);

        // Teal lower chassis band
        glColor3f(0.05f, 0.56f, 0.68f);
        CoasterMesh::drawBox(0.0f, 0.36f, 0.0f, 1.98f, 0.48f, 4.45f);

        // 2. Main streamlined white passenger cabin
        LightingSystem::setMaterial(0.65f, 52.0f);
        glColor3f(0.94f, 0.96f, 0.98f);

        if (isLead) {
            // Main body + aerodynamic front nose
            CoasterMesh::drawBox(0.0f, 1.25f, -0.35f, 1.96f, 1.32f, 3.70f);
            CoasterMesh::drawTaperedPrism(
                1.50f, 1.18f, 1.96f, 1.32f,
                2.75f, 0.72f, 1.32f, 0.78f
            );
            // Sloped front windshield
            glColor3f(0.10f, 0.18f, 0.28f);
            CoasterMesh::drawTaperedPrism(
                1.52f, 1.48f, 1.66f, 0.56f,
                2.32f, 1.14f, 1.24f, 0.44f
            );
            // Glowing LED headlights & top destination strip
            if (isNightOrSunset) {
                LightingSystem::setMaterial(0.9f, 64.0f, 0.95f, 0.98f, 1.0f);
            }
            glColor3f(0.95f, 0.98f, 1.0f);
            CoasterMesh::drawBox(-0.48f, 0.52f, 2.75f, 0.28f, 0.14f, 0.08f);
            CoasterMesh::drawBox( 0.48f, 0.52f, 2.75f, 0.28f, 0.14f, 0.08f);
            if (isNightOrSunset) {
                LightingSystem::setMaterial(0.9f, 64.0f, 0.15f, 0.85f, 1.0f);
            }
            glColor3f(0.20f, 0.88f, 1.0f);
            CoasterMesh::drawBox(0.0f, 1.76f, 1.54f, 1.10f, 0.14f, 0.10f);
            LightingSystem::resetMaterial();
        } else if (isTail) {
            CoasterMesh::drawBox(0.0f, 1.25f, 0.35f, 1.96f, 1.32f, 3.70f);
            CoasterMesh::drawTaperedPrism(
                -2.75f, 0.72f, 1.32f, 0.78f,
                -1.50f, 1.18f, 1.96f, 1.32f
            );
            // Tail lights
            if (isNightOrSunset) {
                LightingSystem::setMaterial(0.8f, 32.0f, 0.98f, 0.12f, 0.10f);
            }
            glColor3f(0.96f, 0.14f, 0.12f);
            CoasterMesh::drawBox(-0.46f, 0.54f, -2.75f, 0.26f, 0.14f, 0.08f);
            CoasterMesh::drawBox( 0.46f, 0.54f, -2.75f, 0.26f, 0.14f, 0.08f);
            LightingSystem::resetMaterial();
        } else {
            CoasterMesh::drawBox(0.0f, 1.25f, 0.0f, 1.96f, 1.32f, 4.45f);
        }

        // 3. Red accent trim stripe & panoramic tinted side windows
        glColor3f(0.88f, 0.20f, 0.18f);
        CoasterMesh::drawBox(0.0f, 0.64f, 0.0f, 2.01f, 0.12f, 4.10f);

        glColor3f(0.12f, 0.20f, 0.30f);
        if (isNightOrSunset) {
            // Warm interior cabin glow through panoramic windows at night
            LightingSystem::setMaterial(0.8f, 48.0f, 0.65f, 0.55f, 0.30f);
            glColor3f(0.95f, 0.85f, 0.58f);
        }
        float winLen = (isLead || isTail) ? 3.30f : 4.00f;
        float winZ   = isLead ? -0.30f : (isTail ? 0.30f : 0.0f);
        CoasterMesh::drawBox(-0.99f, 1.32f, winZ, 0.04f, 0.58f, winLen);
        CoasterMesh::drawBox( 0.99f, 1.32f, winZ, 0.04f, 0.58f, winLen);
        LightingSystem::resetMaterial();

        // Roof AC unit
        glColor3f(0.76f, 0.80f, 0.85f);
        CoasterMesh::drawBox(0.0f, 1.98f, winZ, 1.05f, 0.18f, 1.65f);
    }

    static void drawCouplerBar(const MathCG::Vec3& a, const MathCG::Vec3& b, const MathCG::Vec3& up) {
        MathCG::Vec3 offsetA = a + up * 0.25f;
        MathCG::Vec3 offsetB = b + up * 0.25f;
        MathCG::Vec3 diff    = offsetB - offsetA;
        float len = diff.length();
        if (len < 1e-4f || len > 12.0f) return;

        MathCG::Vec3 fwd = diff / len;
        MathCG::Vec3 rgt = MathCG::cross(up, fwd).normalized();
        MathCG::Vec3 orthoUp = MathCG::cross(fwd, rgt).normalized();
        MathCG::Vec3 mid = (offsetA + offsetB) * 0.5f;

        glColor3f(0.24f, 0.26f, 0.30f);
        MathCG::Mat4 m = MathCG::Mat4::fromBasis(rgt, orthoUp, fwd, mid);
        glPushMatrix();
        glMultMatrixf(m.m);
        CoasterMesh::drawBox(0.0f, 0.0f, 0.0f, 0.28f, 0.18f, len);
        glPopMatrix();
    }

    static void drawLocalOrientationAxes(bool lightingEnabled) {
        glDisable(GL_LIGHTING);
        glLineWidth(2.6f);
        glBegin(GL_LINES);
        // Local +X (Right) - Red
        glColor3f(1.0f, 0.15f, 0.15f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(4.2f, 0.6f, 0.0f);
        // Local +Y (Up) - Green
        glColor3f(0.2f, 1.0f, 0.25f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(0.0f, 4.8f, 0.0f);
        // Local +Z (Tangent P'(t)) - Cyan
        glColor3f(0.15f, 0.95f, 1.0f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(0.0f, 0.6f, 6.2f);
        glEnd();
        glLineWidth(1.0f);
        if (lightingEnabled) glEnable(GL_LIGHTING);
    }
};

/**
 * @brief Complete Asia Park Curves & Trains System (Member 1 Deliverable)
 *        Manages Queen Cobra Roller Coaster and the Park Perimeter Monorail.
 */
class AsiaParkCurvesSystem {
public:
    BezierTrack  queenCobraTrack;
    CoasterTrain queenCobraTrain;

    BezierTrack  monorailTrack;
    CoasterTrain monorailTrain;

    void init() {
        // 1. Queen Cobra Roller Coaster (West Adventure Zone, closed C^1 Piecewise Cubic Bezier)
        std::vector<MathCG::Vec3> queenCobraWaypoints = {
            MathCG::Vec3( -68.0f,  6.0f,  52.0f), // 0: Boarding Station South
            MathCG::Vec3( -68.0f,  6.0f,  26.0f), // 1: Station Exit -> Lift Chain Base
            MathCG::Vec3( -68.0f, 26.0f,  -6.0f), // 2: Lift Hill Climb
            MathCG::Vec3( -72.0f, 47.0f, -44.0f), // 3: Lift Hill Peak (47m)
            MathCG::Vec3( -90.0f, 40.0f, -76.0f), // 4: High Northwest Panorama Turn
            MathCG::Vec3(-103.0f, 13.0f, -48.0f), // 5: Steep First Drop Valley
            MathCG::Vec3( -94.0f, 34.0f, -12.0f), // 6: Airtime Camelback Crest
            MathCG::Vec3(-101.0f, 10.5f,  24.0f), // 7: Second Drop Low Sweep
            MathCG::Vec3( -92.0f, 21.0f,  64.0f), // 8: Banked Southwest Horseshoe
            MathCG::Vec3( -71.0f,  7.5f,  71.0f)  // 9: Brake Run returning straight to Station
        };

        TrackConfig qcCfg;
        qcCfg.trackGauge        = 1.85f;
        qcCfg.railRadius        = 0.14f;
        qcCfg.spineRadius       = 0.38f;
        qcCfg.spineDropOffset   = 0.74f;
        qcCfg.tieSpacing        = 2.35f;
        qcCfg.pillarSpacing     = 14.0f;
        qcCfg.pillarRadius      = 0.46f;
        qcCfg.samplesPerSegment = 28;
        qcCfg.railRadialSteps   = 8;
        qcCfg.maxBankDeg        = 32.0f;
        qcCfg.bankScale         = 11.0f;
        qcCfg.isMonorailBeam    = false;
        qcCfg.railColor         = MathCG::ColorRGB(0.95f, 0.30f, 0.14f);
        qcCfg.spineColor        = MathCG::ColorRGB(0.96f, 0.76f, 0.12f);
        qcCfg.tieColor          = MathCG::ColorRGB(0.24f, 0.27f, 0.34f);
        qcCfg.pillarColor       = MathCG::ColorRGB(0.70f, 0.73f, 0.78f);

        queenCobraTrack.buildFromClosedWaypoints(queenCobraWaypoints, 0.45f, qcCfg);
        queenCobraTrain.init(queenCobraTrack, CoasterStyle::QUEEN_COBRA, 6, 2.75f, 0.32f);

        // 2. Asia Park Sightseeing Monorail (Elevated Perimeter Loop)
        std::vector<MathCG::Vec3> monorailWaypoints = {
            MathCG::Vec3(   0.0f, 12.0f,  108.0f), // 0: South Front Overpass
            MathCG::Vec3(  72.0f, 12.5f,  102.0f), // 1: Southeast Curve
            MathCG::Vec3( 114.0f, 13.0f,   58.0f), // 2: East Plaza Leg
            MathCG::Vec3( 116.0f, 14.0f,    0.0f), // 3: East Mid
            MathCG::Vec3( 114.0f, 13.5f,  -60.0f), // 4: Northeast Curve
            MathCG::Vec3(  72.0f, 13.0f,  -98.0f), // 5: North Behind Sun Wheel East
            MathCG::Vec3(   0.0f, 14.5f, -102.0f), // 6: North Behind Sun Wheel Center
            MathCG::Vec3( -72.0f, 13.0f,  -98.0f), // 7: North Behind Sun Wheel West
            MathCG::Vec3(-114.0f, 12.5f,  -60.0f), // 8: Northwest Curve
            MathCG::Vec3(-116.0f, 12.0f,    0.0f), // 9: West Perimeter (outside Queen Cobra)
            MathCG::Vec3(-114.0f, 12.0f,   58.0f), // 10: Southwest Curve
            MathCG::Vec3( -72.0f, 12.0f,  102.0f)  // 11: South Return
        };

        TrackConfig mrCfg;
        mrCfg.trackGauge        = 1.08f;
        mrCfg.railRadius        = 0.08f;
        mrCfg.spineRadius       = 0.50f;
        mrCfg.spineDropOffset   = 0.65f;
        mrCfg.tieSpacing        = 5.5f;
        mrCfg.pillarSpacing     = 19.0f;
        mrCfg.pillarRadius      = 0.56f;
        mrCfg.samplesPerSegment = 24;
        mrCfg.railRadialSteps   = 6;
        mrCfg.maxBankDeg        = 6.0f;
        mrCfg.bankScale         = 5.0f;
        mrCfg.isMonorailBeam    = true;
        mrCfg.railColor         = MathCG::ColorRGB(0.28f, 0.34f, 0.42f);
        mrCfg.spineColor        = MathCG::ColorRGB(0.78f, 0.80f, 0.82f);
        mrCfg.tieColor          = MathCG::ColorRGB(0.56f, 0.58f, 0.62f);
        mrCfg.pillarColor       = MathCG::ColorRGB(0.72f, 0.74f, 0.78f);

        monorailTrack.buildFromClosedWaypoints(monorailWaypoints, 0.45f, mrCfg);
        monorailTrain.init(monorailTrack, CoasterStyle::MONORAIL, 4, 4.95f, 0.05f);
    }

    void update(float dt) {
        queenCobraTrain.update(dt, queenCobraTrack);
        monorailTrain.update(dt, monorailTrack);
    }

    TrackFrame getQueenCobraCameraFrame() const {
        return queenCobraTrain.getLeadFrame(queenCobraTrack);
    }

    TrackFrame getMonorailCameraFrame() const {
        return monorailTrain.getLeadFrame(monorailTrack);
    }

    void draw(bool showNormalsAndCtrl, bool lightingEnabled, TimeOfDay timeOfDay) const {
        bool isNightOrSunset = (timeOfDay != TimeOfDay::DAY);

        // Draw Boarding Stations
        drawQueenCobraStation(isNightOrSunset);
        drawMonorailStation(isNightOrSunset);

        // Draw Tracks + Control Polygons / Tangent Debug Overlay
        queenCobraTrack.draw(showNormalsAndCtrl, lightingEnabled);
        monorailTrack.draw(showNormalsAndCtrl, lightingEnabled);

        // Draw Animated Multi-Car Trains
        queenCobraTrain.draw(queenCobraTrack, showNormalsAndCtrl, lightingEnabled, isNightOrSunset);
        monorailTrain.draw(monorailTrack, showNormalsAndCtrl, lightingEnabled, isNightOrSunset);
    }

private:
    static void drawQueenCobraStation(bool isNightOrSunset) {
        // Boarding deck alongside flat segment 0: track at x = -68.0, z in [26..52], y = 6.0
        const float cx = -68.0f;
        const float cz = 39.0f;

        // Concrete boarding platform on +X side of track (x in [-66.7 .. -62.1])
        glColor3f(0.42f, 0.44f, 0.48f);
        CoasterMesh::drawBox(cx + 3.6f, 2.7f, cz, 4.6f, 5.4f, 26.0f);

        // Platform deck overhang (clear of car x >= -67.0)
        glColor3f(0.68f, 0.24f, 0.16f);
        CoasterMesh::drawBox(cx + 3.5f, 5.55f, cz, 4.9f, 0.35f, 27.0f);

        // Canopy steel columns grounded at y = 0 (far -X side) & platform edge (+X side)
        glColor3f(0.78f, 0.78f, 0.82f);
        for (int zSide = -1; zSide <= 1; ++zSide) {
            // Far-side full-height column clear of track (-70.6)
            CoasterMesh::drawBox(cx - 2.6f, 5.05f, cz + zSide * 10.5f, 0.38f, 10.1f, 0.38f);
            // Platform-side column (+X outer edge -62.4)
            CoasterMesh::drawBox(cx + 5.6f, 7.80f, cz + zSide * 10.5f, 0.38f,  4.6f, 0.38f);
        }

        // Sloped station canopy roof over track & boarding platform
        glColor3f(0.88f, 0.32f, 0.12f);
        CoasterMesh::drawBox(cx + 1.5f, 10.15f, cz, 9.2f, 0.36f, 27.5f);

        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.8f, 48.0f, 1.0f, 0.78f, 0.32f);
            glColor3f(1.0f, 0.85f, 0.42f);
            CoasterMesh::drawBox(cx + 1.5f, 9.91f, cz, 7.6f, 0.12f, 24.0f);
            LightingSystem::resetMaterial();
        }
    }

    static void drawMonorailStation(bool isNightOrSunset) {
        // Elevated Monorail East Boarding Station near track (116, 14, 0)
        const float cx = 116.0f;
        const float cz = 0.0f;

        // Station concourse tower & elevated platform clear of monorail cars (x >= 117.4)
        glColor3f(0.56f, 0.60f, 0.65f);
        CoasterMesh::drawBox(cx + 4.6f, 6.2f, cz, 5.0f, 12.4f, 22.0f);

        // Teal station trim deck
        glColor3f(0.06f, 0.54f, 0.66f);
        CoasterMesh::drawBox(cx + 4.4f, 12.55f, cz, 5.8f, 0.40f, 23.0f);

        // Modern canopy columns grounded on both sides of the guideway
        glColor3f(0.84f, 0.86f, 0.90f);
        for (int zSide = -1; zSide <= 1; ++zSide) {
            // Inner-park side full-height column clear of car (-X side: x = 113.8)
            CoasterMesh::drawBox(cx - 2.2f,  8.90f, cz + zSide * 9.0f, 0.42f, 17.8f, 0.42f);
            // Platform outer column (+X side: x = 122.8)
            CoasterMesh::drawBox(cx + 6.8f, 15.20f, cz + zSide * 9.0f, 0.42f,  5.2f, 0.42f);
        }
        glColor3f(0.92f, 0.94f, 0.96f);
        CoasterMesh::drawBox(cx + 2.3f, 17.8f, cz, 9.8f, 0.38f, 24.0f);

        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.8f, 48.0f, 0.25f, 0.88f, 1.0f);
            glColor3f(0.30f, 0.90f, 1.0f);
            CoasterMesh::drawBox(cx + 2.3f, 17.55f, cz, 8.2f, 0.12f, 21.0f);
            LightingSystem::resetMaterial();
        }
    }
};

#endif // ROLLER_COASTER_H
