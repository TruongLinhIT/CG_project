#ifndef MINI_COASTER_H
#define MINI_COASTER_H

/**
 * @file MiniCoaster.h
 * @brief Lớp tàu lượn gia đình / tàu lượn nhỏ dùng chung (MiniCoaster - Đợt 5A)
 *
 * Dùng MỘT lớp tham số hóa duy nhất để khởi tạo 4 trò tàu lượn nhỏ tại khu Tây:
 *  1. Garuda Valley        (-124.1, 0, +155.7) [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]
 *  2. Port of Sky Treasure (-224.0, 0,  +22.0) [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]
 *  3. Paradise Fall        (-199.0, 0, -119.9) [Ước lượng từ ảnh vệ tinh; hình dạng đơn giản hóa]
 *     kèm Tòa nhà đá       (-179.8, 0,  -91.1) [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]
 *  4. Highway Boat         (-176.0, 0, -214.0) [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]
 *
 * Đặc điểm kỹ thuật:
 *  - KHÔNG sửa đổi RollerCoaster.h (Queen Cobra).
 *  - Tái sử dụng bộ tạo spline Bezier bậc 3 khép kín liên tục C^1 (BezierTrack) cho đường ray đôi,
 *    ống xương sống, tà-vẹt ngang và hệ thống trụ đỡ xuống mặt đất y = 0.
 *  - Toàn bộ hình học tĩnh (đường ray + trụ đỡ + nhà ga) được biên dịch vào OpenGL Display List
 *    để giữ FPS cao; đoàn tàu 3 toa chạy mượt theo độ dài cung trên đường ray mỗi khung hình.
 */

#include <GL/freeglut.h>
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "curves/BezierTrack.h"
#include "geometry/MeshModel.h"
#include "geometry/MeshBuilders.h"
#include "geometry/ParametricShapes.h"

namespace AsiaParkRides {

struct MiniCoasterConfig {
    MathCG::Vec3        centerPos;
    const MathCG::Vec3* localWaypoints    = nullptr;
    int                 numWaypoints      = 0;

    MathCG::ColorRGB    railColor         = MathCG::ColorRGB(0.92f, 0.35f, 0.15f);
    MathCG::ColorRGB    spineColor        = MathCG::ColorRGB(0.88f, 0.72f, 0.18f);
    MathCG::ColorRGB    tieColor          = MathCG::ColorRGB(0.28f, 0.30f, 0.36f);
    MathCG::ColorRGB    pillarColor       = MathCG::ColorRGB(0.66f, 0.68f, 0.72f);

    MathCG::ColorRGB    carBodyColor      = MathCG::ColorRGB(0.95f, 0.25f, 0.18f);
    MathCG::ColorRGB    carAccentColor    = MathCG::ColorRGB(1.00f, 0.85f, 0.25f);

    int                 numCars           = 3;
    float               carSpacing        = 2.35f;
    float               baseSpeedMps      = 7.8f;
    float               initialPhase01    = 0.0f;

    // Thông số nhà ga đón khách / tòa nhà chủ đề đi kèm
    MathCG::Vec3        stationWorldPos;
    float               stationWidth      = 8.0f;
    float               stationDepth      = 12.0f;
    float               stationHeight     = 4.8f;
    MathCG::ColorRGB    stationWallColor  = MathCG::ColorRGB(0.84f, 0.76f, 0.62f);
    MathCG::ColorRGB    stationRoofColor  = MathCG::ColorRGB(0.76f, 0.24f, 0.14f);
    bool                isStoneMountain   = false; // Riêng Paradise Fall có tòa nhà giả núi đá lớn
    int                 stationStyle      = 0;     // 0: mái Á Đông chuẩn, 1: mái yên ngựa Garuda (BP15), 2: cảng gỗ mái đầu hồi Port of Sky (BP12)
};

class MiniCoaster {
public:
    MiniCoasterConfig cfg;
    BezierTrack       track;
    float             headDistance = 0.0f;

    MiniCoaster() = default;

    ~MiniCoaster() {
        if (staticListSmooth_ != 0) {
            glDeleteLists(staticListSmooth_, 1);
            staticListSmooth_ = 0;
        }
        if (staticListFlat_ != 0) {
            glDeleteLists(staticListFlat_, 1);
            staticListFlat_ = 0;
        }
        if (carListSmooth_ != 0) {
            glDeleteLists(carListSmooth_, 1);
            carListSmooth_ = 0;
        }
        if (carListFlat_ != 0) {
            glDeleteLists(carListFlat_, 1);
            carListFlat_ = 0;
        }
    }

    MiniCoaster(const MiniCoaster&) = delete;
    MiniCoaster& operator=(const MiniCoaster&) = delete;

    void init(const MiniCoasterConfig& config) {
        cfg = config;

        std::vector<MathCG::Vec3> worldWps;
        worldWps.reserve(static_cast<size_t>(cfg.numWaypoints));
        for (int i = 0; i < cfg.numWaypoints; ++i) {
            worldWps.push_back(cfg.centerPos + cfg.localWaypoints[i]);
        }

        TrackConfig tcfg;
        tcfg.trackGauge        = 1.10f;
        tcfg.railRadius        = 0.085f;
        tcfg.spineRadius       = 0.20f;
        tcfg.spineDropOffset   = 0.42f;
        tcfg.tieSpacing        = 1.75f;
        tcfg.pillarSpacing     = 11.0f;
        tcfg.pillarRadius      = 0.26f;
        tcfg.samplesPerSegment = 20;
        tcfg.railRadialSteps   = 6;
        tcfg.maxBankDeg        = 28.0f;
        tcfg.bankScale         = 9.5f;
        tcfg.isMonorailBeam    = false;
        tcfg.railColor         = cfg.railColor;
        tcfg.spineColor        = cfg.spineColor;
        tcfg.tieColor          = cfg.tieColor;
        tcfg.pillarColor       = cfg.pillarColor;

        track.buildFromClosedWaypoints(worldWps, 0.42f, tcfg);
        headDistance = track.getTotalLength() * cfg.initialPhase01;
        initialized_ = true;
    }

