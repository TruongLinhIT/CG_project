#ifndef SUN_WHEEL_H
#define SUN_WHEEL_H

/**
 * @file SunWheel.h
 * @brief Chương 1, 4 & 5: Mô phỏng Vòng quay Mặt Trời Đà Nẵng cao 115m (Da Nang Sun Wheel)
 *
 * Triển khai:
 *  - Thông số thực tế tại Công viên Châu Á (Asia Park Đà Nẵng):
 *      * Đường kính = 100m (Bán kính R = 50m), Chiều cao tâm trục H_center = 65m (Tổng chiều cao = 115m)
 *      * Tọa độ tâm: I(0, 65, -38)
 *      * 32 nan hoa LED hướng tâm, 4 vòng LED cầu vồng đồng tâm, cặp vành khuyên ngoài dạng Toroid
 *      * 64 cabin chở khách (góc lệch delta = 5.625 độ)
 *  - Chương 4 Mô hình hóa phân cấp & Phép biến đổi tọa độ:
 *      * Phép quay vòng quay chính quanh tâm I:
 *        M_wheel = T(x0, y0, z0) * R_z(alpha) * T(-x0, -y0, -z0)
 *      * Treo cabin phân cấp & duy trì trạng thái thẳng đứng:
 *        Tại góc nan hoa beta_k = k * (360 / N), vị trí chốt treo cách tâm một khoảng R.
 *        Phép quay ngược cục bộ: R_z(-(beta_k + alpha)) triệt tiêu vận tốc góc tổng hợp,
 *        đảm bảo sàn cabin luôn nằm ngang và thẳng đứng tuyệt đối ở mọi vị trí!
 *      * Truy vấn tọa độ thế giới cho Camera góc nhìn thứ nhất (POV) trong cabin phục vụ Camera.h
 *  - Chương 1 Mô hình màu HSV sang RGB:
 *      * Hiệu ứng chuyển màu LED động lan tỏa trên 32 nan hoa & 5 vòng qua hàm HSVtoRGB
 *  - Chương 5 Mặt cong tham số:
 *      * Vành khuyên ngoài & cổ trục trung tâm qua ParametricShapes::drawToroid
 *      * Các vòng LED cầu vồng phân đoạn qua ParametricShapes::drawSegmentedLedToroid
 *      * Kết cấu giàn ống thép qua ParametricShapes::drawCylinderBetween
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
    float        radius           = 50.0f;  // R = 50m (Đường kính 100m)
    int          numSpokes        = 32;     // 32 nan hoa chịu lực hướng tâm
    int          numCabins        = 64;     // 64 cabin chở khách (cách đều 5.625 độ)
    float        rotationSpeed    = 12.0f;  // Tốc độ quay tính bằng độ/giây
    float        currentAngleDeg  = 0.0f;   // Góc quay hiện tại của vòng quay alpha

    SunWheel() = default;

    /**
     * @brief Cập nhật bộ đếm thời gian mô phỏng
     */
    void update(float dt) {
        currentAngleDeg = std::fmod(currentAngleDeg + rotationSpeed * dt, 360.0f);
    }

    /**
     * @brief Chương 4: Tính toán chính xác vị trí 3D trong hệ tọa độ thế giới của cabin thứ #index
     *        P_cabin = I + R * (cos(beta_k + alpha), sin(beta_k + alpha), 0) + offset
     *        Dùng cho Camera góc nhìn thứ nhất (POV) trên trò chơi trong Camera.h
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
     * @brief Vẽ toàn bộ tổ hợp Vòng quay Mặt Trời (Sun Wheel)
     * @param ledHueDeg Góc màu H trong khoảng [0, 360) cho hiệu ứng chuyển màu HSV sang RGB
     * @param isNightOrSunset Điều khiển hiệu ứng phát sáng LED ban đêm và đèn neon
     * @param showNormals Hiển thị trực quan vector pháp tuyến bề mặt để gỡ lỗi
     */
    void draw(float ledHueDeg, bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();

        // 1. Tháp đỡ chữ A giàn không gian tĩnh, bệ móng & biển hiệu "SUN WHEEL"
        drawSupportTower(isNightOrSunset);
        drawNeonSign(isNightOrSunset);

        // 2. Tổ hợp vòng quay phân cấp: M = T(I) * R_z(alpha) * T(-I)
        MathCG::Mat4 wheelRot = MathCG::Mat4::rotateZAroundPoint(center, currentAngleDeg);
        glMultMatrixf(wheelRot.m);
        glTranslatef(center.x, center.y, center.z);

        drawWheelStructure(ledHueDeg, isNightOrSunset, showNormals);
        drawCabins(currentAngleDeg, isNightOrSunset, showNormals);

        glPopMatrix();
    }

