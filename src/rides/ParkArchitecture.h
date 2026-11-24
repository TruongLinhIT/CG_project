#ifndef PARK_ARCHITECTURE_H
#define PARK_ARCHITECTURE_H

/**
 * @file ParkArchitecture.h
 * @brief Chương 4, 5 & 6 (Đợt 4): Cụm kiến trúc Quảng trường Trung tâm, Hai cụm Chòi mái ngói Quảng trường,
 *        Sân khấu & Nhà ga chân Sun Wheel (hộp sân khấu + hai màn hình tròn phát sáng + hai chòi hai bên)
 *        và Hai Chòi vọng cảnh bờ Nam hồ nước.
 *
 * Nguyên tắc hình học Đợt 4:
 *  - Mọi bộ phận kiến trúc đều LIỀN KHỐI 100% (mái đặt trực tiếp lên xà/cột/tường, không hở, không lơ lửng).
 *  - Cụm Sân khấu chân Sun Wheel đặt tại ParkLayout::SUN_WHEEL_STAGE_POS (0.0, 0.0, -17.7)
 *    cách hoàn toàn 4 bệ móng & chân ống thép chữ A của Sun Wheel (0% giao cắt).
 *  - Toàn bộ hằng số vị trí và kích thước đọc từ ParkLayout.h.
 *  - Sử dụng OpenGL Display List cho phần hình học tĩnh để tối ưu hóa FPS.
 */

#include <GL/freeglut.h>
#include <cmath>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class ParkArchitecture {
public:
    float spinAngleDeg = 0.0f;

    ParkArchitecture() {
        buildStaticMeshes();
    }

    void update(float dt) {
        spinAngleDeg = std::fmod(spinAngleDeg + 52.0f * dt, 360.0f);
    }

    /**
     * @brief Vẽ toàn bộ Quảng trường lát gạch, Sân khấu chân Sun Wheel, các Chòi mái ngói Đợt 4
     *        và toàn bộ các công trình kiến trúc + đu quay tròn Đợt 5B
     */
    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        // 1. Mặt phẳng Quảng trường Trung tâm lát gạch, Sân bông hoa (#7-#8) & Nền lát hình sao Angkor (#19)
        if (!showNormals) {
            ensureDisplayLists();
            if (plazaStaticList_ != 0) {
                glCallList(plazaStaticList_);
            } else {
                drawTiledCentralPlaza();
            }
        } else {
            drawTiledCentralPlaza();
        }

        // 2. Các khối kiến trúc lưới đa giác tĩnh Đợt 4 & Đợt 5B (dùng Display List khi không bật debug normals)
        if (!showNormals) {
            if (smoothShading && archSmoothList != 0) {
                glCallList(archSmoothList);
            } else if (!smoothShading && archFlatList != 0) {
                glCallList(archFlatList);
            } else {
                staticArchMesh.draw(smoothShading, false);
            }
        } else {
            staticArchMesh.draw(smoothShading, true);
        }

        // 3. Vẽ hai màn hình tròn phát sáng lớn, màn hình LED sân khấu & đèn trang trí ban đêm
        drawStageCircularScreensAndLights(isNightOrSunset);

        // 4. Vẽ 4 trò đu quay tròn / công trình tròn có animation quay Đợt 5B (#9, #10, #13, #14)
        drawAllSpinningRoundRides(isNightOrSunset);
    }

    /**
     * @brief Vẽ riêng các công trình ven bờ Nam hồ nước trong lượt phản xạ mặt hồ y = 0
     */
    void drawLakeReflectionPart(bool smoothShading = true) const {
        ensureDisplayLists();
        if (smoothShading && archSmoothList != 0) {
            glCallList(archSmoothList);
        } else {
            staticArchMesh.draw(smoothShading, false);
        }
    }

    static Geometry::PolygonMesh buildAngkorTempleMesh(const MathCG::Vec3& centerPos = ParkLayout::ANGKOR_STAR_PLAZA_POS) {
        Geometry::PolygonMesh m("AngkorWatTemple");
        appendAngkorStarPlazaTowers(m, centerPos);
        m.computeNormals();
        return m;
    }

