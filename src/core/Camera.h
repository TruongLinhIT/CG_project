#ifndef CAMERA_H
#define CAMERA_H

/**
 * @file Camera.h
 * @brief Chapter 4: 3D Viewing System, Spherical Orbit Camera, Ride Cameras & Projections
 *
 * Implements:
 *  - Spherical coordinates viewing transformation:
 *      theta (azimuth in XZ plane), Phi (elevation from XZ plane),
 *      distance R (from target origin O to eye O'), and projection plane distance D.
 *  - Dynamic switching between:
 *      1. Free Spherical Orbit Camera
 *      2. First-Person Sun Wheel Cabin Camera
 *      3. First-Person Queen Cobra Coaster Camera (oriented along Bezier tangent P'(t))
 *  - Dynamic toggle between:
 *      1. Perspective Projection (gluPerspective derived from view plane distance D)
 *      2. Parallel / Orthographic Projection (glOrtho with Top, Front, Side technical views)
 */

#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"

enum class CameraMode {
    FREE_ORBIT = 0,
    SUN_WHEEL_CABIN = 1,
    COASTER_FIRST_PERSON = 2,
    MONORAIL_FIRST_PERSON = 3
};

enum class ProjectionMode {
    PERSPECTIVE = 0,
    ORTHO_TOP   = 1,
    ORTHO_FRONT = 2,
    ORTHO_SIDE  = 3
};

class Camera {
public:
    // Chapter 4 Spherical Viewing Parameters
    float thetaDeg;      // Azimuth angle theta (degrees)
    float phiDeg;        // Elevation angle Phi (degrees)
    float distanceR;     // Distance R from look-at origin O to eye O'
    float viewPlaneD;    // Distance D from eye to projection plane (controls FOV)
    MathCG::Vec3 target; // Look-at reference point O(x, y, z)

    CameraMode     cameraMode;
    ProjectionMode projectionMode;

    // Ortho half-height scale
    float orthoSize;

    Camera()
        : thetaDeg(35.0f),
          phiDeg(18.0f),
          distanceR(195.0f),
          viewPlaneD(50.0f),
          target(0.0f, 36.0f, -12.0f),
          cameraMode(CameraMode::FREE_ORBIT),
          projectionMode(ProjectionMode::PERSPECTIVE),
          orthoSize(115.0f) {}

    /**
     * @brief Compute eye position O'(x, y, z) from spherical coordinates (R, theta, Phi)
     *        relative to target O:
     *          O'_x = O_x + R * cos(Phi) * sin(theta)
     *          O'_y = O_y + R * sin(Phi)
     *          O'_z = O_z + R * cos(Phi) * cos(theta)
     */
    MathCG::Vec3 getOrbitEyePosition() const {
        float theta = thetaDeg * MathCG::DEG2RAD;
        float phi   = phiDeg   * MathCG::DEG2RAD;
        float ex = target.x + distanceR * std::cos(phi) * std::sin(theta);
        float ey = target.y + distanceR * std::sin(phi);
        float ez = target.z + distanceR * std::cos(phi) * std::cos(theta);
        return MathCG::Vec3(ex, ey, ez);
    }

    /**
     * @brief Configure the OpenGL Projection Matrix (GL_PROJECTION)
     *        Supports Perspective (FOV derived from viewPlaneD) and Orthographic views.
     */
    void applyProjection(int windowWidth, int windowHeight) const {
        if (windowHeight <= 0) windowHeight = 1;
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            // Convert projection plane distance D (with sensor half-height = 25.0) to vertical FOV
            float fovY = 2.0f * std::atan(25.0f / std::max(5.0f, viewPlaneD)) * MathCG::RAD2DEG;
            fovY = std::clamp(fovY, 20.0f, 110.0f);
            gluPerspective(fovY, aspect, 0.5f, 1000.0f);
        } else {
            // Parallel / Orthographic projection
            float halfW = orthoSize * aspect;
            float halfH = orthoSize;
            glOrtho(-halfW, halfW, -halfH, halfH, -500.0f, 500.0f);
        }

