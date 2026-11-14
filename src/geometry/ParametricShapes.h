#ifndef PARAMETRIC_SHAPES_H
#define PARAMETRIC_SHAPES_H

/**
 * @file ParametricShapes.h
 * @brief Chapter 5 & 6: Procedural Parametric Surfaces (Ruled Surfaces & Surfaces of Revolution)
 *
 * Implements explicit parametric surface generation via (u, v) parameter grids:
 *  - Chapter 5:
 *      1. Ruled Cylinder: X(u,v) = r*cos(u), Y(u,v) = r*sin(u), Z(u,v) = h*v
 *      2. Ruled Cone / Frustum: X(u,v) = R(v)*cos(u), Y(u,v) = R(v)*sin(u), Z(u,v) = h*v
 *      3. Surface of Revolution - Sphere: X(u,v) = r*cos(u)*cos(v), Y(u,v) = r*sin(u)*cos(v), Z(u,v) = r*sin(v)
 *      4. Surface of Revolution - Toroid: X(u,v) = (R + a*cos(v))*cos(u), Y(u,v) = (R + a*cos(v))*sin(u), Z(u,v) = a*sin(v)
 *      5. End Caps / Disks (GL_TRIANGLE_FAN)
 *      6. Arbitrary 3D connecting Cylinders & Cones between points A and B
 *      7. Segmented HSV LED Toroid for rides (Sun Wheel, Sky Tower, Carousel)
 *  - Chapter 6:
 *      - Exact analytical unit normal vectors N(u, v) = (dP/du x dP/dv) / ||dP/du x dP/dv||
 *      - Counter-Clockwise (GL_CCW) quad strip winding for correct Back-face Culling (V . N >= 0)
 *      - Debug normal vector visualization lines for teaching & grading defense
 *
 * NOTE: Strictly avoids legacy glutSolid* primitives to satisfy assignment syllabus requirements.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <utility>
#include "core/MathCG.h"

namespace ParametricShapes {

// ============================================================================
// 1. DATA STRUCTURES FOR PROCEDURAL VERTEX/NORMAL CACHING
// ============================================================================

struct SurfaceVertex {
    MathCG::Vec3 position;
    MathCG::Vec3 normal;
    MathCG::Vec2 uv;

    SurfaceVertex() = default;
    SurfaceVertex(const MathCG::Vec3& p, const MathCG::Vec3& n, const MathCG::Vec2& t = MathCG::Vec2())
        : position(p), normal(n), uv(t) {}
};

// ============================================================================
// 2. ANALYTICAL SURFACE EVALUATION (FORMULAS FROM SYLLABUS CHAPTER 5 & 6)
// ============================================================================

/**
 * @brief Chapter 5: Ruled Cylinder Evaluation (Z-axis aligned)
 *        X(u,v) = r * cos(u)
 *        Y(u,v) = r * sin(u)
 *        Z(u,v) = h * v
 *        where u in [0, 2*pi], v in [0, 1]
 *
 * Normal: dP/du = (-r*sin(u), r*cos(u), 0)
 *         dP/dv = (0, 0, h)
 *         N = (dP/du x dP/dv) / ||...|| = (cos(u), sin(u), 0)
 */
