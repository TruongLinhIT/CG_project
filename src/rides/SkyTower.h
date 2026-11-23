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
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"
#include "geometry/MeshBuilders.h"

namespace AsiaParkRides {

class SkyTower {
public:
    MathCG::Vec3 position       = ParkLayout::SKY_TOWER_POS; // (-260.0, 0.0, -54.6) [Ước lượng từ ảnh vệ tinh] ±10m
    float        totalHeight    = 47.0f;
    float        towerRadius    = 1.65f;
    float        gondolaRadius  = 3.35f;
    int          numSeats       = 16;
    float        cycleTime      = 0.0f;  // Đồng hồ hoạt ảnh nội bộ
    float        currentY       = 4.30f; // Độ cao hiện tại của vòng Gondola (Đợt 8b: khớp sàn đón khách +4.20m)
    float        currentYawDeg  = 0.0f;  // Góc quay hiện tại của vòng Gondola

    SkyTower() = default;

    /**
     * @brief Cập nhật vật lý: Điều khiển chu kỳ rơi tự do đa pha
     */
    void update(float dt) {
        cycleTime = std::fmod(cycleTime + dt, 22.0f);

        const float minY = 4.30f;
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
     * @brief Vẽ Tháp Rơi Tự Do Golden Sky Tower (Đợt 8b: Sàn đón +4.20m, nhà điều hành, cầu thang đôi, chòi đón & đỉnh búp sen vàng BP4)
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // 1. Phần tháp tĩnh, sàn đón +4.20m, cầu thang, nhà ga & búp sen vàng trên đỉnh (cache Display List khi không bật debug normals)
        if (!showNormals) {
            if (staticTowerList_ == 0) {
                staticTowerList_ = glGenLists(1);
                if (staticTowerList_ != 0) {
                    glNewList(staticTowerList_, GL_COMPILE);
                    drawStaticTowerAndStation(false);
                    glEndList();
                }
            }
            if (staticTowerList_ != 0) {
                glCallList(staticTowerList_);
            } else {
                drawStaticTowerAndStation(false);
            }
        } else {
            drawStaticTowerAndStation(true);
        }

        // 2. Đèn hiệu nhấp nháy cảnh báo hàng không trên đỉnh búp sen (y = totalHeight + 5.95m) & đèn hiên ban đêm
        glPushMatrix();
        glTranslatef(0.0f, totalHeight + 5.95f, 0.0f);
        if (isNightOrSunset) {
            float strobe = std::fmod(cycleTime * 3.0f, 1.0f);
            float intensity = (strobe < 0.25f) ? 1.0f : 0.18f;
            GLfloat emit[4] = { 1.0f * intensity, 0.12f * intensity, 0.10f * intensity, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(1.0f * intensity, 0.12f * intensity, 0.10f * intensity);
        } else {
            glColor3f(0.90f, 0.15f, 0.15f);
        }
        ParametricShapes::drawSphere(0.42f, 12, 10, false);
        if (isNightOrSunset) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
        glPopMatrix();

        // 3. Khung Gondola chở khách động học phân cấp
        glPushMatrix();
        glTranslatef(0.0f, currentY, 0.0f);
        glRotatef(currentYawDeg, 0.0f, 1.0f, 0.0f);
        drawGondolaRing(isNightOrSunset, showNormals);
        glPopMatrix();

        glPopMatrix();
    }

private:
    mutable GLuint staticTowerList_ = 0;

    void drawStaticTowerAndStation(bool showNormals) const {
        namespace MB = Geometry::MeshBuilders;

        // ====================================================================
        // 1. BỆ MÓNG BÁT GIÁC, PHÒNG ĐIỀU HÀNH TẦNG TRỆT, SÀN ĐÓN KHÁCH TRÒN +4.20M,
        //    CẦU THANG ĐÔI & CHÒI ĐÓN PHÍA ĐÔNG (GÓI GỌN TRONG R <= 8.0M)
        // ====================================================================
        Geometry::PolygonMesh baseMesh("SkyTowerBaseStationAndLotus");
        const MathCG::ColorRGB stoneDark(0.42f, 0.44f, 0.48f);
        const MathCG::ColorRGB stoneLight(0.66f, 0.64f, 0.58f);
        const MathCG::ColorRGB creamWall(0.90f, 0.85f, 0.74f);
        const MathCG::ColorRGB darkTrim(0.28f, 0.24f, 0.22f);
        const MathCG::ColorRGB pillarRed(0.72f, 0.20f, 0.15f);
        const MathCG::ColorRGB goldCol(0.94f, 0.76f, 0.16f);
        const MathCG::ColorRGB saffronGold(0.96f, 0.62f, 0.14f);
        const MathCG::ColorRGB lotusTipGold(0.98f, 0.84f, 0.24f);

        // 1.1. Bệ đá bát giác tầng trệt (apothem 7.1m -> R_vertex ~ 7.69m) & phòng điều hành bát giác dưới sàn
        MB::appendOctagonalPrism(baseMesh, 0.0f, 0.00f, 0.50f, 0.0f, 7.10f, stoneDark);
        MB::appendOctagonalPrism(baseMesh, 0.0f, 0.46f, 0.68f, 0.0f, 6.75f, stoneLight);
        MB::appendOctagonalPrism(baseMesh, 0.0f, 0.65f, 3.88f, 0.0f, 4.75f, creamWall);
        MB::appendOctagonalPrism(baseMesh, 0.0f, 1.65f, 2.95f, 0.0f, 4.82f, darkTrim); // băng cửa sổ phòng điều khiển

        // 8 cột trụ đỡ sàn tròn +4.20m
        for (int i = 0; i < 8; ++i) {
            float a = (22.5f + 45.0f * static_cast<float>(i)) * MathCG::DEG2RAD;
            float px = 6.05f * std::cos(a);
            float pz = 6.05f * std::sin(a);
            baseMesh.append(Geometry::PolygonMesh::createPrismY(px, 0.50f, pz, 0.30f, 3.40f, 10, pillarRed));
            baseMesh.append(Geometry::PolygonMesh::createPrismY(px, 0.50f, pz, 0.38f, 0.35f, 10, stoneLight));
        }

        // 1.2. Sàn đón khách trên cao +4.20m (y = 3.88 .. 4.20m, apothem 6.45m -> R_vertex ~ 6.98m)
        MB::appendOctagonalPrism(baseMesh, 0.0f, 3.88f, 4.06f, 0.0f, 6.55f, goldCol);
        MB::appendOctagonalPrism(baseMesh, 0.0f, 4.04f, 4.20f, 0.0f, 6.40f, stoneLight);

        // 1.3. Hai cầu thang dẫn lên sàn +4.20m ở phía Đông (+X, đặt tại z = ±2.45m, vươn từ x = +4.6m tới +7.2m -> R_max <= 7.88m)
        for (int sz : {-1, 1}) {
            MB::appendStaircase(baseMesh, 4.60f, sz * 2.45f, 1, 0, 0.50f, 1.55f, 2.55f, 3.70f, 10, stoneLight, goldCol, 0.28f);
        }

        // 1.4. Chòi mái cong đón khách ở sảnh Đông trên sàn +4.20m (cx = +5.55m, cz = 0.0m, x thuộc [4.45, 6.65] -> cách vòng ghế R=3.85m là 0.60m!)
        {
            const float gx = 5.55f, gz = 0.0f;
            const float gw = 2.10f, gd = 3.10f;
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    MB::appendBoxByYRange(baseMesh, gx + sx * 0.88f, 4.20f, 6.75f, gz + sz * 1.32f, 0.24f, 0.24f, pillarRed);
                }
            }
            MB::appendBoxByYRange(baseMesh, gx, 6.68f, 6.90f, gz, gw, gd, darkTrim);
            baseMesh.append(Geometry::PolygonMesh::createCurvedAsianRoof(gx, 6.88f, gz, gw + 0.5f, gd + 0.5f, 1.25f, 0.45f, saffronGold));
            baseMesh.append(Geometry::PolygonMesh::createPyramid(gx, 8.08f, gz, 0.32f, 0.32f, 0.45f, lotusTipGold));
        }