        glMatrixMode(GL_MODELVIEW);
    }

    /**
     * @brief Configure the OpenGL ModelView Matrix (GL_MODELVIEW)
     *        based on active ProjectionMode (Ortho technical views) or CameraMode.
     */
    void applyView(
        const MathCG::Vec3& cabinWorldPos,
        float wheelLookYawDeg,
        const MathCG::Vec3& coasterPos,
        const MathCG::Vec3& coasterTangent,
        const MathCG::Vec3& coasterUp       = MathCG::Vec3(0.0f, 1.0f, 0.0f),
        const MathCG::Vec3& monorailPos     = MathCG::Vec3(0.0f, 12.0f, 108.0f),
        const MathCG::Vec3& monorailTangent = MathCG::Vec3(1.0f, 0.0f, 0.0f),
        const MathCG::Vec3& monorailUp      = MathCG::Vec3(0.0f, 1.0f, 0.0f)
    ) const {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Technical orthographic axis-aligned views (Chapter 4 Parallel Projections)
        if (projectionMode == ProjectionMode::ORTHO_TOP) {
            gluLookAt(0.0, 260.0, 0.0,   0.0, 0.0, 0.0,   0.0, 0.0, -1.0);
            return;
        } else if (projectionMode == ProjectionMode::ORTHO_FRONT) {
            gluLookAt(0.0, 50.0, 260.0,  0.0, 50.0, 0.0,  0.0, 1.0, 0.0);
            return;
        } else if (projectionMode == ProjectionMode::ORTHO_SIDE) {
            gluLookAt(260.0, 50.0, 0.0,  0.0, 50.0, 0.0,  0.0, 1.0, 0.0);
            return;
        }

        // Perspective Camera Modes
        if (cameraMode == CameraMode::SUN_WHEEL_CABIN) {
            float yawRad = wheelLookYawDeg * MathCG::DEG2RAD;
            MathCG::Vec3 lookDir(std::sin(yawRad), -0.12f, std::cos(yawRad));
            MathCG::Vec3 center = cabinWorldPos + lookDir * 20.0f;
            gluLookAt(
                cabinWorldPos.x, cabinWorldPos.y, cabinWorldPos.z,
                center.x,        center.y,        center.z,
                0.0,             1.0,             0.0
            );
        } else if (cameraMode == CameraMode::COASTER_FIRST_PERSON) {
            MathCG::Vec3 fwd = coasterTangent.normalized();
            MathCG::Vec3 up  = coasterUp.normalized();
            // Place eye in the front seat of the lead Cobra car, banking smoothly with the track
            MathCG::Vec3 eye    = coasterPos + up * 1.95f + fwd * 0.65f;
            MathCG::Vec3 center = eye + fwd * 14.0f - up * 0.45f;
            gluLookAt(
                eye.x,    eye.y,    eye.z,
                center.x, center.y, center.z,
                up.x,     up.y,     up.z
            );
        } else if (cameraMode == CameraMode::MONORAIL_FIRST_PERSON) {
            MathCG::Vec3 fwd = monorailTangent.normalized();
            MathCG::Vec3 up  = monorailUp.normalized();
            // Place eye at the panoramic front windshield of the Monorail lead cab
            MathCG::Vec3 eye    = monorailPos + up * 1.90f + fwd * 2.15f;
            MathCG::Vec3 center = eye + fwd * 18.0f - up * 0.35f;
            gluLookAt(
                eye.x,    eye.y,    eye.z,
                center.x, center.y, center.z,
                up.x,     up.y,     up.z
            );
        } else {
            // Default Free Spherical Orbit Camera
            MathCG::Vec3 eye = getOrbitEyePosition();
            gluLookAt(
                eye.x,    eye.y,    eye.z,
                target.x, target.y, target.z,
                0.0,      1.0,      0.0
            );
        }
    }

    void orbit(float dTheta, float dPhi) {
        thetaDeg = std::fmod(thetaDeg + dTheta, 360.0f);
        phiDeg   = std::clamp(phiDeg + dPhi, 3.0f, 87.0f);
    }

    void zoom(float dR) {
        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            distanceR = std::clamp(distanceR + dR, 15.0f, 380.0f);
        } else {
            orthoSize = std::clamp(orthoSize + dR * 0.5f, 15.0f, 260.0f);
        }
    }

    void panTarget(float dx, float dy, float dz) {
        target.x = std::clamp(target.x + dx, -120.0f, 120.0f);
        target.y = std::clamp(target.y + dy,   2.0f,  90.0f);
        target.z = std::clamp(target.z + dz, -120.0f, 120.0f);
    }

    const char* getCameraModeName() const {
        switch (cameraMode) {
            case CameraMode::FREE_ORBIT:            return "Camera Quy dao cau (theta, Phi, R, D)";
            case CameraMode::SUN_WHEEL_CABIN:       return "Goc nhin thu 1: Cabin Sun Wheel";
            case CameraMode::COASTER_FIRST_PERSON:  return "Goc nhin thu 1: Tau luon Queen Cobra";
            case CameraMode::MONORAIL_FIRST_PERSON: return "Goc nhin thu 1: Tau dien Monorail";
            default:                                return "Khong xac dinh";
        }
    }

    const char* getProjectionModeName() const {
        switch (projectionMode) {
            case ProjectionMode::PERSPECTIVE: return "Phoi canh (gluPerspective)";
            case ProjectionMode::ORTHO_TOP:   return "Truc giao: Chieu bang TOP (XZ)";
            case ProjectionMode::ORTHO_FRONT: return "Truc giao: Chieu dung FRONT (XY)";
            case ProjectionMode::ORTHO_SIDE:  return "Truc giao: Chieu canh SIDE (ZY)";
            default:                          return "Khong xac dinh";
        }
    }
};

#endif // CAMERA_H
