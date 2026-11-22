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

    void drawStaticTrackAndStation(bool showNormals, bool smoothShading) const {
        // 1. Vẽ đường ray đôi, ống xương sống, tà-vẹt và trụ đỡ (BezierTrack)
        track.draw(showNormals, true);

        // 2. Vẽ nhà ga đón khách hoặc Tòa nhà giả núi đá Paradise Fall (100% liền khối)
        if (cfg.isStoneMountain) {
            drawStoneMountainBuilding(showNormals, smoothShading);
        } else {
            drawCompactBoardingStation(showNormals, smoothShading);
        }
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

    void drawMovingTrain(bool isNightOrSunset, bool showNormals, bool smoothShading) const {
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
        coasters[0].init(c);
    }

    // 2. Port of Sky Treasure (-224.0, 0, +22.0) - Ray xanh ngọc/vàng đồng, nhà ga mái ngói ở phía Tây-Nam
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