inline SurfaceVertex evaluateCylinder(float u, float v, float r, float h) {
    float cu = std::cos(u);
    float su = std::sin(u);
    MathCG::Vec3 pos(r * cu, r * su, h * v);
    MathCG::Vec3 normal(cu, su, 0.0f);
    MathCG::Vec2 uv(u / MathCG::TWO_PI, v);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chapter 5: Ruled Cone / Frustum Evaluation (Z-axis aligned)
 *        R(v) = rBase + (rTop - rBase) * v
 *        X(u,v) = R(v) * cos(u)
 *        Y(u,v) = R(v) * sin(u)
 *        Z(u,v) = h * v
 *
 * Normal: dr = rBase - rTop
 *         slant = sqrt(dr^2 + h^2)
 *         N = ( (h/slant)*cos(u), (h/slant)*sin(u), dr/slant )
 */
inline SurfaceVertex evaluateCone(float u, float v, float rBase, float rTop, float h) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float rCurrent = rBase + (rTop - rBase) * v;
    MathCG::Vec3 pos(rCurrent * cu, rCurrent * su, h * v);

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float nz = dr / slant;
    MathCG::Vec3 normal(nr * cu, nr * su, nz);

    MathCG::Vec2 uv(u / MathCG::TWO_PI, v);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chapter 5: Surface of Revolution - Sphere Evaluation (Z-axis pole)
 *        u in [0, 2*pi] (azimuth), v in [-pi/2, pi/2] (latitude)
 *        X(u,v) = r * cos(u) * cos(v)
 *        Y(u,v) = r * sin(u) * cos(v)
 *        Z(u,v) = r * sin(v)
 *
 * Normal: N(u,v) = (cos(u)*cos(v), sin(u)*cos(v), sin(v))
 */
inline SurfaceVertex evaluateSphere(float u, float v, float r) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float cv = std::cos(v);
    float sv = std::sin(v);

    MathCG::Vec3 normal(cu * cv, su * cv, sv);
    MathCG::Vec3 pos = normal * r;
    MathCG::Vec2 uv(u / MathCG::TWO_PI, (v + MathCG::HALF_PI) / MathCG::PI);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chapter 5: Surface of Revolution - Toroid Evaluation (Z-axis revolution)
 *        u in [0, 2*pi] (revolution angle), v in [0, 2*pi] (tube cross-section angle)
 *        R = major radius, a = tube radius
 *        X(u,v) = (R + a * cos(v)) * cos(u)
 *        Y(u,v) = (R + a * cos(v)) * sin(u)
 *        Z(u,v) = a * sin(v)
 *
 * Normal: N(u,v) = (cos(v)*cos(u), cos(v)*sin(u), sin(v))
 */
inline SurfaceVertex evaluateToroid(float u, float v, float R, float a) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float cv = std::cos(v);
    float sv = std::sin(v);

    MathCG::Vec3 normal(cv * cu, cv * su, sv);
    MathCG::Vec3 pos((R + a * cv) * cu, (R + a * cv) * su, a * sv);
    MathCG::Vec2 uv(u / MathCG::TWO_PI, v / MathCG::TWO_PI);
    return SurfaceVertex(pos, normal, uv);
}

// ============================================================================
// 3. DEBUG NORMAL VECTOR RENDERING (Chapter 6)
// ============================================================================

/**
 * @brief Draw debug normal whiskers for teaching & grading defense
 */
inline void drawNormalWhiskers(
    const std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>>& whiskers,
    float length = 1.0f,
    float r = 1.0f, float g = 1.0f, float b = 0.0f
) {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    if (lightingWasOn) glDisable(GL_LIGHTING);

    glColor3f(r, g, b);
    glBegin(GL_LINES);
    for (const auto& w : whiskers) {
        glVertex3f(w.first.x, w.first.y, w.first.z);
        MathCG::Vec3 tip = w.first + w.second * length;
        glVertex3f(tip.x, tip.y, tip.z);
    }
    glEnd();

    if (lightingWasOn) glEnable(GL_LIGHTING);
}

// ============================================================================
// 4. PLANAR DISK & END CAP GENERATION (GL_TRIANGLE_FAN)
// ============================================================================

/**
 * @brief Draw a circular planar disk (end-cap for cylinder/cone) at z = height
 *        using GL_TRIANGLE_FAN with correct normal (+Z or -Z)
 */
inline void drawDisk(float r, int slices, float z = 0.0f, bool flipNormal = false) {
    if (slices < 3) slices = 3;
    float nz = flipNormal ? -1.0f : 1.0f;

    glNormal3f(0.0f, 0.0f, nz);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        if (flipNormal) {
            // Clockwise from top -> CCW when viewed from negative Z
            u = -u;
        }
        glVertex3f(r * std::cos(u), r * std::sin(u), z);
    }
    glEnd();
}

/**
 * @brief Draw a circular planar disk in the XZ plane (Y-aligned)
 */
inline void drawDiskY(float r, int slices, float y = 0.0f, bool flipNormal = false) {
    if (slices < 3) slices = 3;
    float ny = flipNormal ? -1.0f : 1.0f;

    glNormal3f(0.0f, ny, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, y, 0.0f);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        if (flipNormal) {
            u = -u;
        }
        glVertex3f(r * std::cos(u), y, r * std::sin(u));
    }
    glEnd();
}

// ============================================================================
// 5. PARAMETRIC CYLINDER (MẶT TRỤ - MẶT KẺ)
// ============================================================================

/**
 * @brief Chapter 5: Draw parametric cylinder aligned with local Z-axis [0, h]
 *        using GL_QUAD_STRIP with explicit normal glNormal3f.
 *        Front faces wind CCW viewed from exterior.
 */