        // 1.5. Lan can bảo vệ quanh chu vi sàn +4.20m (chừa lối vào sảnh Đông góc [-35, +35] độ)
        constexpr int kRailSegs = 24;
        const float railR = 6.45f;
        for (int i = 0; i < kRailSegs; ++i) {
            float deg0 = -180.0f + (360.0f / kRailSegs) * i;
            float deg1 = deg0 + (360.0f / kRailSegs);
            float midDeg = 0.5f * (deg0 + deg1);
            if (std::fabs(midDeg) < 34.0f) continue; // Chừa cửa cầu thang & chòi Đông (+X)
            float a0 = deg0 * MathCG::DEG2RAD;
            float a1 = deg1 * MathCG::DEG2RAD;
            float x0 = railR * std::cos(a0), z0 = railR * std::sin(a0);
            float x1 = railR * std::cos(a1), z1 = railR * std::sin(a1);
            baseMesh.append(Geometry::PolygonMesh::createPrismY(x0, 4.20f, z0, 0.11f, 0.92f, 8, goldCol));
            MB::appendOrientedPolygon(baseMesh, {
                MathCG::Vec3(x0, 4.92f, z0), MathCG::Vec3(x1, 4.92f, z1),
                MathCG::Vec3(x1, 5.06f, z1), MathCG::Vec3(x0, 5.06f, z0)
            }, goldCol, MathCG::Vec3(std::cos(midDeg * MathCG::DEG2RAD), 0.2f, std::sin(midDeg * MathCG::DEG2RAD)));
        }