    void update(float dt) {
        if (!initialized_ || track.getTotalLength() <= 1e-3f) return;

        TrackFrame curF = track.evaluateFrameAtDistance(headDistance);
        // Điều biến vận tốc nhẹ theo độ cao để tàu tăng tốc khi đổ dốc và chậm lại khi leo đồi
        float refY = 5.5f;
        float vSq  = cfg.baseSpeedMps * cfg.baseSpeedMps + 10.5f * (refY - curF.position.y);
        float spd  = std::clamp(std::sqrt(std::max(16.0f, vSq)), 4.2f, 13.0f);
        headDistance = track.wrapDistance(headDistance + spd * dt);
    }

    void draw(bool isNightOrSunset, bool showNormals, bool smoothShading = true) const {
        if (!initialized_) return;

        if (!showNormals) {
            GLuint& listId = smoothShading ? staticListSmooth_ : staticListFlat_;
            if (listId == 0) {
                listId = glGenLists(1);
                if (listId != 0) {
                    glNewList(listId, GL_COMPILE);
                    drawStaticTrackAndStation(false, smoothShading);
                    glEndList();
                }
            }
            if (listId != 0) {
                glCallList(listId);
            } else {
                drawStaticTrackAndStation(false, smoothShading);
            }
        } else {
            drawStaticTrackAndStation(true, smoothShading);
        }

        // Vẽ đoàn tàu đang chuyển động trên đường ray
        drawMovingTrain(isNightOrSunset, showNormals, smoothShading);
    }

private:
    bool           initialized_      = false;
    mutable GLuint staticListSmooth_ = 0;
    mutable GLuint staticListFlat_   = 0;
    mutable GLuint carListSmooth_    = 0;
    mutable GLuint carListFlat_      = 0;

    void drawStaticTrackAndStation(bool showNormals, bool smoothShading) const {
        // 1. Vẽ đường ray đôi, ống xương sống, tà-vẹt và trụ đỡ (BezierTrack)
        track.draw(showNormals, true);

        // 2. Vẽ nhà ga đón khách hoặc Tòa nhà giả núi đá Paradise Fall (100% liền khối)
        if (cfg.isStoneMountain) {
            drawStoneMountainBuilding(showNormals, smoothShading);
        } else if (cfg.stationStyle == 1) {
            drawMinangkabauSaddleStation(showNormals, smoothShading);
        } else if (cfg.stationStyle == 2) {
            drawWesternPortGableStation(showNormals, smoothShading);
        } else {
            drawCompactBoardingStation(showNormals, smoothShading);
        }
    }