inline void drawCylinder(
    float r, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float z0 = h * v0;
        float z1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            // Outward unit normal: (cos(u), sin(u), 0)
            glNormal3f(cu, su, 0.0f);
            // Emit top (z1) then bottom (z0) for CCW winding outward
            glVertex3f(r * cu, r * su, z1);
            glVertex3f(r * cu, r * su, z0);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * cu, r * su, (z0 + z1) * 0.5f),
                    MathCG::Vec3(cu, su, 0.0f)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        drawDisk(r, slices, 0.0f, true);  // Bottom cap (normal -Z)
        drawDisk(r, slices, h, false);    // Top cap (normal +Z)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 1.0f, 1.0f, 0.0f);
    }
}

/**
 * @brief Chapter 5: Draw parametric cylinder aligned with local Y-axis [0, h]
 *        Useful for upright park pillars, Sky Tower columns, carousel poles.
 */
inline void drawCylinderY(
    float r, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float y0 = h * v0;
        float y1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(cu, 0.0f, su);
            glVertex3f(r * cu, y1, r * su);
            glVertex3f(r * cu, y0, r * su);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * cu, (y0 + y1) * 0.5f, r * su),
                    MathCG::Vec3(cu, 0.0f, su)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        drawDiskY(r, slices, 0.0f, true); // Bottom cap (normal -Y)
        drawDiskY(r, slices, h, false);   // Top cap (normal +Y)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 1.0f, 1.0f, 0.0f);
    }
}

// ============================================================================
// 6. PARAMETRIC CONE / FRUSTUM (MẶT NÓN - MẶT KẺ)
// ============================================================================

/**
 * @brief Chapter 5: Draw parametric cone or frustum aligned with local Z-axis [0, h]
 *        Base at z = 0 with radius rBase, top at z = h with radius rTop.
 *        If rTop == 0.0f, produces a sharp cone tip.
 */
inline void drawCone(
    float rBase, float rTop, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float nz = dr / slant;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float r0 = rBase + (rTop - rBase) * v0;
        float r1 = rBase + (rTop - rBase) * v1;
        float z0 = h * v0;
        float z1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(nr * cu, nr * su, nz);
            glVertex3f(r1 * cu, r1 * su, z1);
            glVertex3f(r0 * cu, r0 * su, z0);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((r0 + r1) * 0.5f * cu, (r0 + r1) * 0.5f * su, (z0 + z1) * 0.5f),
                    MathCG::Vec3(nr * cu, nr * su, nz)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        if (rBase > 1e-5f) drawDisk(rBase, slices, 0.0f, true); // Base cap (-Z)
        if (rTop  > 1e-5f) drawDisk(rTop,  slices, h, false);   // Top cap (+Z)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, rBase * 0.35f, 1.0f, 0.8f, 0.1f);
    }
}

/**
 * @brief Chapter 5: Draw parametric cone or frustum aligned with local Y-axis [0, h]
 *        Useful for Carousel conical roofs, pagodas, decorative pinnacles.
 */
inline void drawConeY(
    float rBase, float rTop, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float ny = dr / slant;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float r0 = rBase + (rTop - rBase) * v0;
        float r1 = rBase + (rTop - rBase) * v1;
        float y0 = h * v0;
        float y1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(nr * cu, ny, nr * su);
            glVertex3f(r1 * cu, y1, r1 * su);
            glVertex3f(r0 * cu, y0, r0 * su);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((r0 + r1) * 0.5f * cu, (y0 + y1) * 0.5f, (r0 + r1) * 0.5f * su),
                    MathCG::Vec3(nr * cu, ny, nr * su)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        if (rBase > 1e-5f) drawDiskY(rBase, slices, 0.0f, true); // Base cap (-Y)
        if (rTop  > 1e-5f) drawDiskY(rTop,  slices, h, false);   // Top cap (+Y)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, rBase * 0.35f, 1.0f, 0.8f, 0.1f);
    }
}

// ============================================================================
// 7. PARAMETRIC SPHERE (MẶT CẦU - MẶT TRÒN XOAY)
// ============================================================================

/**
 * @brief Chapter 5: Draw parametric sphere centered at origin (0,0,0)
 *        Latitude-longitude mesh using GL_QUAD_STRIP with analytical normals
 */
