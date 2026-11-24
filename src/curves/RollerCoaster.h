#ifndef ROLLER_COASTER_H
#define ROLLER_COASTER_H

/**
 * @file RollerCoaster.h
 * @brief Chương 4 & 5: Mô phỏng Tàu lượn Siêu tốc Queen Cobra & Tàu điện một ray Monorail Asia Park
 *
 * Cài đặt:
 *  - Các đoàn tàu nhiều toa nối khớp di chuyển dọc theo đường ray Bezier bậc 3 từng đoạn khép kín liên tục C^1
 *  - Giãn cách toa tàu tính chính xác theo độ dài cung (Arc-length) giúp các toa giữ khoảng cách vật lý không đổi
 *  - Định hướng 3D cục bộ động (Yaw, Pitch, Roll) tính từ tiếp tuyến Bezier P'(t) & độ cong đường ray
 *  - Biểu đồ tốc độ dựa trên bảo toàn năng lượng cho tàu Queen Cobra (kéo lên đỉnh dốc -> lao dốc tốc độ cao & đồi không trọng lực)
 *  - Lưới đa giác 3D sinh tự động tùy chỉnh (cụm bánh xe bogie, mũi tàu Hổ Mang, ghế ngồi, chân ôm dầm monorail,
 *    nhà ga đón khách) với pháp tuyến hướng ra ngoài tường minh (không dùng hàm glutSolid* có sẵn)
 */

#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
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
 * @brief Vẽ hình hộp cục bộ căn chỉnh theo trục có tâm tại (cx, cy, cz) với kích thước toàn phần (sx, sy, sz)
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
    emitQuad(f00, f10, f11, f01); // +Z Mặt trước
    emitQuad(b10, b00, b01, b11); // -Z Mặt sau
    emitQuad(f10, b10, b11, f11); // +X Mặt phải
    emitQuad(b00, f00, f01, b01); // -X Mặt trái
    emitQuad(f01, f11, b11, b01); // +Y Mặt trên
    emitQuad(b00, b10, f10, f00); // -Y Mặt dưới
    glEnd();
}