        // ====================================================================
        // 2. ĐỈNH THÁP HÌNH BÚP SEN VÀNG NHIỀU CÁNH (LOTUS-BUD CROWN, y = 47.0 .. 52.8m)
        // ====================================================================
        const float yTop = totalHeight; // 47.0m
        // Cổ tháp & đài sen dưới (16 cánh đài vươn rộng từ R=1.85m ra R=2.95m)
        MB::appendOctagonalPrism(baseMesh, 0.0f, yTop - 0.15f, yTop + 0.55f, 0.0f, 1.95f, darkTrim);
        constexpr int kLotusSides = 16;
        for (int s = 0; s < kLotusSides; ++s) {
            int sNext = (s + 1) % kLotusSides;
            float a0 = (static_cast<float>(s)     / kLotusSides) * MathCG::TWO_PI;
            float a1 = (static_cast<float>(sNext) / kLotusSides) * MathCG::TWO_PI;
            const MathCG::ColorRGB& cCol = (s % 2 == 0) ? goldCol : saffronGold;
            MathCG::Vec3 b0(1.92f * std::cos(a0), yTop + 0.50f, 1.92f * std::sin(a0));
            MathCG::Vec3 b1(1.92f * std::cos(a1), yTop + 0.50f, 1.92f * std::sin(a1));
            MathCG::Vec3 t1(2.95f * std::cos(a1), yTop + 1.25f, 2.95f * std::sin(a1));
            MathCG::Vec3 t0(2.95f * std::cos(a0), yTop + 1.25f, 2.95f * std::sin(a0));
            MB::appendOrientedPolygon(baseMesh, { b0, b1, t1, t0 }, cCol, MathCG::Vec3(0.0f, -1.0f, 0.0f));
            // Khép mặt trên từ mép đài sen (R=2.95) thu vào chân búp sen (R=2.15)
            MathCG::Vec3 u1(2.15f * std::cos(a1), yTop + 1.45f, 2.15f * std::sin(a1));
            MathCG::Vec3 u0(2.15f * std::cos(a0), yTop + 1.45f, 2.15f * std::sin(a0));
            MB::appendOrientedPolygon(baseMesh, { t0, t1, u1, u0 }, lotusTipGold, MathCG::Vec3(0.0f, 1.0f, 0.0f));
        }

