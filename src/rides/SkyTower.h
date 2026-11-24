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

namespace AsiaParkRides {

class SkyTower {
public:
    MathCG::Vec3 position       = ParkLayout::SKY_TOWER_POS;             // (-260.0, 0.0, -54.6) [Ước lượng từ ảnh vệ tinh] ±10m
    float        totalHeight    = ParkLayout::SKY_TOWER_SHAFT_HEIGHT;    // 47.0m [Đã xác minh]
    float        lotusHeight    = ParkLayout::SKY_TOWER_LOTUS_CROWN_HEIGHT; // ~6.0m [Bản vẽ minh họa AI, tham khảo hình dạng]
    float        towerRadius    = 1.65f;
    float        gondolaRadius  = 3.35f;
    int          numSeats       = 16;
    float        cycleTime      = 0.0f;  // Đồng hồ hoạt ảnh nội bộ
    float        currentY       = ParkLayout::SKY_TOWER_GONDOLA_MIN_Y;   // Độ cao khởi đầu ngay trên sàn đón +4.2m
    float        currentYawDeg  = 0.0f;  // Góc quay hiện tại của vòng Gondola

    SkyTower() {
        buildStaticStationMesh();
    }

    ~SkyTower() {
        if (staticList_ != 0) {
            glDeleteLists(staticList_, 1);
            staticList_ = 0;
        }
        if (gondolaList_ != 0) {
            glDeleteLists(gondolaList_, 1);
            gondolaList_ = 0;
        }
        if (stationLampsList_ != 0) {
            glDeleteLists(stationLampsList_, 1);
            stationLampsList_ = 0;
        }
    }

    SkyTower(const SkyTower&) = delete;
    SkyTower& operator=(const SkyTower&) = delete;

    /**
     * @brief Cập nhật vật lý: Điều khiển chu kỳ rơi tự do đa pha (giữ nguyên animation, khớp sàn đón +4.2m)
     */
    void update(float dt) {
        cycleTime = std::fmod(cycleTime + dt, 22.0f);

        const float minY = ParkLayout::SKY_TOWER_GONDOLA_MIN_Y; // 4.35m (vừa trên sàn +4.20m)
        const float maxY = 42.0f;
        const float liftDuration = 13.0f;
        const float holdDuration = 2.5f;
        const float dropDuration = 2.5f;
        const float brakeDuration = 2.0f;

        float t = cycleTime;

        if (t < liftDuration) {
            float u = t / liftDuration;
            float ease = u * u * (3.0f - 2.0f * u);
            currentY = minY + (maxY - minY) * ease;
            currentYawDeg = u * 360.0f * 1.5f;
        }
        else if (t < liftDuration + holdDuration) {
            currentY = maxY;
            float holdT = t - liftDuration;
            currentYawDeg += holdT * 4.0f * dt;
        }
        else if (t < liftDuration + holdDuration + dropDuration) {
            float dropT = t - (liftDuration + holdDuration);
            float u = dropT / dropDuration;
            float fallDist = (maxY - (minY + 6.0f)) * (u * u);
            currentY = maxY - fallDist;
        }
        else if (t < liftDuration + holdDuration + dropDuration + brakeDuration) {
            float brakeT = t - (liftDuration + holdDuration + dropDuration);
            float u = brakeT / brakeDuration;
            float ease = 1.0f - (1.0f - u) * (1.0f - u);
            currentY = (minY + 6.0f) - 6.0f * ease;
        }
        else {
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
     * @brief Vẽ Tháp Rơi Tự Do Golden Sky Tower (dùng Display List cho toàn bộ phần tĩnh & khung ghế)
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // 1. Phần hình học tĩnh (Đế, sàn đón +4.2m, cầu thang, buồng ĐK, chòi mái ngói, cột 47m & chỏm sen ~6m)
        if (!showNormals) {
            ensureDisplayLists();
            if (staticList_ != 0) {
                glCallList(staticList_);
            } else {
                drawStaticStructureDirect(false);
            }
        } else {
            drawStaticStructureDirect(true);
        }

        // 2. Đèn hiệu cảnh báo hàng không gắn khít ngay trên đỉnh búp hoa sen (y ~ 53.0m)
        glPushMatrix();
        glTranslatef(0.0f, totalHeight + lotusHeight - 0.02f, 0.0f);
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
            LightingSystem::resetMaterial();
        }
        glPopMatrix();

        // 2b. 4 đèn vàng ấm phát sáng ban đêm/hoàng hôn (2 đèn dưới mái chòi sảnh Đông +4.2m & 2 đèn trụ lan can sàn đón)
        if (isNightOrSunset) {
            GLfloat warmEmit[4] = { 1.0f, 0.88f, 0.42f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, warmEmit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, warmEmit);
        } else {
            glColor3f(0.96f, 0.90f, 0.68f);
        }
        if (!showNormals) {
            ensureDisplayLists();
            if (stationLampsList_ != 0) {
                glCallList(stationLampsList_);
            } else {
                stationLampsMesh_.draw(true, false);
            }
        } else {
            stationLampsMesh_.draw(true, true);
        }
        if (isNightOrSunset) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            LightingSystem::resetMaterial();
        }