inline void drawSphere(
    float r,
    int slices = 24, int stacks = 16,
    bool drawNormals = false
) {
    if (slices < 4) slices = 4;
    if (stacks < 2) stacks = 2;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float phi0 = -MathCG::HALF_PI + (static_cast<float>(j)     / stacks) * MathCG::PI;
        float phi1 = -MathCG::HALF_PI + (static_cast<float>(j + 1) / stacks) * MathCG::PI;
        float cPhi0 = std::cos(phi0), sPhi0 = std::sin(phi0);
        float cPhi1 = std::cos(phi1), sPhi1 = std::sin(phi1);

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float theta = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cTh = std::cos(theta);
            float sTh = std::sin(theta);

            // Upper ring (phi1)
            float nx1 = cPhi1 * cTh;
            float ny1 = sPhi1;
            float nz1 = cPhi1 * sTh;
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(r * nx1, r * ny1, r * nz1);

            // Lower ring (phi0)
            float nx0 = cPhi0 * cTh;
            float ny0 = sPhi0;
            float nz0 = cPhi0 * sTh;
            glNormal3f(nx0, ny0, nz0);
            glVertex3f(r * nx0, r * ny0, r * nz0);

            if (drawNormals && (j % 3 == 0) && (i % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * nx0, r * ny0, r * nz0),
                    MathCG::Vec3(nx0, ny0, nz0)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 0.2f, 1.0f, 0.2f);
    }
}

// ============================================================================
// 8. PARAMETRIC TOROID (MẶT XUYẾN - MẶT TRÒN XOAY)
// ============================================================================

/**
 * @brief Chapter 5: Draw parametric Toroid in XY plane revolving around Z axis
 *        X(u,v) = (R + a*cos(v))*cos(u)
 *        Y(u,v) = (R + a*cos(v))*sin(u)
 *        Z(u,v) = a*sin(v)
 *        Useful for Sun Wheel rims, hub rings, decorative toroids.
 */
inline void drawToroid(
    float R, float a,
    int uSteps = 32, int vSteps = 16,
    bool drawNormals = false
) {
    if (uSteps < 3) uSteps = 3;
    if (vSteps < 3) vSteps = 3;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int i = 0; i < uSteps; ++i) {
        float u0 = (static_cast<float>(i)     / uSteps) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(i + 1) / uSteps) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            // Unit normal: (cv * cos(u), cv * sin(u), sv)
            glNormal3f(cv * cu0, cv * su0, sv);
            glVertex3f((R + a * cv) * cu0, (R + a * cv) * su0, a * sv);

            glNormal3f(cv * cu1, cv * su1, sv);
            glVertex3f((R + a * cv) * cu1, (R + a * cv) * su1, a * sv);

            if (drawNormals && (i % 3 == 0) && (j % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((R + a * cv) * cu0, (R + a * cv) * su0, a * sv),
                    MathCG::Vec3(cv * cu0, cv * su0, sv)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, a * 1.5f, 0.0f, 1.0f, 1.0f);
    }
}

/**
 * @brief Chapter 5: Draw parametric Toroid in XZ plane revolving around Y axis
 *        Useful for Sky Tower passenger ring, horizontal circular collars.
 */
inline void drawToroidY(
    float R, float a,
    int uSteps = 32, int vSteps = 16,
    bool drawNormals = false
) {
    if (uSteps < 3) uSteps = 3;
    if (vSteps < 3) vSteps = 3;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int i = 0; i < uSteps; ++i) {
        float u0 = (static_cast<float>(i)     / uSteps) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(i + 1) / uSteps) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            glNormal3f(cv * cu0, sv, cv * su0);
            glVertex3f((R + a * cv) * cu0, a * sv, (R + a * cv) * su0);

            glNormal3f(cv * cu1, sv, cv * su1);
            glVertex3f((R + a * cv) * cu1, a * sv, (R + a * cv) * su1);

            if (drawNormals && (i % 3 == 0) && (j % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((R + a * cv) * cu0, a * sv, (R + a * cv) * su0),
                    MathCG::Vec3(cv * cu0, sv, cv * su0)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, a * 1.5f, 0.0f, 1.0f, 1.0f);
    }
}

// ============================================================================
// 9. SWEPT 3D CONNECTORS (ARBITRARY POINTS A -> B)
// ============================================================================

/**
 * @brief Draw a tapered cylinder / frustum connecting arbitrary points A and B
 *        in 3D space with radius rA at A and rB at B.
 *        Computes an orthonormal coordinate frame {fwd, right, up} along A->B.
 */
