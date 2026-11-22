#ifndef CAMERA_H
#define CAMERA_H

/**
 * @file Camera.h
 * @brief Chương 4: Hệ thống Quan sát 3D, Camera Quỹ đạo Cầu, Camera Góc nhìn Trò chơi & Các Phép Chiếu
 *
 * Cài đặt:
 *  - Phép biến đổi quan sát dùng tọa độ cầu (Spherical coordinates):
 *      góc phương vị theta (trong mặt phẳng XZ), góc nâng Phi (tính từ mặt phẳng XZ),
 *      khoảng cách R (từ điểm nhìn gốc O tới mắt O'), và khoảng cách mặt phẳng chiếu D.
 *  - Chuyển đổi linh hoạt giữa các chế độ camera:
 *      1. Camera Quỹ đạo Cầu Tự do (Free Spherical Orbit Camera)
 *      2. Camera Góc nhìn thứ nhất trong Cabin Vòng quay Sun Wheel
 *      3. Camera Góc nhìn thứ nhất trên Tàu lượn Queen Cobra (định hướng theo tiếp tuyến Bezier P'(t))
 *  - Chuyển đổi linh hoạt giữa các phép chiếu:
 *      1. Phép chiếu Phối cảnh (gluPerspective tính từ khoảng cách màn chiếu D)
 *      2. Phép chiếu Song song / Trực giao (glOrtho với các góc nhìn kỹ thuật Bằng, Đứng, Cạnh)
 */

#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"

enum class CameraMode {
    FREE_ORBIT = 0,
    SUN_WHEEL_CABIN = 1,
    COASTER_FIRST_PERSON = 2,
    MONORAIL_FIRST_PERSON = 3,
    DRONE_FREE = 4
};

enum class ProjectionMode {
    PERSPECTIVE = 0,
    ORTHO_TOP   = 1,
    ORTHO_FRONT = 2,
    ORTHO_SIDE  = 3
};

class Camera {
public:
    // Các tham số quan sát tọa độ cầu theo Chương 4
    float thetaDeg;      // Góc phương vị theta (độ)
    float phiDeg;        // Góc nâng Phi (độ)
    float distanceR;     // Khoảng cách R từ tâm nhìn O tới vị trí mắt O'
    float viewPlaneD;    // Khoảng cách D từ mắt tới mặt phẳng chiếu (điều khiển góc mở FOV)
    MathCG::Vec3 target; // Điểm đích quan sát O(x, y, z)

    CameraMode     cameraMode;
    ProjectionMode projectionMode;

    // Nửa chiều cao khung chiếu trực giao (Ortho)
    float orthoSize;

    // Các tham số Camera Drone Bay Tự do (Vị trí, Vận tốc làm mượt, Góc quay Yaw/Pitch)
    MathCG::Vec3 dronePos;
    MathCG::Vec3 droneVelocity;
    float droneYawDeg;    // 0 độ = Hướng Bắc (-Z), +90 = Hướng Đông (+X), -90 = Hướng Tây (-X)
    float dronePitchDeg;  // Giới hạn trong khoảng [-85, +85] độ
    float droneSpeed;     // Tốc độ bay hành trình (m/s): 12 (Chi tiết), 35 (Bình thường), 75 (Nhanh)

    Camera()
        : thetaDeg(28.0f),
          phiDeg(22.0f),
          distanceR(320.0f),
          viewPlaneD(50.0f),
          target(0.0f, 36.0f, -25.0f),
          cameraMode(CameraMode::FREE_ORBIT),
          projectionMode(ProjectionMode::PERSPECTIVE),
          orthoSize(360.0f),
          dronePos(60.0f, 56.0f, 145.0f),
          droneVelocity(0.0f, 0.0f, 0.0f),
          droneYawDeg(-22.0f),
          dronePitchDeg(-10.0f),
          droneSpeed(40.0f) {}

    /**
     * @brief Tính vị trí đặt mắt O'(x, y, z) từ tọa độ cầu (R, theta, Phi)
     *        so với tâm quan sát O:
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
     * @brief Thiết lập Ma trận Chiếu của OpenGL (GL_PROJECTION)
     *        Hỗ trợ phép chiếu Phối cảnh (FOV tính từ viewPlaneD) và các phép chiếu Trực giao.
     */
    void applyProjection(int windowWidth, int windowHeight) const {
        if (windowHeight <= 0) windowHeight = 1;
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            // Chuyển đổi khoảng cách mặt phẳng chiếu D (với nửa chiều cao cảm biến = 25.0) sang góc mở dọc FOV
            float fovY = 2.0f * std::atan(25.0f / std::max(5.0f, viewPlaneD)) * MathCG::RAD2DEG;
            fovY = std::clamp(fovY, 20.0f, 110.0f);
            gluPerspective(fovY, aspect, 1.0f, 2200.0f);
        } else {
            // Phép chiếu song song / trực giao (Orthographic projection)
            float halfW = orthoSize * aspect;
            float halfH = orthoSize;
            glOrtho(-halfW, halfW, -halfH, halfH, -1200.0f, 1200.0f);
        }