    /**
     * @brief Đợt 8b: Nhà ga mái yên ngựa cong kiểu Minangkabau / Batak (Indonesia) cho Garuda Valley (BP15)
     *        Đặt gọn trong khung bao vệ tinh hiện có, có cầu thang lên sàn đón khách và hai đầu sừng mái vút cao.
     */
    void drawMinangkabauSaddleStation(bool showNormals, bool smoothShading) const {
        namespace MB = Geometry::MeshBuilders;
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float w = cfg.stationWidth;  // 6.4 m
        const float d = cfg.stationDepth;  // 10.5 m

        Geometry::PolygonMesh m("GarudaMinangkabauStation");
        const MathCG::ColorRGB plinthCol(0.46f, 0.44f, 0.41f);
        const MathCG::ColorRGB stoneCol(0.64f, 0.60f, 0.53f);
        const MathCG::ColorRGB woodDark(0.38f, 0.22f, 0.14f);
        const MathCG::ColorRGB pillarRed(0.68f, 0.22f, 0.14f);
        const MathCG::ColorRGB wallCream(0.88f, 0.80f, 0.64f);
        const MathCG::ColorRGB roofTerracotta(0.76f, 0.25f, 0.13f);
        const MathCG::ColorRGB roofRidgeGold(0.95f, 0.76f, 0.20f);

        // 1. Bệ đá & sàn nhà ga nâng cao (y = 0 .. 2.45 m) kèm cầu thang bậc phía Nam (+Z)
        MB::appendBoxByYRange(m, 0.0f, 0.00f, 0.42f, 0.0f, w + 0.8f, d + 0.8f, plinthCol);
        MB::appendBoxByYRange(m, 0.0f, 0.38f, 2.30f, 0.0f, w,        d,        stoneCol);
        MB::appendBoxByYRange(m, 0.0f, 2.25f, 2.50f, 0.0f, w + 0.4f, d + 0.4f, woodDark);
        MB::appendStaircase(m, 0.0f, d * 0.5f, 0, 1, 0.0f, 2.8f, 2.2f, 2.30f, 7, stoneCol, plinthCol);

        // 2. Tường hậu chạm khắc phía Tây (-X), lan can gỗ & 6 cột trụ đỡ mái (y = 2.48 .. 5.75 m)
        MB::appendBoxByYRange(m, -w * 0.38f, 2.48f, 5.70f, 0.0f, w * 0.20f, d * 0.88f, wallCream);
        MB::appendBoxByYRange(m, -w * 0.27f, 3.30f, 4.90f, 0.0f, 0.12f,     d * 0.72f, woodDark);
        // Lan can gỗ mặt trước & hai đầu hồi
        MB::appendBoxByYRange(m,  w * 0.44f, 2.48f, 3.18f,  d * 0.32f, 0.18f, d * 0.28f, woodDark);
        MB::appendBoxByYRange(m,  w * 0.44f, 2.48f, 3.18f, -d * 0.32f, 0.18f, d * 0.28f, woodDark);

        const float colX[2] = { -w * 0.42f, w * 0.42f };
        const float colZ[3] = { -d * 0.42f, 0.0f, d * 0.42f };
        for (float cx : colX) {
            for (float cz : colZ) {
                MB::appendBoxByYRange(m, cx, 2.48f, 2.85f, cz, 0.62f, 0.62f, plinthCol);
                MB::appendBoxByYRange(m, cx, 2.82f, 5.72f, cz, 0.46f, 0.46f, pillarRed);
            }
        }
        // Hệ xà dầm cổ diềm đỡ mái
        MB::appendBoxByYRange(m, 0.0f, 5.58f, 5.92f, 0.0f, w + 0.3f, d + 0.3f, woodDark);
        MB::appendBoxByYRange(m, 0.0f, 5.88f, 6.08f, 0.0f, w + 0.7f, d + 0.7f, roofRidgeGold);

        // 3. Mái yên ngựa cong Minangkabau (sống nóc võng ở giữa y ~ 7.75 m, hai đầu sừng vút lên y ~ 10.65 m)
        constexpr int kSlicesZ = 14;
        constexpr int kSlopeSeg = 3;
        const float halfSpanZ = d * 0.62f; // ~6.51 m (vươn nhẹ ra hai đầu hồi)
        auto evalProfile = [&](int sz, float& zOut, float& yEaveOut, float& yRidgeOut, float& halfWOut) {
            float u = (2.0f * static_cast<float>(sz) / static_cast<float>(kSlicesZ)) - 1.0f; // [-1, +1]
            float u2 = u * u;
            float uHorn = u2 * (0.35f + 0.65f * std::fabs(u));
            zOut      = u * halfSpanZ;
            yEaveOut  = 5.95f + 0.85f * u2;
            yRidgeOut = 7.75f + 2.90f * uHorn;
            halfWOut  = (w * 0.5f + 1.15f) * (1.0f - 0.16f * u2);
        };

        for (int sz = 0; sz < kSlicesZ; ++sz) {
            float z0, ye0, yr0, hw0;
            float z1, ye1, yr1, hw1;
            evalProfile(sz,     z0, ye0, yr0, hw0);
            evalProfile(sz + 1, z1, ye1, yr1, hw1);

            for (int side : {-1, 1}) {
                for (int k = 0; k < kSlopeSeg; ++k) {
                    float t0 = static_cast<float>(k) / static_cast<float>(kSlopeSeg);
                    float t1 = static_cast<float>(k + 1) / static_cast<float>(kSlopeSeg);
                    // t = 0 tại sống nóc (x = 0), t = 1 tại diềm mái (x = side * hw), độ võng mặt mái pow(1-t, 1.55)
                    float x00 = side * hw0 * t0, y00 = ye0 + (yr0 - ye0) * std::pow(1.0f - t0, 1.55f);
                    float x01 = side * hw0 * t1, y01 = ye0 + (yr0 - ye0) * std::pow(1.0f - t1, 1.55f);
                    float x10 = side * hw1 * t0, y10 = ye1 + (yr1 - ye1) * std::pow(1.0f - t0, 1.55f);
                    float x11 = side * hw1 * t1, y11 = ye1 + (yr1 - ye1) * std::pow(1.0f - t1, 1.55f);

                    MB::appendOrientedPolygon(m, {
                        MathCG::Vec3(x00, y00, z0), MathCG::Vec3(x01, y01, z0),
                        MathCG::Vec3(x11, y11, z1), MathCG::Vec3(x10, y10, z1)
                    }, roofTerracotta, MathCG::Vec3(side * 0.6f, 1.0f, 0.0f));
                }
            }
            // Mặt trần đóng kín dưới đáy mái + sống nóc mạ vàng kèm mào nhọn trang trí
            MB::appendOrientedPolygon(m, {
                MathCG::Vec3(-hw0, ye0, z0), MathCG::Vec3( hw0, ye0, z0),
                MathCG::Vec3( hw1, ye1, z1), MathCG::Vec3(-hw1, ye1, z1)
            }, woodDark, MathCG::Vec3(0.0f, -1.0f, 0.0f));

            float zMid = 0.5f * (z0 + z1);
            float yMid = 0.5f * (yr0 + yr1);
            float dz   = std::fabs(z1 - z0);
            MB::appendBoxByYRange(m, 0.0f, yMid - 0.12f, yMid + 0.14f, zMid, 0.28f, dz + 0.05f, roofRidgeGold);
            m.append(Geometry::PolygonMesh::createPyramid(0.0f, yMid + 0.12f, zMid, 0.26f, dz * 0.72f, 0.42f, roofRidgeGold));
        }

        // Đóng kín 2 tam giác đầu hồi Bắc (-Z) & Nam (+Z) + 2 chóp sừng vàng ở hai đỉnh mái
        for (int endIdx : {0, kSlicesZ}) {
            float ze, ye, yre, hwe;
            evalProfile(endIdx, ze, ye, yre, hwe);
            float nz = (endIdx == 0) ? -1.0f : 1.0f;
            MB::appendOrientedPolygon(m, {
                MathCG::Vec3(-hwe, ye, ze), MathCG::Vec3(hwe, ye, ze), MathCG::Vec3(0.0f, yre, ze)
            }, wallCream, MathCG::Vec3(0.0f, 0.2f, nz));
            m.append(Geometry::PolygonMesh::createPyramid(0.0f, yre + 0.10f, ze, 0.42f, 0.42f, 0.85f, roofRidgeGold));
        }

        m.computeNormals();
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        m.draw(smoothShading, showNormals);
        glPopMatrix();
    }