        // 3. Khung Gondola chở khách động học phân cấp (Display List khung + LED ban đêm)
        glPushMatrix();
        glTranslatef(0.0f, currentY, 0.0f);
        glRotatef(currentYawDeg, 0.0f, 1.0f, 0.0f);

        if (!showNormals) {
            ensureDisplayLists();
            if (gondolaList_ != 0) {
                glCallList(gondolaList_);
            } else {
                drawGondolaRingDirect(false);
            }
        } else {
            drawGondolaRingDirect(true);
        }

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

        glPopMatrix();
        glPopMatrix();
    }

private:
    mutable Geometry::PolygonMesh staticStationMesh_;
    mutable Geometry::PolygonMesh stationLampsMesh_;
    mutable GLuint staticList_       = 0;
    mutable GLuint gondolaList_      = 0;
    mutable GLuint stationLampsList_ = 0;

    void ensureDisplayLists() const {
        if (staticList_ == 0) {
            staticList_ = glGenLists(1);
            if (staticList_ != 0) {
                glNewList(staticList_, GL_COMPILE);
                drawStaticStructureDirect(false);
                glEndList();
            }
        }
        if (gondolaList_ == 0) {
            gondolaList_ = glGenLists(1);
            if (gondolaList_ != 0) {
                glNewList(gondolaList_, GL_COMPILE);
                drawGondolaRingDirect(false);
                glEndList();
            }
        }
        if (stationLampsList_ == 0) {
            stationLampsList_ = glGenLists(1);
            if (stationLampsList_ != 0) {
                glNewList(stationLampsList_, GL_COMPILE);
                stationLampsMesh_.draw(true, false);
                glEndList();
            }
        }
    }