        glMatrixMode(GL_MODELVIEW);
    }

    /**
     * @brief Thiết lập Ma trận ModelView của OpenGL (GL_MODELVIEW)
     *        dựa trên chế độ chiếu hiện hành (các hình chiếu trục đo kỹ thuật Ortho) hoặc chế độ Camera.
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

        // Các góc nhìn kỹ thuật trực giao dọc theo trục tọa độ (Chương 4: Phép chiếu song song)
        if (projectionMode == ProjectionMode::ORTHO_TOP) {
            gluLookAt(0.0, 620.0, -140.0,   0.0, 0.0, -140.0,   0.0, 0.0, -1.0);
            return;
        } else if (projectionMode == ProjectionMode::ORTHO_FRONT) {
            gluLookAt(0.0, 55.0, 620.0,     0.0, 55.0, -100.0,  0.0, 1.0, 0.0);
            return;
        } else if (projectionMode == ProjectionMode::ORTHO_SIDE) {
            gluLookAt(620.0, 55.0, -140.0,  0.0, 55.0, -140.0,  0.0, 1.0, 0.0);
            return;
        }

        // Các chế độ Camera Phối cảnh
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
            // Đặt mắt tại ghế trước của toa đầu tàu Queen Cobra, nghiêng mượt theo góc nghiêng đường ray
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
            // Đặt mắt tại kính chắn gió toàn cảnh phía trước của toa đầu tàu Monorail
            MathCG::Vec3 eye    = monorailPos + up * 1.90f + fwd * 2.15f;
            MathCG::Vec3 center = eye + fwd * 18.0f - up * 0.35f;
            gluLookAt(
                eye.x,    eye.y,    eye.z,
                center.x, center.y, center.z,
                up.x,     up.y,     up.z
            );
        } else if (cameraMode == CameraMode::DRONE_FREE) {
            MathCG::Vec3 fwd    = getDroneForward();
            MathCG::Vec3 center = dronePos + fwd * 25.0f;
            gluLookAt(
                dronePos.x, dronePos.y, dronePos.z,
                center.x,   center.y,   center.z,
                0.0,        1.0,        0.0
            );
        } else {
            // Mặc định: Camera Quỹ đạo Cầu Tự do
            MathCG::Vec3 eye = getOrbitEyePosition();
            gluLookAt(
                eye.x,    eye.y,    eye.z,
                target.x, target.y, target.z,
                0.0,      1.0,      0.0
            );
        }
    }

    // ------------------------------------------------------------------------
    // Điều khiển Gimbal & Bay 6 bậc tự do (6DOF) cho Camera Drone Tự do
    // ------------------------------------------------------------------------

    MathCG::Vec3 getDroneForward() const {
        float yaw   = droneYawDeg   * MathCG::DEG2RAD;
        float pitch = dronePitchDeg * MathCG::DEG2RAD;
        return MathCG::Vec3(
            std::cos(pitch) * std::sin(yaw),
            std::sin(pitch),
            -std::cos(pitch) * std::cos(yaw)
        ).normalized();
    }

    MathCG::Vec3 getDroneRight() const {
        MathCG::Vec3 fwd = getDroneForward();
        return MathCG::cross(fwd, MathCG::Vec3(0.0f, 1.0f, 0.0f)).normalized();
    }

    void clampDroneBounds() {
        dronePos.x = std::clamp(dronePos.x, -560.0f, 560.0f);
        dronePos.y = std::clamp(dronePos.y,    1.2f, 360.0f);
        dronePos.z = std::clamp(dronePos.z, -560.0f, 360.0f);
    }

    void droneLook(float dYawDeg, float dPitchDeg) {
        droneYawDeg = std::fmod(droneYawDeg + dYawDeg, 360.0f);
        if (droneYawDeg > 180.0f)  droneYawDeg -= 360.0f;
        if (droneYawDeg < -180.0f) droneYawDeg += 360.0f;
        dronePitchDeg = std::clamp(dronePitchDeg + dPitchDeg, -85.0f, 85.0f);
    }

    void syncDroneFromPose(const MathCG::Vec3& eye, const MathCG::Vec3& lookDir) {
        dronePos      = eye;
        droneVelocity = MathCG::Vec3(0.0f, 0.0f, 0.0f);
        clampDroneBounds();

        if (lookDir.lengthSquared() < 1e-6f) return;
        MathCG::Vec3 dir = lookDir.normalized();
        dronePitchDeg = std::clamp(
            std::asin(std::clamp(dir.y, -0.99f, 0.99f)) * MathCG::RAD2DEG,
            -85.0f, 85.0f
        );
        if (dir.x * dir.x + dir.z * dir.z > 1e-5f) {
            droneYawDeg = std::atan2(dir.x, -dir.z) * MathCG::RAD2DEG;
        }
    }

    void cycleDroneSpeed() {
        if (droneSpeed < 20.0f) {
            droneSpeed = 40.0f;
        } else if (droneSpeed < 65.0f) {
            droneSpeed = 100.0f;
        } else {
            droneSpeed = 12.0f;
        }
    }

    void adjustDroneSpeed(float delta) {
        droneSpeed = std::clamp(droneSpeed + delta, 5.0f, 220.0f);
    }

    void droneNudgeForward(float dist) {
        dronePos += getDroneForward() * dist;
        clampDroneBounds();
    }

    void updateDrone(
        float dt,
        float inputFwd,
        float inputRight,
        float inputUp,
        float turnYawRateDeg,
        float turnPitchRateDeg
    ) {
        if (dt <= 0.0f) return;
        droneLook(turnYawRateDeg * dt, turnPitchRateDeg * dt);

        MathCG::Vec3 fwd   = getDroneForward();
        MathCG::Vec3 right = getDroneRight();
        MathCG::Vec3 up(0.0f, 1.0f, 0.0f);

        MathCG::Vec3 desiredDir = fwd * inputFwd + right * inputRight + up * inputUp;
        float dirLen = desiredDir.length();
        if (dirLen > 1.0f) {
            desiredDir = desiredDir / dirLen;
        }

        MathCG::Vec3 targetVel = desiredDir * droneSpeed;
        float blend = 1.0f - std::exp(-10.0f * dt);
        droneVelocity = MathCG::lerp(droneVelocity, targetVel, blend);
        if (dirLen < 1e-4f && droneVelocity.length() < 0.02f) {
            droneVelocity = MathCG::Vec3(0.0f, 0.0f, 0.0f);
        }

        dronePos += droneVelocity * dt;
        clampDroneBounds();
    }

    void getActiveEyeAndLookDir(
        const MathCG::Vec3& cabinWorldPos,
        const MathCG::Vec3& coasterPos,
        const MathCG::Vec3& coasterTangent,
        const MathCG::Vec3& coasterUp,
        const MathCG::Vec3& monorailPos,
        const MathCG::Vec3& monorailTangent,
        const MathCG::Vec3& monorailUp,
        MathCG::Vec3& outEye,
        MathCG::Vec3& outLookDir
    ) const {
        if (cameraMode == CameraMode::DRONE_FREE) {
            outEye     = dronePos;
            outLookDir = getDroneForward();
        } else if (cameraMode == CameraMode::SUN_WHEEL_CABIN) {
            float yawRad = thetaDeg * MathCG::DEG2RAD;
            outEye       = cabinWorldPos;
            outLookDir   = MathCG::Vec3(std::sin(yawRad), -0.12f, std::cos(yawRad)).normalized();
        } else if (cameraMode == CameraMode::COASTER_FIRST_PERSON) {
            MathCG::Vec3 fwd = coasterTangent.normalized();
            MathCG::Vec3 up  = coasterUp.normalized();
            outEye           = coasterPos + up * 1.95f + fwd * 0.65f;
            outLookDir       = (fwd * 14.0f - up * 0.45f).normalized();
        } else if (cameraMode == CameraMode::MONORAIL_FIRST_PERSON) {
            MathCG::Vec3 fwd = monorailTangent.normalized();
            MathCG::Vec3 up  = monorailUp.normalized();
            outEye           = monorailPos + up * 1.90f + fwd * 2.15f;
            outLookDir       = (fwd * 18.0f - up * 0.35f).normalized();
        } else {
            outEye     = getOrbitEyePosition();
            outLookDir = (target - outEye).normalized();
        }
    }

    float getMinimapYawDeg(const MathCG::Vec3& lookDir) const {
        return std::atan2(lookDir.x, -lookDir.z) * MathCG::RAD2DEG;
    }

    void orbit(float dTheta, float dPhi) {
        thetaDeg = std::fmod(thetaDeg + dTheta, 360.0f);
        phiDeg   = std::clamp(phiDeg + dPhi, 3.0f, 87.0f);
    }

    void zoom(float dR) {
        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            if (cameraMode == CameraMode::DRONE_FREE) {
                droneNudgeForward(-dR * 0.65f);
            } else {
                distanceR = std::clamp(distanceR + dR, 15.0f, 880.0f);
            }
        } else {
            orthoSize = std::clamp(orthoSize + dR * 1.2f, 20.0f, 620.0f);
        }
    }

    void panTarget(float dx, float dy, float dz) {
        target.x = std::clamp(target.x + dx, -420.0f, 420.0f);
        target.y = std::clamp(target.y + dy,   2.0f,  160.0f);
        target.z = std::clamp(target.z + dz, -480.0f, 250.0f);
    }

    const char* getCameraModeName() const {
        switch (cameraMode) {
            case CameraMode::FREE_ORBIT:            return "Camera Quy dao cau (theta, Phi, R, D)";
            case CameraMode::SUN_WHEEL_CABIN:       return "Goc nhin thu 1: Cabin Sun Wheel";
            case CameraMode::COASTER_FIRST_PERSON:  return "Goc nhin thu 1: Tau luon Queen Cobra";
            case CameraMode::MONORAIL_FIRST_PERSON: return "Goc nhin thu 1: Tau dien Monorail";
            case CameraMode::DRONE_FREE:            return "Camera Drone Tu Do (WASD + E/Q + Chuot)";
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