    /**
     * @brief Đợt 8b: Nhà ga bến cảng phương Tây khung gỗ mái đầu hồi + tháp canh cho Port of Sky Treasure (BP12)
     */
    void drawWesternPortGableStation(bool showNormals, bool smoothShading) const {
        namespace MB = Geometry::MeshBuilders;
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float w = cfg.stationWidth;  // 6.8 m
        const float d = cfg.stationDepth;  // 12.0 m

        Geometry::PolygonMesh m("PortSkyWesternGableStation");
        const MathCG::ColorRGB stonePier(0.48f, 0.46f, 0.43f);
        const MathCG::ColorRGB stoneLight(0.66f, 0.62f, 0.56f);
        const MathCG::ColorRGB timberDark(0.36f, 0.22f, 0.13f);
        const MathCG::ColorRGB timberWarm(0.56f, 0.36f, 0.20f);
        const MathCG::ColorRGB plasterWall(0.89f, 0.83f, 0.70f);
        const MathCG::ColorRGB slateRoof(0.68f, 0.24f, 0.15f);
        const MathCG::ColorRGB brassGold(0.92f, 0.74f, 0.20f);

        // 1. Bệ cầu cảng đá & các cọc gỗ tròn chống quanh mép bến (y = 0 .. 2.45 m) + cầu thang lên phía Nam
        MB::appendBoxByYRange(m, 0.0f, 0.00f, 0.45f, 0.0f, w + 0.9f, d + 0.9f, stonePier);
        MB::appendBoxByYRange(m, 0.0f, 0.40f, 2.30f, 0.0f, w,        d,        stoneLight);
        MB::appendBoxByYRange(m, 0.0f, 2.26f, 2.50f, 0.0f, w + 0.5f, d + 0.5f, timberWarm);
        MB::appendStaircase(m, 0.0f, d * 0.5f, 0, 1, 0.0f, 3.0f, 2.3f, 2.30f, 7, timberWarm, timberDark);

        // Cọc gỗ bến cảng dọc mặt Đông (+X) và Tây (-X)
        const float pilingZ[4] = { -d * 0.42f, -d * 0.14f, d * 0.14f, d * 0.42f };
        for (int sx : {-1, 1}) {
            for (float pz : pilingZ) {
                m.append(Geometry::PolygonMesh::createPrismY(sx * (w * 0.5f + 0.32f), 0.0f, pz, 0.24f, 3.15f, 8, timberDark));
                m.append(Geometry::PolygonMesh::createPrismY(sx * (w * 0.5f + 0.32f), 2.55f, pz, 0.27f, 0.22f, 8, brassGold));
            }
        }

        // 2. Khung nhà gỗ nửa tường (half-timbered) & cột gỗ đỡ mái (y = 2.48 .. 5.95 m)
        MB::appendBoxByYRange(m, -w * 0.38f, 2.48f, 5.88f, 0.0f, w * 0.22f, d * 0.90f, plasterWall);
        // Khung nẹp gỗ đứng & ngang trên tường hậu
        for (float tz : {-d * 0.42f, -d * 0.14f, d * 0.14f, d * 0.42f}) {
            MB::appendBoxByYRange(m, -w * 0.26f, 2.48f, 5.88f, tz, 0.14f, 0.24f, timberDark);
        }
        MB::appendBoxByYRange(m, -w * 0.26f, 4.05f, 4.25f, 0.0f, 0.14f, d * 0.88f, timberDark);

        const float postX[2] = { -w * 0.42f, w * 0.42f };
        const float postZ[4] = { -d * 0.44f, -d * 0.15f, d * 0.15f, d * 0.44f };
        for (float px : postX) {
            for (float pz : postZ) {
                MB::appendBoxByYRange(m, px, 2.48f, 5.90f, pz, 0.46f, 0.46f, timberDark);
            }
        }
        // Lan can gỗ mặt Đông (+X)
        MB::appendBoxByYRange(m, w * 0.42f, 3.10f, 3.28f, -d * 0.30f, 0.18f, d * 0.28f, timberWarm);
        MB::appendBoxByYRange(m, w * 0.42f, 3.10f, 3.28f,  d * 0.30f, 0.18f, d * 0.28f, timberWarm);
        // Dầm đỡ diềm mái
        MB::appendBoxByYRange(m, 0.0f, 5.75f, 6.02f, 0.0f, w + 0.4f, d + 0.4f, timberDark);

        // 3. Mái đầu hồi dốc 2 phía (Western Gable Roof, y = 5.95 .. 8.70 m) + vì kèo tam giác hai đầu hồi
        const float hw = w * 0.5f + 0.95f;
        const float hd = d * 0.5f + 0.85f;
        const float yEave  = 5.95f;
        const float yRidge = 8.70f;

        // Hai mặt mái dốc Đông (+X) và Tây (-X)
        MB::appendOrientedPolygon(m, {
            MathCG::Vec3(0.0f, yRidge, -hd), MathCG::Vec3( hw, yEave, -hd),
            MathCG::Vec3( hw, yEave,   hd), MathCG::Vec3(0.0f, yRidge,  hd)
        }, slateRoof, MathCG::Vec3(1.0f, 1.0f, 0.0f));
        MB::appendOrientedPolygon(m, {
            MathCG::Vec3(0.0f, yRidge,  hd), MathCG::Vec3(-hw, yEave,  hd),
            MathCG::Vec3(-hw, yEave, -hd), MathCG::Vec3(0.0f, yRidge, -hd)
        }, slateRoof, MathCG::Vec3(-1.0f, 1.0f, 0.0f));
        // Đáy trần mái đóng kín
        MB::appendOrientedPolygon(m, {
            MathCG::Vec3(-hw, yEave, -hd), MathCG::Vec3( hw, yEave, -hd),
            MathCG::Vec3( hw, yEave,  hd), MathCG::Vec3(-hw, yEave,  hd)
        }, timberDark, MathCG::Vec3(0.0f, -1.0f, 0.0f));
        // Sống nóc gỗ-đồng
        MB::appendBoxByYRange(m, 0.0f, yRidge - 0.14f, yRidge + 0.14f, 0.0f, 0.32f, hd * 2.0f + 0.1f, timberDark);

        // Hai tam giác đầu hồi Bắc (-Z) & Nam (+Z) kèm trụ kèo giữa (king-post truss)
        for (int sz : {-1, 1}) {
            float zGable = sz * (hd - 0.15f);
            MB::appendOrientedPolygon(m, {
                MathCG::Vec3(-hw + 0.2f, yEave, zGable),
                MathCG::Vec3( hw - 0.2f, yEave, zGable),
                MathCG::Vec3(0.0f, yRidge - 0.08f, zGable)
            }, plasterWall, MathCG::Vec3(0.0f, 0.0f, static_cast<float>(sz)));
            MB::appendBoxByYRange(m, 0.0f, yEave, yRidge - 0.1f, zGable + sz * 0.08f, 0.30f, 0.18f, timberDark);
            MB::appendBoxByYRange(m, 0.0f, yEave + 0.95f, yEave + 1.18f, zGable + sz * 0.08f, hw * 1.15f, 0.18f, timberDark);
        }

        // 4. Tháp canh cảng biển (Harbor Cupola Lantern) mọc trên giữa sống mái (y = 7.85 .. 10.85 m)
        MB::appendBoxByYRange(m, 0.0f, 7.80f, 9.45f, 0.0f, 2.55f, 2.75f, plasterWall);
        MB::appendBoxByYRange(m, 0.0f, 8.35f, 9.05f, 0.0f, 2.65f, 1.45f, timberDark);
        MB::appendBoxByYRange(m, 0.0f, 8.35f, 9.05f, 0.0f, 1.45f, 2.85f, timberDark);
        MB::appendBoxByYRange(m, 0.0f, 9.38f, 9.62f, 0.0f, 2.95f, 3.15f, timberDark);
        m.append(Geometry::PolygonMesh::createPyramid(0.0f, 9.58f, 0.0f, 3.15f, 3.35f, 1.05f, slateRoof));
        m.append(Geometry::PolygonMesh::createPyramid(0.0f, 10.55f, 0.0f, 0.36f, 0.36f, 0.45f, brassGold));

        m.computeNormals();
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        m.draw(smoothShading, showNormals);
        glPopMatrix();
    }