private:
    /**
     * @brief Tháp đỡ chữ A giàn không gian & các trụ móng bê tông dưới lòng hồ
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

        // Các trụ móng bê tông cốt thép
        LightingSystem::setMaterial(0.20f, 16.0f);
        glColor3f(0.48f, 0.50f, 0.54f);
        for (int sz = -1; sz <= 1; sz += 2) {
            for (int sx = -1; sx <= 1; sx += 2) {
                MathCG::Vec3 basePt = legPointAtY(sz, sx, baseY);
                drawBox(basePt.x, 1.35f, basePt.z, 5.4f, 3.8f, 5.4f);
                drawBox(basePt.x, 3.30f, basePt.z, 3.8f, 0.5f, 3.8f);
            }
        }

        // Chân tháp chữ A bằng thép giàn không gian & các thanh giằng ngang (mặt phẳng trước & sau)
        LightingSystem::setMaterial(0.65f, 48.0f);
        glColor3f(0.84f, 0.86f, 0.90f);
        for (int sz = -1; sz <= 1; sz += 2) {
            MathCG::Vec3 hubEnd(center.x, center.y, center.z + sz * topSpreadZ);
            MathCG::Vec3 leftBase  = legPointAtY(sz, -1, baseY);
            MathCG::Vec3 rightBase = legPointAtY(sz,  1, baseY);

            // Các cột chân nghiêng chính
            ParametricShapes::drawCylinderBetween(leftBase,  hubEnd, 1.05f, 0.78f, 10, false);
            ParametricShapes::drawCylinderBetween(rightBase, hubEnd, 1.05f, 0.78f, 10, false);

            // Các dầm ngang & nhịp giằng chéo chữ X
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

        // Trục thép trung tâm chính nối ổ đỡ phía trước và phía sau
        ParametricShapes::drawCylinderBetween(
            MathCG::Vec3(center.x, center.y, center.z - topSpreadZ - 1.2f),
            MathCG::Vec3(center.x, center.y, center.z + topSpreadZ + 1.2f),
            1.45f, 1.45f, 14, true
        );
        LightingSystem::resetMaterial();
    }

    /**
     * @brief Biển hiệu 3D Neon "SUN WHEEL" gắn trên dầm ngang khung chữ A phía trước
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

        // Thanh khung đỡ phía sau biển hiệu
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
     * @brief Kết cấu phần quay của vòng quay: Trục trung tâm, Vành ngoài, Vòng LED đồng tâm, 32 Nan hoa
     */
    void drawWheelStructure(float ledHueDeg, bool isNightOrSunset, bool showNormals) const {
        // 1. Đĩa trục trung tâm phát sáng
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

        // 2. Cặp vành chịu lực ngoài cùng (Bán kính = 49.45m, mặt trước & mặt sau z = +/- 2.1m)
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

        // 3. Các vòng kết cấu LED cầu vồng đồng tâm (Chương 1 & 5)
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

        // 4. 32 Nan hoa hướng tâm với dải đèn LED HSV chuyển màu nối tiếp
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

            // Thanh giằng ngang nối vành trước & vành sau tại mỗi nan hoa
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
     * @brief 64 Cabin chở khách áp dụng phép quay ngược phân cấp Chương 4 -(beta_k + alpha)
     */
    void drawCabins(float alpha, bool isNightOrSunset, bool showNormals) const {
        for (int k = 0; k < numCabins; ++k) {
            float cabinAngleDeg = (360.0f / numCabins) * k;

            glPushMatrix();
            // Bước 1: Định vị cabin dọc theo nan hoa tại khoảng cách R
            glRotatef(cabinAngleDeg, 0.0f, 0.0f, 1.0f);
            glTranslatef(radius, 0.0f, 0.0f);

            // Bước 2: PHÉP QUAY NGƯỢC PHÂN CẤP BẮT BUỘC:
            // Triệt tiêu cả góc nan hoa của cabin (beta_k) lẫn góc quay của bánh xe (alpha)
            // để mọi cabin luôn treo thẳng đứng hoàn hảo theo phương trọng lực thế giới!
            glRotatef(-(cabinAngleDeg + alpha), 0.0f, 0.0f, 1.0f);

            drawSingleCabin(isNightOrSunset, showNormals);

            glPopMatrix();
        }
    }

    /**
     * @brief Mô hình 3D chi tiết của một Cabin chở khách
     */
    void drawSingleCabin(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;

        // Thanh treo cabin & chốt trục xoay
        glColor3f(0.78f, 0.80f, 0.85f);
        drawBox(0.0f, -0.65f, 0.0f, 0.14f, 1.30f, 0.14f);

        // Nắp mui khí động học
        glColor3f(0.95f, 0.72f, 0.15f); // Viền vàng kim
        drawBox(0.0f, -1.05f, 0.0f, 1.75f, 0.22f, 1.40f);

        // Thân chính của cabin màu đỏ thẫm
        LightingSystem::setMaterial(0.55f, 40.0f);
        glColor3f(0.86f, 0.12f, 0.14f);
        drawBox(0.0f, -1.95f, 0.0f, 1.85f, 1.75f, 1.50f);

        // Cửa sổ kính toàn cảnh (Phát ánh sáng vàng ấm áp vào ban đêm)
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
    // CÁC HÀM HỖ TRỢ VẼ HÌNH HỘP THỦ TỤC & KÝ TỰ ĐÈN NEON
    // ========================================================================

    static void drawBox(float cx, float cy, float cz, float sx, float sy, float sz) {
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

        glBegin(GL_QUADS);
        // Mặt trước +Z
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);

        // Mặt sau -Z
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);

        // Mặt trên +Y
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        // Mặt dưới -Y
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);

        // Mặt trái -X
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz - hz);

        // Mặt phải +X
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
