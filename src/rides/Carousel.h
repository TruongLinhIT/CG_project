#ifndef CAROUSEL_H
#define CAROUSEL_H

/**
 * @file Carousel.h
 * @brief Chương 4 & 5: Mô phỏng Đu Quay Ngựa Gỗ Lễ Hội (Festival Carousel tại Asia Park)
 *
 * Triển khai:
 *  - Thông số thực tế của Đu Quay Lễ Hội (Festival Carousel):
 *      * Bán kính R = 8.5m, Chiều cao H = 8.8m
 *      * Vị trí: (45.0, 0.0, 48.0) tại Quảng trường Lễ hội phía Đông Nam
 *      * Các chú ngựa gỗ phi nước đại xếp trên 2 vòng đồng tâm (Vòng ngoài R=6.2m, Vòng trong R=4.2m)
 *  - Chương 4 Mô hình hóa phân cấp (Cấu trúc phân cấp biến đổi 2 cấp):
 *      * Cấp 1 (Cha - Parent): Sàn gỗ tròn xoay, trụ lõi gương trung tâm
 *        và mái vòm hình nón lễ hội cùng quay quanh trục Y thế giới theo góc gamma = carouselAngleDeg:
 *        M_parent = T(pos) * R_y(gamma)
 *      * Cấp 2 (Con - Child): Các cột đồng thau và các chú ngựa kế thừa góc quay cha gamma,
 *        cộng thêm dao động điều hòa thẳng đứng độc lập dọc theo từng cột:
 *        y_i(t) = y_base + A * sin(omega * t + phi_i)
 *        với pha lệch nhau phi_i = i * (2*pi / N) tạo nên hiệu ứng lượn sóng phi nước đại sống động!
 *  - Chương 5 Mặt cong tham số:
 *      * Mái vòm hình nón lớn: ParametricShapes::drawConeY
 *      * Các cột đồng thau thẳng đứng: ParametricShapes::drawCylinderY
 *      * Vòng viền mạ vàng: ParametricShapes::drawToroidY
 *      * Quả cầu trang trí trên đỉnh mái: ParametricShapes::drawSphere
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
    float        rotationSpeed  = 35.0f; // độ/giây
    float        currentAngleDeg= 0.0f;  // gamma: góc quay của sàn đu quay
    float        oscillationPhase= 0.0f; // omega * t: pha nhấp nhô của ngựa gỗ
    int          numHorses      = 12;    // 12 chú ngựa gỗ phi nước đại

    Carousel() = default;

    /**
     * @brief Cập nhật bộ đếm thời gian mô phỏng cho cả chuyển động quay và nhịp nhấp nhô của ngựa
     */
    void update(float dt) {
        currentAngleDeg  = std::fmod(currentAngleDeg + rotationSpeed * dt, 360.0f);
        oscillationPhase = std::fmod(oscillationPhase + 3.2f * dt, MathCG::TWO_PI);
    }

    /**
     * @brief Lấy tọa độ thế giới của yên ngựa phục vụ camera góc nhìn thứ nhất trên lưng ngựa
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
     * @brief Vẽ toàn bộ tổ hợp Đu Quay Ngựa Gỗ Lễ Hội
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ====================================================================
        // 1. Bệ nền bê tông hình tròn tĩnh dưới mặt đất
        // ====================================================================
        glColor3f(0.48f, 0.50f, 0.54f);
        ParametricShapes::drawCylinderY(platformRadius + 0.8f, 0.35f, 28, 2, true, false);
        glColor3f(0.85f, 0.70f, 0.18f); // Vòng viền vàng ngoài cùng
        ParametricShapes::drawToroidY(platformRadius + 0.82f, 0.10f, 32, 6, false);

        // ====================================================================
        // 2. CẤP 1 (CHA - PARENT): Sàn gỗ xoay, Cột trung tâm & Mái vòm hình nón
        // ====================================================================
        glPushMatrix();
        glRotatef(currentAngleDeg, 0.0f, 1.0f, 0.0f); // Quay quanh trục đứng theo góc gamma

        // Sàn gỗ đánh bóng đang xoay
        glColor3f(0.68f, 0.42f, 0.22f); // Màu gỗ tếch
        ParametricShapes::drawCylinderY(platformRadius, 0.55f, 28, 2, true, showNormals);
        glColor3f(0.95f, 0.80f, 0.15f); // Viền vàng quanh mép sàn
        ParametricShapes::drawToroidY(platformRadius + 0.05f, 0.12f, 32, 8, false);

        // Cột lõi trung tâm chạm khắc hoa văn với các đường gờ mạ vàng
        glColor3f(0.88f, 0.85f, 0.80f); // Màu đá cẩm thạch kem
        ParametricShapes::drawCylinderY(2.1f, canopyHeight - 2.5f, 16, 4, true, showNormals);
        glColor3f(0.92f, 0.75f, 0.16f); // Các đai vàng quanh cột trung tâm
        for (float y = 1.5f; y < canopyHeight - 2.5f; y += 1.8f) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(2.15f, 0.12f, 20, 6, false);
            glPopMatrix();
        }

        // ====================================================================
        // 3. Mái vòm hình nón (ParametricShapes::drawConeY)
        // ====================================================================
        glPushMatrix();
        glTranslatef(0.0f, canopyHeight - 2.5f, 0.0f);

        // Diềm mái trang trí xung quanh hiên
        glColor3f(0.95f, 0.75f, 0.15f);
        ParametricShapes::drawCylinderY(platformRadius + 0.45f, 0.65f, 28, 2, true, false);

        // Mái hình nón chính (Mái lều phong cách Á Đông tông đỏ thẫm và ngà)
        glColor3f(0.88f, 0.18f, 0.14f); // Đỏ Á Đông
        ParametricShapes::drawConeY(platformRadius + 0.45f, 0.0f, 3.2f, 28, 4, true, showNormals);

        // Chóp vương miện trang trí & quả cầu vàng trên đỉnh mái
        glTranslatef(0.0f, 3.2f, 0.0f);
        glColor3f(0.98f, 0.82f, 0.18f); // Vàng nguyên chất
        ParametricShapes::drawConeY(0.9f, 0.0f, 1.2f, 16, 2, true, false);
        glTranslatef(0.0f, 1.35f, 0.0f);
        ParametricShapes::drawSphere(0.48f, 16, 12, false);

        glPopMatrix();

        // Dải đèn trang trí ánh sáng vàng ấm quanh hiên mái vào ban đêm
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
        // 4. CẤP 2 (CON - CHILDREN): 12 Ngựa gỗ phi nước đại + Cột đồng thẳng đứng
        // ====================================================================
        for (int i = 0; i < numHorses; ++i) {
            float angleDeg = (360.0f / numHorses) * i;
            float ringR    = (i % 2 == 0) ? 6.2f : 4.2f; // Xen kẽ 2 vòng trong và ngoài
            float phaseOffset = (MathCG::TWO_PI / numHorses) * i;

            // Dao động điều hòa theo phương thẳng đứng: y(t) = y_base + A * sin(omega*t + phi_i)
            float horseY = 1.35f + 0.45f * std::sin(oscillationPhase + phaseOffset);

            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(ringR, 0.0f, 0.0f);

            // Cấp 2.1: Cột đỡ bằng đồng thau gắn cố định giữa sàn và trần mái
            glColor3f(0.92f, 0.78f, 0.22f); // Đồng thau đánh bóng
            ParametricShapes::drawCylinderY(0.065f, canopyHeight - 2.5f, 10, 4, true, false);

            // Cấp 2.2: Ngựa gỗ phi nước đại trượt lên xuống dọc theo cột đồng
            glTranslatef(0.0f, horseY, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Hướng mặt theo phương tiếp tuyến chiều quay đu quay

            drawCarouselHorse(i, isNightOrSunset);

            glPopMatrix();
        }

        glPopMatrix(); // Kết thúc khối Cấp 1 (Cha)

        glPopMatrix();
    }

private:
    /**
     * @brief Mô hình 3D chi tiết của Ngựa gỗ với yên cương hoa văn, bờm và tư thế phi nước đại
     */
    void drawCarouselHorse(int index, bool isNightOrSunset) const {
        (void)isNightOrSunset;

        // Màu lông ngựa: xen kẽ giữa Trắng tuyết, Nâu hạt dẻ và Đen tuyền
        MathCG::ColorRGB coatCol;
        if (index % 3 == 0)      coatCol = MathCG::ColorRGB(0.95f, 0.95f, 0.97f); // Trắng
        else if (index % 3 == 1) coatCol = MathCG::ColorRGB(0.55f, 0.28f, 0.16f); // Nâu hạt dẻ
        else                     coatCol = MathCG::ColorRGB(0.18f, 0.18f, 0.20f); // Đen

        LightingSystem::setMaterial(0.55f, 32.0f);

        // 1. Thân mình ngựa
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        drawBox(0.0f, 0.0f, 0.0f, 0.55f, 0.45f, 1.35f);

        // 2. Phần mông / thân sau
        drawBox(0.0f, 0.06f, -0.65f, 0.50f, 0.42f, 0.45f);

        // 3. Ngực & vai trước
        drawBox(0.0f, 0.08f, 0.62f, 0.52f, 0.48f, 0.45f);

        // 4. Cổ ngựa uốn cong
        glPushMatrix();
        glTranslatef(0.0f, 0.48f, 0.75f);
        glRotatef(38.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.32f, 0.65f, 0.38f);
        glPopMatrix();

        // 5. Đầu & Mõm ngựa
        glPushMatrix();
        glTranslatef(0.0f, 0.85f, 0.98f);
        glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.30f, 0.32f, 0.42f); // Phần đầu
        // Phần mõm thon gọn
        glTranslatef(0.0f, -0.10f, 0.26f);
        drawBox(0.0f, 0.0f, 0.0f, 0.22f, 0.22f, 0.28f);
        // Hai tai ngựa
        glColor3f(coatCol.r * 0.9f, coatCol.g * 0.9f, coatCol.b * 0.9f);
        drawBox(-0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        drawBox( 0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        glPopMatrix();

        // 6. Bờm ngựa màu vàng óng
        glColor3f(0.96f, 0.78f, 0.15f); // Bờm vàng
        glPushMatrix();
        glTranslatef(0.0f, 0.62f, 0.65f);
        glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.18f, -0.12f, 0.12f, 0.55f, 0.18f);
        glPopMatrix();

        // 7. Yên ngựa mạ vàng & Thảm lót yên màu đỏ thẫm
        // Thảm lót yên màu đỏ thẫm
        glColor3f(0.85f, 0.12f, 0.14f);
        drawBox(0.0f, 0.18f, 0.0f, 0.58f, 0.14f, 0.75f);
        // Yên ngựa mạ vàng
        glColor3f(0.96f, 0.80f, 0.18f);
        drawBox(0.0f, 0.26f, 0.02f, 0.48f, 0.12f, 0.52f);
        // Gờ sau & mỏm trước của yên ngựa
        drawBox(0.0f, 0.35f, -0.20f, 0.42f, 0.16f, 0.10f);
        drawBox(0.0f, 0.35f,  0.22f, 0.38f, 0.14f, 0.09f);

        // 8. Cặp chân trước đang phi nước đại (vươn về phía trước)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.20f, -0.18f, 0.60f);
            glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // Vươn ra trước
            drawBox(0.0f, -0.22f, 0.0f, 0.12f, 0.48f, 0.14f);
            glTranslatef(0.0f, -0.42f, 0.0f);
            glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.10f, 0.42f, 0.11f);
            // Móng ngựa mạ vàng
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.12f, 0.10f, 0.14f);
            glPopMatrix();
        }

        // 9. Cặp chân sau đang đạp bật nhảy (duỗi về phía sau)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.19f, -0.16f, -0.62f);
            glRotatef(-35.0f, 1.0f, 0.0f, 0.0f); // Duỗi ra sau
            drawBox(0.0f, -0.22f, 0.0f, 0.14f, 0.50f, 0.16f);
            glTranslatef(0.0f, -0.46f, 0.0f);
            glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.11f, 0.42f, 0.12f);
            // Móng ngựa mạ vàng
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.13f, 0.10f, 0.15f);
            glPopMatrix();
        }

        // 10. Đuôi ngựa màu vàng tung bay
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