    void drawCompactBoardingStation(bool showNormals, bool smoothShading) const {
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float w = cfg.stationWidth;
        const float d = cfg.stationDepth;
        const float h = cfg.stationHeight;

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);

        // Bệ sàn nhà ga liền khối từ mặt đất y = 0 lên cao độ ray đón khách (~2.3m)
        auto baseMesh = Geometry::PolygonMesh::createBox(
            0.0f, 1.15f, 0.0f,
            w, 2.30f, d,
            MathCG::ColorRGB(0.52f, 0.50f, 0.48f)
        );
        baseMesh.draw(smoothShading, showNormals);

        // Tường chắn lưng nhà ga và 4 cột trụ góc đỡ mái
        auto backWall = Geometry::PolygonMesh::createBox(
            -w * 0.36f, 2.30f + h * 0.5f, 0.0f,
            w * 0.24f, h, d * 0.92f,
            cfg.stationWallColor
        );
        backWall.draw(smoothShading, showNormals);

        const float colX[2] = { -w * 0.42f, w * 0.42f };
        const float colZ[2] = { -d * 0.42f, d * 0.42f };
        for (int ix = 0; ix < 2; ++ix) {
            for (int iz = 0; iz < 2; ++iz) {
                auto col = Geometry::PolygonMesh::createBox(
                    colX[ix], 2.30f + h * 0.5f, colZ[iz],
                    0.45f, h, 0.45f,
                    MathCG::ColorRGB(0.62f, 0.22f, 0.16f)
                );
                col.draw(smoothShading, showNormals);
            }
        }

        // Dầm cổ diềm đỡ mái tại y = 2.30 + h
        auto beam = Geometry::PolygonMesh::createBox(
            0.0f, 2.30f + h - 0.15f, 0.0f,
            w * 0.94f, 0.32f, d * 0.94f,
            cfg.stationWallColor
        );
        beam.draw(smoothShading, showNormals);

