#ifndef NGO_MON_GATE_H
#define NGO_MON_GATE_H

/**
 * @file NgoMonGate.h
 * @brief Chương 4 & 6: Mô phỏng Kiến trúc Cổng Chính Công viên Châu Á (Asia Park):
 *        Cổng Ngọ Môn (Meridian Imperial Gate) & Tháp Đồng Hồ (Central Clock Tower)
 *
 * Triển khai:
 *  - Kiến trúc Cổng vào đặc trưng của Asia Park Đà Nẵng:
 *      * Vị trí: (0.0, 0.0, 86.0) tại Quảng trường Cổng Nam hướng ra trục đường đón khách
 *      * Kích thước mặt bằng: Rộng 48m x Sâu 16m x Đỉnh tháp cao 28m
 *  - Chương 4 Biểu diễn Hình học 3D qua Geometry::PolygonMesh:
 *      * 1. Bệ Thành Lũy Đá Đồ Sộ với các lớp đá xây kiên cố
 *      * 2. Ba Cổng Vòm Cuốn (Arched Barrel Vaults):
 *           - Cổng vòm chính giữa (Lối đi chính Ngọ Môn)
 *           - Hai cổng vòm hai bên (Tả Đoan Môn & Hữu Đoan Môn)
 *           được khởi tạo bằng hàm Geometry::PolygonMesh::createArchVault
 *      * 3. Lầu Ngũ Phụng:
 *           - Hệ cột gỗ sơn son (createPrismY) & lan can đá trắng chạm trổ
 *           - Hệ mái cổ truyền Á Đông 2 tầng đao cong (createCurvedAsianRoof)
 *      * 4. Tháp Đồng Hồ Trung Tâm:
 *           - Mặt đồng hồ phát sáng 4 hướng kèm kim giờ và kim phút
 *           - Mái tháp nhiều tầng & chóp đỉnh mạ vàng
 *  - Chương 6 Chiếu sáng, Tô bóng & Tính toán Pháp tuyến:
 *      * Tính toán đầy đủ pháp tuyến bề mặt & pháp tuyến đỉnh qua PolygonMesh::computeNormals()
 *      * Mặt đồng hồ phát sáng nền & dàn đèn lồng đỏ cung đình rực sáng vào ban đêm
 */

#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include <vector>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class NgoMonGate {
public:
    MathCG::Vec3 position       = ParkLayout::NGO_MON_GATE_POS; // (+16.1, 0.0, +139.1) [Ước lượng từ ảnh vệ tinh] ±8m
    float        gateScale      = ParkLayout::NGO_MON_GATE_SCALE; // 0.76f (bề rộng thực tế ~35m)
    MathCG::Vec3 clockTowerPos  = ParkLayout::CLOCK_TOWER_POS;    // (+10.7, 0.0, +61.6) Tháp Đồng Hồ (2019), scale 1.0f
    float        clockTowerScale= ParkLayout::CLOCK_TOWER_SCALE;  // 1.00f
    float        width          = 46.0f;
    float        depth          = 15.0f;
    float        bastionHeight  = 6.8f;
    float        towerHeight    = 27.5f;

    NgoMonGate() {
        buildGateMesh();
    }

    /**
     * @brief Vẽ Cổng Ngọ Môn (+16.1, 0, +139.1, scale 0.76) & Tháp Đồng Hồ tách riêng (+10.7, 0, +61.6, scale 1.0)
     */
    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        // A. Vẽ Cổng Thành Ngọ Môn & Lầu Ngũ Phụng tại lối vào phía Nam (+16.1, 0.0, +139.1), scale = 0.76
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(gateScale, gateScale, gateScale);

        // 1. Vẽ lưới đa giác kiến trúc Cổng Ngọ Môn (PolygonMesh)
        gateMesh.draw(smoothShading, showNormals);

        // 2. Vẽ dàn đèn lồng đỏ cung đình treo dưới hiên Lầu Ngũ Phụng
        drawLanterns(isNightOrSunset, showNormals);

        glPopMatrix();

        // B. Vẽ Tháp Đồng Hồ Trung Tâm tách riêng tại Quảng trường Tháp Đồng Hồ (+10.7, 0.0, +61.6), scale = 1.0
        glPushMatrix();
        glTranslatef(clockTowerPos.x, clockTowerPos.y, clockTowerPos.z);
        glScalef(clockTowerScale, clockTowerScale, clockTowerScale);

        // 1. Vẽ lưới đa giác Tháp Đồng Hồ (đặt tiếp đất nguyên khối)
        clockTowerMesh.draw(smoothShading, showNormals);

        // 2. Vẽ 4 mặt đồng hồ phát sáng trên 4 phía thân tháp
        drawClockFaces(isNightOrSunset);

        glPopMatrix();
    }