        // Búp sen chính (16 múi cánh hoa sen phẳng khép kín phình ở giữa rồi thu nhọn lên đỉnh y = yTop + 5.75m)
        constexpr int kLotusRings = 6;
        const float lotusY[kLotusRings] = {
            yTop + 1.45f, yTop + 2.25f, yTop + 3.20f, yTop + 4.15f, yTop + 5.00f, yTop + 5.75f
        };
        const float lotusR[kLotusRings] = {
            2.15f, 2.82f, 2.68f, 1.95f, 0.95f, 0.0f
        };
        for (int r = 0; r < kLotusRings - 1; ++r) {
            const bool toApex = (r == kLotusRings - 2);
            const float nyHint = (r == 0) ? -1.0f : 1.0f;
            for (int s = 0; s < kLotusSides; ++s) {
                int sNext = (s + 1) % kLotusSides;
                float a0 = (static_cast<float>(s)     / kLotusSides) * MathCG::TWO_PI;
                float a1 = (static_cast<float>(sNext) / kLotusSides) * MathCG::TWO_PI;
                const MathCG::ColorRGB& petalCol = ((s + r) % 2 == 0) ? goldCol : ((r >= 3) ? lotusTipGold : saffronGold);
                MathCG::Vec3 hint(0.0f, nyHint, 0.0f);

                MathCG::Vec3 p00(lotusR[r] * std::cos(a0), lotusY[r], lotusR[r] * std::sin(a0));
                MathCG::Vec3 p01(lotusR[r] * std::cos(a1), lotusY[r], lotusR[r] * std::sin(a1));
                if (toApex) {
                    MathCG::Vec3 apex(0.0f, lotusY[r + 1], 0.0f);
                    MB::appendOrientedPolygon(baseMesh, { p00, p01, apex }, lotusTipGold, hint);
                } else {
                    MathCG::Vec3 p11(lotusR[r + 1] * std::cos(a1), lotusY[r + 1], lotusR[r + 1] * std::sin(a1));
                    MathCG::Vec3 p10(lotusR[r + 1] * std::cos(a0), lotusY[r + 1], lotusR[r + 1] * std::sin(a0));
                    MB::appendOrientedPolygon(baseMesh, { p00, p01, p11, p10 }, petalCol, hint);
                }
            }
        }

        baseMesh.computeNormals();
        baseMesh.draw(true, showNormals);

        // ====================================================================
        // 3. CỘT THÁP TRỤ TRUNG TÂM (H = 47m), 4 RAY DẪN HƯỚNG & ĐAI VÀNG
        // ====================================================================
        LightingSystem::setMaterial(0.60f, 48.0f);
        glColor3f(0.94f, 0.93f, 0.88f);
        ParametricShapes::drawCylinderY(towerRadius, totalHeight, 24, 16, true, showNormals);

        glColor3f(0.24f, 0.26f, 0.30f);
        for (int i = 0; i < 4; ++i) {
            float u = (static_cast<float>(i) / 4) * MathCG::TWO_PI;
            float rx = std::cos(u);
            float rz = std::sin(u);
            MathCG::Vec3 railBase(rx * (towerRadius + 0.12f), 4.20f, rz * (towerRadius + 0.12f));
            MathCG::Vec3 railTop(rx * (towerRadius + 0.12f), totalHeight, rz * (towerRadius + 0.12f));
            ParametricShapes::drawCylinderBetween(railBase, railTop, 0.14f, 0.14f, 6, false);
        }

        glColor3f(0.95f, 0.76f, 0.12f);
        const float ringY[] = { 10.0f, 18.0f, 26.0f, 34.0f, 42.0f };
        for (float y : ringY) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(towerRadius + 0.08f, 0.11f, 24, 8, false);
            glPopMatrix();
        }
    }
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