        // Mái ngói cong 4 mặt ăn khớp trực tiếp lên đỉnh cột và dầm
        auto roof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 2.30f + h, 0.0f,
            w + 1.8f, d + 1.8f, 2.1f,
            0.35f,
            cfg.stationRoofColor
        );
        roof.draw(smoothShading, showNormals);

        glPopMatrix();
    }

    void drawStoneMountainBuilding(bool showNormals, bool smoothShading) const {
        // Tòa nhà giả núi đá Paradise Fall tại PARADISE_FALL_BLDG_POS (-179.8, 0, -91.1)
        // Tổ hợp nhiều khối đá nâu-xám chồng nhiều tầng, bờ vách gồ ghề với góc xoay/độ nghiêng khác nhau,
        // có hang đá & cổng vòm ở mặt Đông và mặt Tây-Bắc (100% liền khối).
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float w = cfg.stationWidth; // 24.0 m
        const float d = cfg.stationDepth; // 14.0 m

        const MathCG::ColorRGB stoneDark  (0.38f, 0.34f, 0.29f);
        const MathCG::ColorRGB stoneMid   (0.47f, 0.42f, 0.36f);
        const MathCG::ColorRGB stoneWarm  (0.54f, 0.48f, 0.40f);
        const MathCG::ColorRGB stoneLight (0.60f, 0.54f, 0.45f);
        const MathCG::ColorRGB caveDark   (0.12f, 0.11f, 0.10f);

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);

        // 1. Tầng bệ đá gốc rộng bám đất (y = 0 .. 3.4m) + các mỏm đá chân núi nhô lệch góc
        auto baseCore = Geometry::PolygonMesh::createBox(0.0f, 1.7f, 0.0f, w, 3.4f, d, stoneDark);
        baseCore.draw(smoothShading, showNormals);

        struct RockBlockSpec {
            float tx, ty, tz;
            float rotY, tiltX, tiltZ;
            float sx, sy, sz;
            MathCG::ColorRGB col;
        };

        // 10 khối đá nhiều tầng chồng lên nhau với kích thước, góc xoay và độ nghiêng khác nhau tạo vách núi gồ ghề
        const RockBlockSpec blocks[10] = {
            // Tầng 1 - các vách đá sườn núi xung quanh
            { -6.5f, 2.4f, -2.2f, -14.0f,  4.0f, -5.0f,  9.5f, 4.8f,  9.2f, stoneMid   },
            {  5.2f, 2.3f,  1.8f,  18.0f, -3.0f,  4.0f, 10.2f, 4.6f,  8.6f, stoneWarm  },
            { -1.2f, 2.6f,  3.2f,  -9.0f,  5.0f,  2.0f, 11.0f, 5.2f,  6.8f, stoneDark  },
            {  1.8f, 2.5f, -3.0f,  22.0f, -4.0f, -3.0f, 10.5f, 5.0f,  7.2f, stoneMid   },
            // Tầng 2 - các khối đá giữa núi
            { -4.8f, 5.0f, -0.8f,  16.0f, -6.0f,  5.0f,  8.8f, 4.6f,  7.6f, stoneWarm  },
            {  3.6f, 4.8f, -0.5f, -21.0f,  5.0f, -6.0f,  8.4f, 4.4f,  7.4f, stoneLight },
            { -0.6f, 5.4f,  1.4f,   8.0f,  4.0f,  4.0f,  9.2f, 4.8f,  6.4f, stoneMid   },
            // Tầng 3 - các mỏm đỉnh núi cao (y lên tới ~10.8m)
            { -3.2f, 7.6f, -1.2f, -19.0f,  7.0f, -5.0f,  6.4f, 4.2f,  5.6f, stoneLight },
            {  2.4f, 7.2f,  0.8f,  24.0f, -5.0f,  6.0f,  5.8f, 3.8f,  5.2f, stoneWarm  },
            { -0.4f, 8.6f, -0.2f,  11.0f,  3.0f, -4.0f,  5.0f, 3.6f,  4.6f, stoneMid   }
        };

        for (const auto& b : blocks) {
            glPushMatrix();
            glTranslatef(b.tx, b.ty, b.tz);
            glRotatef(b.rotY,  0.0f, 1.0f, 0.0f);
            glRotatef(b.tiltX, 1.0f, 0.0f, 0.0f);
            glRotatef(b.tiltZ, 0.0f, 0.0f, 1.0f);
            auto m = Geometry::PolygonMesh::createBox(0.0f, 0.0f, 0.0f, b.sx, b.sy, b.sz, b.col);
            m.draw(smoothShading, showNormals);
            glPopMatrix();
        }

        // 3 chỏm đá nhọn (createPyramid) trên đỉnh núi tạo đường viền núi đá tự nhiên
        const float peaks[3][6] = {
            { -3.4f, 9.2f, -1.2f,  4.8f, 4.4f, 3.2f },
            {  2.5f, 8.7f,  0.8f,  4.4f, 4.2f, 2.8f },
            { -0.4f,10.0f, -0.2f,  4.2f, 4.0f, 3.4f }
        };
        for (int i = 0; i < 3; ++i) {
            auto pk = Geometry::PolygonMesh::createPyramid(
                peaks[i][0], peaks[i][1], peaks[i][2],
                peaks[i][3], peaks[i][4], peaks[i][5],
                (i == 1) ? stoneWarm : stoneLight
            );
            pk.draw(smoothShading, showNormals);
        }

        // 2. Hang đá & Cổng vòm chính ở mặt Đông (nhìn ra trục lối đi)
        auto caveEastBox = Geometry::PolygonMesh::createBox(w * 0.46f, 1.6f, 0.0f, 1.8f, 3.2f, 3.8f, caveDark);
        caveEastBox.draw(smoothShading, showNormals);

        auto archEast = Geometry::PolygonMesh::createArchVault(
            0.0f, 0.0f, w * 0.40f, w * 0.54f,
            1.85f, 2.95f, 12,
            stoneWarm
        );
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        archEast.draw(smoothShading, showNormals);
        glPopMatrix();

        // 3. Hang đá & Cổng vòm phụ ở mặt Tây-Bắc (hướng thẳng sang vòng ray số 8 Paradise Fall)
        auto caveWestBox = Geometry::PolygonMesh::createBox(-w * 0.46f, 1.5f, -1.2f, 1.8f, 3.0f, 3.4f, caveDark);
        caveWestBox.draw(smoothShading, showNormals);

        auto archWest = Geometry::PolygonMesh::createArchVault(
            1.2f, 0.0f, -w * 0.54f, -w * 0.40f,
            1.65f, 2.65f, 12,
            stoneMid
        );
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        archWest.draw(smoothShading, showNormals);
        glPopMatrix();

        glPopMatrix();
    }

    void compileSingleCarGeometry(bool showNormals, bool smoothShading) const {
        // Thân toa tàu
        auto body = Geometry::PolygonMesh::createBox(
            0.0f, 0.32f, 0.0f,
            1.18f, 0.58f, 1.92f,
            cfg.carBodyColor
        );
        body.draw(smoothShading, showNormals);

        // Viền mũi / dải trang trí toa tàu
        auto stripe = Geometry::PolygonMesh::createBox(
            0.0f, 0.42f, 0.15f,
            1.24f, 0.22f, 1.45f,
            cfg.carAccentColor
        );
        stripe.draw(smoothShading, showNormals);

        // Tựa lưng ghế & hành khách ngồi trong toa
        auto seat = Geometry::PolygonMesh::createBox(
            0.0f, 0.65f, -0.25f,
            0.88f, 0.42f, 0.35f,
            MathCG::ColorRGB(0.22f, 0.24f, 0.28f)
        );
        seat.draw(smoothShading, showNormals);
    }

    void drawMovingTrain(bool isNightOrSunset, bool showNormals, bool smoothShading) const {
        GLuint& carList = smoothShading ? carListSmooth_ : carListFlat_;
        if (!showNormals && carList == 0) {
            carList = glGenLists(1);
            if (carList != 0) {
                glNewList(carList, GL_COMPILE);
                compileSingleCarGeometry(false, smoothShading);
                glEndList();
            }
        }

        for (int c = 0; c < cfg.numCars; ++c) {
            float d = track.wrapDistance(headDistance - static_cast<float>(c) * cfg.carSpacing);
            TrackFrame f = track.evaluateFrameAtDistance(d);
            MathCG::Vec3 pos = f.position + f.up * 0.36f;

            glPushMatrix();
            // Ma trận định hướng 3D (column-major) từ cơ sở trực chuẩn (right, up, tangent) tại điểm đường ray
            const float orient[16] = {
                f.right.x,   f.right.y,   f.right.z,   0.0f,
                f.up.x,      f.up.y,      f.up.z,      0.0f,
                f.tangent.x, f.tangent.y, f.tangent.z, 0.0f,
                pos.x,       pos.y,       pos.z,       1.0f
            };
            glMultMatrixf(orient);

            if (!showNormals && carList != 0) {
                glCallList(carList);
            } else {
                compileSingleCarGeometry(showNormals, smoothShading);
            }

            // Đèn pha mũi toa đầu tiên khi trời tối / hoàng hôn
            if (c == 0 && isNightOrSunset) {
                GLfloat emit[4] = { 1.0f, 0.95f, 0.55f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                auto lamp = Geometry::PolygonMesh::createBox(
                    0.0f, 0.34f, 0.98f,
                    0.65f, 0.18f, 0.08f,
                    MathCG::ColorRGB(1.0f, 0.95f, 0.55f)
                );
                lamp.draw(smoothShading, showNormals);
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }

            glPopMatrix();
        }
    }
};