private:
    mutable Geometry::PolygonMesh gateMesh;
    mutable Geometry::PolygonMesh clockTowerMesh;

    /**
     * @brief Khởi tạo thủ tục toàn bộ lưới đa giác (PolygonMesh) của Cổng Ngọ Môn & Tháp Đồng Hồ
     */
    void buildGateMesh() {
        gateMesh.clear();
        gateMesh.name = "NgoMonImperialGate";

        MathCG::ColorRGB stoneCol(0.62f, 0.60f, 0.58f); // Đá xám cổ kính xây thành
        MathCG::ColorRGB redPillarCol(0.78f, 0.16f, 0.14f); // Sơn son đỏ cung đình
        MathCG::ColorRGB goldRoofCol(0.95f, 0.70f, 0.12f); // Ngói hoàng lưu ly vàng rực
        MathCG::ColorRGB darkTrimCol(0.22f, 0.20f, 0.22f); // Dầm xà gỗ màu sẫm
        MathCG::ColorRGB balustradeCol(0.88f, 0.88f, 0.90f); // Lan can đá cẩm thạch trắng chạm khắc

        // ====================================================================
        // 1. BỆ ĐÀI THÀNH CỔ & HAI CÁNH THÀNH BÊN
        // ====================================================================
        // Khối đài cổng trung tâm (nơi bố trí 3 vòm cổng)
        float centerW = 24.0f;
        float wingW   = (width - centerW) * 0.5f; // Mỗi cánh rộng 11m
        float hZ      = depth * 0.5f;

        // Khối bệ trung tâm phía trên các vòm cổng (y = 5.2 .. 6.8)
        auto centerTop = Geometry::PolygonMesh::createBox(
            0.0f, 6.0f, 0.0f,
            centerW, 1.6f, depth,
            stoneCol
        );
        gateMesh.append(centerTop);

        // Các trụ đá thành lũy bên trái & bên phải tạo thành 3 lối cổng vòm
        // Trụ 0: Ngoài cùng bên trái (x = -10.5 .. -8.0)
        auto pierL0 = Geometry::PolygonMesh::createBox(-9.25f, 2.6f, 0.0f, 2.5f, 5.2f, depth, stoneCol);
        // Trụ 1: Giữa bên trái (x = -4.0 .. -2.0)
        auto pierL1 = Geometry::PolygonMesh::createBox(-3.0f,  2.6f, 0.0f, 2.0f, 5.2f, depth, stoneCol);
        // Trụ 2: Giữa bên phải (x = 2.0 .. 4.0)
        auto pierR1 = Geometry::PolygonMesh::createBox( 3.0f,  2.6f, 0.0f, 2.0f, 5.2f, depth, stoneCol);
        // Trụ 3: Ngoài cùng bên phải (x = 8.0 .. 10.5)
        auto pierR0 = Geometry::PolygonMesh::createBox( 9.25f, 2.6f, 0.0f, 2.5f, 5.2f, depth, stoneCol);

        gateMesh.append(pierL0);
        gateMesh.append(pierL1);
        gateMesh.append(pierR1);
        gateMesh.append(pierR0);

        // Hai cánh thành phía Đông & phía Tây
        auto wingLeft = Geometry::PolygonMesh::createBox(
            -(centerW * 0.5f + wingW * 0.5f), 3.4f, 0.0f,
            wingW, bastionHeight, depth,
            stoneCol
        );
        auto wingRight = Geometry::PolygonMesh::createBox(
             (centerW * 0.5f + wingW * 0.5f), 3.4f, 0.0f,
            wingW, bastionHeight, depth,
            stoneCol
        );
        gateMesh.append(wingLeft);
        gateMesh.append(wingRight);

        // Chân đế đá / Bệ móng dưới cùng
        auto basePlinth = Geometry::PolygonMesh::createBox(0.0f, 0.25f, 0.0f, width + 2.0f, 0.5f, depth + 1.6f, MathCG::ColorRGB(0.48f, 0.46f, 0.44f));
        gateMesh.append(basePlinth);

        // ====================================================================
        // 2. BA VÒM CUỐN CỔNG NGỌ MÔN
        // ====================================================================
        // Cổng vòm chính giữa (Bán kính = 2.0m, Chiều cao = 4.8m)
        auto centralVault = Geometry::PolygonMesh::createArchVault(
            0.0f, 2.8f, -hZ, hZ,
            1.95f, 2.55f, 14,
            MathCG::ColorRGB(0.74f, 0.70f, 0.65f)
        );
        gateMesh.append(centralVault);

        // Cổng vòm bên trái (Bán kính = 1.4m, Chiều cao = 4.2m)
        auto leftVault = Geometry::PolygonMesh::createArchVault(
            -6.0f, 2.4f, -hZ, hZ,
            1.40f, 1.85f, 12,
            MathCG::ColorRGB(0.70f, 0.66f, 0.62f)
        );
        gateMesh.append(leftVault);

        // Cổng vòm bên phải (Bán kính = 1.4m, Chiều cao = 4.2m)
        auto rightVault = Geometry::PolygonMesh::createArchVault(
             6.0f, 2.4f, -hZ, hZ,
            1.40f, 1.85f, 12,
            MathCG::ColorRGB(0.70f, 0.66f, 0.62f)
        );
        gateMesh.append(rightVault);

        // ====================================================================
        // 3. LAN CAN ĐÁ TRẮNG & SÀN SÂN THƯỢNG LẦU NGŨ PHỤNG
        // ====================================================================
        // Mặt sàn sân thượng trên đài thành
        auto terraceFloor = Geometry::PolygonMesh::createBox(0.0f, bastionHeight + 0.15f, 0.0f, width + 0.6f, 0.30f, depth + 0.6f, MathCG::ColorRGB(0.75f, 0.72f, 0.70f));
        gateMesh.append(terraceFloor);

        // Tường lan can chạm khắc bao quanh chu vi (cao h = 0.9m)
        float bY = bastionHeight + 0.30f + 0.45f;
        // Lan can mặt trước & mặt sau
        auto balFront = Geometry::PolygonMesh::createBox(0.0f, bY,  hZ + 0.2f, width + 0.4f, 0.90f, 0.30f, balustradeCol);
        auto balBack  = Geometry::PolygonMesh::createBox(0.0f, bY, -hZ - 0.2f, width + 0.4f, 0.90f, 0.30f, balustradeCol);
        // Lan can mặt trái & mặt phải
        auto balLeft  = Geometry::PolygonMesh::createBox(-width * 0.5f - 0.2f, bY, 0.0f, 0.30f, 0.90f, depth + 0.4f, balustradeCol);
        auto balRight = Geometry::PolygonMesh::createBox( width * 0.5f + 0.2f, bY, 0.0f, 0.30f, 0.90f, depth + 0.4f, balustradeCol);
        gateMesh.append(balFront);
        gateMesh.append(balBack);
        gateMesh.append(balLeft);
        gateMesh.append(balRight);

        // ====================================================================
        // 4. LẦU NGŨ PHỤNG (Mái Cổ Truyền Á Đông Hai Tầng Đao Cong)
        // ====================================================================
        float pavY = bastionHeight + 0.30f;
        float pavW = 34.0f;
        float pavD = 10.5f;

        // Hàng cột gỗ sơn son đỡ mái tầng dưới
        const int colCols = 8;
        for (int c = 0; c < colCols; ++c) {
            float frac = static_cast<float>(c) / (colCols - 1);
            float px = -pavW * 0.45f + frac * pavW * 0.90f;
            for (int s = -1; s <= 1; s += 2) {
                float pz = s * (pavD * 0.42f);
                auto col = Geometry::PolygonMesh::createPrismY(px, pavY, pz, 0.26f, 3.8f, 10, redPillarCol);
                gateMesh.append(col);
            }
        }

        // Trần lầu tầng dưới & hệ dầm xà ngang
        auto lowerLintel = Geometry::PolygonMesh::createBox(0.0f, pavY + 3.8f, 0.0f, pavW + 1.2f, 0.35f, pavD + 1.2f, darkTrimCol);
        gateMesh.append(lowerLintel);

        // Mái đao cong Á Đông tầng dưới (Ngói Hoàng Lưu Ly vàng óng)
        auto lowerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, pavY + 4.15f, 0.0f,
            pavW + 3.2f, pavD + 3.2f,
            2.2f, 2.8f,
            goldRoofCol
        );
        gateMesh.append(lowerRoof);

        // Tường & cột tầng lửng phía trên của Lầu Ngũ Phụng
        float upPavY = pavY + 4.15f + 1.8f;
        float upPavW = 24.0f;
        float upPavD = 7.8f;

        auto upChamber = Geometry::PolygonMesh::createBox(0.0f, upPavY + 1.4f, 0.0f, upPavW, 2.8f, upPavD, MathCG::ColorRGB(0.72f, 0.14f, 0.12f));
        gateMesh.append(upChamber);

        // Mái đao cong Á Đông tầng trên
        auto upperRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, upPavY + 2.8f, 0.0f,
            upPavW + 3.0f, upPavD + 3.0f,
            2.5f, 2.6f,
            goldRoofCol
        );
        gateMesh.append(upperRoof);

        // ====================================================================
        // 5. THÁP ĐỒNG HỒ TRUNG TÂM (Tách sang clockTowerMesh tại (+10.7, 0.0, +61.6), scale 1.0)
        // ====================================================================
        clockTowerMesh.clear();
        clockTowerMesh.name = "CentralClockTower2019";

        // Giữ nguyên hoàn toàn cấu trúc hình học của Tháp Đồng Hồ:
        // Chân đế tháp nối từ mặt đất (y = 0..17.15m) + Thân tháp + Gờ phào chỉ + Mái đao cong + Chóp đỉnh mạ vàng (cao 27.5m)
        float towerBaseY = upPavY + 2.8f + 1.2f; // 17.15m
        float towerSideW = 6.4f;
        float towerBodyH = 7.5f;

        // Bệ móng & Thân trụ dưới của Tháp Đồng Hồ (từ mặt đất y = 0 lên tới y = towerBaseY)
        auto towerPedestal = Geometry::PolygonMesh::createBox(
            0.0f, towerBaseY * 0.5f, 0.0f,
            towerSideW + 1.6f, towerBaseY, towerSideW + 1.6f,
            stoneCol
        );
        clockTowerMesh.append(towerPedestal);

        // Thân Tháp Đồng Hồ nơi gắn 4 mặt đồng hồ (y = 17.15 .. 24.65m)
        auto clockBody = Geometry::PolygonMesh::createBox(
            0.0f, towerBaseY + towerBodyH * 0.5f, 0.0f,
            towerSideW, towerBodyH, towerSideW,
            stoneCol
        );
        clockTowerMesh.append(clockBody);

        // Gờ phào chỉ và diềm mái mạ vàng trên đỉnh thân tháp
        auto towerCornice = Geometry::PolygonMesh::createBox(
            0.0f, towerBaseY + towerBodyH + 0.25f, 0.0f,
            towerSideW + 1.2f, 0.50f, towerSideW + 1.2f,
            goldRoofCol
        );
        clockTowerMesh.append(towerCornice);

        // Chóp Mái Tháp Đồng Hồ đao cong cổ truyền
        auto towerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, towerBaseY + towerBodyH + 0.50f, 0.0f,
            towerSideW + 2.2f, towerSideW + 2.2f,
            2.6f, 1.8f,
            goldRoofCol
        );
        clockTowerMesh.append(towerRoof);

        // Chóp nhọn mạ vàng tại đỉnh cao nhất (H = 27.5m)
        float peakY = towerBaseY + towerBodyH + 0.50f + 2.6f;
        auto spire = Geometry::PolygonMesh::createPyramid(
            0.0f, peakY, 0.0f,
            1.2f, 1.2f, 2.4f,
            MathCG::ColorRGB(0.98f, 0.85f, 0.20f)
        );
        clockTowerMesh.append(spire);

        // Tính toán toàn bộ vector pháp tuyến cho cả Cổng Ngọ Môn và Tháp Đồng Hồ
        gateMesh.computeNormals();
        clockTowerMesh.computeNormals();
    }

    /**
     * @brief Vẽ 4 mặt đồng hồ phát sáng trên bốn phía của tháp trung tâm
     */
    void drawClockFaces(bool isNightOrSunset) const {
        float clockY = bastionHeight + 0.30f + 4.15f + 1.8f + 2.8f + 1.2f + 3.8f; // ~23m
        float offset = 3.25f; // nhô nhẹ ra khỏi mặt tường tháp (towerSideW / 2 + 0.05)

        for (int side = 0; side < 4; ++side) {
            glPushMatrix();
            glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, clockY, offset);

            // Viền khung mặt đồng hồ (Hình xuyến Toroid mạ vàng)
            glColor3f(0.95f, 0.78f, 0.15f);
            ParametricShapes::drawToroid(2.10f, 0.15f, 24, 8, false);

            // Đĩa mặt đồng hồ (Màu trắng kèm hiệu ứng phát sáng ấm vào ban đêm)
            if (isNightOrSunset) {
                GLfloat emit[4] = { 0.95f, 0.95f, 0.85f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(1.0f, 1.0f, 0.92f);
            } else {
                glColor3f(0.98f, 0.98f, 0.96f);
            }
            ParametricShapes::drawDisk(2.05f, 24, 0.0f, false);
            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }

            // Các vạch chia giờ màu sẫm quanh mặt đồng hồ (12 vạch)
            glColor3f(0.12f, 0.12f, 0.14f);
            glLineWidth(2.5f);
            glBegin(GL_LINES);
            for (int h = 0; h < 12; ++h) {
                float th = (static_cast<float>(h) / 12) * MathCG::TWO_PI;
                float cTh = std::cos(th), sTh = std::sin(th);
                float rInner = (h % 3 == 0) ? 1.55f : 1.72f;
                float rOuter = 1.92f;
                glVertex3f(rInner * cTh, rInner * sTh, 0.02f);
                glVertex3f(rOuter * cTh, rOuter * sTh, 0.02f);
            }
            // Kim giờ (chỉ hướng 2 giờ)
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(0.85f * std::cos(MathCG::DEG2RAD * 30.0f), 0.85f * std::sin(MathCG::DEG2RAD * 30.0f), 0.03f);
            // Kim phút (chỉ hướng 10 phút)
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(1.35f * std::cos(MathCG::DEG2RAD * 60.0f), 1.35f * std::sin(MathCG::DEG2RAD * 60.0f), 0.03f);
            glEnd();
            glLineWidth(1.0f);

            // Chốt tròn mạ vàng ở tâm đồng hồ
            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawSphere(0.15f, 10, 8, false);

            glPopMatrix();
        }
    }

    /**
     * @brief Vẽ dàn đèn lồng đỏ cung đình lễ hội treo dọc theo hiên Lầu Ngũ Phụng
     */
    void drawLanterns(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        float eaveY = bastionHeight + 0.30f + 4.15f;
        float lanternZ = depth * 0.5f + 1.2f;

        const float lanternX[] = { -14.0f, -8.0f, -2.5f, 2.5f, 8.0f, 14.0f };
        for (float lx : lanternX) {
            glPushMatrix();
            glTranslatef(lx, eaveY - 0.75f, lanternZ);

            // Dây treo đèn lồng
            glColor3f(0.85f, 0.70f, 0.20f);
            ParametricShapes::drawCylinderY(0.025f, 0.75f, 6, 2, true, false);

            // Bầu đèn lồng hình cầu màu đỏ thẫm
            if (isNightOrSunset) {
                GLfloat emit[4] = { 1.0f, 0.18f, 0.12f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(1.0f, 0.25f, 0.15f);
            } else {
                glColor3f(0.88f, 0.16f, 0.12f);
            }
            ParametricShapes::drawSphere(0.42f, 12, 10, false);
            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }

            // Tua rua vàng trang trí phía dưới đèn lồng
            glTranslatef(0.0f, -0.45f, 0.0f);
            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawConeY(0.14f, 0.0f, -0.35f, 8, 2, true, false);

            glPopMatrix();
        }
    }
};

} // namespace AsiaParkRides

#endif // NGO_MON_GATE_H