    /**
     * @brief Xây dựng toàn bộ phần tĩnh trạm Sky Tower & chỏm hoa sen 6m bằng PolygonMesh
     *        [Bản vẽ minh họa AI, tham khảo hình dạng] - tất cả nằm gọn trong bán kính <= 8.0m
     */
    void buildStaticStationMesh() {
        using Mesh = Geometry::PolygonMesh;
        staticStationMesh_.clear();
        staticStationMesh_.name = "SkyTowerStationAndLotusA1";
        stationLampsMesh_.clear();
        stationLampsMesh_.name = "SkyTowerWarmStationLamps";

        const float deckY = ParkLayout::SKY_TOWER_BOARDING_DECK_Y; // +4.20m
        const float deckR = 6.00f;                                 // Giữ toàn bộ hạng mục gọn trong bán kính <= 8.0m

        const MathCG::ColorRGB plinthCol(0.42f, 0.44f, 0.48f);
        const MathCG::ColorRGB concreteCol(0.76f, 0.75f, 0.72f);
        const MathCG::ColorRGB deckStoneCol(0.84f, 0.80f, 0.72f);
        const MathCG::ColorRGB steelDarkCol(0.24f, 0.27f, 0.32f);
        const MathCG::ColorRGB railGrayCol(0.55f, 0.58f, 0.62f);
        const MathCG::ColorRGB goldCol(0.95f, 0.76f, 0.16f);
        const MathCG::ColorRGB deepGoldCol(0.82f, 0.60f, 0.12f);
        const MathCG::ColorRGB crimsonCol(0.76f, 0.16f, 0.14f);
        const MathCG::ColorRGB roofTileCol(0.68f, 0.22f, 0.14f);
        const MathCG::ColorRGB glassCol(0.20f, 0.44f, 0.64f);

        // 1. Móng bê tông nền bát giác & lõi trụ tầng trệt (R = 7.45m -> y = 0 .. 0.36m, lõi R = 3.55m -> 4.18m)
        staticStationMesh_.append(Mesh::createPrismY(0.0f, 0.00f, 0.0f, 7.45f, 0.36f, 16, plinthCol));
        staticStationMesh_.append(Mesh::createPrismY(0.0f, 0.32f, 0.0f, 3.60f, deckY - 0.30f, 12, concreteCol));

        // 12 Cột thép đỡ vành sàn đón khách +4.20m (đặt ở R = 5.45m)
        for (int i = 0; i < 12; ++i) {
            float a = (static_cast<float>(i) / 12.0f) * MathCG::TWO_PI;
            float cx = 5.45f * std::cos(a);
            float cz = 5.45f * std::sin(a);
            staticStationMesh_.append(Mesh::createPrismY(cx, 0.32f, cz, 0.20f, deckY - 0.34f, 8, steelDarkCol));
        }

        // 2. Sàn đón khách tròn trên cao tại y = +4.20m (dày 3.84 .. 4.20m, R = 6.00m) & Gờ viền vàng
        staticStationMesh_.append(Mesh::createPrismY(0.0f, deckY - 0.36f, 0.0f, deckR, 0.36f, 20, deckStoneCol));
        staticStationMesh_.append(Mesh::createPrismY(0.0f, deckY - 0.32f, 0.0f, deckR + 0.14f, 0.22f, 20, goldCol));

        // 3. Hàng rào bảo vệ chu vi sàn đón khách (R = 5.84m, y = 4.20 .. 5.25m)
        //    Chừa lối mở sạch tại đầu cầu thang lên (góc Đông-Nam) & trước cửa chòi đón (phía Đông)
        const int railSegs = 24;
        const float railR = 5.84f;
        for (int i = 0; i < railSegs; ++i) {
            float aMid = ((i + 0.5f) / railSegs) * 360.0f;
            if ((aMid >= 8.0f && aMid <= 38.0f) || (aMid >= 332.0f || aMid < 8.0f)) continue;
            float rad0 = (static_cast<float>(i) / railSegs) * MathCG::TWO_PI;
            float rad1 = (static_cast<float>(i + 1) / railSegs) * MathCG::TWO_PI;
            float x0 = railR * std::cos(rad0), z0 = railR * std::sin(rad0);
            float x1 = railR * std::cos(rad1), z1 = railR * std::sin(rad1);
            staticStationMesh_.append(Mesh::createPrismY(x0, deckY - 0.05f, z0, 0.07f, 1.08f, 6, railGrayCol));
            staticStationMesh_.append(Mesh::createPrismY(x1, deckY - 0.05f, z1, 0.07f, 1.08f, 6, railGrayCol));
            float mx = 0.5f * (x0 + x1), mz = 0.5f * (z0 + z1);
            float segLen = std::sqrt((x1 - x0) * (x1 - x0) + (z1 - z0) * (z1 - z0));
            float yaw = std::atan2(x1 - x0, z1 - z0) * MathCG::RAD2DEG;
            Mesh topBar = Mesh::createBox(0.0f, deckY + 1.00f, 0.0f, 0.09f, 0.08f, segLen, railGrayCol);
            Mesh midBar = Mesh::createBox(0.0f, deckY + 0.55f, 0.0f, 0.07f, 0.06f, segLen, steelDarkCol);
            topBar.rotateY(yaw); topBar.translate(mx, 0.0f, mz);
            midBar.rotateY(yaw); midBar.translate(mx, 0.0f, mz);
            staticStationMesh_.append(topBar);
            staticStationMesh_.append(midBar);
        }

        // 4. Cầu thang lên sàn đón khách +4.20m ở góc Đông-Nam (nằm trọn trong R <= 7.65m <= 8.0m, ngoài R >= 4.4m)
        {
            float sx0 = 3.15f, sz0 = 5.75f; // chân thang dưới đất (R = 6.56m, cả thành thang <= 7.55m)
            float sx1 = 5.05f, sz1 = 1.80f; // đỉnh thang chạm khít mặt sàn đón +4.2m (R = 5.36m < deckR)
            float dx = sx0 - sx1, dz = sz0 - sz1;
            float runLen = std::sqrt(dx * dx + dz * dz);
            float yawDeg = std::atan2(dx, dz) * MathCG::RAD2DEG;
            staticStationMesh_.append(Mesh::createSteppedStaircase(
                sx1, sz1, yawDeg, 0.0f, deckY,
                runLen, 1.70f, 11, deckStoneCol, steelDarkCol
            ));
        }

        // 5. Chòi mái ngói nhỏ đón/chờ khách ở cạnh Đông sàn +4.20m (tâm (5.40, -1.05), R_max <= 7.20m <= 8.0m)
        {
            const float gx = 5.40f, gz = -1.05f;
            Mesh::appendBoxByYRange(staticStationMesh_, gx, 0.00f, deckY + 0.04f, gz, 2.00f, 2.30f, plinthCol);
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    staticStationMesh_.append(Mesh::createPrismY(
                        gx + sx * 0.80f, deckY, gz + sz * 0.95f,
                        0.12f, 2.25f, 8, crimsonCol
                    ));
                }
            }
            Mesh::appendBoxByYRange(staticStationMesh_, gx + 0.80f, deckY, deckY + 1.05f, gz, 0.22f, 2.00f, concreteCol);
            Mesh::appendBoxByYRange(staticStationMesh_, gx, 6.36f, 6.64f, gz, 1.98f, 2.25f, steelDarkCol);
            staticStationMesh_.append(Mesh::createCurvedAsianRoof(
                gx, 6.60f, gz, 2.00f, 2.30f, 1.05f, 0.60f, roofTileCol
            ));