/**
 * @brief Khởi tạo 4 trò tàu lượn nhỏ Đợt 5A dùng chung lớp MiniCoaster từ hằng số ParkLayout.h.
 */
inline void initWave5aMiniCoasters(std::array<MiniCoaster, 4>& coasters) {
    // 1. Garuda Valley (-124.1, 0, +155.7) - Ray xanh lá/cam vàng, nhà ga mái ngói ở cạnh Tây
    {
        MiniCoasterConfig c;
        c.centerPos        = ParkLayout::GARUDA_VALLEY_CENTER;
        c.localWaypoints   = ParkLayout::GARUDA_VALLEY_LOCAL_WPS;
        c.numWaypoints     = ParkLayout::NUM_MINI_COASTER_WAYPOINTS;
        c.railColor        = MathCG::ColorRGB(0.24f, 0.78f, 0.28f);
        c.spineColor       = MathCG::ColorRGB(0.92f, 0.64f, 0.14f);
        c.carBodyColor     = MathCG::ColorRGB(0.95f, 0.42f, 0.12f);
        c.carAccentColor   = MathCG::ColorRGB(1.00f, 0.88f, 0.22f);
        c.numCars          = 3;
        c.baseSpeedMps     = 7.6f;
        c.initialPhase01   = 0.15f;
        c.stationWorldPos  = ParkLayout::GARUDA_VALLEY_CENTER + MathCG::Vec3(-14.0f, 0.0f, -3.5f);
        c.stationWidth     = 6.4f;
        c.stationDepth     = 10.5f;
        c.stationHeight    = 4.2f;
        c.stationWallColor = MathCG::ColorRGB(0.86f, 0.78f, 0.62f);
        c.stationRoofColor = MathCG::ColorRGB(0.78f, 0.26f, 0.14f);
        c.isStoneMountain  = false;
        c.stationStyle     = 1; // Đợt 8b: Mái yên ngựa cong Minangkabau (BP15)
        coasters[0].init(c);
    }

    // 2. Port of Sky Treasure (-224.0, 0, +22.0) - Ray xanh ngọc/vàng đồng, nhà ga bến cảng gỗ mái đầu hồi ở phía Tây-Nam
    {
        MiniCoasterConfig c;
        c.centerPos        = ParkLayout::PORT_SKY_TREASURE_CENTER;
        c.localWaypoints   = ParkLayout::PORT_SKY_TREASURE_LOCAL_WPS;
        c.numWaypoints     = ParkLayout::NUM_MINI_COASTER_WAYPOINTS;
        c.railColor        = MathCG::ColorRGB(0.14f, 0.74f, 0.86f);
        c.spineColor       = MathCG::ColorRGB(0.92f, 0.72f, 0.18f);
        c.carBodyColor     = MathCG::ColorRGB(0.16f, 0.52f, 0.88f);
        c.carAccentColor   = MathCG::ColorRGB(0.98f, 0.84f, 0.24f);
        c.numCars          = 3;
        c.baseSpeedMps     = 8.4f;
        c.initialPhase01   = 0.42f;
        c.stationWorldPos  = ParkLayout::PORT_SKY_TREASURE_CENTER + MathCG::Vec3(-13.2f, 0.0f, -4.0f);
        c.stationWidth     = 6.8f;
        c.stationDepth     = 12.0f;
        c.stationHeight    = 4.4f;
        c.stationWallColor = MathCG::ColorRGB(0.85f, 0.79f, 0.66f);
        c.stationRoofColor = MathCG::ColorRGB(0.72f, 0.22f, 0.14f);
        c.isStoneMountain  = false;
        c.stationStyle     = 2; // Đợt 8b: Nhà ga bến cảng khung gỗ mái đầu hồi + tháp canh (BP12)
        coasters[1].init(c);
    }

    // 3. Paradise Fall (-199.0, 0, -119.9) - Ray số 8 đỏ cam/vàng kèm Tòa nhà đá PARADISE_FALL_BLDG_POS (-179.8, 0, -91.1)
    {
        MiniCoasterConfig c;
        c.centerPos        = ParkLayout::PARADISE_FALL_TRACK_CENTER;
        c.localWaypoints   = ParkLayout::PARADISE_FALL_LOCAL_WPS;
        c.numWaypoints     = ParkLayout::NUM_MINI_COASTER_WAYPOINTS;
        c.railColor        = MathCG::ColorRGB(0.94f, 0.36f, 0.16f);
        c.spineColor       = MathCG::ColorRGB(0.78f, 0.56f, 0.24f);
        c.carBodyColor     = MathCG::ColorRGB(0.88f, 0.22f, 0.18f);
        c.carAccentColor   = MathCG::ColorRGB(0.96f, 0.78f, 0.24f);
        c.numCars          = 3;
        c.baseSpeedMps     = 8.0f;
        c.initialPhase01   = 0.68f;
        c.stationWorldPos  = ParkLayout::PARADISE_FALL_BLDG_POS;
        c.stationWidth     = 24.0f;
        c.stationDepth     = 14.0f;
        c.stationHeight    = 7.8f;
        c.stationWallColor = MathCG::ColorRGB(0.54f, 0.51f, 0.46f);
        c.stationRoofColor = MathCG::ColorRGB(0.68f, 0.26f, 0.16f);
        c.isStoneMountain  = true;
        coasters[2].init(c);
    }

    // 4. Highway Boat (-176.0, 0, -214.0) - Ray xanh dương/cam, toa thuyền chạy gần Ga Monorail Nepal
    {
        MiniCoasterConfig c;
        c.centerPos        = ParkLayout::HIGHWAY_BOAT_CENTER;
        c.localWaypoints   = ParkLayout::HIGHWAY_BOAT_LOCAL_WPS;
        c.numWaypoints     = ParkLayout::NUM_MINI_COASTER_WAYPOINTS;
        c.railColor        = MathCG::ColorRGB(0.20f, 0.54f, 0.92f);
        c.spineColor       = MathCG::ColorRGB(0.94f, 0.58f, 0.16f);
        c.carBodyColor     = MathCG::ColorRGB(0.96f, 0.52f, 0.14f);
        c.carAccentColor   = MathCG::ColorRGB(0.22f, 0.76f, 0.96f);
        c.numCars          = 3;
        c.baseSpeedMps     = 7.8f;
        c.initialPhase01   = 0.85f;
        c.stationWorldPos  = ParkLayout::HIGHWAY_BOAT_CENTER + MathCG::Vec3(-13.8f, 0.0f, -2.5f);
        c.stationWidth     = 6.4f;
        c.stationDepth     = 10.5f;
        c.stationHeight    = 4.2f;
        c.stationWallColor = MathCG::ColorRGB(0.84f, 0.78f, 0.66f);
        c.stationRoofColor = MathCG::ColorRGB(0.18f, 0.44f, 0.72f);
        c.isStoneMountain  = false;
        coasters[3].init(c);
    }
}

} // namespace AsiaParkRides

#endif // MINI_COASTER_H
