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

enum class MiniStationStyle {
    COMPACT_ASIAN      = 0,
    GARUDA_MINANGKABAU = 1,
    PORT_TIMBER_GABLE  = 2
};

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
    bool                isStoneMountain   = false; // Riêng Paradise Fall có tòa nhà giả núi đá lớn + lầu ga 2 tầng
    MiniStationStyle    stationStyle      = MiniStationStyle::COMPACT_ASIAN;
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
        // 1. Vẽ đường ray đôi, ống xương sống, tà-vẹt và trụ đỡ (BezierTrack - giữ nguyên 100%)
        track.draw(showNormals, true);

        // 2. Vẽ nhà ga đón khách (Lô A2 nâng cấp 3 nhà ga theo [Bản vẽ minh họa AI, tham khảo hình dạng])
        if (cfg.isStoneMountain) {
            drawStoneMountainBuilding(showNormals, smoothShading);
        } else if (cfg.stationStyle == MiniStationStyle::GARUDA_MINANGKABAU) {
            drawGarudaMinangkabauStation(showNormals, smoothShading);
        } else if (cfg.stationStyle == MiniStationStyle::PORT_TIMBER_GABLE) {
            drawPortTimberGableStation(showNormals, smoothShading);
        } else {
            drawCompactBoardingStation(showNormals, smoothShading);
        }
    }

    /**
     * @brief Lô A2 - Nhà ga Garuda Valley kiểu Minangkabau [Bản vẽ minh họa AI, tham khảo hình dạng]
     *        Mái yên ngựa cong vểnh 2 đầu sừng + mái phụ lối vào, đặt trên cột với bệ đón +2.05m & cầu thang phía Tây;
     *        chừa hành lang ray dx in [-1.75, +1.55] thông thoáng 100%, không chạm ray/đoàn tàu.
     */
    void drawGarudaMinangkabauStation(bool showNormals, bool smoothShading) const {
        using Mesh = Geometry::PolygonMesh;
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float deckY = ParkLayout::GARUDA_STATION_DECK_Y;      // 2.05m
        const float eaveY = ParkLayout::GARUDA_STATION_EAVE_Y;      // 5.35m
        const float hornH = ParkLayout::GARUDA_STATION_ROOF_HORN_H; // 4.65m -> đỉnh ~10.0m

        const MathCG::ColorRGB plinthCol (0.50f, 0.48f, 0.44f);
        const MathCG::ColorRGB deckCol   (0.80f, 0.72f, 0.56f);
        const MathCG::ColorRGB woodCol   (0.46f, 0.24f, 0.14f);
        const MathCG::ColorRGB pillarCol (0.64f, 0.20f, 0.14f);
        const MathCG::ColorRGB trimCol   (0.24f, 0.18f, 0.15f);
        const MathCG::ColorRGB roofCol   (0.62f, 0.22f, 0.14f);

        Mesh st("GarudaMinangkabauStation");

        // 1. Bệ móng nền & sàn chờ đón khách phía Tây đường ray (dx in [-4.85, -1.25], y = 0 .. 2.05m)
        Mesh::appendBoxByYRange(st, -3.05f, 0.00f, 0.30f, 0.0f, 4.10f, 9.60f, plinthCol);
        Mesh::appendBoxByYRange(st, -3.05f, 1.82f, deckY, 0.0f, 3.65f, 9.20f, deckCol);
        // Phần bệ dưới đỡ sàn đón khách với 6 cột chân đế & vách lửng dưới gầm sàn
        Mesh::appendBoxByYRange(st, -3.20f, 0.25f, 1.85f, 0.0f, 2.50f, 7.60f, plinthCol);
        // Sàn bệ nền ray dưới hành lang (y = 0 .. 1.82m, nằm hẳn dưới tà-vẹt y ~ 2.2m)
        Mesh::appendBoxByYRange(st,  0.15f, 0.00f, 1.82f, 0.0f, 3.00f, 8.80f, plinthCol);

        // Lan can bảo vệ sàn đón khách cạnh Tây (chừa lối cầu thang dz in [-0.8, +1.8]), Bắc & Nam
        Mesh::appendBoxByYRange(st, -4.75f, deckY, deckY + 0.88f, -2.75f, 0.16f, 3.70f, woodCol);
        Mesh::appendBoxByYRange(st, -4.75f, deckY, deckY + 0.88f,  3.20f, 0.16f, 2.80f, woodCol);
        Mesh::appendBoxByYRange(st, -3.05f, deckY, deckY + 0.88f, -4.52f, 3.45f, 0.16f, woodCol);
        Mesh::appendBoxByYRange(st, -3.05f, deckY, deckY + 0.88f,  4.52f, 3.45f, 0.16f, woodCol);

        // 2. Cầu thang gỗ-đá từ nền đất lên sàn ga phía Tây (dx = -4.75 xuống -7.50, dz = +0.5)
        st.append(Mesh::createSteppedStaircase(
            -4.75f, 0.50f, -90.0f, 0.00f, deckY,
            2.75f, 2.50f, 7, deckCol, woodCol
        ));

        // 3. Hệ cột gỗ lớn vươn lên đỡ mái yên ngựa (đặt ngoài hành lang ray dx in [-1.75, +1.55]):
        //    - Cạnh Tây (dx = -4.65m): 3 cột tại dz = -3.4, 0.0, +3.4
        //    - Cạnh giữ nhịp cạnh sàn (dx = -2.15m): 3 cột tại dz = -3.4, 0.0, +3.4
        //    - Cạnh Đông bên kia ray (dx = +1.95m): 2 cột tại dz = -3.2, +3.2 (hở ray >= 1.85m!)
        const float zWestCols[3] = { -3.4f, 0.0f, 3.4f };
        for (float cz : zWestCols) {
            Mesh::appendBoxByYRange(st, -4.65f, 0.00f, 0.45f, cz, 0.62f, 0.62f, plinthCol);
            st.append(Mesh::createPrismY(-4.65f, 0.40f, cz, 0.22f, eaveY - 0.35f, 10, pillarCol));
            st.append(Mesh::createPrismY(-2.15f, deckY - 0.10f, cz, 0.19f, eaveY - deckY + 0.15f, 10, pillarCol));
        }
        const float zEastCols[2] = { -3.2f, 3.2f };
        for (float cz : zEastCols) {
            Mesh::appendBoxByYRange(st, 1.95f, 0.00f, 0.45f, cz, 0.62f, 0.62f, plinthCol);
            st.append(Mesh::createPrismY(1.95f, 0.40f, cz, 0.22f, eaveY - 0.35f, 10, pillarCol));
        }

        // Hệ xà dọc & xà ngang liên kết đỉnh cột đỡ khung mái (y = 5.05 .. 5.40m, cao trên đỉnh tàu >= 1.2m)
        Mesh::appendBoxByYRange(st, -4.65f, eaveY - 0.32f, eaveY + 0.04f, 0.0f, 0.36f, 8.80f, trimCol);
        Mesh::appendBoxByYRange(st, -2.15f, eaveY - 0.32f, eaveY + 0.04f, 0.0f, 0.32f, 8.80f, trimCol);
        Mesh::appendBoxByYRange(st,  1.95f, eaveY - 0.32f, eaveY + 0.04f, 0.0f, 0.36f, 8.40f, trimCol);
        for (float cz : zEastCols) {
            Mesh::appendBoxByYRange(st, -1.35f, eaveY - 0.28f, eaveY + 0.06f, cz, 6.80f, 0.34f, trimCol);
        }

        // 4. Mái yên ngựa cong kiểu Minangkabau chính (trục sừng hướng Bắc-Nam) + Mái sừng phụ lối cầu thang phía Tây
        st.append(Mesh::createMinangkabauRoof(
            -1.35f, eaveY, 0.0f,
            7.60f, 9.60f, 2.15f, hornH, 2.30f, 7,
            roofCol, trimCol, 0.0f
        ));
        st.append(Mesh::createMinangkabauRoof(
            -4.10f, eaveY - 0.12f, 0.50f,
            3.30f, 3.60f, 1.35f, 2.75f, 1.15f, 5,
            roofCol, trimCol, -90.0f
        ));

        st.computeNormals();
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        st.draw(smoothShading, showNormals);
        glPopMatrix();
    }

    /**
     * @brief Lô A2 - Nhà ga Port of Sky Treasure bến cảng gỗ [Bản vẽ minh họa AI, tham khảo hình dạng]
     *        Kết cấu bến cảng gỗ, mái 2 dốc có vì kèo tam giác lộ thiên, GIAN GIỮA CAO HƠN HAI BÊN;
     *        sàn đón & cầu thang nằm bên Đông (hướng PATH_WEST_SPINE), hành lang ray phía Tây thông thoáng 100%.
     */
    void drawPortTimberGableStation(bool showNormals, bool smoothShading) const {
        using Mesh = Geometry::PolygonMesh;
        const MathCG::Vec3& p = cfg.stationWorldPos;
        const float deckY   = ParkLayout::PORT_SKY_STATION_DECK_Y; // 2.15m
        const float wingEav = ParkLayout::PORT_SKY_WING_EAVE_Y;    // 5.45m
        const float midEav  = ParkLayout::PORT_SKY_CENTER_EAVE_Y;  // 6.25m

        const MathCG::ColorRGB stonePierCol(0.50f, 0.48f, 0.45f);
        const MathCG::ColorRGB woodDeckCol (0.64f, 0.44f, 0.24f);
        const MathCG::ColorRGB timberCol   (0.42f, 0.25f, 0.14f);
        const MathCG::ColorRGB trussCol    (0.56f, 0.36f, 0.20f);
        const MathCG::ColorRGB darkTrimCol (0.24f, 0.17f, 0.13f);
        const MathCG::ColorRGB roofWingCol (0.58f, 0.24f, 0.16f);
        const MathCG::ColorRGB roofMidCol  (0.68f, 0.28f, 0.16f);

        Mesh st("PortSkyTimberGableStation");

        // 1. Sàn bến cảng gỗ & hệ cọc cầu cảng phía Đông đường ray (dx in [+1.15, +4.45], dz in [-6.5, +6.5])
        Mesh::appendBoxByYRange(st, 2.80f, 0.00f, 0.28f, 0.0f, 3.80f, 13.4f, stonePierCol);
        Mesh::appendBoxByYRange(st, 2.80f, deckY - 0.22f, deckY, 0.0f, 3.30f, 13.0f, woodDeckCol);
        Mesh::appendBoxByYRange(st, 2.80f, deckY - 0.28f, deckY - 0.18f, 0.0f, 3.45f, 13.2f, darkTrimCol);

        // Nền bệ cầu cảng dưới hành lang đường ray (dx in [-2.65, +1.15], y = 0 .. 1.90m - thấp hơn đường ray)
        Mesh::appendBoxByYRange(st, -0.75f, 0.00f, 1.88f, 0.0f, 4.00f, 12.6f, stonePierCol);

        // Lan can gỗ bến cảng dọc cạnh Đông (chừa lối cầu thang dz in [-1.5, +1.5]), Bắc & Nam
        Mesh::appendBoxByYRange(st, 4.35f, deckY, deckY + 0.90f, -4.00f, 0.16f, 4.90f, trussCol);
        Mesh::appendBoxByYRange(st, 4.35f, deckY, deckY + 0.90f,  4.00f, 0.16f, 4.90f, trussCol);
        Mesh::appendBoxByYRange(st, 2.80f, deckY, deckY + 0.90f, -6.40f, 3.20f, 0.16f, trussCol);
        Mesh::appendBoxByYRange(st, 2.80f, deckY, deckY + 0.90f,  6.40f, 3.20f, 0.16f, trussCol);

        // 2. Cầu thang gỗ bến cảng đi xuống hướng Đông (dx = +4.35 xuống +6.95, dz = 0.0)
        st.append(Mesh::createSteppedStaircase(
            4.35f, 0.00f, 90.0f, 0.00f, deckY,
            2.60f, 2.80f, 7, woodDeckCol, timberCol
        ));

        // 3. Hệ cột gỗ chính & trụ đỡ chân đá chia 3 gian (cạnh Tây dx = -2.60m, cạnh Đông dx = +4.30m)
        //    Gian giữa: dz = +-2.55m nâng cao tới midEav (6.25m); Hai đầu hồi Bắc/Nam: dz = +-6.35m cao tới wingEav (5.45m)
        const float xColWest = -2.60f;
        const float xColEast =  4.30f;
        const float zWingPosts[2] = { -6.30f, 6.30f };
        for (float cz : zWingPosts) {
            for (float cx : { xColWest, xColEast }) {
                Mesh::appendBoxByYRange(st, cx, 0.00f, 0.42f, cz, 0.62f, 0.62f, stonePierCol);
                st.append(Mesh::createPrismY(cx, 0.38f, cz, 0.20f, wingEav - 0.34f, 8, timberCol));
            }
            Mesh::appendBoxByYRange(st, 0.85f, wingEav - 0.28f, wingEav + 0.04f, cz, 7.10f, 0.26f, darkTrimCol);
        }
        const float zCenterPosts[2] = { -2.55f, 2.55f };
        for (float cz : zCenterPosts) {
            for (float cx : { xColWest, xColEast }) {
                Mesh::appendBoxByYRange(st, cx, 0.00f, 0.42f, cz, 0.68f, 0.68f, stonePierCol);
                st.append(Mesh::createPrismY(cx, 0.38f, cz, 0.23f, midEav - 0.34f, 8, timberCol));
            }
            // Dầm kẹp gian giữa & thanh chống xiên trang trí đầu cột
            Mesh::appendBoxByYRange(st, 0.85f, midEav - 0.30f, midEav + 0.04f, cz, 7.15f, 0.28f, darkTrimCol);
        }
        // Dầm dọc gác mái hai gian bên (y = wingEav) và hai dầm dọc gian giữa (y = midEav)
        for (float cx : { xColWest, xColEast }) {
            Mesh::appendBoxByYRange(st, cx, wingEav - 0.28f, wingEav + 0.04f, -4.45f, 0.28f, 3.90f, darkTrimCol);
            Mesh::appendBoxByYRange(st, cx, wingEav - 0.28f, wingEav + 0.04f,  4.45f, 0.28f, 3.90f, darkTrimCol);
            Mesh::appendBoxByYRange(st, cx, midEav  - 0.30f, midEav  + 0.04f,  0.00f, 0.32f, 5.40f, darkTrimCol);
        }
        // Tường gỗ chắn gió lửng ở hai gian bên phía Đông (cách hành lang ray, dưới mái bên)
        Mesh::appendBoxByYRange(st, xColEast - 0.10f, wingEav - 1.15f, wingEav - 0.25f, -4.45f, 0.20f, 3.60f, trussCol);
        Mesh::appendBoxByYRange(st, xColEast - 0.10f, wingEav - 1.15f, wingEav - 0.25f,  4.45f, 0.20f, 3.60f, trussCol);

        // 4. Hai mái gỗ 2 dốc cho 2 gian bên thấp (Bắc dz = -4.55m & Nam dz = +4.55m, nóc cao 7.25m)
        st.append(Mesh::createGableRoofWithTruss(
            0.85f, wingEav, -4.55f, 7.10f, 4.10f, 1.80f, 0.55f,
            roofWingCol, darkTrimCol, trussCol, 2, 0.0f
        ));
        st.append(Mesh::createGableRoofWithTruss(
            0.85f, wingEav,  4.55f, 7.10f, 4.10f, 1.80f, 0.55f,
            roofWingCol, darkTrimCol, trussCol, 2, 0.0f
        ));

        // 5. Gian giữa CAO HƠN HAI BÊN xoay ngang lộ rõ vì kèo tam giác hướng Đông (+X) về phía lối đi (nóc cao 8.85m!)
        st.append(Mesh::createGableRoofWithTruss(
            0.85f, midEav, 0.00f, 5.40f, 7.50f, 2.60f, 0.65f,
            roofMidCol, darkTrimCol, trussCol, 2, 90.0f
        ));
        // Cột đỡ trung tâm dầm mái gian giữa tại chuyển tiếp với 2 gian bên
        Mesh::appendBoxByYRange(st, xColWest, wingEav - 0.15f, midEav - 0.20f, 0.0f, 0.26f, 0.26f, timberCol);
        Mesh::appendBoxByYRange(st, xColEast, wingEav - 0.15f, midEav - 0.20f, 0.0f, 0.26f, 0.26f, timberCol);

        st.computeNormals();
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        st.draw(smoothShading, showNormals);
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

        using Mesh = Geometry::PolygonMesh;
        Mesh mountainAndPav("ParadiseFallMountainAndStationA2");

        // 1. Tầng bệ đá gốc rộng bám đất (y = 0 .. 3.4m) + các mỏm đá chân núi nhô lệch góc (giữ nguyên hình học Đợt 5B)
        mountainAndPav.append(Mesh::createBox(0.0f, 1.7f, 0.0f, w, 3.4f, d, stoneDark));

        struct RockBlockSpec {
            float tx, ty, tz;
            float rotY, tiltX, tiltZ;
            float sx, sy, sz;
            MathCG::ColorRGB col;
        };

        // 10 khối đá nhiều tầng chồng lên nhau với kích thước, góc xoay và độ nghiêng khác nhau tạo vách núi gồ ghề
        const RockBlockSpec blocks[10] = {
            { -6.5f, 2.4f, -2.2f, -14.0f,  4.0f, -5.0f,  9.5f, 4.8f,  9.2f, stoneMid   },
            {  5.2f, 2.3f,  1.8f,  18.0f, -3.0f,  4.0f, 10.2f, 4.6f,  8.6f, stoneWarm  },
            { -1.2f, 2.6f,  3.2f,  -9.0f,  5.0f,  2.0f, 11.0f, 5.2f,  6.8f, stoneDark  },
            {  1.8f, 2.5f, -3.0f,  22.0f, -4.0f, -3.0f, 10.5f, 5.0f,  7.2f, stoneMid   },
            { -4.8f, 5.0f, -0.8f,  16.0f, -6.0f,  5.0f,  8.8f, 4.6f,  7.6f, stoneWarm  },
            {  3.6f, 4.8f, -0.5f, -21.0f,  5.0f, -6.0f,  8.4f, 4.4f,  7.4f, stoneLight },
            { -0.6f, 5.4f,  1.4f,   8.0f,  4.0f,  4.0f,  9.2f, 4.8f,  6.4f, stoneMid   },
            { -3.2f, 7.6f, -1.2f, -19.0f,  7.0f, -5.0f,  6.4f, 4.2f,  5.6f, stoneLight },
            {  2.4f, 7.2f,  0.8f,  24.0f, -5.0f,  6.0f,  5.8f, 3.8f,  5.2f, stoneWarm  },
            { -0.4f, 8.6f, -0.2f,  11.0f,  3.0f, -4.0f,  5.0f, 3.6f,  4.6f, stoneMid   }
        };

        for (const auto& b : blocks) {
            Mesh m = Mesh::createBox(0.0f, 0.0f, 0.0f, b.sx, b.sy, b.sz, b.col);
            m.rotateZ(b.tiltZ);
            m.rotateX(b.tiltX);
            m.rotateY(b.rotY);
            m.translate(b.tx, b.ty, b.tz);
            mountainAndPav.append(m);
        }

        // 3 chỏm đá nhọn (createPyramid) trên đỉnh núi tạo đường viền núi đá tự nhiên
        const float peaks[3][6] = {
            { -3.4f, 9.2f, -1.2f,  4.8f, 4.4f, 3.2f },
            {  2.5f, 8.7f,  0.8f,  4.4f, 4.2f, 2.8f },
            { -0.4f,10.0f, -0.2f,  4.2f, 4.0f, 3.4f }
        };
        for (int i = 0; i < 3; ++i) {
            mountainAndPav.append(Mesh::createPyramid(
                peaks[i][0], peaks[i][1], peaks[i][2],
                peaks[i][3], peaks[i][4], peaks[i][5],
                (i == 1) ? stoneWarm : stoneLight
            ));
        }

        // 2. Hang đá & Cổng vòm chính ở mặt Đông (nhìn ra trục lối đi)
        mountainAndPav.append(Mesh::createBox(w * 0.46f, 1.6f, 0.0f, 1.8f, 3.2f, 3.8f, caveDark));
        {
            Mesh archEast = Mesh::createArchVault(0.0f, 0.0f, w * 0.40f, w * 0.54f, 1.85f, 2.95f, 12, stoneWarm);
            archEast.rotateY(90.0f);
            mountainAndPav.append(archEast);
        }

        // 3. Hang đá & Cổng vòm phụ ở mặt Tây-Bắc (hướng thẳng sang vòng ray số 8 Paradise Fall)
        mountainAndPav.append(Mesh::createBox(-w * 0.46f, 1.5f, -1.2f, 1.8f, 3.0f, 3.4f, caveDark));
        {
            Mesh archWest = Mesh::createArchVault(1.2f, 0.0f, -w * 0.54f, -w * 0.40f, 1.65f, 2.65f, 12, stoneMid);
            archWest.rotateY(90.0f);
            mountainAndPav.append(archWest);
        }

        // 4. Lô A2: LẦU GA MÁI CONG 2 TẦNG TRÊN BỆ CÓ CẦU THANG nối cạnh Bắc-Đông núi đá Paradise Fall
        //    [Bản vẽ minh họa AI, tham khảo hình dạng] - giữ nguyên núi đá, cách đường ray số 8 >= 7m!
        {
            Mesh& pav = mountainAndPav;
            const float ox = ParkLayout::PARADISE_FALL_GAZEBO_OFFSET.x; // +7.3m (world X = -172.5)
            const float oz = ParkLayout::PARADISE_FALL_GAZEBO_OFFSET.z; // -12.7m (world Z = -103.8)
            const float dkY = ParkLayout::PARADISE_FALL_GAZEBO_DECK_Y;  // +2.60m

            const MathCG::ColorRGB deckStoneCol(0.76f, 0.72f, 0.64f);
            const MathCG::ColorRGB railStoneCol(0.86f, 0.84f, 0.80f);
            const MathCG::ColorRGB wallCreamCol(0.86f, 0.78f, 0.62f);
            const MathCG::ColorRGB pillarRedCol(0.74f, 0.18f, 0.14f);
            const MathCG::ColorRGB beamDarkCol (0.24f, 0.19f, 0.16f);
            const MathCG::ColorRGB tileRoofCol (0.72f, 0.24f, 0.14f);

            // 4.1. Móng & 8 trụ đá nâng bệ sàn ga cao +2.60m + nhịp cầu nối thẳng vào sườn Bắc núi đá
            Mesh::appendBoxByYRange(pav, ox, 0.00f, 0.34f, oz, 10.6f, 8.2f, stoneDark);
            Mesh::appendBoxByYRange(pav, ox, dkY - 0.34f, dkY, oz, 10.2f, 7.8f, deckStoneCol);
            // Nhịp cầu nối đặc liền khối từ mép Nam bệ sàn ga (oz + 3.9 = -8.8) sang chân sườn núi đá (z = -6.8)
            Mesh::appendBoxByYRange(pav, ox - 1.2f, 0.00f, dkY, -7.85f, 4.4f, 2.3f, stoneMid);
            Mesh::appendBoxByYRange(pav, ox - 1.2f, dkY - 0.25f, dkY + 0.02f, -7.85f, 4.6f, 2.4f, deckStoneCol);

            for (float px : { ox - 4.6f, ox - 1.5f, ox + 1.5f, ox + 4.6f }) {
                for (float pz : { oz - 3.4f, oz + 3.4f }) {
                    Mesh::appendBoxByYRange(pav, px, 0.30f, dkY - 0.30f, pz, 0.72f, 0.72f, stoneWarm);
                }
            }

            // Lan can đá trắng bao quanh bệ sàn +2.60m (chừa lối cầu thang bên Đông & cầu nối sang núi đá bên Nam)
            Mesh::appendBoxByYRange(pav, ox, dkY, dkY + 0.84f, oz - 3.78f, 9.9f, 0.24f, railStoneCol);
            Mesh::appendBoxByYRange(pav, ox - 4.95f, dkY, dkY + 0.84f, oz, 0.24f, 7.6f, railStoneCol);
            Mesh::appendBoxByYRange(pav, ox + 2.55f, dkY, dkY + 0.84f, oz + 3.78f, 4.9f, 0.24f, railStoneCol);
            Mesh::appendBoxByYRange(pav, ox - 4.10f, dkY, dkY + 0.84f, oz + 3.78f, 1.8f, 0.24f, railStoneCol);
            Mesh::appendBoxByYRange(pav, ox + 4.95f, dkY, dkY + 0.84f, oz - 2.45f, 0.24f, 2.8f, railStoneCol);
            Mesh::appendBoxByYRange(pav, ox + 4.95f, dkY, dkY + 0.84f, oz + 2.45f, 0.24f, 2.8f, railStoneCol);

            // 4.2. Cầu thang đá bên Đông dẫn từ đất y = 0.0 lên bệ sàn +2.60m (ox + 5.05 -> ox + 7.75)
            pav.append(Mesh::createSteppedStaircase(
                ox + 4.95f, oz, 90.0f, 0.00f, dkY,
                2.80f, 2.80f, 8, deckStoneCol, railStoneCol
            ));

            // 4.3. Lầu ga Tầng 1 (8 cột đỏ + tường hậu chắn gió phía Bắc + dầm + mái cong 4 dốc 9.4 x 7.0m)
            const float t1TopY = 5.45f;
            for (float px : { ox - 4.1f, ox - 1.4f, ox + 1.4f, ox + 4.1f }) {
                for (float pz : { oz - 2.9f, oz + 2.9f }) {
                    pav.append(Mesh::createPrismY(px, dkY, pz, 0.20f, t1TopY - dkY + 0.05f, 10, pillarRedCol));
                }
            }
            Mesh::appendBoxByYRange(pav, ox, dkY, t1TopY - 0.10f, oz - 2.75f, 8.2f, 0.32f, wallCreamCol);
            Mesh::appendBoxByYRange(pav, ox, t1TopY - 0.15f, t1TopY + 0.22f, oz, 9.2f, 6.8f, beamDarkCol);
            pav.append(Mesh::createCurvedAsianRoof(
                ox, t1TopY + 0.18f, oz, 9.4f, 7.0f, 1.65f, 1.35f, tileRoofCol
            ));

            // 4.4. Lầu ga Tầng 2 thu gọn mọc liền từ sống mái tầng 1 (y = 6.30 .. 8.35m) + mái cong tầng 2
            const float t2BotY = 6.30f;
            const float t2TopY = 8.35f;
            Mesh::appendBoxByYRange(pav, ox, t2BotY, t2TopY, oz, 5.2f, 3.8f, wallCreamCol);
            for (float sx : { -2.65f, 2.65f }) {
                for (float sz : { -1.95f, 1.95f }) {
                    pav.append(Mesh::createPrismY(ox + sx, t2BotY, oz + sz, 0.16f, t2TopY - t2BotY + 0.05f, 8, pillarRedCol));
                }
            }
            Mesh::appendBoxByYRange(pav, ox, t2TopY - 0.12f, t2TopY + 0.18f, oz, 5.8f, 4.4f, beamDarkCol);
            pav.append(Mesh::createCurvedAsianRoof(
                ox, t2TopY + 0.14f, oz, 6.0f, 4.6f, 1.50f, 1.15f, tileRoofCol
            ));

            pav.computeNormals();
            pav.draw(smoothShading, showNormals);
        }

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
        c.stationStyle     = MiniStationStyle::GARUDA_MINANGKABAU;
        coasters[0].init(c);
    }

    // 2. Port of Sky Treasure (-224.0, 0, +22.0) - Ray xanh ngọc/vàng đồng, nhà ga bến cảng gỗ 2 dốc vì kèo
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
        c.stationStyle     = MiniStationStyle::PORT_TIMBER_GABLE;
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
