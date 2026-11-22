#ifndef SKY_TOWER_H
#define SKY_TOWER_H

/**
 * @file SkyTower.h
 * @brief Chương 4 & 5: Mô phỏng Tháp Rơi Tự Do Golden Sky Tower cao 47m tại Asia Park
 *
 * Triển khai:
 *  - Thông số thực tế của Golden Sky Tower:
 *      * Chiều cao H = 47m, Tọa độ chân tháp: (76.0, 0.0, -22.0) tại Quảng trường Phiêu lưu phía Đông
 *      * Cột tháp: Hình trụ tham số với các thanh ray dẫn hướng bằng thép thẳng đứng & nắp chụp máy móc trên đỉnh
 *      * Đèn chớp cảnh báo hàng không tại đỉnh tháp
 *  - Chương 4 Mô hình hóa phân cấp & Chu kỳ vật lý động học:
 *      * Cấp 1: Cột trụ chịu lực trung tâm tĩnh & bệ móng hình bát giác
 *      * Cấp 2: Vòng Gondola chở khách (Toroid + 16 ghế ngồi hướng ra ngoài)
 *        thực hiện chuyển động tịnh tiến thẳng đứng tuần hoàn đa pha Y(t) và quay phương vị:
 *          1. Kéo tời lên chậm        (0.0s -> 13.0s): Leo lên với tốc độ ~3 m/s kèm góc quay toàn cảnh 360 độ
 *          2. Giữ hồi hộp ở đỉnh      (13.0s -> 15.5s): Treo lơ lửng ở độ cao 42m (tạm dừng tạo kịch tính)
 *          3. Rơi tự do tốc độ cao    (15.5s -> 18.0s): Lao xuống dưới gia tốc trọng trường
 *          4. Hãm phanh từ trường     (18.0s -> 20.0s): Giảm tốc êm ái bằng dòng điện xoáy (eddy-current)
 *          5. Tạm dừng đón trả khách  (20.0s -> 22.0s): Nghỉ tại sàn ga trước khi bắt đầu chu kỳ tiếp theo
 *  - Chương 5 Mặt cong tham số:
 *      * Cột tháp trung tâm: ParametricShapes::drawCylinderY
 *      * Đai kết cấu vòng Gondola: ParametricShapes::drawToroidY
 *      * Chóp đỉnh máy móc & mái vòm: ParametricShapes::drawConeY
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"

namespace AsiaParkRides {

class SkyTower {
public:
    MathCG::Vec3 position       = MathCG::Vec3(76.0f, 0.0f, -22.0f);
    float        totalHeight    = 47.0f;
    float        towerRadius    = 1.65f;
    float        gondolaRadius  = 3.35f;
    int          numSeats       = 16;
    float        cycleTime      = 0.0f;  // Đồng hồ hoạt ảnh nội bộ
    float        currentY       = 3.5f;  // Độ cao hiện tại của vòng Gondola
    float        currentYawDeg  = 0.0f;  // Góc quay hiện tại của vòng Gondola

    SkyTower() = default;

    /**
     * @brief Cập nhật vật lý: Điều khiển chu kỳ rơi tự do đa pha
     */
    void update(float dt) {
        cycleTime = std::fmod(cycleTime + dt, 22.0f);

        const float minY = 3.5f;
        const float maxY = 42.0f;
        const float liftDuration = 13.0f;
        const float holdDuration = 2.5f;
        const float dropDuration = 2.5f;
        const float brakeDuration = 2.0f;

        float t = cycleTime;

        if (t < liftDuration) {
            // Pha 1: Kéo tời lên chậm kèm xoay toàn cảnh 360 độ
            float u = t / liftDuration;
            // Nội suy trơn (Smooth-step ease-in / ease-out)
            float ease = u * u * (3.0f - 2.0f * u);
            currentY = minY + (maxY - minY) * ease;
            currentYawDeg = u * 360.0f * 1.5f;
        }
        else if (t < liftDuration + holdDuration) {
            // Pha 2: Giữ hồi hộp ở đỉnh (treo lơ lửng trên cao)
            currentY = maxY;
            // Xoay nhẹ tại chỗ khi chờ rơi
            float holdT = t - liftDuration;
            currentYawDeg += holdT * 4.0f * dt;
        }
        else if (t < liftDuration + holdDuration + dropDuration) {
            // Pha 3: Rơi tự do dưới tác dụng trọng lực: y(t) = maxY - 0.5 * g_eff * dt^2
            float dropT = t - (liftDuration + holdDuration);
            float u = dropT / dropDuration;
            // Gia tốc rơi bậc hai hướng xuống
            float fallDist = (maxY - (minY + 6.0f)) * (u * u);
            currentY = maxY - fallDist;
        }
        else if (t < liftDuration + holdDuration + dropDuration + brakeDuration) {
            // Pha 4: Đệm hãm phanh từ trường bằng dòng điện xoáy
            float brakeT = t - (liftDuration + holdDuration + dropDuration);
            float u = brakeT / brakeDuration;
            float ease = 1.0f - (1.0f - u) * (1.0f - u);
            currentY = (minY + 6.0f) - 6.0f * ease;
        }
        else {
            // Pha 5: Tạm dừng đón trả khách tại sàn ga
            currentY = minY;
        }
    }

    /**
     * @brief Lấy tọa độ thế giới của ghế hành khách (phục vụ camera góc nhìn trên trò chơi)
     */
    MathCG::Vec3 getPassengerWorldPosition(int seatIndex = 0) const {
        float seatAngleDeg = currentYawDeg + (360.0f / numSeats) * seatIndex;
        float rad = seatAngleDeg * MathCG::DEG2RAD;
        return MathCG::Vec3(
            position.x + gondolaRadius * std::cos(rad),
            position.y + currentY + 0.95f,
            position.z + gondolaRadius * std::sin(rad)
        );
    }

    /**
     * @brief Vẽ Tháp Rơi Tự Do Golden Sky Tower
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ====================================================================
        // 1. Bệ móng tĩnh & Sàn chờ đón khách
        // ====================================================================
        // Bục bê tông hình bát giác
        glColor3f(0.42f, 0.44f, 0.48f);
        ParametricShapes::drawCylinderY(7.2f, 1.2f, 8, 2, true, showNormals);
        glColor3f(0.85f, 0.70f, 0.18f); // Vòng viền mạ vàng quanh chu vi
        ParametricShapes::drawToroidY(7.25f, 0.15f, 16, 8, showNormals);

        // Bậc thang sàn lên tàu & các cột lan can bảo vệ xung quanh
        glColor3f(0.35f, 0.38f, 0.42f);
        for (int i = 0; i < 8; ++i) {
            float u = (static_cast<float>(i) / 8) * MathCG::TWO_PI;
            float px = 7.0f * std::cos(u);
            float pz = 7.0f * std::sin(u);
            ParametricShapes::drawCylinderY(0.12f, 2.2f, 8, 2, true, false);
            glPushMatrix();
            glTranslatef(px, 0.0f, pz);
            ParametricShapes::drawCylinderY(0.12f, 2.2f, 8, 2, true, false);
            glPopMatrix();
        }

        // ====================================================================
        // 2. Cột tháp kết cấu trung tâm (H = 47m)
        // ====================================================================
        // Thân cột chính tông màu trắng-vàng (Hình trụ tham số)
        LightingSystem::setMaterial(0.60f, 48.0f);
        glColor3f(0.92f, 0.94f, 0.96f);
        ParametricShapes::drawCylinderY(towerRadius, totalHeight, 24, 16, true, showNormals);

        // 4 Thanh ray dẫn hướng bằng thép chạy dọc toàn bộ chiều cao tháp
        glColor3f(0.24f, 0.26f, 0.30f);
        for (int i = 0; i < 4; ++i) {
            float u = (static_cast<float>(i) / 4) * MathCG::TWO_PI;
            float rx = std::cos(u);
            float rz = std::sin(u);
            MathCG::Vec3 railBase(rx * (towerRadius + 0.12f), 1.2f, rz * (towerRadius + 0.12f));
            MathCG::Vec3 railTop(rx * (towerRadius + 0.12f), totalHeight, rz * (towerRadius + 0.12f));
            ParametricShapes::drawCylinderBetween(railBase, railTop, 0.14f, 0.14f, 6, false);
        }

        // Các vòng tròn trang trí mạ vàng bao quanh thân tháp
        glColor3f(0.95f, 0.76f, 0.12f);
        const float ringY[] = { 10.0f, 20.0f, 30.0f, 40.0f };
        for (float y : ringY) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(towerRadius + 0.08f, 0.10f, 24, 8, false);
            glPopMatrix();
        }

        // ====================================================================
        // 3. Mũ chụp buồng máy trên đỉnh & Đèn chớp cảnh báo hàng không
        // ====================================================================
        glPushMatrix();
        glTranslatef(0.0f, totalHeight, 0.0f);

        // Cổ trụ bao quanh buồng máy
        glColor3f(0.88f, 0.72f, 0.16f); // Vương miện vàng
        ParametricShapes::drawCylinderY(towerRadius + 0.45f, 2.4f, 16, 2, true, showNormals);

        // Chóp nón trên đỉnh tháp
        glTranslatef(0.0f, 2.4f, 0.0f);
        glColor3f(0.95f, 0.32f, 0.14f);
        ParametricShapes::drawConeY(towerRadius + 0.45f, 0.0f, 3.2f, 16, 4, true, showNormals);

        // Đèn hiệu nhấp nháy cảnh báo hàng không tại điểm cao nhất
        glTranslatef(0.0f, 3.35f, 0.0f);
        if (isNightOrSunset) {
            // Chớp sáng đỏ theo chu kỳ
            float strobe = std::fmod(cycleTime * 3.0f, 1.0f);
            float intensity = (strobe < 0.25f) ? 1.0f : 0.15f;
            GLfloat emit[4] = { 1.0f * intensity, 0.1f * intensity, 0.1f * intensity, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(1.0f * intensity, 0.1f * intensity, 0.1f * intensity);
        } else {
            glColor3f(0.90f, 0.15f, 0.15f);
        }
        ParametricShapes::drawSphere(0.45f, 12, 10, false);
        if (isNightOrSunset) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
        glPopMatrix();

        // ====================================================================
        // 4. Khung Gondola chở khách động học phân cấp
        // ====================================================================
        glPushMatrix();
        // Bước 1: Tịnh tiến theo phương thẳng đứng Y(t)
        glTranslatef(0.0f, currentY, 0.0f);
        // Bước 2: Quay phương vị quanh trục đứng Yaw(t)
        glRotatef(currentYawDeg, 0.0f, 1.0f, 0.0f);

        drawGondolaRing(isNightOrSunset, showNormals);

        glPopMatrix();

        glPopMatrix();
    }

private:
    /**
     * @brief Vòng Gondola: Đai Toroid, 16 ghế hành khách hướng ra ngoài, mái che nắng
     */
    void drawGondolaRing(bool isNightOrSunset, bool showNormals) const {
        // 1. Ống lót kết cấu bên trong trượt dọc theo cột tháp
        glColor3f(0.18f, 0.20f, 0.24f);
        ParametricShapes::drawCylinderY(towerRadius + 0.22f, 2.1f, 20, 2, true, false);

        // 2. Vòng đỡ sàn hành khách chính (Hình xuyến tham số Toroid)
        LightingSystem::setMaterial(0.70f, 48.0f);
        glColor3f(0.96f, 0.78f, 0.12f); // Vòng vàng rực rỡ
        glPushMatrix();
        glTranslatef(0.0f, 0.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius, 0.32f, 32, 10, showNormals);
        glPopMatrix();

        // 3. Vòng mái che nắng hình tròn phía trên đầu
        glColor3f(0.92f, 0.28f, 0.14f); // Mái vòm đỏ son Á Đông
        glPushMatrix();
        glTranslatef(0.0f, 2.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius + 0.15f, 0.22f, 32, 8, false);
        glPopMatrix();

        // 4. 16 Ghế hành khách hướng mặt ra ngoài bố trí quanh vòng tròn
        for (int i = 0; i < numSeats; ++i) {
            float angleDeg = (360.0f / numSeats) * i;
            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, gondolaRadius);

            // Giá đỡ ghế nối vào đai ống lót bên trong
            glColor3f(0.35f, 0.38f, 0.42f);
            ParametricShapes::drawCylinderBetween(
                MathCG::Vec3(0.0f, 0.35f, -gondolaRadius + towerRadius + 0.25f),
                MathCG::Vec3(0.0f, 0.35f, 0.0f),
                0.08f, 0.08f, 6, false
            );

            // Đệm ghế thể thao đúc khuôn (Màu đỏ thẫm)
            glColor3f(0.85f, 0.14f, 0.12f);
            // Mặt ghế ngồi
            drawLocalBox(0.0f, 0.55f, 0.0f, 0.65f, 0.14f, 0.55f);
            // Tựa lưng ghế
            drawLocalBox(0.0f, 1.05f, -0.24f, 0.62f, 0.90f, 0.12f);
            // Tựa đầu
            drawLocalBox(0.0f, 1.62f, -0.22f, 0.42f, 0.28f, 0.14f);

            // Đai an toàn vắt qua vai (Màu vàng)
            glColor3f(0.98f, 0.82f, 0.15f);
            drawLocalBox(-0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);
            drawLocalBox( 0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);

            // Thanh gác chân treo bên dưới
            glColor3f(0.50f, 0.52f, 0.56f);
            drawLocalBox(0.0f, 0.05f, 0.12f, 0.50f, 0.06f, 0.20f);

            glPopMatrix();
        }

        // Đèn LED chiếu gầm ban đêm bên dưới vòng Gondola
        if (isNightOrSunset) {
            GLfloat emit[4] = { 0.2f, 0.8f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(0.2f, 0.8f, 1.0f);
            glPushMatrix();
            glTranslatef(0.0f, 0.05f, 0.0f);
            ParametricShapes::drawToroidY(gondolaRadius, 0.08f, 24, 6, false);
            glPopMatrix();
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }

        LightingSystem::resetMaterial();
    }

    static void drawLocalBox(float cx, float cy, float cz, float sx, float sy, float sz) {
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

#endif // SKY_TOWER_H