            // 2 đèn lồng treo vàng ấm gắn khít dưới trần chòi sảnh Đông +4.2m (không dùng GL_LIGHT)
            const MathCG::ColorRGB warmLampCol(1.0f, 0.90f, 0.45f);
            for (int sz : {-1, 1}) {
                float lz = gz + sz * 0.50f;
                staticStationMesh_.append(Mesh::createPrismY(gx, 6.22f, lz, 0.045f, 0.16f, 6, deepGoldCol));
                staticStationMesh_.append(Mesh::createPrismY(gx, 6.18f, lz, 0.09f,  0.05f, 8, steelDarkCol));
                stationLampsMesh_.append(Mesh::createBox(gx, 6.08f, lz, 0.17f, 0.20f, 0.17f, warmLampCol));
            }
        }

        // 5b. 2 đèn trụ vàng ấm gắn khít trên trụ lan can sàn +4.20m (góc 45 độ đầu cầu thang & 180 độ phía Tây)
        {
            const MathCG::ColorRGB warmLampCol(1.0f, 0.90f, 0.45f);
            const float lampAnglesDeg[2] = { 45.0f, 180.0f };
            for (float deg : lampAnglesDeg) {
                float rad = deg * MathCG::DEG2RAD;
                float lx = railR * std::cos(rad);
                float lz = railR * std::sin(rad);
                staticStationMesh_.append(Mesh::createPrismY(lx, deckY + 1.00f, lz, 0.10f, 0.10f, 8, goldCol));
                stationLampsMesh_.append(Mesh::createBox(lx, deckY + 1.19f, lz, 0.17f, 0.18f, 0.17f, warmLampCol));
                staticStationMesh_.append(Mesh::createPyramid(lx, deckY + 1.28f, lz, 0.22f, 0.22f, 0.10f, steelDarkCol));
            }
        }

        // 6. Buồng điều khiển nhỏ (Control Room) ở mặt Nam (-1.25, +5.20) [R_in = 4.28m > 3.72m, R_out <= 6.80m <= 8.0m]
        {
            const float cx = -1.25f, cz = 5.20f;
            Mesh::appendBoxByYRange(staticStationMesh_, cx, 0.00f, deckY + 0.04f, cz, 2.20f, 1.85f, plinthCol);
            Mesh::appendBoxByYRange(staticStationMesh_, cx, deckY, 5.15f, cz, 2.20f, 1.85f, concreteCol);
            Mesh::appendBoxByYRange(staticStationMesh_, cx, 5.10f, 6.05f, cz + 0.08f, 2.08f, 1.66f, glassCol);
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    Mesh::appendBoxByYRange(staticStationMesh_, cx + sx * 0.98f, 5.10f, 6.22f, cz + sz * 0.80f, 0.22f, 0.22f, steelDarkCol);
                }
            }
            Mesh::appendBoxByYRange(staticStationMesh_, cx, 6.18f, 6.42f, cz, 2.40f, 2.05f, steelDarkCol);
            Mesh::appendBoxByYRange(staticStationMesh_, cx, 6.40f, 6.56f, cz, 2.28f, 1.95f, goldCol);
        }

        // 7. Chỏm HOA SEN VÀNG ở đỉnh cột 47m (cao ~6.0m: y = 47.00 .. 53.00m)
        //    [Bản vẽ minh họa AI, tham khảo hình dạng]
        staticStationMesh_.append(Mesh::createLotusCrown(
            0.0f, totalHeight, 0.0f, towerRadius, lotusHeight, 10, goldCol, deepGoldCol
        ));

        staticStationMesh_.computeNormals();
        stationLampsMesh_.computeNormals();
    }

    /**
     * @brief Hàm vẽ toàn bộ phần tĩnh (Station PolygonMesh + cột tháp 47m + ray thép + vòng vàng)
     */
    void drawStaticStructureDirect(bool showNormals) const {
        // 1. Toàn bộ bệ, sàn đón +4.2m, cầu thang, chòi mái ngói, buồng điều khiển & chỏm hoa sen 6m
        staticStationMesh_.draw(true, showNormals);

        // 2. Cột tháp kết cấu trung tâm (H = 47m, y = 0 .. 47m)
        LightingSystem::setMaterial(0.60f, 48.0f);
        glColor3f(0.92f, 0.94f, 0.96f);
        ParametricShapes::drawCylinderY(towerRadius, totalHeight, 22, 12, true, showNormals);

        // 3. 4 Thanh ray dẫn hướng bằng thép chạy dọc chiều cao tháp (từ sàn đón lên đỉnh 47m)
        glColor3f(0.24f, 0.26f, 0.30f);
        for (int i = 0; i < 4; ++i) {
            float u = (static_cast<float>(i) / 4) * MathCG::TWO_PI;
            float rx = std::cos(u);
            float rz = std::sin(u);
            MathCG::Vec3 railBase(rx * (towerRadius + 0.12f), 4.0f, rz * (towerRadius + 0.12f));
            MathCG::Vec3 railTop(rx * (towerRadius + 0.12f), totalHeight, rz * (towerRadius + 0.12f));
            ParametricShapes::drawCylinderBetween(railBase, railTop, 0.13f, 0.13f, 6, false);
        }

        // 4. Các vòng tròn trang trí mạ vàng bao quanh thân tháp
        glColor3f(0.95f, 0.76f, 0.12f);
        const float ringY[] = { 10.0f, 20.0f, 30.0f, 40.0f, 46.85f };
        for (float y : ringY) {
            glPushMatrix();
            glTranslatef(0.0f, y, 0.0f);
            ParametricShapes::drawToroidY(towerRadius + 0.08f, 0.10f, 20, 6, false);
            glPopMatrix();
        }
        LightingSystem::resetMaterial();
    }

    /**
     * @brief Vòng Gondola: Đai Toroid, 16 ghế hành khách hướng ra ngoài, mái che nắng (compile vào Display List)
     */
    void drawGondolaRingDirect(bool showNormals) const {
        // 1. Ống lót kết cấu bên trong trượt dọc theo cột tháp
        glColor3f(0.18f, 0.20f, 0.24f);
        ParametricShapes::drawCylinderY(towerRadius + 0.22f, 2.1f, 18, 2, true, false);

        // 2. Vòng đỡ sàn hành khách chính (Hình xuyến tham số Toroid)
        LightingSystem::setMaterial(0.70f, 48.0f);
        glColor3f(0.96f, 0.78f, 0.12f);
        glPushMatrix();
        glTranslatef(0.0f, 0.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius, 0.32f, 28, 8, showNormals);
        glPopMatrix();

        // 3. Vòng mái che nắng hình tròn phía trên đầu
        glColor3f(0.92f, 0.28f, 0.14f);
        glPushMatrix();
        glTranslatef(0.0f, 2.35f, 0.0f);
        ParametricShapes::drawToroidY(gondolaRadius + 0.15f, 0.22f, 28, 8, false);
        glPopMatrix();

        // 4. 16 Ghế hành khách hướng mặt ra ngoài bố trí quanh vòng tròn
        for (int i = 0; i < numSeats; ++i) {
            float angleDeg = (360.0f / numSeats) * i;
            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, gondolaRadius);

            glColor3f(0.35f, 0.38f, 0.42f);
            ParametricShapes::drawCylinderBetween(
                MathCG::Vec3(0.0f, 0.35f, -gondolaRadius + towerRadius + 0.25f),
                MathCG::Vec3(0.0f, 0.35f, 0.0f),
                0.08f, 0.08f, 6, false
            );

            glColor3f(0.85f, 0.14f, 0.12f);
            drawLocalBox(0.0f, 0.55f, 0.0f, 0.65f, 0.14f, 0.55f);
            drawLocalBox(0.0f, 1.05f, -0.24f, 0.62f, 0.90f, 0.12f);
            drawLocalBox(0.0f, 1.62f, -0.22f, 0.42f, 0.28f, 0.14f);

            glColor3f(0.98f, 0.82f, 0.15f);
            drawLocalBox(-0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);
            drawLocalBox( 0.16f, 1.08f, -0.04f, 0.09f, 0.72f, 0.28f);

            glColor3f(0.50f, 0.52f, 0.56f);
            drawLocalBox(0.0f, 0.05f, 0.12f, 0.50f, 0.06f, 0.20f);

            glPopMatrix();
        }
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