inline void drawCylinderBetween(
    const MathCG::Vec3& a,
    const MathCG::Vec3& b,
    float rA,
    float rB,
    int slices = 12,
    bool drawCaps = true
) {
    MathCG::Vec3 diff = b - a;
    float len = diff.length();
    if (len < 1e-4f) return;

    MathCG::Vec3 fwd = diff / len;
    MathCG::Vec3 helper = (std::fabs(fwd.y) < 0.90f)
        ? MathCG::Vec3(0.0f, 1.0f, 0.0f)
        : MathCG::Vec3(1.0f, 0.0f, 0.0f);
    MathCG::Vec3 right = MathCG::cross(helper, fwd).normalized();
    MathCG::Vec3 up    = MathCG::cross(fwd, right).normalized();

    float dr = rA - rB;
    float slant = std::sqrt(dr * dr + len * len);
    if (slant < 1e-6f) slant = 1.0f;
    float nr  = len / slant; // radial normal weight
    float nL  = dr  / slant; // longitudinal normal weight

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        float cu = std::cos(u);
        float su = std::sin(u);

        MathCG::Vec3 radialDir = right * cu + up * su;
        MathCG::Vec3 normal    = (radialDir * nr + fwd * nL).normalized();

        MathCG::Vec3 pA = a + radialDir * rA;
        MathCG::Vec3 pB = b + radialDir * rB;

        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(pB.x, pB.y, pB.z);
        glVertex3f(pA.x, pA.y, pA.z);
    }
    glEnd();

    if (drawCaps) {
        if (rA > 1e-4f) {
            glNormal3f(-fwd.x, -fwd.y, -fwd.z);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(a.x, a.y, a.z);
            for (int i = 0; i <= slices; ++i) {
                float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
                MathCG::Vec3 radialDir = right * std::cos(-u) + up * std::sin(-u);
                MathCG::Vec3 p = a + radialDir * rA;
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
        if (rB > 1e-4f) {
            glNormal3f(fwd.x, fwd.y, fwd.z);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(b.x, b.y, b.z);
            for (int i = 0; i <= slices; ++i) {
                float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
                MathCG::Vec3 radialDir = right * std::cos(u) + up * std::sin(u);
                MathCG::Vec3 p = b + radialDir * rB;
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
    }
}

// ============================================================================
// 10. SEGMENTED HSV LED TOROID (SUN WHEEL / CAROUSEL / SKY TOWER NIGHT EFFECTS)
// ============================================================================

/**
 * @brief Chapter 1 & 5: Segmented Toroid for multi-color LED animations.
 *        Renders N sectors independently, computing dynamic HSV -> RGB colors
 *        with optional emission for stunning night simulation.
 */
inline void drawSegmentedLedToroid(
    float R, float a,
    int numSectors,
    int vSteps,
    float baseHue,
    float hueStepPerSector,
    float saturation = 0.92f,
    float value = 1.0f,
    float emissiveIntensity = 0.85f
) {
    if (numSectors < 3) numSectors = 3;
    if (vSteps < 3) vSteps = 3;

    for (int s = 0; s < numSectors; ++s) {
        float hue = std::fmod(baseHue + s * hueStepPerSector, 360.0f);
        if (hue < 0.0f) hue += 360.0f;
        float lr = 1.0f, lg = 1.0f, lb = 1.0f;
        MathCG::HSVtoRGB(hue, saturation, value, lr, lg, lb);

        float u0 = (static_cast<float>(s)     / numSectors) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(s + 1) / numSectors) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        GLfloat matDiff[4] = { lr, lg, lb, 1.0f };
        GLfloat matEmit[4] = {
            lr * emissiveIntensity,
            lg * emissiveIntensity,
            lb * emissiveIntensity,
            1.0f
        };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  matDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmit);
        glColor3f(lr, lg, lb);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            glNormal3f(cv * cu0, cv * su0, sv);
            glVertex3f((R + a * cv) * cu0, (R + a * cv) * su0, a * sv);

            glNormal3f(cv * cu1, cv * su1, sv);
            glVertex3f((R + a * cv) * cu1, (R + a * cv) * su1, a * sv);
        }
        glEnd();
    }

    // Reset emission to zero
    GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
}

} // namespace ParametricShapes

#endif // PARAMETRIC_SHAPES_H