/**
 * @brief Vẽ khối lăng trụ khí động học thuôn nhọn từ z0 (phía sau) tới z1 (phía trước, z1 > z0)
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
    emitQuad(f00, f10, f11, f01); // +Z Mặt trước
    emitQuad(b10, b00, b01, b11); // -Z Mặt sau
    emitQuad(f10, b10, b11, f11); // +X Mặt phải
    emitQuad(b00, f00, f01, b01); // -X Mặt trái
    emitQuad(f01, f11, b11, b01); // +Y Mặt trên
    emitQuad(b00, b10, f10, f00); // -Y Mặt dưới
    glEnd();
}

/**
 * @brief Vẽ hình trụ bánh xe tùy chỉnh định hướng dọc theo trục X cục bộ kèm hai nắp chắn hướng pháp tuyến ra ngoài
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

    // Nắp chắn +X (góc u tăng dần tạo thứ tự đỉnh CCW khi nhìn từ phía +X)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x1, cy, cz);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i % slices) / static_cast<float>(slices)) * MathCG::TWO_PI;
        glVertex3f(x1, cy + radius * std::cos(u), cz + radius * std::sin(u));
    }
    glEnd();

    // Nắp chắn -X (góc u giảm dần tạo thứ tự đỉnh CCW khi nhìn từ phía -X)
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
    float        paramT             = 0.0f;  // Tham số t của toa dẫn đầu thuộc [0, 1)
    float        currentDistMeters  = 0.0f;  // Quãng đường độ dài cung của toa dẫn đầu
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

        // Tìm độ cao đỉnh lớn nhất của đường ray để tính tốc độ theo thế năng trọng trường
        trackMaxY = 10.0f;
        for (const auto& s : track.samples) {
            trackMaxY = std::max(trackMaxY, s.position.y);
        }

        if (style == CoasterStyle::MONORAIL) {
            useGravityPhysics = false;
            cruiseSpeedMps    = ParkLayout::MONORAIL_CRUISE_SPEED_MPS; // 9.2 m/s (~3.45 phút / vòng 1.90 km)
            lastSpeedMps      = cruiseSpeedMps;
        } else {
            useGravityPhysics = true;
            cruiseSpeedMps    = 14.5f;
            minSpeedMps       = 14.5f;
            maxSpeedMps       = 38.0f;
        }
    }

    /**
     * @brief Cập nhật vị trí đoàn tàu tiến dọc theo BezierTrack sau khoảng thời gian dt (giây).
     */
    void update(float dt, const BezierTrack& track) {
        float totalLen = track.getTotalLength();
        if (totalLen <= 1e-4f || dt <= 0.0f) return;

        float speed = cruiseSpeedMps;
        if (useGravityPhysics) {
            TrackFrame lead = track.evaluateFrameAtDistance(currentDistMeters);
            float dropH = std::max(0.0f, trackMaxY - lead.position.y);
            // Xấp xỉ định luật bảo toàn cơ năng: v = sqrt(v_min^2 + 2*g_eff*dh)
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
     * @brief Vẽ toàn bộ các toa tàu nối khớp được định hướng qua góc Yaw, Pitch, Roll dọc theo P'(t).
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

            // Vẽ thanh khớp nối giữa toa thứ i và toa thứ i+1
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

        // 1. Khung gầm thép bogie & 4 bánh xe lăn căn chỉnh khớp với hai thanh ray đôi
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

        // 2. Thân chính của toa tàu Hổ Mang (Vàng ánh kim phối viền Đỏ thẫm thể thao)
        const float baseY = wheelY + 0.26f;
        LightingSystem::setMaterial(0.60f, 48.0f);

        if (carIndex == 0) {
            // Toa đầu tàu Hổ Mang (Lead Cobra Head Car): mũi tàu khí động học thuôn nhọn
            glColor3f(1.00f, 0.78f, 0.08f);
            CoasterMesh::drawTaperedPrism(
                -1.15f, baseY + 0.32f, 1.92f, 0.64f,
                 0.25f, baseY + 0.30f, 1.88f, 0.60f
            );
            // Chóp mũi tàu thuôn nhọn vươn về phía trước tới z = +1.60
            glColor3f(0.95f, 0.34f, 0.08f);
            CoasterMesh::drawTaperedPrism(
                 0.25f, baseY + 0.30f, 1.88f, 0.60f,
                 1.62f, baseY + 0.12f, 0.78f, 0.26f
            );
            // Mào đầu Hổ Mang / kính chắn gió trước
            glColor3f(0.10f, 0.12f, 0.18f);
            CoasterMesh::drawTaperedPrism(
                -0.10f, baseY + 0.68f, 1.45f, 0.20f,
                 0.75f, baseY + 0.50f, 1.15f, 0.12f
            );

            // Cặp đèn pha đôi phát sáng ở đầu tàu Hổ Mang
            if (isNightOrSunset) {
                LightingSystem::setMaterial(0.9f, 64.0f, 1.0f, 0.92f, 0.55f);
            }
            glColor3f(1.0f, 0.96f, 0.78f);
            CoasterMesh::drawBox(-0.46f, baseY + 0.24f, 1.25f, 0.24f, 0.14f, 0.22f);
            CoasterMesh::drawBox( 0.46f, baseY + 0.24f, 1.25f, 0.24f, 0.14f, 0.22f);
            LightingSystem::resetMaterial();
        } else {
            // Vỏ toa hành khách tiêu chuẩn
            glColor3f(1.00f, 0.78f, 0.08f);
            CoasterMesh::drawBox(0.0f, baseY + 0.26f, 0.0f, 1.90f, 0.52f, 2.25f);

            // Dải sọc đỏ thẫm trang trí hai bên hông toa
            glColor3f(0.86f, 0.16f, 0.12f);
            CoasterMesh::drawBox(-0.97f, baseY + 0.30f, 0.0f, 0.05f, 0.18f, 2.15f);
            CoasterMesh::drawBox( 0.97f, baseY + 0.30f, 0.0f, 0.05f, 0.18f, 2.15f);
        }

        // 3. Cặp ghế hành khách đôi + Tựa lưng cao + Thanh đai an toàn
        glColor3f(0.14f, 0.16f, 0.22f);
        CoasterMesh::drawBox(-0.44f, baseY + 0.78f, -0.58f, 0.62f, 0.66f, 0.22f);
        CoasterMesh::drawBox( 0.44f, baseY + 0.78f, -0.58f, 0.62f, 0.66f, 0.22f);
        // Đệm tựa đầu
        glColor3f(0.85f, 0.18f, 0.14f);
        CoasterMesh::drawBox(-0.44f, baseY + 1.15f, -0.58f, 0.36f, 0.22f, 0.20f);
        CoasterMesh::drawBox( 0.44f, baseY + 1.15f, -0.58f, 0.36f, 0.22f, 0.20f);
        // Thanh khóa an toàn ngang đùi / vai
        glColor3f(0.75f, 0.78f, 0.84f);
        CoasterMesh::drawBox(0.0f, baseY + 0.74f, 0.08f, 1.56f, 0.09f, 0.09f);

        // Đèn hậu màu đỏ phía đuôi toa
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

        // 1. Phần yếm khung gầm hai bên ôm sát dầm hộp bê tông của đường ray Monorail
        LightingSystem::setMaterial(0.40f, 32.0f);
        glColor3f(0.15f, 0.20f, 0.26f);
        CoasterMesh::drawBox(-0.70f, -0.30f, 0.0f, 0.22f, 0.95f, 4.30f);
        CoasterMesh::drawBox( 0.70f, -0.30f, 0.0f, 0.22f, 0.95f, 4.30f);

        // Dải khung gầm dưới màu xanh cổ vịt (Teal)
        glColor3f(0.05f, 0.56f, 0.68f);
        CoasterMesh::drawBox(0.0f, 0.36f, 0.0f, 1.98f, 0.48f, 4.45f);

        // 2. Khoang hành khách chính màu trắng khí động học
        LightingSystem::setMaterial(0.65f, 52.0f);
        glColor3f(0.94f, 0.96f, 0.98f);

        if (isLead) {
            // Thân chính + Mũi tàu khí động học phía trước
            CoasterMesh::drawBox(0.0f, 1.25f, -0.35f, 1.96f, 1.32f, 3.70f);
            CoasterMesh::drawTaperedPrism(
                1.50f, 1.18f, 1.96f, 1.32f,
                2.75f, 0.72f, 1.32f, 0.78f
            );
            // Kính chắn gió vát nghiêng phía trước
            glColor3f(0.10f, 0.18f, 0.28f);
            CoasterMesh::drawTaperedPrism(
                1.52f, 1.48f, 1.66f, 0.56f,
                2.32f, 1.14f, 1.24f, 0.44f
            );
            // Đèn pha LED phát sáng & bảng đèn hiển thị lộ trình trên nóc
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
            // Đèn hậu đuôi tàu
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

        // 3. Đường viền chỉ đỏ trang trí & dải cửa sổ kính toàn cảnh hai bên hông
        glColor3f(0.88f, 0.20f, 0.18f);
        CoasterMesh::drawBox(0.0f, 0.64f, 0.0f, 2.01f, 0.12f, 4.10f);

        glColor3f(0.12f, 0.20f, 0.30f);
        if (isNightOrSunset) {
            // Ánh sáng ấm áp bên trong khoang tàu hắt qua cửa sổ toàn cảnh vào ban đêm
            LightingSystem::setMaterial(0.8f, 48.0f, 0.65f, 0.55f, 0.30f);
            glColor3f(0.95f, 0.85f, 0.58f);
        }
        float winLen = (isLead || isTail) ? 3.30f : 4.00f;
        float winZ   = isLead ? -0.30f : (isTail ? 0.30f : 0.0f);
        CoasterMesh::drawBox(-0.99f, 1.32f, winZ, 0.04f, 0.58f, winLen);
        CoasterMesh::drawBox( 0.99f, 1.32f, winZ, 0.04f, 0.58f, winLen);
        LightingSystem::resetMaterial();

        // Cụm điều hòa không khí trên mui toa tàu
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
        // Trục +X cục bộ (Hướng phải) - Màu đỏ
        glColor3f(1.0f, 0.15f, 0.15f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(4.2f, 0.6f, 0.0f);
        // Trục +Y cục bộ (Hướng lên) - Màu xanh lá
        glColor3f(0.2f, 1.0f, 0.25f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(0.0f, 4.8f, 0.0f);
        // Trục +Z cục bộ (Tiếp tuyến P'(t)) - Màu xanh lơ
        glColor3f(0.15f, 0.95f, 1.0f);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(0.0f, 0.6f, 6.2f);
        glEnd();
        glLineWidth(1.0f);
        if (lightingEnabled) glEnable(GL_LIGHTING);
    }
};

/**
 * @brief Hệ thống Đường cong & Đoàn tàu Hoàn chỉnh của Asia Park (Phần việc Thành viên 1)
 *        Quản lý Tàu lượn Siêu tốc Queen Cobra và Tàu điện một ray Monorail chạy quanh công viên.
 */
class AsiaParkCurvesSystem {
public:
    BezierTrack  queenCobraTrack;
    CoasterTrain queenCobraTrain;

    BezierTrack  monorailTrack;
    CoasterTrain monorailTrain;

    void init() {
        // 1. Tàu lượn Siêu tốc Queen Cobra (Phân khu Cảm giác mạnh phía Tây, đường cong Bezier bậc 3 từng đoạn khép kín C^1)
        std::vector<MathCG::Vec3> queenCobraWaypoints = {
            MathCG::Vec3( -68.0f,  6.0f,  52.0f), // 0: Nhà ga đón khách phía Nam
            MathCG::Vec3( -68.0f,  6.0f,  26.0f), // 1: Lối ra nhà ga -> Chân dốc kéo xích
            MathCG::Vec3( -68.0f, 26.0f,  -6.0f), // 2: Đoạn leo dốc kéo xích (Lift Hill)
            MathCG::Vec3( -72.0f, 47.0f, -44.0f), // 3: Đỉnh dốc cao nhất (47m)
            MathCG::Vec3( -90.0f, 40.0f, -76.0f), // 4: Khúc cua toàn cảnh trên cao phía Tây Bắc
            MathCG::Vec3(-103.0f, 13.0f, -48.0f), // 5: Thung lũng sau cú lao dốc đứng đầu tiên
            MathCG::Vec3( -94.0f, 34.0f, -12.0f), // 6: Đỉnh đồi lạc đà tạo cảm giác không trọng lực (Camelback)
            MathCG::Vec3(-101.0f, 10.5f,  24.0f), // 7: Đáy lượn thấp sau cú lao dốc thứ hai
            MathCG::Vec3( -92.0f, 21.0f,  64.0f), // 8: Khúc cua móng ngựa nghiêng phía Tây Nam
            MathCG::Vec3( -71.0f,  7.5f,  71.0f)  // 9: Đoạn phanh giảm tốc trở về thẳng nhà ga
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

        // 2. Tàu điện một ray Ngắm cảnh Asia Park Monorail (Vòng tuyến ~1.87 km qua 20 điểm thực tế trên ảnh vệ tinh 2019)
        std::vector<MathCG::Vec3> monorailWaypoints(
            std::begin(ParkLayout::MONORAIL_WAYPOINTS),
            std::end(ParkLayout::MONORAIL_WAYPOINTS)
        );

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

    static MathCG::Vec3 transformQueenCobraPoint(const MathCG::Vec3& p) {
        const float rad = ParkLayout::QUEEN_COBRA_ROT_Y_DEG * MathCG::DEG2RAD;
        const float c   = std::cos(rad);
        const float s   = std::sin(rad);
        const float dx  = p.x - ParkLayout::QUEEN_COBRA_ORIG_CENTER.x;
        const float dz  = p.z - ParkLayout::QUEEN_COBRA_ORIG_CENTER.z;
        return MathCG::Vec3(
            ParkLayout::QUEEN_COBRA_TARGET_CENTER.x + (dx * c + dz * s),
            p.y,
            ParkLayout::QUEEN_COBRA_TARGET_CENTER.z + (-dx * s + dz * c)
        );
    }

    static MathCG::Vec3 transformQueenCobraDir(const MathCG::Vec3& v) {
        const float rad = ParkLayout::QUEEN_COBRA_ROT_Y_DEG * MathCG::DEG2RAD;
        const float c   = std::cos(rad);
        const float s   = std::sin(rad);
        return MathCG::Vec3(
            v.x * c + v.z * s,
            v.y,
            -v.x * s + v.z * c
        ).normalized();
    }

    TrackFrame getQueenCobraCameraFrame() const {
        TrackFrame f = queenCobraTrain.getLeadFrame(queenCobraTrack);
        f.position = transformQueenCobraPoint(f.position);
        f.tangent  = transformQueenCobraDir(f.tangent);
        f.right    = transformQueenCobraDir(f.right);
        f.up       = transformQueenCobraDir(f.up);
        f.yawDeg  += ParkLayout::QUEEN_COBRA_ROT_Y_DEG;
        return f;
    }

    TrackFrame getMonorailCameraFrame() const {
        return monorailTrain.getLeadFrame(monorailTrack);
    }

    void draw(bool showNormalsAndCtrl, bool lightingEnabled, TimeOfDay timeOfDay) const {
        bool isNightOrSunset = (timeOfDay != TimeOfDay::DAY);

        // 1. Vẽ cụm Tàu lượn Queen Cobra (Phương án B: chỉ tịnh tiến + xoay -25°, KHÔNG scale, giữ nguyên 100% hình học & animation)
        glPushMatrix();
        glTranslatef(
            ParkLayout::QUEEN_COBRA_TARGET_CENTER.x,
            0.0f,
            ParkLayout::QUEEN_COBRA_TARGET_CENTER.z
        );
        glRotatef(ParkLayout::QUEEN_COBRA_ROT_Y_DEG, 0.0f, 1.0f, 0.0f);
        glTranslatef(
            -ParkLayout::QUEEN_COBRA_ORIG_CENTER.x,
            0.0f,
            -ParkLayout::QUEEN_COBRA_ORIG_CENTER.z
        );
        if (!showNormalsAndCtrl) {
            const int modeIdx = isNightOrSunset ? 1 : 0;
            if (qcStaticList_[modeIdx] == 0) {
                qcStaticList_[modeIdx] = glGenLists(1);
                if (qcStaticList_[modeIdx] != 0) {
                    glNewList(qcStaticList_[modeIdx], GL_COMPILE);
                    drawQueenCobraStation(isNightOrSunset);
                    queenCobraTrack.draw(false, true);
                    glEndList();
                }
            }
            if (qcStaticList_[modeIdx] != 0) {
                glCallList(qcStaticList_[modeIdx]);
            } else {
                drawQueenCobraStation(isNightOrSunset);
                queenCobraTrack.draw(false, lightingEnabled);
            }
        } else {
            drawQueenCobraStation(isNightOrSunset);
            queenCobraTrack.draw(true, lightingEnabled);
        }
        // Đoàn tàu vẫn vẽ trực tiếp mỗi khung
        queenCobraTrain.draw(queenCobraTrack, showNormalsAndCtrl, lightingEnabled, isNightOrSunset);
        glPopMatrix();

        // 2. Vẽ cụm Tàu điện Monorail (~1.87 km) cùng 2 nhà ga (Ga Nam & Ga Nepal)
        if (!showNormalsAndCtrl) {
            const int modeIdx = isNightOrSunset ? 1 : 0;
            if (mrStaticList_[modeIdx] == 0) {
                mrStaticList_[modeIdx] = glGenLists(1);
                if (mrStaticList_[modeIdx] != 0) {
                    glNewList(mrStaticList_[modeIdx], GL_COMPILE);
                    drawMonorailStation(isNightOrSunset);
                    monorailTrack.draw(false, true);
                    glEndList();
                }
            }
            if (mrStaticList_[modeIdx] != 0) {
                glCallList(mrStaticList_[modeIdx]);
            } else {
                drawMonorailStation(isNightOrSunset);
                monorailTrack.draw(false, lightingEnabled);
            }
        } else {
            drawMonorailStation(isNightOrSunset);
            monorailTrack.draw(true, lightingEnabled);
        }
        // Đoàn tàu Monorail vẫn vẽ trực tiếp mỗi khung
        monorailTrain.draw(monorailTrack, showNormalsAndCtrl, lightingEnabled, isNightOrSunset);
    }

private:
    mutable GLuint qcStaticList_[2] = { 0, 0 };
    mutable GLuint mrStaticList_[2] = { 0, 0 };
    static void drawQueenCobraStation(bool isNightOrSunset) {
        // Sàn đón khách dọc theo đoạn thẳng 0: đường ray tại x = -68.0, z thuộc [26..52], y = 6.0
        const float cx = -68.0f;
        const float cz = 39.0f;

        // Bệ nhà ga bê tông nằm về phía +X của đường ray (x thuộc [-66.7 .. -62.1])
        glColor3f(0.42f, 0.44f, 0.48f);
        CoasterMesh::drawBox(cx + 3.6f, 2.7f, cz, 4.6f, 5.4f, 26.0f);

        // Sàn chờ nhô ra (cách an toàn với thân toa tàu x >= -67.0)
        glColor3f(0.68f, 0.24f, 0.16f);
        CoasterMesh::drawBox(cx + 3.5f, 5.55f, cz, 4.9f, 0.35f, 27.0f);

        // Hệ cột thép đỡ mái vòm cắm xuống đất tại y = 0 (bên -X) & dọc mép sàn ga (bên +X)
        glColor3f(0.78f, 0.78f, 0.82f);
        for (int zSide = -1; zSide <= 1; ++zSide) {
            // Cột cao bên ngoài cách xa đường ray (-70.6)
            CoasterMesh::drawBox(cx - 2.6f, 5.05f, cz + zSide * 10.5f, 0.38f, 10.1f, 0.38f);
            // Cột phía sàn nhà ga (mép ngoài +X tại -62.4)
            CoasterMesh::drawBox(cx + 5.6f, 7.80f, cz + zSide * 10.5f, 0.38f,  4.6f, 0.38f);
        }

        // Mái che nhà ga che phủ đường ray & sàn đón khách
        glColor3f(0.88f, 0.32f, 0.12f);
        CoasterMesh::drawBox(cx + 1.5f, 10.15f, cz, 9.2f, 0.36f, 27.5f);

        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.8f, 48.0f, 1.0f, 0.78f, 0.32f);
            glColor3f(1.0f, 0.85f, 0.42f);
            CoasterMesh::drawBox(cx + 1.5f, 9.91f, cz, 7.6f, 0.12f, 24.0f);
            LightingSystem::resetMaterial();
        }
    }

    static void drawSingleMonorailStationBuilding(
        const MathCG::Vec3& stationPos,
        float rotYDeg,
        bool isNightOrSunset
    ) {
        glPushMatrix();
        glTranslatef(stationPos.x, stationPos.y, stationPos.z);
        glRotatef(rotYDeg, 0.0f, 1.0f, 0.0f);

        // Tháp sảnh nhà ga & sàn đón khách trên cao tách biệt an toàn với toa tàu monorail (cách tim ray +4.6m)
        glColor3f(0.56f, 0.60f, 0.65f);
        CoasterMesh::drawBox(4.6f, 6.2f, 0.0f, 5.0f, 12.4f, 22.0f);

        // Sàn viền màu xanh cổ vịt của nhà ga
        glColor3f(0.06f, 0.54f, 0.66f);
        CoasterMesh::drawBox(4.4f, 12.55f, 0.0f, 5.8f, 0.40f, 23.0f);

        // Các cột đỡ mái vòm hiện đại cắm xuống đất ở cả hai bên dầm dẫn hướng
        glColor3f(0.84f, 0.86f, 0.90f);
        for (int zSide = -1; zSide <= 1; ++zSide) {
            CoasterMesh::drawBox(-2.2f,  8.90f, zSide * 9.0f, 0.42f, 17.8f, 0.42f);
            CoasterMesh::drawBox( 6.8f, 15.20f, zSide * 9.0f, 0.42f,  5.2f, 0.42f);
        }
        glColor3f(0.92f, 0.94f, 0.96f);
        CoasterMesh::drawBox(2.3f, 17.8f, 0.0f, 9.8f, 0.38f, 24.0f);

        if (isNightOrSunset) {
            LightingSystem::setMaterial(0.8f, 48.0f, 0.25f, 0.88f, 1.0f);
            glColor3f(0.30f, 0.90f, 1.0f);
            CoasterMesh::drawBox(2.3f, 17.55f, 0.0f, 8.2f, 0.12f, 21.0f);
            LightingSystem::resetMaterial();
        }
        glPopMatrix();
    }

    static void drawMonorailStation(bool isNightOrSunset) {
        // 1. Nhà ga Monorail Nam (gần Cổng chính Ngọ Môn: MONORAIL_SOUTH_STATION_POS (-24.6, 0, +128.0))
        drawSingleMonorailStationBuilding(
            ParkLayout::MONORAIL_SOUTH_STATION_POS,
            ParkLayout::MONORAIL_SOUTH_STATION_ROT_Y_DEG,
            isNightOrSunset
        );
        // 2. Nhà ga Monorail Nepal (khu Tây-Bắc: MONORAIL_NEPAL_STATION_POS (-217.2, 0, -220.6))
        drawSingleMonorailStationBuilding(
            ParkLayout::MONORAIL_NEPAL_STATION_POS,
            ParkLayout::MONORAIL_NEPAL_STATION_ROT_Y_DEG,
            isNightOrSunset
        );
    }
};

#endif // ROLLER_COASTER_H