private:
    mutable Geometry::PolygonMesh staticArchMesh;
    mutable Geometry::PolygonMesh emissiveLampMesh;
    mutable GLuint archSmoothList   = 0;
    mutable GLuint archFlatList     = 0;
    mutable GLuint emissiveLampList = 0;
    mutable GLuint plazaStaticList_ = 0;
    mutable GLuint stageToroidsList_ = 0;
    mutable GLuint stageFlatScreensList_[2] = { 0, 0 };
    mutable GLuint roundRideSpinLists[4] = { 0, 0, 0, 0 };

    void ensureDisplayLists() const {
        if (plazaStaticList_ == 0) {
            plazaStaticList_ = glGenLists(1);
            if (plazaStaticList_ != 0) {
                glNewList(plazaStaticList_, GL_COMPILE);
                drawTiledCentralPlaza();
                glEndList();
            }
        }
        if (stageToroidsList_ == 0) {
            stageToroidsList_ = glGenLists(1);
            if (stageToroidsList_ != 0) {
                glNewList(stageToroidsList_, GL_COMPILE);
                drawStageToroidRingsDirect();
                glEndList();
            }
        }
        for (int m = 0; m < 2; ++m) {
            if (stageFlatScreensList_[m] == 0) {
                stageFlatScreensList_[m] = glGenLists(1);
                if (stageFlatScreensList_[m] != 0) {
                    glNewList(stageFlatScreensList_[m], GL_COMPILE);
                    drawStageFlatGraphicsDirect(m == 1);
                    glEndList();
                }
            }
        }
        if (archSmoothList == 0) {
            archSmoothList = glGenLists(1);
            if (archSmoothList != 0) {
                glNewList(archSmoothList, GL_COMPILE);
                staticArchMesh.draw(true, false);
                glEndList();
            }
        }
        if (archFlatList == 0) {
            archFlatList = glGenLists(1);
            if (archFlatList != 0) {
                glNewList(archFlatList, GL_COMPILE);
                staticArchMesh.draw(false, false);
                glEndList();
            }
        }
        if (roundRideSpinLists[0] == 0) {
            struct SpinSpec {
                float radius;
                float height;
                MathCG::ColorRGB c1;
                MathCG::ColorRGB c2;
                int seats;
                bool colonnade;
            };
            const SpinSpec specs[4] = {
                { 8.5f, 8.2f, MathCG::ColorRGB(0.95f, 0.45f, 0.12f), MathCG::ColorRGB(0.96f, 0.95f, 0.92f), 16, false },
                { 6.5f, 6.4f, MathCG::ColorRGB(0.18f, 0.56f, 0.92f), MathCG::ColorRGB(0.98f, 0.84f, 0.22f), 12, false },
                { 9.0f, 8.8f, MathCG::ColorRGB(0.84f, 0.22f, 0.16f), MathCG::ColorRGB(0.96f, 0.78f, 0.20f), 16, true  },
                { 7.5f, 7.4f, MathCG::ColorRGB(0.14f, 0.68f, 0.72f), MathCG::ColorRGB(0.98f, 0.86f, 0.26f), 14, false }
            };
            for (int i = 0; i < 4; ++i) {
                roundRideSpinLists[i] = glGenLists(1);
                if (roundRideSpinLists[i] != 0) {
                    glNewList(roundRideSpinLists[i], GL_COMPILE);
                    compileSpinningCanopyAndSeats(
                        specs[i].radius, specs[i].height,
                        specs[i].c1, specs[i].c2,
                        specs[i].seats, specs[i].colonnade
                    );
                    glEndList();
                }
            }
        }
    }

    static void appendBoxByYRange(
        Geometry::PolygonMesh& target,
        float cx, float yBottom, float yTop, float cz,
        float sx, float sz,
        const MathCG::ColorRGB& col
    ) {
        float sy = yTop - yBottom;
        if (sy <= 1e-4f) return;
        float cy = 0.5f * (yBottom + yTop);
        target.append(Geometry::PolygonMesh::createBox(cx, cy, cz, sx, sy, sz, col));
    }

    /**
     * @brief Xoay lưới cục bộ quanh trục Y góc rotYDeg rồi tịnh tiến tới worldPos và gộp vào target
     */
    static void appendRotatedTranslatedMesh(
        Geometry::PolygonMesh& target,
        Geometry::PolygonMesh localMesh,
        const MathCG::Vec3& worldPos,
        float rotYDeg
    ) {
        float rad = rotYDeg * MathCG::DEG2RAD;
        float c = std::cos(rad);
        float s = std::sin(rad);
        for (auto& v : localMesh.vertices) {
            float rx =  v.x * c + v.z * s;
            float rz = -v.x * s + v.z * c;
            v.x = worldPos.x + rx;
            v.y = worldPos.y + v.y;
            v.z = worldPos.z + rz;
        }
        target.append(localMesh);
    }

    /**
     * @brief MỘT HÀM VẼ NHÀ MÁI NGÓI THAM SỐ HÓA DÙNG CHUNG (ĐỢT 5B & 6A):
     *        Tham số: vị trí, góc xoay, rộng (w), sâu (d), cao thân (wallH), màu mái (roofCol),
     *        kiểu mái cong (roofStyle), và tùy chọn mở thông 4 mặt tường thành nhà vòm cột (openColonnadeOnly)
     *        để bao quanh sân xe điện đụng BumperCars mà không che khuất xe bên trong.
     */
    static void appendTileRoofedHall(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos,
        float w, float d, float wallH,
        float rotYDeg,
        const MathCG::ColorRGB& roofCol,
        int roofStyle,
        bool openColonnadeOnly = false
    ) {
        Geometry::PolygonMesh local("TileRoofedHall");

        const MathCG::ColorRGB plinthCol(0.52f, 0.50f, 0.47f);
        const MathCG::ColorRGB stepCol(0.68f, 0.65f, 0.59f);
        const MathCG::ColorRGB wallCol = (roofStyle == 1)
            ? MathCG::ColorRGB(0.93f, 0.94f, 0.95f)
            : MathCG::ColorRGB(0.88f, 0.83f, 0.72f);
        const MathCG::ColorRGB colColor = (roofStyle == 1)
            ? MathCG::ColorRGB(0.82f, 0.86f, 0.90f)
            : MathCG::ColorRGB(0.66f, 0.22f, 0.16f);
        const MathCG::ColorRGB beamCol = (roofStyle == 1)
            ? MathCG::ColorRGB(0.94f, 0.95f, 0.97f)
            : MathCG::ColorRGB(0.32f, 0.24f, 0.18f);
        const MathCG::ColorRGB doorCol(0.24f, 0.18f, 0.14f);
        const MathCG::ColorRGB goldCol(0.94f, 0.78f, 0.22f);

        const float baseTopY = openColonnadeOnly ? 0.006f : 0.65f;
        if (openColonnadeOnly) {
            // Bệ sàn đá mỏng y = 0.0 .. 0.006m nằm dưới mặt sàn BumperCars (y = 0.01m) để không đè lên sàn xe
            appendBoxByYRange(local, 0.0f, -0.02f, 0.006f, 0.0f, w + 2.4f, d + 2.4f, stepCol);
        } else {
            // 1. Bệ đá nền & bậc tam cấp mặt tiền (+Z) và mặt hậu (-Z) (y = 0.0 .. 0.65m)
            appendBoxByYRange(local, 0.0f, 0.0f, 0.35f, 0.0f, w + 1.8f, d + 1.8f, plinthCol);
            appendBoxByYRange(local, 0.0f, 0.30f, 0.65f, 0.0f, w + 0.8f, d + 0.8f, stepCol);
            appendBoxByYRange(local, 0.0f, 0.0f, 0.32f,  d * 0.5f + 1.2f, 5.4f, 1.4f, stepCol);
            appendBoxByYRange(local, 0.0f, 0.0f, 0.32f, -d * 0.5f - 1.2f, 5.4f, 1.4f, stepCol);
        }

        // 2. Thân nhà đặc liền khối (nếu không phải nhà vòm cột mở) và hệ cột trụ xung quanh
        const float yWallTop = baseTopY + wallH;
        if (!openColonnadeOnly) {
            appendBoxByYRange(local, 0.0f, 0.60f, yWallTop, 0.0f, w, d, wallCol);
        }

        // Hệ cột trụ nổi dọc 4 mặt biên đỡ mái
        const int numColsX = 5;
        for (int i = 0; i < numColsX; ++i) {
            float tx = -w * 0.46f + (w * 0.92f) * (static_cast<float>(i) / (numColsX - 1));
            for (int sz : {-1, 1}) {
                appendBoxByYRange(local, tx, baseTopY, yWallTop + 0.05f, sz * (d * 0.5f + 0.08f), 0.52f, 0.42f, colColor);
            }
        }
        for (int sx : {-1, 1}) {
            for (int iz = -1; iz <= 1; ++iz) {
                appendBoxByYRange(local, sx * (w * 0.5f + 0.08f), baseTopY, yWallTop + 0.05f, iz * d * 0.32f, 0.42f, 0.52f, colColor);
            }
        }

        if (!openColonnadeOnly) {
            // Ô cửa chính ở mặt trước (+Z) và mặt sau (-Z)
            for (int sz : {-1, 1}) {
                appendBoxByYRange(local, 0.0f, 0.65f, 0.65f + wallH * 0.72f, sz * (d * 0.5f + 0.04f), 3.6f, 0.18f, doorCol);
            }
        }

        // 3. Dầm cổ diềm đỡ mái tầng 1 (y = yWallTop - 0.15 .. yWallTop + 0.25)
        appendBoxByYRange(local, 0.0f, yWallTop - 0.15f, yWallTop + 0.25f, 0.0f, w + 0.6f, d + 0.6f, beamCol);

        // 4. Mái ngói cong tầng 1 đặt trực tiếp trên cổ diềm tại y = yWallTop + 0.20
        const float roof1Y = yWallTop + 0.20f;
        const float roof1H = (roofStyle == 1) ? 2.15f : 1.95f;
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, roof1Y, 0.0f,
            w + 0.8f, d + 0.8f, roof1H, 1.9f, roofCol
        ));

        // 5. Cổ lầu tầng 2 mọc liền khối từ thân mái tầng 1 lên mái cong tầng 2
        const float neckW  = (roofStyle == 1) ? w * 0.66f : w * 0.60f;
        const float neckD  = (roofStyle == 1) ? d * 0.66f : d * 0.58f;
        const float neckY0 = roof1Y + 0.75f;
        const float neckY1 = roof1Y + roof1H + 0.65f;
        appendBoxByYRange(local, 0.0f, neckY0, neckY1, 0.0f, neckW, neckD, (roofStyle == 1) ? wallCol : colColor);
        appendBoxByYRange(local, 0.0f, neckY1 - 0.12f, neckY1 + 0.18f, 0.0f, neckW + 0.4f, neckD + 0.4f, beamCol);

        const float roof2H = (roofStyle == 1) ? 2.05f : 1.85f;
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, neckY1 + 0.14f, 0.0f,
            neckW + 0.5f, neckD + 0.5f, roof2H, 1.55f, roofCol
        ));

        // Đỉnh bờ nóc mạ vàng
        local.append(Geometry::PolygonMesh::createBox(
            0.0f, neckY1 + 0.14f + roof2H - 0.05f, 0.0f,
            neckW * 0.36f, 0.28f, 0.32f,
            (roofStyle == 1) ? beamCol : goldCol
        ));

        if (!openColonnadeOnly) {
            // 6. Sảnh hiên đón khách phía trước (+Z) có 2 cột trụ và mái cong liền khối
            const float porchZ = d * 0.5f + 1.15f;
            const float porchH = wallH * 0.78f;
            for (int sx : {-1, 1}) {
                appendBoxByYRange(local, sx * 2.3f, 0.32f, 0.65f + porchH, porchZ + 0.55f, 0.36f, 0.36f, colColor);
            }
            appendBoxByYRange(local, 0.0f, 0.65f + porchH - 0.12f, 0.65f + porchH + 0.16f, porchZ, 5.2f, 1.8f, beamCol);
            local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                0.0f, 0.65f + porchH + 0.12f, porchZ,
                5.4f, 2.0f, 1.15f, 0.95f, roofCol
            ));
        }

        appendRotatedTranslatedMesh(target, local, centerPos, rotYDeg);
    }

    /**
     * @brief Dựng 3 dãy quầy nhỏ mái sọc đỏ trắng (#6 - STRIPED_STALLS_POS)
     */
    static void appendStripedStallsComplex(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos
    ) {
        const float cx = centerPos.x;
        const float cz = centerPos.z;
        const MathCG::ColorRGB padCol(0.66f, 0.63f, 0.57f);
        const MathCG::ColorRGB counterCol(0.58f, 0.38f, 0.22f);
        const MathCG::ColorRGB wallCol(0.92f, 0.87f, 0.76f);
        const MathCG::ColorRGB postCol(0.38f, 0.28f, 0.22f);
        const MathCG::ColorRGB redStripe(0.86f, 0.18f, 0.16f);
        const MathCG::ColorRGB whiteStripe(0.95f, 0.94f, 0.92f);

        // Nền sân khu hội chợ gian hàng (16 x 32 m)
        appendBoxByYRange(target, cx, 0.0f, 0.16f, cz, 16.0f, 32.0f, padCol);

        // 3 dãy quầy song song theo chiều Bắc - Nam (rowX = -5.2m, 0.0m, +5.2m)
        const float rowOffsets[3] = { -5.2f, 0.0f, 5.2f };
        const int   numStallsPerRow = 6;
        const float stallLenZ = 4.6f;
        const float stallW    = 3.2f;

        for (int r = 0; r < 3; ++r) {
            float rx = cx + rowOffsets[r];
            for (int s = 0; s < numStallsPerRow; ++s) {
                float sz = cz - 12.2f + s * 4.88f;

                // Bục quầy bán hàng và vách giữa liền khối (y = 0.15 .. 2.55m)
                appendBoxByYRange(target, rx, 0.15f, 1.15f, sz, stallW * 0.88f, stallLenZ * 0.90f, counterCol);
                appendBoxByYRange(target, rx, 1.10f, 2.55f, sz, 0.45f, stallLenZ * 0.86f, wallCol);

                // 4 cột góc đỡ khung bạt mái sọc
                for (int ix : {-1, 1}) {
                    for (int iz : {-1, 1}) {
                        appendBoxByYRange(
                            target,
                            rx + ix * (stallW * 0.42f),
                            0.15f, 2.55f,
                            sz + iz * (stallLenZ * 0.42f),
                            0.18f, 0.18f, postCol
                        );
                    }
                }
                // Xà đỡ mái quầy
                appendBoxByYRange(target, rx, 2.48f, 2.66f, sz, stallW * 0.94f, stallLenZ * 0.94f, postCol);

                // Mái lều chóp sọc đỏ - trắng xen kẽ (chia 4 dải sọc dọc chiều dài mỗi gian hàng)
                const int numStripes = 4;
                const float stripeZ = stallLenZ / static_cast<float>(numStripes);
                for (int k = 0; k < numStripes; ++k) {
                    float kz = (sz - stallLenZ * 0.5f) + (k + 0.5f) * stripeZ;
                    const MathCG::ColorRGB& sc = ((s + k + r) % 2 == 0) ? redStripe : whiteStripe;
                    // Tầng diềm mái sọc
                    appendBoxByYRange(target, rx, 2.64f, 2.96f, kz, stallW + 0.35f, stripeZ * 0.98f, sc);
                    // Tầng chóp mái sọc
                    appendBoxByYRange(target, rx, 2.94f, 3.32f, kz, stallW * 0.62f, stripeZ * 0.98f, sc);
                }
            }
        }
    }

    /**
     * @brief Lô B1 - Dựng quần thể Đền Angkor Wat đầy đủ tại Quảng trường Ngôi sao (#19 - ANGKOR_STAR_PLAZA_POS)
     *        - Vị trí & sân sao: [Chưa xác minh tên/chi tiết chính thức] tại (-185.0, 0.0, -254.0).
     *        - Hình dáng [Bản vẽ minh họa AI, tham khảo hình dạng] (#06) thu gọn vào 26x26m trên sân sao 27.8x27.8m
     *          để chừa 7.15m tới tim dầm Monorail: nền đá 5 bậc giật cấp hình chữ thập, 4 cầu thang trục chữ thập,
     *          hành lang cột bao quanh + 4 cổng Gopura đầu hồi tam giác và 5 tháp búp sen Prang Khmer.
     */
    static void appendAngkorStarPlazaTowers(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos
    ) {
        const float cx = centerPos.x;
        const float cz = centerPos.z;
        const MathCG::ColorRGB sandLight(0.75f, 0.66f, 0.52f);
        const MathCG::ColorRGB sandMid  (0.62f, 0.53f, 0.41f);
        const MathCG::ColorRGB sandDark (0.44f, 0.37f, 0.29f);
        const MathCG::ColorRGB roofSlate(0.49f, 0.43f, 0.36f);
        const MathCG::ColorRGB goldTip  (0.88f, 0.70f, 0.24f);

        const float topY   = ParkLayout::ANGKOR_PLATFORM_TOP_Y;     // 2.85 m
        const float eaveY  = ParkLayout::ANGKOR_GALLERY_EAVE_Y;     // 5.65 m
        const float roofY  = ParkLayout::ANGKOR_GALLERY_ROOF_TOP_Y; // 6.45 m

        // 1. Nền đá 5 bậc giật cấp hình chữ thập đối xứng 4 hướng (neo âm -0.04m, đỉnh +2.85m;
        //    bậc trên cùng mở nhánh chữ thập tới ±8.90m để đỡ trọn 4 cổng Gopura không hở đáy)
        const int numBaseSteps = 5;
        const float stepH = topY / static_cast<float>(numBaseSteps);
        for (int t = 0; t < numBaseSteps; ++t) {
            float yBot = t * stepH - 0.04f;
            float yTop = (t + 1) * stepH + ((t < numBaseSteps - 1) ? 0.02f : 0.0f);
            float mainS = 18.80f - static_cast<float>(t) * 1.12f; // 18.80 -> 14.32 m
            float armL  = 21.80f - static_cast<float>(t) * 1.00f; // 21.80 -> 17.80 m (±8.90 m)
            float armW  =  5.40f - static_cast<float>(t) * 0.35f; //  5.40 ->  4.00 m
            const MathCG::ColorRGB& sc = (t % 2 == 0) ? sandDark : sandMid;

            appendBoxByYRange(target, cx, yBot, yTop, cz, mainS, mainS, sc);
            appendBoxByYRange(target, cx, yBot, yTop, cz, armL,  armW,  sc);
            appendBoxByYRange(target, cx, yBot, yTop, cz, armW,  armL,  sc);
        }

        // 2. 4 Cầu thang đá trục chữ thập từ mép ngoài (±12.88m <= 13.0m) bước lên sân tầng 1 (+2.85m)
        const int stairSteps = 8;
        const float dOuter = 12.88f; // 2 * 12.88 = 25.76 m <= 26.0 m
        const float dInner =  8.40f;
        const float runPerStep = (dOuter - dInner) / static_cast<float>(stairSteps);
        const float risePerStep = topY / static_cast<float>(stairSteps);

        auto addAxisStairAndGopura = [&](float dirX, float dirZ, float rotYDeg) {
            bool alongX = (std::fabs(dirX) > 0.5f);
            float perpX = alongX ? 0.0f : 1.0f;
            float perpZ = alongX ? 1.0f : 0.0f;

            // 2a. 8 bậc đá đặc liền khối neo xuống sân sao (y = -0.04m)
            for (int k = 0; k < stairSteps; ++k) {
                float d0 = dOuter - k * runPerStep;
                float d1 = dOuter - (k + 1) * runPerStep - 0.10f; // gối khít vào bậc trên
                float dm = 0.5f * (d0 + d1);
                float sl = std::fabs(d0 - d1);
                float stY = (k + 1) * risePerStep;
                const MathCG::ColorRGB& stepCol = (k % 2 == 0) ? sandLight : sandMid;

                float bx = cx + dirX * dm;
                float bz = cz + dirZ * dm;
                appendBoxByYRange(target, bx, -0.04f, stY, bz,
                                  alongX ? sl : 3.00f, alongX ? 3.00f : sl, stepCol);
            }
            // 2b. Má lan can đá bậc giật cấp hai bên cầu thang & trụ chân thang
            for (int side : {-1, 1}) {
                float offP = side * 1.66f;
                for (int blk = 0; blk < 4; ++blk) {
                    float dA = dOuter - blk * (2.0f * runPerStep);
                    float dB = dOuter - (blk + 1) * (2.0f * runPerStep) - 0.06f;
                    float dm = 0.5f * (dA + dB);
                    float bl = std::fabs(dA - dB);
                    float topH = (blk + 1) * (2.0f * risePerStep) + 0.36f;
                    appendBoxByYRange(target, cx + dirX * dm + perpX * offP, -0.04f, topH,
                                      cz + dirZ * dm + perpZ * offP,
                                      alongX ? bl : 0.36f, alongX ? 0.36f : bl, sandDark);
                }
                // Trụ đầu chân cầu thang Khmer (nằm gọn trong <= 12.86m)
                float pDist = dOuter - 0.28f;
                appendBoxByYRange(target, cx + dirX * pDist + perpX * offP, -0.04f, 1.15f,
                                  cz + dirZ * pDist + perpZ * offP, 0.48f, 0.48f, sandLight);
                target.append(Geometry::PolygonMesh::createPyramid(
                    cx + dirX * pDist + perpX * offP, 1.14f, cz + dirZ * pDist + perpZ * offP,
                    0.44f, 0.44f, 0.38f, sandDark
                ));
            }

            // 2c. Tiền sảnh & Cổng đầu hồi tam giác Gopura tại ±7.95m trên trục chữ thập
            float gDist = 7.95f;
            float gx = cx + dirX * gDist;
            float gz = cz + dirZ * gDist;
            // Sàn tiền sảnh nối đầu thang vào hành lang
            appendBoxByYRange(target, gx, topY - 0.18f, topY + 0.04f, gz,
                              alongX ? 1.80f : 3.90f, alongX ? 3.90f : 1.80f, sandLight);
            // 4 trụ cổng tiền sảnh + vách cửa tối
            for (int fb : {-1, 1}) {
                float fd = (fb > 0) ? 8.55f : 7.35f;
                for (int lr : {-1, 1}) {
                    float colX = cx + dirX * fd + perpX * (lr * 1.48f);
                    float colZ = cz + dirZ * fd + perpZ * (lr * 1.48f);
                    appendBoxByYRange(target, colX, topY, topY + 0.24f, colZ, 0.62f, 0.62f, sandDark);
                    appendBoxByYRange(target, colX, topY + 0.24f, eaveY - 0.10f, colZ, 0.46f, 0.46f, sandLight);
                    appendBoxByYRange(target, colX, eaveY - 0.14f, eaveY + 0.10f, colZ, 0.62f, 0.62f, sandMid);
                }
            }
            // Khối xà nối cửa vào và vòm tam giác Gopura nhô cao 2 tầng (+7.75m)
            appendBoxByYRange(target, gx, eaveY - 0.22f, eaveY + 0.16f, gz,
                              alongX ? 2.10f : 3.80f, alongX ? 3.80f : 2.10f, sandDark);
            target.append(Geometry::PolygonMesh::createKhmerGopuraPediment(
                gx, eaveY + 0.12f, gz,
                3.85f, 2.20f, 2.10f,
                sandLight, roofSlate, rotYDeg
            ));
        };

        addAxisStairAndGopura( 1.0f,  0.0f,  90.0f); // Đông (+X)
        addAxisStairAndGopura(-1.0f,  0.0f, -90.0f); // Tây (-X)
        addAxisStairAndGopura( 0.0f,  1.0f,   0.0f); // Nam (+Z)
        addAxisStairAndGopura( 0.0f, -1.0f, 180.0f); // Bắc (-Z)

        // 3. Sân tầng 1 (+2.85m) & Hệ hành lang 20 cột đá bao quanh cùng mái đá Khmer
        appendBoxByYRange(target, cx, topY - 0.16f, topY + 0.02f, cz, 14.30f, 14.30f, sandLight);

        // 20 cột vòng ngoài chạy theo 4 cạnh ±6.65m (bỏ vị trí 0 cho 4 cổng Gopura)
        const float colGrid[6] = { -6.65f, -4.45f, -2.25f, 2.25f, 4.45f, 6.65f };
        auto addGalleryColumn = [&](float lx, float lz) {
            float wx = cx + lx, wz = cz + lz;
            appendBoxByYRange(target, wx, topY, topY + 0.24f, wz, 0.56f, 0.56f, sandDark);
            appendBoxByYRange(target, wx, topY + 0.24f, eaveY - 0.16f, wz, 0.40f, 0.40f, sandLight);
            appendBoxByYRange(target, wx, eaveY - 0.18f, eaveY + 0.06f, wz, 0.56f, 0.56f, sandMid);
        };
        for (int i = 0; i < 6; ++i) {
            addGalleryColumn(colGrid[i], -6.65f); // Cạnh Bắc
            addGalleryColumn(colGrid[i],  6.65f); // Cạnh Nam
            if (i > 0 && i < 5) {
                addGalleryColumn(-6.65f, colGrid[i]); // Cạnh Tây (không trùng 4 góc)
                addGalleryColumn( 6.65f, colGrid[i]); // Cạnh Đông
            }
        }

        // Khối lõi trung tâm tầng 2 & tường trong hành lang (có 4 hốc cửa sảnh nối chữ thập)
        appendBoxByYRange(target, cx, topY, roofY - 0.10f, cz, 9.50f, 9.50f, sandMid);
        appendBoxByYRange(target, cx, topY, eaveY, cz, 10.10f, 3.00f, sandDark); // Hành lang chữ thập lõi Đ-T
        appendBoxByYRange(target, cx, topY, eaveY, cz, 3.00f, 10.10f, sandDark); // Hành lang chữ thập lõi B-N
        // Cánh tường nối mái chữ thập từ tâm ra 4 cổng Gopura (theo Top Roof Plan #06)
        appendBoxByYRange(target, cx, eaveY - 0.12f, roofY + 0.10f, cz, 14.20f, 2.30f, roofSlate);
        appendBoxByYRange(target, cx, eaveY - 0.12f, roofY + 0.10f, cz, 2.30f, 14.20f, roofSlate);

        // Vòng xà & mái hành lang đá Khmer 2 cấp bao quanh 4 mặt (khép kín vòng vuông tại R_mid = 5.75m)
        const float galMid = 5.75f;
        const float galLen = 14.10f;
        for (int side : {-1, 1}) {
            // Dầm xà dọc cạnh Bắc/Nam (Z = ±5.75) & Đông/Tây (X = ±5.75)
            appendBoxByYRange(target, cx, eaveY - 0.12f, eaveY + 0.24f, cz + side * galMid, galLen, 2.35f, sandDark);
            appendBoxByYRange(target, cx + side * galMid, eaveY - 0.12f, eaveY + 0.24f, cz, 2.35f, galLen, sandDark);
            // Mái bậc dưới hành lang
            appendBoxByYRange(target, cx, eaveY + 0.22f, eaveY + 0.56f, cz + side * galMid, galLen - 0.25f, 1.90f, roofSlate);
            appendBoxByYRange(target, cx + side * galMid, eaveY + 0.22f, eaveY + 0.56f, cz, 1.90f, galLen - 0.25f, roofSlate);
            // Gờ nóc mái hành lang
            appendBoxByYRange(target, cx, eaveY + 0.54f, roofY, cz + side * galMid, galLen - 0.65f, 1.05f, sandMid);
            appendBoxByYRange(target, cx + side * galMid, eaveY + 0.54f, roofY, cz, 1.05f, galLen - 0.65f, sandMid);
        }

        // 4. Bệ đền thượng trung tâm & Hệ 5 Tháp Búp Sen (Prang Khmer) đối xứng
        appendBoxByYRange(target, cx, roofY - 0.20f, ParkLayout::ANGKOR_CENTER_PRANG_BASE_Y + 0.08f, cz, 6.20f, 6.20f, sandMid);
        appendBoxByYRange(target, cx, roofY - 0.18f, ParkLayout::ANGKOR_CENTER_PRANG_BASE_Y + 0.06f, cz, 10.40f, 2.70f, sandMid);
        appendBoxByYRange(target, cx, roofY - 0.18f, ParkLayout::ANGKOR_CENTER_PRANG_BASE_Y + 0.06f, cz, 2.70f, 10.40f, sandMid);

        // 4 Tháp góc Prang Khmer đứng trên 4 giao điểm vòng mái hành lang (±5.75m, ±5.75m), cao tới 11.70m
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                target.append(Geometry::PolygonMesh::createKhmerPrangTower(
                    cx + sx * galMid,
                    ParkLayout::ANGKOR_CORNER_PRANG_BASE_Y,
                    cz + sz * galMid,
                    2.75f,
                    ParkLayout::ANGKOR_CORNER_PRANG_HEIGHT,
                    5,
                    sandMid, sandLight, sandDark, goldTip
                ));
            }
        }
        // 1 Tháp Trung tâm Prang Khmer nổi bật tại tâm (0, 0) -> cao tới 15.00m
        target.append(Geometry::PolygonMesh::createKhmerPrangTower(
            cx,
            ParkLayout::ANGKOR_CENTER_PRANG_BASE_Y,
            cz,
            4.15f,
            ParkLayout::ANGKOR_CENTER_PRANG_HEIGHT,
            6,
            sandMid, sandLight, sandDark, goldTip
        ));
    }

    /**
     * @brief Dựng phần bệ tròn tĩnh (và vòng hành lang cột nếu có) của các trò đu quay tròn (#9, #10, #13, #14)
     */
    static void appendRoundRideStaticBase(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& pos,
        float radius,
        float height,
        const MathCG::ColorRGB& roofCol,
        bool hasOuterColonnade
    ) {
        const MathCG::ColorRGB baseCol(0.54f, 0.52f, 0.49f);
        const MathCG::ColorRGB deckCol(0.76f, 0.72f, 0.65f);
        const MathCG::ColorRGB colColor(0.74f, 0.22f, 0.16f);

        // Bệ tròn 2 bậc liền khối & trụ trục chính ở tâm
        target.append(Geometry::PolygonMesh::createPrismY(pos.x, 0.0f,  pos.z, radius + 0.6f, 0.30f, 20, baseCol));
        target.append(Geometry::PolygonMesh::createPrismY(pos.x, 0.28f, pos.z, radius,        0.30f, 20, deckCol));
        target.append(Geometry::PolygonMesh::createPrismY(pos.x, 0.55f, pos.z, radius * 0.18f, height * 0.65f, 14, MathCG::ColorRGB(0.82f, 0.72f, 0.32f)));

        // Nếu là Công trình tròn có hành lang khu giữa (#13): dựng vòng cột hành lang bao quanh và mái vành khuyên
        if (hasOuterColonnade) {
            const int numCols = 12;
            const float colR  = radius - 0.65f;
            const float colH  = height * 0.48f;
            for (int i = 0; i < numCols; ++i) {
                float a = (static_cast<float>(i) / numCols) * MathCG::TWO_PI;
                float px = pos.x + colR * std::cos(a);
                float pz = pos.z + colR * std::sin(a);
                target.append(Geometry::PolygonMesh::createPrismY(px, 0.55f, pz, 0.24f, colH, 8, colColor));
            }
            // Dầm cổ diềm & mái hành lang tròn tầng dưới
            target.append(Geometry::PolygonMesh::createPrismY(pos.x, 0.55f + colH - 0.10f, pos.z, radius + 0.45f, 0.45f, 20, colColor));
            target.append(Geometry::PolygonMesh::createPrismY(pos.x, 0.55f + colH + 0.32f, pos.z, radius * 0.88f, 0.42f, 20, roofCol));
        }
    }

    /**
     * @brief ĐỢT 6A: Sinh toàn bộ trụ đèn lối đi, ghế nghỉ, bồn hoa hộp màu, cột cờ, biển chỉ dẫn
     *        và dãy đèn lồng dọc Cổng Ngọ Môn cùng các chòi mái ngói.
     */
    void appendWave6aPropsAndLanterns(Geometry::PolygonMesh& target) {
        emissiveLampMesh.clear();
        emissiveLampMesh.name = "Wave6aEmissiveBulbsAndLanterns";

        const MathCG::ColorRGB ironCol(0.24f, 0.25f, 0.28f);
        const MathCG::ColorRGB goldCol(0.92f, 0.74f, 0.20f);
        const MathCG::ColorRGB bulbWarmCol(1.00f, 0.92f, 0.58f);
        const MathCG::ColorRGB lanternRedCol(0.94f, 0.22f, 0.14f);

        // 1. 24 Trụ đèn lối đi & quảng trường (PARK_PATH_LAMP_POS)
        for (int i = 0; i < ParkLayout::NUM_PARK_PATH_LAMPS; ++i) {
            float lx = ParkLayout::PARK_PATH_LAMP_POS[i].x;
            float lz = ParkLayout::PARK_PATH_LAMP_POS[i].z;
            // Chân đế & thân trụ đèn cao 4.2m
            appendBoxByYRange(target, lx, 0.0f, 0.45f, lz, 0.55f, 0.55f, ironCol);
            target.append(Geometry::PolygonMesh::createPrismY(lx, 0.40f, lz, 0.12f, 3.80f, 6, ironCol));
            appendBoxByYRange(target, lx, 4.10f, 4.30f, lz, 0.48f, 0.48f, goldCol);
            // Bóng đèn phát sáng (đưa vào emissiveLampMesh) + nón chụp trên đỉnh
            emissiveLampMesh.append(Geometry::PolygonMesh::createPrismY(lx, 4.28f, lz, 0.28f, 0.45f, 8, bulbWarmCol));
            target.append(Geometry::PolygonMesh::createPyramid(lx, 4.70f, lz, 0.68f, 0.68f, 0.42f, ironCol));
        }

        // 2. 16 Ghế nghỉ công viên (PARK_BENCH_POS)
        const MathCG::ColorRGB woodBenchCol(0.62f, 0.38f, 0.18f);
        for (int i = 0; i < ParkLayout::NUM_PARK_BENCHES; ++i) {
            const auto& bp = ParkLayout::PARK_BENCH_POS[i];
            Geometry::PolygonMesh bench("Bench");
            appendBoxByYRange(bench, -0.95f, 0.0f, 0.48f, 0.0f, 0.16f, 0.62f, ironCol);
            appendBoxByYRange(bench,  0.95f, 0.0f, 0.48f, 0.0f, 0.16f, 0.62f, ironCol);
            appendBoxByYRange(bench,  0.0f,  0.42f, 0.54f, 0.0f, 2.25f, 0.65f, woodBenchCol);
            appendBoxByYRange(bench,  0.0f,  0.52f, 1.05f, -0.26f, 2.25f, 0.12f, woodBenchCol);
            appendRotatedTranslatedMesh(target, bench, MathCG::Vec3(bp.x, 0.0f, bp.z), bp.rotYDeg);
        }

        // 3. 16 Bồn hoa hộp màu (PARK_FLOWER_PLANTER_POS)
        const MathCG::ColorRGB planterStone(0.74f, 0.70f, 0.64f);
        const MathCG::ColorRGB leafGreen(0.18f, 0.56f, 0.22f);
        const MathCG::ColorRGB flowerColors[3] = {
            MathCG::ColorRGB(0.94f, 0.22f, 0.24f), // Hoa đỏ
            MathCG::ColorRGB(0.98f, 0.82f, 0.16f), // Hoa vàng
            MathCG::ColorRGB(0.92f, 0.32f, 0.76f)  // Hoa hồng tím
        };
        for (int i = 0; i < ParkLayout::NUM_PARK_FLOWER_PLANTERS; ++i) {
            const auto& fp = ParkLayout::PARK_FLOWER_PLANTER_POS[i];
            Geometry::PolygonMesh pl("Planter");
            appendBoxByYRange(pl, 0.0f, 0.0f, 0.58f, 0.0f, 2.8f, 1.4f, planterStone);
            appendBoxByYRange(pl, 0.0f, 0.55f, 0.82f, 0.0f, 2.5f, 1.1f, leafGreen);
            const MathCG::ColorRGB& fc = flowerColors[fp.variant % 3];
            for (int k = -1; k <= 1; ++k) {
                appendBoxByYRange(pl, k * 0.75f, 0.78f, 1.08f, 0.0f, 0.55f, 0.65f, fc);
            }
            appendRotatedTranslatedMesh(target, pl, MathCG::Vec3(fp.x, 0.0f, fp.z), fp.rotYDeg);
        }

        // 4. 6 Cột cờ lễ hội tại Quảng trường Cổng Ngọ Môn và Sân khấu Sun Wheel (PARK_FLAGPOLE_POS)
        const MathCG::ColorRGB poleSilver(0.86f, 0.88f, 0.90f);
        const MathCG::ColorRGB flagRed(0.90f, 0.14f, 0.14f);
        for (int i = 0; i < ParkLayout::NUM_PARK_FLAGPOLES; ++i) {
            float fx = ParkLayout::PARK_FLAGPOLE_POS[i].x;
            float fz = ParkLayout::PARK_FLAGPOLE_POS[i].z;
            appendBoxByYRange(target, fx, 0.0f, 0.65f, fz, 1.1f, 1.1f, planterStone);
            target.append(Geometry::PolygonMesh::createPrismY(fx, 0.60f, fz, 0.11f, 11.2f, 8, poleSilver));
            // Lá cờ đỏ sao vàng tung bay ở đỉnh cột
            appendBoxByYRange(target, fx + 1.15f, 10.1f, 11.5f, fz, 2.2f, 0.10f, flagRed);
            appendBoxByYRange(target, fx + 1.15f, 10.6f, 11.0f, fz, 0.45f, 0.14f, goldCol);
        }

        // 5. 6 Biển chỉ dẫn đa hướng tại các ngã ba lối đi chính (PARK_SIGNBOARD_POS)
        const MathCG::ColorRGB signCols[3] = {
            MathCG::ColorRGB(0.16f, 0.54f, 0.78f),
            MathCG::ColorRGB(0.88f, 0.44f, 0.14f),
            MathCG::ColorRGB(0.22f, 0.64f, 0.32f)
        };
        for (int i = 0; i < ParkLayout::NUM_PARK_SIGNBOARDS; ++i) {
            const auto& sp = ParkLayout::PARK_SIGNBOARD_POS[i];
            Geometry::PolygonMesh sg("Signboard");
            appendBoxByYRange(sg, 0.0f, 0.0f, 0.35f, 0.0f, 0.65f, 0.65f, planterStone);
            appendBoxByYRange(sg, 0.0f, 0.30f, 3.40f, 0.0f, 0.18f, 0.18f, woodBenchCol);
            // 3 tấm bảng mũi tên chỉ hướng ở độ cao 2.2m .. 3.2m
            appendBoxByYRange(sg,  0.45f, 2.90f, 3.22f, 0.0f, 1.35f, 0.12f, signCols[0]);
            appendBoxByYRange(sg, -0.45f, 2.50f, 2.82f, 0.0f, 1.35f, 0.12f, signCols[1]);
            appendBoxByYRange(sg,  0.0f,  2.10f, 2.42f, 0.45f, 0.12f, 1.35f, signCols[2]);
            appendRotatedTranslatedMesh(target, sg, MathCG::Vec3(sp.x, 0.0f, sp.z), sp.rotYDeg);
        }

        // 6. Dãy đèn lồng đỏ dọc 2 cánh tường thành Cổng Ngọ Môn và dưới hiên các chòi mái ngói
        auto addHangingLantern = [&](float lx, float ly, float lz) {
            appendBoxByYRange(target, lx, ly + 0.22f, ly + 0.55f, lz, 0.08f, 0.08f, ironCol);
            emissiveLampMesh.append(Geometry::PolygonMesh::createPrismY(lx, ly - 0.25f, lz, 0.26f, 0.48f, 8, lanternRedCol));
        };

        // Đèn lồng dọc hai cánh tường thành Cổng Ngọ Môn (Z = 142.2m và Z = 137.8m)
        for (int s : {-1, 1}) {
            for (int k = 1; k <= 5; ++k) {
                float wx = ParkLayout::NGO_MON_GATE_POS.x + s * (16.0f + k * 7.5f);
                addHangingLantern(wx, 4.8f, 142.0f);
            }
        }
        // Đèn lồng treo ở 4 góc mái của các chòi Quảng trường, chòi Sun Wheel và chòi bờ hồ
        const MathCG::Vec3 pavCenters[6] = {
            ParkLayout::PLAZA_PAVILION_WEST_POS,
            ParkLayout::PLAZA_PAVILION_EAST_POS,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_WEST_POS,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_EAST_POS,
            ParkLayout::LAKE_SHORE_PAVILION_WEST_POS,
            ParkLayout::LAKE_SHORE_PAVILION_EAST_POS
        };
        const float pavHalfW[6] = { 7.6f, 7.6f, 5.8f, 5.8f, 4.8f, 4.8f };
        const float pavHalfD[6] = { 4.3f, 4.3f, 5.8f, 5.8f, 3.8f, 3.8f };
        for (int p = 0; p < 6; ++p) {
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    addHangingLantern(
                        pavCenters[p].x + sx * pavHalfW[p],
                        3.95f,
                        pavCenters[p].z + sz * pavHalfD[p]
                    );
                }
            }
        }

        // 8 đèn đá chân công trình phát sáng ấm vào ban đêm tại 4 cổng Gopura Đền Angkor Wat
        const MathCG::Vec3& ap = ParkLayout::ANGKOR_STAR_PLAZA_POS;
        const float legD = 8.95f;
        const float legOff = 1.65f;
        const MathCG::ColorRGB stoneDark(0.42f, 0.36f, 0.28f);
        const float lanternCoords[8][2] = {
            { legD, -legOff }, { legD,  legOff }, { -legD, -legOff }, { -legD,  legOff },
            { -legOff, legD }, {  legOff, legD }, { -legOff, -legD }, {  legOff, -legD }
        };
        for (int k = 0; k < 8; ++k) {
            float lx = ap.x + lanternCoords[k][0];
            float lz = ap.z + lanternCoords[k][1];
            appendBoxByYRange(target, lx, ParkLayout::ANGKOR_PLATFORM_TOP_Y, ParkLayout::ANGKOR_PLATFORM_TOP_Y + 0.42f, lz, 0.30f, 0.30f, stoneDark);
            emissiveLampMesh.append(Geometry::PolygonMesh::createBox(lx, ParkLayout::ANGKOR_PLATFORM_TOP_Y + 0.54f, lz, 0.24f, 0.22f, 0.24f, bulbWarmCol));
            target.append(Geometry::PolygonMesh::createPyramid(lx, ParkLayout::ANGKOR_PLATFORM_TOP_Y + 0.65f, lz, 0.38f, 0.38f, 0.24f, stoneDark));
        }

        emissiveLampMesh.computeNormals();
    }

    /**
     * @brief ĐỢT 6B: Sinh 123 cây xanh đơn giản (3 kiểu: tán tròn, tùng tháp, cọ dừa)
     *        vào lưới tĩnh để biên dịch chung trong OpenGL Display List.
     */
    static void appendWave6bTrees(Geometry::PolygonMesh& target) {
        const MathCG::ColorRGB trunkBrown(0.44f, 0.29f, 0.16f);
        const MathCG::ColorRGB palmTrunk(0.54f, 0.42f, 0.26f);
        const MathCG::ColorRGB foliagePalette[4] = {
            MathCG::ColorRGB(0.18f, 0.52f, 0.22f),
            MathCG::ColorRGB(0.24f, 0.58f, 0.24f),
            MathCG::ColorRGB(0.15f, 0.45f, 0.19f),
            MathCG::ColorRGB(0.28f, 0.62f, 0.26f)
        };

        for (int i = 0; i < ParkLayout::NUM_PARK_TREES; ++i) {
            const auto& tr = ParkLayout::PARK_TREES[i];
            const float tx = tr.x;
            const float tz = tr.z;
            const float sc = tr.scale;
            const MathCG::ColorRGB& leafCol1 = foliagePalette[i % 4];
            const MathCG::ColorRGB& leafCol2 = foliagePalette[(i + 1) % 4];

            if (tr.style == 0) {
                // Kiểu 0: Cây bóng mát tán tròn 2 tầng
                float th = 2.6f * sc;
                target.append(Geometry::PolygonMesh::createPrismY(tx, 0.0f, tz, 0.28f * sc, th, 6, trunkBrown));
                target.append(Geometry::PolygonMesh::createPrismY(tx, th - 0.2f, tz, 2.15f * sc, 2.10f * sc, 8, leafCol1));
                target.append(Geometry::PolygonMesh::createPrismY(tx, th + 1.5f * sc, tz, 1.45f * sc, 1.55f * sc, 8, leafCol2));
            } else if (tr.style == 1) {
                // Kiểu 1: Cây tùng / thông tán tháp nón 3 tầng
                float th = 1.9f * sc;
                target.append(Geometry::PolygonMesh::createPrismY(tx, 0.0f, tz, 0.24f * sc, th, 6, trunkBrown));
                target.append(Geometry::PolygonMesh::createPyramid(tx, th - 0.1f,        tz, 3.4f * sc, 3.4f * sc, 2.2f * sc, leafCol1));
                target.append(Geometry::PolygonMesh::createPyramid(tx, th + 1.4f * sc,   tz, 2.6f * sc, 2.6f * sc, 2.0f * sc, leafCol2));
                target.append(Geometry::PolygonMesh::createPyramid(tx, th + 2.8f * sc,   tz, 1.7f * sc, 1.7f * sc, 1.7f * sc, leafCol1));
            } else {
                // Kiểu 2: Cây cọ / dừa cảnh quan nhiệt đới
                float th = 4.6f * sc;
                target.append(Geometry::PolygonMesh::createPrismY(tx, 0.0f, tz, 0.24f * sc, th, 6, palmTrunk));
                // Búp đỉnh & 4 tàu lá cọ vươn ra 4 hướng
                appendBoxByYRange(target, tx, th - 0.2f, th + 0.55f * sc, tz, 1.1f * sc, 1.1f * sc, leafCol2);
                appendBoxByYRange(target, tx, th + 0.05f * sc, th + 0.35f * sc, tz, 4.4f * sc, 0.85f * sc, leafCol1);
                appendBoxByYRange(target, tx, th + 0.05f * sc, th + 0.35f * sc, tz, 0.85f * sc, 4.4f * sc, leafCol1);
            }
        }
    }

    /**
     * @brief Sinh một chòi mái ngói Á Đông liền khối (bệ đá + cột gỗ đỏ + lan can + xà đỡ + mái cong 2 tầng)
     */
    static void appendAsianPavilion(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos,
        float w, float d,
        float colH,
        bool twoTierRoof
    ) {
        const MathCG::ColorRGB plinthCol(0.54f, 0.52f, 0.49f);
        const MathCG::ColorRGB floorCol(0.76f, 0.72f, 0.64f);
        const MathCG::ColorRGB pillarCol(0.76f, 0.18f, 0.14f);
        const MathCG::ColorRGB beamCol(0.28f, 0.22f, 0.18f);
        const MathCG::ColorRGB roofCol(0.84f, 0.36f, 0.14f);
        const MathCG::ColorRGB goldCol(0.95f, 0.78f, 0.22f);

        const float cx = centerPos.x;
        const float y0 = centerPos.y;
        const float cz = centerPos.z;

        // 1. Bệ móng đá bậc thềm (y = 0.0 .. 0.55m)
        appendBoxByYRange(target, cx, y0, y0 + 0.30f, cz, w + 1.2f, d + 1.2f, plinthCol);
        appendBoxByYRange(target, cx, y0 + 0.25f, y0 + 0.55f, cz, w + 0.4f, d + 0.4f, floorCol);

        // 2. Hệ cột trụ gỗ sơn đỏ ở các góc và giữa cạnh dài (y = 0.50 .. 0.55 + colH)
        const float yColTop = y0 + 0.55f + colH;
        const float ox = w * 0.44f;
        const float oz = d * 0.42f;
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                target.append(Geometry::PolygonMesh::createPrismY(
                    cx + sx * ox, y0 + 0.50f, cz + sz * oz,
                    0.24f, colH + 0.08f, 10, pillarCol
                ));
            }
        }
        if (w >= 13.0f) {
            for (int sz : {-1, 1}) {
                target.append(Geometry::PolygonMesh::createPrismY(
                    cx, y0 + 0.50f, cz + sz * oz,
                    0.24f, colH + 0.08f, 10, pillarCol
                ));
            }
        }

        // Lan can gỗ thấp nối giữa các cột bên hông
        appendBoxByYRange(target, cx - ox, y0 + 0.52f, y0 + 1.25f, cz, 0.22f, d * 0.82f, pillarCol);
        appendBoxByYRange(target, cx + ox, y0 + 0.52f, y0 + 1.25f, cz, 0.22f, d * 0.82f, pillarCol);

        // 3. Hệ xà dầm đỡ diềm mái (y = yColTop - 0.10 .. yColTop + 0.32)
        appendBoxByYRange(target, cx, yColTop - 0.10f, yColTop + 0.32f, cz, w * 0.94f, d * 0.92f, beamCol);

        // 4. Mái ngói cong tầng dưới đặt trực tiếp trên xà đỡ (y = yColTop + 0.28)
        const float roof1Y = yColTop + 0.28f;
        const float roof1H = twoTierRoof ? 1.65f : 2.10f;
        target.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            cx, roof1Y, cz,
            w, d, roof1H, 1.8f, roofCol
        ));

        if (twoTierRoof) {
            // Cổ diềm tầng trên mọc liền khối xuyên qua đỉnh mái dưới
            const float neckY0 = roof1Y + 0.85f;
            const float neckY1 = roof1Y + roof1H + 0.55f;
            appendBoxByYRange(target, cx, neckY0, neckY1, cz, w * 0.58f, d * 0.56f, pillarCol);
            appendBoxByYRange(target, cx, neckY1 - 0.12f, neckY1 + 0.16f, cz, w * 0.62f, d * 0.60f, beamCol);

            target.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                cx, neckY1 + 0.12f, cz,
                w * 0.64f, d * 0.62f, 1.55f, 1.4f, roofCol
            ));
            target.append(Geometry::PolygonMesh::createPyramid(
                cx, neckY1 + 0.12f + 1.45f, cz,
                0.7f, 0.7f, 0.95f, goldCol
            ));
        } else {
            target.append(Geometry::PolygonMesh::createPyramid(
                cx, roof1Y + roof1H - 0.10f, cz,
                0.75f, 0.75f, 1.0f, goldCol
            ));
        }
    }

    /**
     * @brief Xây dựng toàn bộ lưới tĩnh của Đợt 4:
     *        - Cụm Sân khấu & Nhà ga chân Sun Wheel (hộp sân khấu + 2 tháp màn hình tròn + 2 chòi bên)
     *        - Hai cụm chòi mái ngói hai bên Quảng trường Trung tâm
     *        - Hai chòi vọng cảnh nhỏ ven bờ Nam hồ nước
     */
    void buildStaticMeshes() {
        staticArchMesh.clear();
        staticArchMesh.name = "Wave4PlazaStageAndPavilions";

        const MathCG::ColorRGB plinthCol(0.50f, 0.49f, 0.47f);
        const MathCG::ColorRGB creamWallCol(0.90f, 0.87f, 0.80f);
        const MathCG::ColorRGB darkFrameCol(0.32f, 0.25f, 0.22f);
        const MathCG::ColorRGB stageDeckCol(0.24f, 0.28f, 0.38f);
        const MathCG::ColorRGB goldArchCol(0.95f, 0.76f, 0.18f);
        const MathCG::ColorRGB roofBrownCol(0.58f, 0.28f, 0.14f);
        const MathCG::ColorRGB redCol(0.76f, 0.18f, 0.14f);

        // ====================================================================
        // 1. CỤM SÂN KHẤU & NHÀ GA CHÂN SUN WHEEL (ParkLayout::SUN_WHEEL_STAGE_POS)
        //    Đặt tại (0.0, 0.0, -17.7), sâu 9.0m (Z thuộc [-22.2, -13.2]), rộng 52.0m (X thuộc [-26, +26])
        //    -> Cách hoàn toàn 2 bệ móng chân chữ A phía Nam (Z thuộc [-28.2, -22.8]), 0% xuyên cắt!
        // ====================================================================
        const float stZ  = ParkLayout::SUN_WHEEL_STAGE_POS.z; // -17.7m
        const float stD  = ParkLayout::SUN_WHEEL_STAGE_DEPTH; // 9.0m (Z: -22.2 .. -13.2)
        const float stW  = ParkLayout::SUN_WHEEL_STAGE_TOTAL_WIDTH; // 52.0m

        // 1.1. Bệ móng chung chạy suốt chiều ngang 52m (y = 0.0 .. 0.65m)
        appendBoxByYRange(staticArchMesh, 0.0f, 0.0f, 0.65f, stZ, stW, stD, plinthCol);

        // 1.2. Bục Sân khấu Trung tâm nhô ra phía Nam (X thuộc [-13.5, +13.5], Z thuộc [-22.2, -10.6], y = 0.0 .. 1.15m)
        appendBoxByYRange(staticArchMesh, 0.0f, 0.0f, 1.15f, stZ + 1.3f, 27.0f, stD + 2.6f, stageDeckCol);
        // Bậc tam cấp bước lên sân khấu ở mặt Nam (Z = -10.6 .. -9.4m)
        appendBoxByYRange(staticArchMesh, 0.0f, 0.0f, 0.40f, stZ + 5.8f, 18.0f, 1.2f, plinthCol);
        appendBoxByYRange(staticArchMesh, 0.0f, 0.0f, 0.78f, stZ + 5.2f, 18.0f, 1.2f, plinthCol);

        // 1.3. Thân hậu đài Sân khấu Trung tâm (X thuộc [-13.5, +13.5], Z thuộc [-22.2, -14.8], y = 1.10 .. 8.80m)
        appendBoxByYRange(staticArchMesh, 0.0f, 1.10f, 8.80f, stZ - 0.7f, 27.0f, stD - 1.4f, darkFrameCol);
        // Hai cột trụ lớn mạ vàng kẹp hai bên khung sân khấu (cx = ±12.2m, y = 1.10 .. 9.10m)
        for (int s : {-1, 1}) {
            appendBoxByYRange(staticArchMesh, s * 12.2f, 1.10f, 9.10f, stZ + 1.2f, 2.2f, 2.4f, goldArchCol);
        }
        // Dầm cổng vòm vàng rực phía trên sân khấu (y = 7.80 .. 9.40m) và mái cong sân khấu trung tâm (y = 9.35 .. 11.60m)
        appendBoxByYRange(staticArchMesh, 0.0f, 7.80f, 9.40f, stZ + 1.2f, 26.8f, 2.6f, goldArchCol);
        staticArchMesh.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 9.35f, stZ - 0.5f,
            26.0f, stD - 0.8f, 2.15f, 2.0f, roofBrownCol
        ));

        // 1.4. Hai Khối Nhà Cánh Đối Xứng gắn HAI MÀN HÌNH TRÒN LỚN (cx = ±19.75m, rộng 12.5m mỗi bên: X thuộc [±13.5, ±26.0])
        for (int side : {-1, 1}) {
            float wingCX = side * 19.75f;
            // Thân nhà tường trắng kem viền khung cột nâu sẫm (y = 0.60 .. 8.60m)
            appendBoxByYRange(staticArchMesh, wingCX, 0.60f, 8.60f, stZ, 12.5f, stD, creamWallCol);
            // Bổ trụ nâu sẫm kẹp hai mép mặt tiền khối màn hình tròn
            appendBoxByYRange(staticArchMesh, wingCX - side * 5.6f, 0.60f, 8.70f, stZ + stD * 0.5f + 0.12f, 1.1f, 0.30f, darkFrameCol);
            appendBoxByYRange(staticArchMesh, wingCX + side * 5.6f, 0.60f, 8.70f, stZ + stD * 0.5f + 0.12f, 1.1f, 0.30f, darkFrameCol);
            // Xà đỡ mái khối cánh (y = 8.50 .. 8.90m)
            appendBoxByYRange(staticArchMesh, wingCX, 8.50f, 8.90f, stZ, 13.0f, stD + 0.5f, darkFrameCol);

            // Mái ngói cong tầng 1 của khối màn hình tròn đặt trực tiếp tại y = 8.85m
            staticArchMesh.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                wingCX, 8.85f, stZ,
                12.8f, stD + 0.4f, 1.80f, 1.6f, roofBrownCol
            ));
            // Cổ lầu tầng 2 mọc liền khối từ y = 9.75m lên y = 11.40m + mái cong tầng 2 (giống 2 tháp chòi trên nóc khối màn hình tròn trong ảnh đêm 2 & 3)
            appendBoxByYRange(staticArchMesh, wingCX, 9.75f, 11.40f, stZ, 7.6f, 5.8f, redCol);
            appendBoxByYRange(staticArchMesh, wingCX, 11.30f, 11.60f, stZ, 8.0f, 6.2f, darkFrameCol);
            staticArchMesh.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                wingCX, 11.55f, stZ,
                8.2f, 6.4f, 1.65f, 1.4f, roofBrownCol
            ));
        }

        // 1.5. Hai Chòi Mái Ngói Hai Bên Chân Sun Wheel (tại (±40.0, 0.0, -24.0), rộng 12x12m)
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_WEST_POS,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_SIZE,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_SIZE,
            3.8f, true
        );
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_EAST_POS,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_SIZE,
            ParkLayout::SUN_WHEEL_SIDE_PAVILION_SIZE,
            3.8f, true
        );

        // ====================================================================
        // 2. HAI CỤM CHÒI MÁI NGÓI HAI BÊN QUẢNG TRƯỜNG TRUNG TÂM
        //    Tây (-24.0, 0.0, +65.0) & Đông (+48.0, 0.0, +65.0), kích thước 16x9m
        // ====================================================================
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::PLAZA_PAVILION_WEST_POS,
            ParkLayout::PLAZA_PAVILION_WIDTH,
            ParkLayout::PLAZA_PAVILION_DEPTH,
            3.6f, true
        );
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::PLAZA_PAVILION_EAST_POS,
            ParkLayout::PLAZA_PAVILION_WIDTH,
            ParkLayout::PLAZA_PAVILION_DEPTH,
            3.6f, true
        );

        // ====================================================================
        // 3. HAI CHÒI VỌNG CẢNH NHỎ VEN BỜ NAM HỒ NƯỚC (CẠNH THỦY ĐÌNH)
        //    Tây (-49.2, 0.0, -88.0) & Đông (+40.7, 0.0, -91.1), kích thước 10x8m
        // ====================================================================
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::LAKE_SHORE_PAVILION_WEST_POS,
            ParkLayout::LAKE_SHORE_PAVILION_WIDTH,
            ParkLayout::LAKE_SHORE_PAVILION_DEPTH,
            3.4f, false
        );
        appendAsianPavilion(
            staticArchMesh,
            ParkLayout::LAKE_SHORE_PAVILION_EAST_POS,
            ParkLayout::LAKE_SHORE_PAVILION_WIDTH,
            ParkLayout::LAKE_SHORE_PAVILION_DEPTH,
            3.4f, false
        );

        // Cầu gỗ nối từ đường dạo bờ Nam hồ (Z = -66.8m) ra 2 chòi bờ hồ Tây và Đông (cao y = 0.24m)
        const MathCG::ColorRGB bridgeWoodCol(0.56f, 0.35f, 0.20f);
        appendBoxByYRange(
            staticArchMesh,
            ParkLayout::LAKE_SHORE_PAVILION_WEST_POS.x,
            0.05f, 0.26f,
            0.5f * (-66.8f + (ParkLayout::LAKE_SHORE_PAVILION_WEST_POS.z + 4.0f)),
            2.6f,
            std::fabs((-66.8f) - (ParkLayout::LAKE_SHORE_PAVILION_WEST_POS.z + 4.0f)),
            bridgeWoodCol
        );
        appendBoxByYRange(
            staticArchMesh,
            ParkLayout::LAKE_SHORE_PAVILION_EAST_POS.x,
            0.05f, 0.26f,
            0.5f * (-65.7f + (ParkLayout::LAKE_SHORE_PAVILION_EAST_POS.z + 4.0f)),
            2.6f,
            std::fabs((-65.7f) - (ParkLayout::LAKE_SHORE_PAVILION_EAST_POS.z + 4.0f)),
            bridgeWoodCol
        );

        // ====================================================================
        // 4. ĐỢT 5B: 5 NHÀ MÁI NGÓI (MỘT HÀM THAM SỐ HÓA appendTileRoofedHall)
        //    [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
        // ====================================================================
        const MathCG::ColorRGB tealBlueRoof1(0.14f, 0.48f, 0.64f);
        const MathCG::ColorRGB tealBlueRoof2(0.16f, 0.52f, 0.68f);
        const MathCG::ColorRGB vermilionRoof(0.80f, 0.24f, 0.14f);

        // #7: Nhà mái ngói xanh Bắc sân bông hoa (-113.4, 0, +55.0), 26x12m, xoay +12°
        appendTileRoofedHall(staticArchMesh, ParkLayout::BLUE_ROOF_HALL_NORTH_POS, 26.0f, 12.0f, 4.2f,  12.0f, tealBlueRoof1, 0);
        // #8: Nhà mái ngói xanh Nam sân bông hoa (-102.7, 0, +118.0), 22x11m, xoay -15°
        appendTileRoofedHall(staticArchMesh, ParkLayout::BLUE_ROOF_HALL_SOUTH_POS, 22.0f, 11.0f, 4.0f, -15.0f, tealBlueRoof1, 0);
        // #12: Nhà ngói đỏ lớn ven hồ (-94.2, 0, -71.2), 22x18m, xoay 0°
        appendTileRoofedHall(staticArchMesh, ParkLayout::RED_ROOF_LAKESIDE_HALL_POS, 22.0f, 18.0f, 4.4f,  0.0f, vermilionRoof, 0);
        // #15: Nhà mái xanh-trắng khu Tây (-256.0, 0, -128.7), 20x20m, kiểu mái vòm rộng viền trắng (roofStyle = 1)
        appendTileRoofedHall(staticArchMesh, ParkLayout::WEST_CANOPY_THEATER_POS,   20.0f, 20.0f, 4.5f,   0.0f, tealBlueRoof2, 1);
        // #18: Nhà mái xanh khu Angkor ven hồ (-138.0, 0, -242.0), 20x14m, xoay -22°
        appendTileRoofedHall(staticArchMesh, ParkLayout::ANGKOR_LAKESIDE_HALL_POS,  20.0f, 14.0f, 4.2f, -22.0f, tealBlueRoof1, 0);

        // ====================================================================
        // 5. ĐỢT 5B: DÃY 3 HÀNG QUẦY NHỎ MÁI SỌC ĐỎ-TRẮNG (#6 - STRIPED_STALLS_POS)
        // ====================================================================
        appendStripedStallsComplex(staticArchMesh, ParkLayout::STRIPED_STALLS_POS);

        // ====================================================================
        // 6. ĐỢT 5B: QUẢNG TRƯỜNG NGÔI SAO & 4 THÁP CHÒI NHỌN ANGKOR (#19)
        // ====================================================================
        appendAngkorStarPlazaTowers(staticArchMesh, ParkLayout::ANGKOR_STAR_PLAZA_POS);

        // ====================================================================
        // 7. ĐỢT 5B: PHẦN BỆ TĨNH & HÀNH LANG CỘT CỦA 4 TRÒ ĐU QUAY TRÒN (#9, #10, #13, #14)
        // ====================================================================
        appendRoundRideStaticBase(staticArchMesh, ParkLayout::ROUND_SWING_RIDE_1_POS,   8.5f, 8.2f, MathCG::ColorRGB(0.92f, 0.45f, 0.14f), false);
        appendRoundRideStaticBase(staticArchMesh, ParkLayout::ROUND_SPIN_RIDE_2_POS,    6.5f, 6.4f, MathCG::ColorRGB(0.20f, 0.56f, 0.88f), false);
        appendRoundRideStaticBase(staticArchMesh, ParkLayout::CENTRAL_ROTUNDA_RIDE_POS, 9.0f, 8.8f, MathCG::ColorRGB(0.78f, 0.24f, 0.16f), true);
        appendRoundRideStaticBase(staticArchMesh, ParkLayout::ROUND_SPIN_RIDE_3_POS,    7.5f, 7.4f, MathCG::ColorRGB(0.16f, 0.64f, 0.68f), false);

        // ====================================================================
        // 8. ĐỢT 6A: BỆ SÀN ĐÁ/GẠCH MỎNG DƯỚI PLAYGROUND & PIRATE SHIP +
        //    NHÀ VÒM CỘT MÁI NGÓI BAO QUANH BUMPER CARS (KHÔNG SỬA 3 FILE CỦA BẠN)
        // ====================================================================
        // 8.1. Mái che + hàng cột + bệ đá bao quanh BumperCars (-206.5, 0, +108.1, R=10.0m) bằng chính hàm appendTileRoofedHall (openColonnadeOnly = true)
        appendTileRoofedHall(staticArchMesh, ParkLayout::BUMPER_CARS_TARGET_POS, 23.0f, 23.0f, 3.8f, 0.0f, tealBlueRoof1, 0, true);

        // 8.2. Bệ sàn đá/gạch mỏng dưới Playground (-104.9, 0, -24.7; trải từ X:-118..+93, Z:-28..-13)
        {
            const float pgX = ParkLayout::PLAYGROUND_TARGET_POS.x - 0.8f;
            const float pgZ = ParkLayout::PLAYGROUND_TARGET_POS.z + 4.2f;
            appendBoxByYRange(staticArchMesh, pgX, -0.01f, 0.024f, pgZ, 26.8f, 15.8f, MathCG::ColorRGB(0.56f, 0.53f, 0.48f));
            appendBoxByYRange(staticArchMesh, pgX,  0.022f, 0.032f, pgZ, 25.4f, 14.4f, MathCG::ColorRGB(0.84f, 0.74f, 0.56f));
            // Viền bó vỉa thấp 4 góc bệ sân chơi
            appendBoxByYRange(staticArchMesh, pgX,  0.030f, 0.20f, pgZ - 7.7f, 26.6f, 0.35f, MathCG::ColorRGB(0.68f, 0.34f, 0.22f));
            appendBoxByYRange(staticArchMesh, pgX,  0.030f, 0.20f, pgZ + 7.7f, 26.6f, 0.35f, MathCG::ColorRGB(0.68f, 0.34f, 0.22f));
        }

        // 8.3. Bệ sàn đá/gạch mỏng dưới PirateShip (-172.0, 0, -2.0; khung 18x13m)
        {
            const float psX = ParkLayout::PIRATE_SHIP_TARGET_POS.x;
            const float psZ = ParkLayout::PIRATE_SHIP_TARGET_POS.z;
            appendBoxByYRange(staticArchMesh, psX, -0.01f, 0.024f, psZ, 21.6f, 15.2f, MathCG::ColorRGB(0.52f, 0.50f, 0.46f));
            appendBoxByYRange(staticArchMesh, psX,  0.022f, 0.032f, psZ, 20.2f, 13.8f, MathCG::ColorRGB(0.78f, 0.72f, 0.62f));
            // Hai bục lan can đá thấp dọc hai cạnh Đông - Tây ngoài hành trình văng thuyền
            appendBoxByYRange(staticArchMesh, psX - 10.2f, 0.030f, 0.45f, psZ, 0.65f, 13.6f, MathCG::ColorRGB(0.62f, 0.58f, 0.52f));
            appendBoxByYRange(staticArchMesh, psX + 10.2f, 0.030f, 0.45f, psZ, 0.65f, 13.6f, MathCG::ColorRGB(0.62f, 0.58f, 0.52f));
        }

        // ====================================================================
        // 9. ĐỢT 6A: TRỤ ĐÈN LỐI ĐI, GHẾ NGHỈ, BỒN HOA HỘP MÀU, CỘT CỜ,
        //    BIỂN CHỈ DẪN & DÃY ĐÈN LỒNG DỌC CỔNG NGỌ MÔN VÀ CÁC CHÒI
        // ====================================================================
        appendWave6aPropsAndLanterns(staticArchMesh);

        // ====================================================================
        // 10. ĐỢT 6B: 123 CÂY XANH ĐƠN GIẢN (3 KIỂU: TÁN TRÒN, TÙNG THÁP, CỌ DỪA)
        //     Nén trực tiếp vào Display List tĩnh để giữ FPS > 130
        // ====================================================================
        appendWave6bTrees(staticArchMesh);

        staticArchMesh.computeNormals();
    }

    /**
     * @brief Vẽ mặt nền Quảng trường Trung tâm lát gạch chia ô phẳng tại y = 0.028m
     */
    void drawTiledCentralPlaza() const {
        const float cx = ParkLayout::CENTRAL_PLAZA_CENTER.x;
        const float cz = ParkLayout::CENTRAL_PLAZA_CENTER.z;
        const float hw = ParkLayout::CENTRAL_PLAZA_WIDTH * 0.5f;
        const float hd = ParkLayout::CENTRAL_PLAZA_DEPTH * 0.5f;
        const float y  = ParkLayout::CENTRAL_PLAZA_Y;

        glNormal3f(0.0f, 1.0f, 0.0f);

        // 1. Nền đá lát chính của Quảng trường Trung tâm (X thuộc [-26.9, +59.1], Z thuộc [-3.0, +115.0])
        glColor3f(0.68f, 0.65f, 0.59f);
        glBegin(GL_QUADS);
        glVertex3f(cx - hw, y, cz - hd);
        glVertex3f(cx - hw, y, cz + hd);
        glVertex3f(cx + hw, y, cz + hd);
        glVertex3f(cx + hw, y, cz - hd);
        glEnd();

        // 2. Mở rộng sân lát gạch phía Bắc Quảng trường nối thẳng tới trước Sân khấu Sun Wheel (X thuộc [-36, +36], Z thuộc [-13.2, -3.0])
        glColor3f(0.72f, 0.69f, 0.63f);
        glBegin(GL_QUADS);
        glVertex3f(-36.0f, y + 0.002f, -13.2f);
        glVertex3f(-36.0f, y + 0.002f,  28.0f);
        glVertex3f( 36.0f, y + 0.002f,  28.0f);
        glVertex3f( 36.0f, y + 0.002f, -13.2f);
        glEnd();

        // 3. Các đường kẻ chỉ ô gạch lát đá quảng trường (nâng nhẹ y + 0.004m tránh Z-fighting)
        glColor3f(0.54f, 0.52f, 0.47f);
        glLineWidth(1.2f);
        glBegin(GL_LINES);
        for (float gx = cx - hw; gx <= cx + hw + 0.1f; gx += 10.75f) {
            glVertex3f(gx, y + 0.004f, cz - hd);
            glVertex3f(gx, y + 0.004f, cz + hd);
        }
        for (float gz = cz - hd; gz <= cz + hd + 0.1f; gz += 11.8f) {
            glVertex3f(cx - hw, y + 0.004f, gz);
            glVertex3f(cx + hw, y + 0.004f, gz);
        }
        glEnd();
        glLineWidth(1.0f);

        // 4. Sân lát hình Bông Hoa 8 cánh giữa 2 nhà mái ngói xanh #7 & #8 (FLOWER_PLAZA_CENTER: -113.4, 0, +81.5)
        {
            const float fx = ParkLayout::FLOWER_PLAZA_CENTER.x;
            const float fz = ParkLayout::FLOWER_PLAZA_CENTER.z;
            const float fy = 0.044f;
            // Đĩa nền đá tròn ngoại tiếp (thứ tự đỉnh CCW khi nhìn từ +Y xuống: sin(-a))
            glColor3f(0.58f, 0.56f, 0.52f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(fx, fy, fz);
            for (int i = 0; i <= 32; ++i) {
                float a = (static_cast<float>(i) / 32.0f) * MathCG::TWO_PI;
                glVertex3f(fx + 10.8f * std::cos(a), fy, fz - 10.8f * std::sin(a));
            }
            glEnd();
            // Hoa văn 8 cánh hoa vàng nghệ ở tâm sân
            glColor3f(0.95f, 0.78f, 0.20f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(fx, fy + 0.004f, fz);
            for (int i = 0; i <= 64; ++i) {
                float a = (static_cast<float>(i) / 64.0f) * MathCG::TWO_PI;
                float r = 5.2f + 4.2f * std::fabs(std::cos(4.0f * a));
                glVertex3f(fx + r * std::cos(a), fy + 0.004f, fz - r * std::sin(a));
            }
            glEnd();
        }

        // 5. Nền lát hình Ngôi Sao 8 cánh tại Quảng trường Angkor (#19 - ANGKOR_STAR_PLAZA_POS: -185.0, 0, -254.0)
        //    Mở rộng lên 27.8 x 27.8 m (vệ tinh ~28x28m) để ôm trọn 4 góc chân đền 26 x 26 m
        {
            const float ax = ParkLayout::ANGKOR_STAR_PLAZA_POS.x;
            const float az = ParkLayout::ANGKOR_STAR_PLAZA_POS.z;
            const float ay = 0.044f;
            const float hs = ParkLayout::ANGKOR_STAR_PLAZA_HALF;
            const float starOuterR = ParkLayout::ANGKOR_STAR_OUTER_RADIUS;
            // Nền đá vuông
            glColor3f(0.60f, 0.55f, 0.47f);
            glBegin(GL_QUADS);
            glVertex3f(ax - hs, ay, az - hs);
            glVertex3f(ax - hs, ay, az + hs);
            glVertex3f(ax + hs, ay, az + hs);
            glVertex3f(ax + hs, ay, az - hs);
            glEnd();
            // Ngôi sao 8 cánh lát đá vàng-đỏ ở chính giữa quảng trường Angkor (CCW khi nhìn từ +Y xuống)
            glColor3f(0.95f, 0.74f, 0.22f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(ax, ay + 0.004f, az);
            for (int i = 0; i <= 16; ++i) {
                float a = (static_cast<float>(i) / 16.0f) * MathCG::TWO_PI;
                float r = (i % 2 == 0) ? starOuterR : 5.2f;
                glVertex3f(ax + r * std::cos(a), ay + 0.004f, az - r * std::sin(a));
            }
            glEnd();
        }

        // 6. Nền cỏ thống nhất một màu theo đa giác ranh giới công viên (y = -0.04m trong main.cpp),
        //    đã bỏ các mảng chữ nhật grassZones nhiều màu để mặt cỏ liền mạch tự nhiên.

        // 7. ĐỢT 6A: Các mảng sân lát đá / gạch bao quanh các cụm trò chơi và nhà mái ngói khu Tây (y = 0.022m)
        struct PavedCourtRect {
            float cx, cz, w, d;
            float r, g, b;
        };
        const PavedCourtRect courts[8] = {
            { -124.1f, 155.7f, 36.0f, 28.0f, 0.62f, 0.58f, 0.52f }, // Sân đá dưới Garuda Valley
            { -159.4f, 102.5f, 21.0f, 21.0f, 0.66f, 0.61f, 0.54f }, // Sân gạch quanh Đu quay Ngựa gỗ Carousel
            { -224.0f,  22.0f, 36.0f, 48.0f, 0.60f, 0.57f, 0.52f }, // Sân đá dưới Port of Sky Treasure
            { -113.5f,  15.0f, 64.0f, 24.0f, 0.65f, 0.61f, 0.55f }, // Sân chung nối 2 Đu quay tròn #9 và #10
            {  -94.2f, -71.2f, 28.0f, 24.0f, 0.67f, 0.62f, 0.56f }, // Sân gạch quanh Nhà ngói đỏ lớn ven hồ #12
            { -188.0f,-106.0f, 56.0f, 46.0f, 0.58f, 0.55f, 0.50f }, // Sân đá khu Núi đá & Ray số 8 Paradise Fall
            { -142.0f,-158.0f, 26.0f, 66.0f, 0.64f, 0.60f, 0.54f }, // Sân gạch nối Công trình tròn #13 & Đu quay #14
            { -176.0f,-214.0f, 36.0f, 28.0f, 0.61f, 0.58f, 0.53f }  // Sân đá dưới tàu lượn Highway Boat
        };
        glBegin(GL_QUADS);
        for (const auto& ct : courts) {
            float hw2 = ct.w * 0.5f;
            float hd2 = ct.d * 0.5f;
            glColor3f(ct.r, ct.g, ct.b);
            glVertex3f(ct.cx - hw2, 0.022f, ct.cz - hd2);
            glVertex3f(ct.cx - hw2, 0.022f, ct.cz + hd2);
            glVertex3f(ct.cx + hw2, 0.022f, ct.cz + hd2);
            glVertex3f(ct.cx + hw2, 0.022f, ct.cz - hd2);
        }
        glEnd();
    }

    /**
     * @brief Vẽ HAI MÀN HÌNH TRÒN phát sáng lớn và màn hình sân khấu dưới chân Sun Wheel
     *        (Đợt 7: Đổi màn hình giữa sân khấu từ trắng phẳng sang nền xanh dịu chuyển sắc
     *         kèm biểu tượng Vòng quay / mặt trời vàng, sóng nước và dòng chữ "ASIA PARK" đơn giản)
     */
    void drawStageToroidRingsDirect() const {
        const float stZ     = ParkLayout::SUN_WHEEL_STAGE_POS.z; // -15.9m
        const float frontZ  = stZ + ParkLayout::SUN_WHEEL_STAGE_DEPTH * 0.5f + 0.08f;
        const float screenR = ParkLayout::SUN_WHEEL_SCREEN_RADIUS; // 3.1m
        glColor3f(0.92f, 0.74f, 0.20f);
        for (int side : {-1, 1}) {
            glPushMatrix();
            glTranslatef(side * 19.75f, 4.65f, frontZ);
            ParametricShapes::drawToroid(screenR + 0.12f, 0.22f, 28, 8, false);
            glPopMatrix();
        }
    }

    void drawStageFlatGraphicsDirect(bool isNightOrSunset) const {
        const float stZ     = ParkLayout::SUN_WHEEL_STAGE_POS.z; // -15.9m
        const float frontZ  = stZ + ParkLayout::SUN_WHEEL_STAGE_DEPTH * 0.5f + 0.08f;
        const float screenR = ParkLayout::SUN_WHEEL_SCREEN_RADIUS; // 3.1m

        // 1. Mặt đĩa hai Màn hình Tròn lớn ở hai khối nhà cánh (cx = ±19.75m, cy = 4.65m)
        for (int side : {-1, 1}) {
            float sx = side * 19.75f;
            float sy = 4.65f;

            glPushMatrix();
            glTranslatef(sx, sy, frontZ);

            glColor3f(isNightOrSunset ? 0.18f : 0.22f, isNightOrSunset ? 0.52f : 0.58f, isNightOrSunset ? 0.78f : 0.84f);
            ParametricShapes::drawDisk(screenR, 28, 0.0f, false);

            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.03f);
            glColor3f(0.32f, 0.74f, 0.88f);
            ParametricShapes::drawDisk(screenR * 0.68f, 24, 0.0f, false);

            glTranslatef(0.0f, 0.0f, 0.02f);
            glColor3f(0.98f, 0.84f, 0.36f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0.0f, 0.0f, 0.0f);
            for (int i = 0; i <= 32; ++i) {
                float a = (static_cast<float>(i) / 32.0f) * MathCG::TWO_PI;
                float r = 0.75f + 0.65f * std::fabs(std::cos(4.0f * a));
                glVertex3f(r * std::cos(a), r * std::sin(a), 0.0f);
            }
            glEnd();
            glPopMatrix();

            glPopMatrix();
        }

        // 2. Màn hình LED chính giữa Sân khấu Sun Wheel (X thuộc [-9.5, +9.5], Y thuộc [1.55, 7.25])
        const float scrZ = stZ + 4.54f; // Ngay phía trước tường hậu đài sân khấu

        // 2.1. Khung viền kim loại sẫm bao quanh màn hình
        glColor3f(0.16f, 0.20f, 0.28f);
        glBegin(GL_QUADS);
        glVertex3f(-9.8f, 1.42f, scrZ - 0.03f);
        glVertex3f( 9.8f, 1.42f, scrZ - 0.03f);
        glVertex3f( 9.8f, 7.38f, scrZ - 0.03f);
        glVertex3f(-9.8f, 7.38f, scrZ - 0.03f);
        glEnd();

        // 2.2. Nền màn hình xanh dịu chuyển sắc dọc
        glBegin(GL_QUADS);
        glColor3f(0.18f, 0.56f, 0.78f);
        glVertex3f(-9.5f, 1.55f, scrZ);
        glVertex3f( 9.5f, 1.55f, scrZ);
        glColor3f(0.09f, 0.30f, 0.56f);
        glVertex3f( 9.5f, 7.25f, scrZ);
        glVertex3f(-9.5f, 7.25f, scrZ);
        glEnd();

        // 2.3. Hoa văn dải sóng nước xanh sáng dưới chân màn hình (y = 1.65 .. 2.35m)
        const float zFg = scrZ + 0.03f;
        glColor3f(0.32f, 0.76f, 0.90f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 24; ++i) {
            float tx = -9.4f + 18.8f * (static_cast<float>(i) / 24.0f);
            float wy = 2.05f + 0.22f * std::sin(tx * 0.65f);
            glVertex3f(tx, 1.58f, zFg);
            glVertex3f(tx, wy,    zFg);
        }
        glEnd();

        // 2.4. Hoa văn biểu tượng Sun Wheel / Mặt trời vàng ở tâm màn hình (cx = 0, cy = 4.05m, R = 1.65m)
        const float embY = 4.05f;
        glColor3f(0.96f, 0.80f, 0.26f);
        glLineWidth(2.2f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 28; ++i) {
            float a = (static_cast<float>(i) / 28.0f) * MathCG::TWO_PI;
            glVertex3f(1.65f * std::cos(a), embY + 1.65f * std::sin(a), zFg + 0.01f);
        }
        glEnd();
        glColor3f(0.45f, 0.88f, 0.96f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 24; ++i) {
            float a = (static_cast<float>(i) / 24.0f) * MathCG::TWO_PI;
            glVertex3f(1.10f * std::cos(a), embY + 1.10f * std::sin(a), zFg + 0.01f);
        }
        glEnd();
        // 12 nan hoa hướng tâm trên biểu tượng màn hình
        glColor3f(0.98f, 0.86f, 0.38f);
        glBegin(GL_LINES);
        for (int i = 0; i < 12; ++i) {
            float a = (static_cast<float>(i) / 12.0f) * MathCG::TWO_PI;
            glVertex3f(0.0f, embY, zFg + 0.01f);
            glVertex3f(1.65f * std::cos(a), embY + 1.65f * std::sin(a), zFg + 0.01f);
        }
        glEnd();

        // 2.5. Dòng chữ cách điệu "ASIA PARK" bằng nét вектор ở phần trên màn hình (y = 6.0 .. 6.8m)
        glColor3f(0.98f, 0.88f, 0.42f);
        glLineWidth(2.6f);
        auto drawScreenStrokeLetter = [&](char ch, float lx, float ly, float w, float h) {
            float x0 = lx, x1 = lx + w, xm = lx + w * 0.5f;
            float y0 = ly, y1 = ly + h, ym = ly + h * 0.5f;
            const float zt = zFg + 0.02f;
            glBegin(GL_LINES);
            switch (ch) {
                case 'A':
                    glVertex3f(x0, y0, zt); glVertex3f(xm, y1, zt);
                    glVertex3f(xm, y1, zt); glVertex3f(x1, y0, zt);
                    glVertex3f(x0 + w*0.2f, ym, zt); glVertex3f(x1 - w*0.2f, ym, zt);
                    break;
                case 'S':
                    glVertex3f(x1, y1, zt); glVertex3f(x0, y1, zt);
                    glVertex3f(x0, y1, zt); glVertex3f(x0, ym, zt);
                    glVertex3f(x0, ym, zt); glVertex3f(x1, ym, zt);
                    glVertex3f(x1, ym, zt); glVertex3f(x1, y0, zt);
                    glVertex3f(x1, y0, zt); glVertex3f(x0, y0, zt);
                    break;
                case 'I':
                    glVertex3f(x0, y1, zt); glVertex3f(x1, y1, zt);
                    glVertex3f(xm, y0, zt); glVertex3f(xm, y1, zt);
                    glVertex3f(x0, y0, zt); glVertex3f(x1, y0, zt);
                    break;
                case 'P':
                    glVertex3f(x0, y0, zt); glVertex3f(x0, y1, zt);
                    glVertex3f(x0, y1, zt); glVertex3f(x1, y1, zt);
                    glVertex3f(x1, y1, zt); glVertex3f(x1, ym, zt);
                    glVertex3f(x1, ym, zt); glVertex3f(x0, ym, zt);
                    break;
                case 'R':
                    glVertex3f(x0, y0, zt); glVertex3f(x0, y1, zt);
                    glVertex3f(x0, y1, zt); glVertex3f(x1, y1, zt);
                    glVertex3f(x1, y1, zt); glVertex3f(x1, ym, zt);
                    glVertex3f(x1, ym, zt); glVertex3f(x0, ym, zt);
                    glVertex3f(xm, ym, zt); glVertex3f(x1, y0, zt);
                    break;
                case 'K':
                    glVertex3f(x0, y0, zt); glVertex3f(x0, y1, zt);
                    glVertex3f(x0, ym, zt); glVertex3f(x1, y1, zt);
                    glVertex3f(x0, ym, zt); glVertex3f(x1, y0, zt);
                    break;
                default: break;
            }
            glEnd();
        };
        const char* title = "ASIA PARK";
        float startX = -4.8f;
        for (int idx = 0; title[idx] != '\0'; ++idx) {
            if (title[idx] != ' ') {
                drawScreenStrokeLetter(title[idx], startX + idx * 1.12f, 5.98f, 0.78f, 0.85f);
            }
        }
        glLineWidth(1.0f);
    }

    void drawStageCircularScreensAndLights(bool isNightOrSunset) const {
        ensureDisplayLists();
        const int modeIdx = isNightOrSunset ? 1 : 0;

        // 1. Vành khuyên viền mạ vàng quanh 2 màn hình tròn (theo chế độ chiếu sáng hiện tại)
        if (stageToroidsList_ != 0) {
            glCallList(stageToroidsList_);
        } else {
            drawStageToroidRingsDirect();
        }

        // 2. Đồ họa tĩnh trên các màn hình (bọc glDisable/glEnable(GL_LIGHTING) ở ngoài Display List)
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);
        if (stageFlatScreensList_[modeIdx] != 0) {
            glCallList(stageFlatScreensList_[modeIdx]);
        } else {
            drawStageFlatGraphicsDirect(isNightOrSunset);
        }
        if (wasLit) glEnable(GL_LIGHTING);

        // 3. ĐỢT 6A: Bóng đèn trụ đèn lối đi & dãy đèn lồng phát sáng bằng vật liệu emissive khi chiều/tối
        if (isNightOrSunset) {
            GLfloat warmBulbEmit[4] = { 1.0f, 0.88f, 0.45f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, warmBulbEmit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, warmBulbEmit);
        }
        if (emissiveLampList != 0) {
            glCallList(emissiveLampList);
        } else {
            emissiveLampMesh.draw(true, false);
        }
        if (isNightOrSunset) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
    }

    static void compileSpinningCanopyAndSeats(
        float radius,
        float height,
        const MathCG::ColorRGB& primaryCol,
        const MathCG::ColorRGB& secondaryCol,
        int numSeats,
        bool hasOuterColonnade
    ) {
        const float innerR   = hasOuterColonnade ? (radius * 0.72f) : (radius * 0.88f);
        const float colH     = height * 0.65f;
        const int   numWedges = 16;
        const float canopyY0 = colH - 0.15f;
        const float canopyY1 = height;

        // 1. Mái nón chia 16 múi sọc màu xen kẽ (primaryCol / secondaryCol) đặt liền khối trên đỉnh trụ
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < numWedges; ++i) {
            float a0 = (static_cast<float>(i)     / numWedges) * MathCG::TWO_PI;
            float a1 = (static_cast<float>(i + 1) / numWedges) * MathCG::TWO_PI;
            float am = 0.5f * (a0 + a1);
            const MathCG::ColorRGB& c = (i % 2 == 0) ? primaryCol : secondaryCol;
            glColor3f(c.r, c.g, c.b);
            glNormal3f(std::cos(am) * 0.45f, 0.89f, std::sin(am) * 0.45f);
            glVertex3f(0.0f, canopyY1, 0.0f);
            glVertex3f(innerR * std::cos(a1), canopyY0, innerR * std::sin(a1));
            glVertex3f(innerR * std::cos(a0), canopyY0, innerR * std::sin(a0));
        }
        glEnd();

        // 2. Vành diềm tán mái & chóp cầu đỉnh mái
        glPushMatrix();
        glTranslatef(0.0f, canopyY0, 0.0f);
        glColor3f(primaryCol.r, primaryCol.g, primaryCol.b);
        ParametricShapes::drawCylinderY(innerR, 0.35f, 20, 1, true, false);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, canopyY1 + 0.25f, 0.0f);
        glColor3f(0.96f, 0.82f, 0.22f);
        ParametricShapes::drawSphere(0.45f, 10, 8, false);
        glPopMatrix();

        // 3. Dàn ghế treo / cabin xoay văng nhẹ ra ngoài theo lực ly tâm
        const float seatOrbitR = innerR * 0.82f;
        const float cableLen   = colH * 0.56f;
        for (int s = 0; s < numSeats; ++s) {
            float a = (static_cast<float>(s) / numSeats) * MathCG::TWO_PI;
            float sx = seatOrbitR * std::cos(a);
            float sz = seatOrbitR * std::sin(a);
            float yawDeg = -a * MathCG::RAD2DEG;

            glPushMatrix();
            glTranslatef(sx, canopyY0, sz);
            glRotatef(yawDeg, 0.0f, 1.0f, 0.0f);
            glRotatef(14.0f, 0.0f, 0.0f, 1.0f);

            glColor3f(0.75f, 0.76f, 0.78f);
            glPushMatrix();
            glTranslatef(0.0f, -cableLen, 0.0f);
            ParametricShapes::drawCylinderY(0.04f, cableLen, 6, 1, false, false);

            const MathCG::ColorRGB& sc = (s % 2 == 0) ? primaryCol : secondaryCol;
            glColor3f(sc.r, sc.g, sc.b);
            ParametricShapes::drawCylinderY(0.36f, 0.32f, 8, 1, true, false);
            glPopMatrix();

            glPopMatrix();
        }
    }

    /**
     * @brief MỘT HÀM VẼ ĐU QUAY TRÒN THAM SỐ HÓA DÙNG CHUNG (ĐỢT 5B):
     *        Tham số: vị trí (pos), bán kính (radius), độ cao (height), màu chính/phụ (primaryCol, secondaryCol),
     *        số ghế treo (numSeats), góc quay hoạt ảnh (curSpinDeg), và biến thể hành lang (hasOuterColonnade).
     *        Dùng chung cho #9, #10, #13 (biến thể có hành lang cột), #14.
     */
    void drawSpinningRoundRide(
        int listIndex,
        const MathCG::Vec3& pos,
        float radius,
        float height,
        const MathCG::ColorRGB& primaryCol,
        const MathCG::ColorRGB& secondaryCol,
        int numSeats,
        float curSpinDeg,
        bool hasOuterColonnade
    ) const {
        glPushMatrix();
        glTranslatef(pos.x, pos.y + 0.56f, pos.z);
        glRotatef(curSpinDeg, 0.0f, 1.0f, 0.0f);
        if (listIndex >= 0 && listIndex < 4 && roundRideSpinLists[listIndex] != 0) {
            glCallList(roundRideSpinLists[listIndex]);
        } else {
            compileSpinningCanopyAndSeats(radius, height, primaryCol, secondaryCol, numSeats, hasOuterColonnade);
        }
        glPopMatrix();
    }

    void drawAllSpinningRoundRides(bool isNightOrSunset) const {
        (void)isNightOrSunset;
        ensureDisplayLists();
        // #9: Đu quay tròn 1 mái cam-trắng (-132.7, 0, +12.9), ĐK 17m (R=8.5m), cao 8.2m, 16 ghế
        drawSpinningRoundRide(
            0, ParkLayout::ROUND_SWING_RIDE_1_POS,
            8.5f, 8.2f,
            MathCG::ColorRGB(0.95f, 0.45f, 0.12f),
            MathCG::ColorRGB(0.96f, 0.95f, 0.92f),
            16, spinAngleDeg, false
        );
        // #10: Đu quay tròn 2 đĩa tròn nhỏ (-94.2, 0, +17.3), ĐK 13m (R=6.5m), cao 6.4m, 12 ghế
        drawSpinningRoundRide(
            1, ParkLayout::ROUND_SPIN_RIDE_2_POS,
            6.5f, 6.4f,
            MathCG::ColorRGB(0.18f, 0.56f, 0.92f),
            MathCG::ColorRGB(0.98f, 0.84f, 0.22f),
            12, -spinAngleDeg * 1.25f, false
        );
        // #13: Công trình tròn có hành lang khu giữa (-141.3, 0, -137.6), ĐK 18m (R=9.0m), cao 8.8m, 16 ghế
        drawSpinningRoundRide(
            2, ParkLayout::CENTRAL_ROTUNDA_RIDE_POS,
            9.0f, 8.8f,
            MathCG::ColorRGB(0.84f, 0.22f, 0.16f),
            MathCG::ColorRGB(0.96f, 0.78f, 0.20f),
            16, spinAngleDeg * 0.80f, true
        );
        // #14: Đu quay tròn 3 khu giữa (-143.4, 0, -179.6), ĐK 15m (R=7.5m), cao 7.4m, 14 ghế
        drawSpinningRoundRide(
            3, ParkLayout::ROUND_SPIN_RIDE_3_POS,
            7.5f, 7.4f,
            MathCG::ColorRGB(0.14f, 0.68f, 0.72f),
            MathCG::ColorRGB(0.98f, 0.86f, 0.26f),
            14, -spinAngleDeg * 1.10f, false
        );
    }
};

} // namespace AsiaParkRides

#endif // PARK_ARCHITECTURE_H
