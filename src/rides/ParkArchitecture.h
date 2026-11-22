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
        drawTiledCentralPlaza();

        // 2. Các khối kiến trúc lưới đa giác tĩnh Đợt 4 & Đợt 5B (dùng Display List khi không bật debug normals)
        if (!showNormals) {
            ensureDisplayLists();
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

private:
    mutable Geometry::PolygonMesh staticArchMesh;
    mutable GLuint archSmoothList = 0;
    mutable GLuint archFlatList   = 0;
    mutable GLuint roundRideSpinLists[4] = { 0, 0, 0, 0 };

    void ensureDisplayLists() const {
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
     * @brief MỘT HÀM VẼ NHÀ MÁI NGÓI THAM SỐ HÓA DÙNG CHUNG (ĐỢT 5B):
     *        Tham số: vị trí, góc xoay, rộng (w), sâu (d), cao thân (wallH), màu mái (roofCol), kiểu mái cong (roofStyle).
     *        Dùng chung cho 4 nhà mái xanh/teal (#7, #8, #15, #18) và nhà mái đỏ ven hồ (#12).
     *        100% liền khối: mái đặt trực tiếp lên xà đỡ, cột và thân tường, có bậc thềm và sảnh hiên.
     *        - roofStyle = 0: Mái ngói cong 2 tầng truyền thống có cổ lầu và hiên sảnh (#7, #8, #12, #18)
     *        - roofStyle = 1: Mái ngói cong 2 tầng vòm rộng viền diềm trắng kiểu nhà biểu diễn (#15)
     */
    static void appendTileRoofedHall(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos,
        float w, float d, float wallH,
        float rotYDeg,
        const MathCG::ColorRGB& roofCol,
        int roofStyle
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

        // 1. Bệ đá nền & bậc tam cấp mặt tiền (+Z) và mặt hậu (-Z) (y = 0.0 .. 0.65m)
        appendBoxByYRange(local, 0.0f, 0.0f, 0.35f, 0.0f, w + 1.8f, d + 1.8f, plinthCol);
        appendBoxByYRange(local, 0.0f, 0.30f, 0.65f, 0.0f, w + 0.8f, d + 0.8f, stepCol);
        appendBoxByYRange(local, 0.0f, 0.0f, 0.32f,  d * 0.5f + 1.2f, 5.4f, 1.4f, stepCol);
        appendBoxByYRange(local, 0.0f, 0.0f, 0.32f, -d * 0.5f - 1.2f, 5.4f, 1.4f, stepCol);

        // 2. Thân nhà đặc liền khối từ bệ đá lên đỉnh tường (y = 0.60 .. 0.65 + wallH)
        const float yWallTop = 0.65f + wallH;
        appendBoxByYRange(local, 0.0f, 0.60f, yWallTop, 0.0f, w, d, wallCol);

        // Hệ cột trụ nổi dọc 4 mặt tường tạo nhịp kiến trúc Á Đông
        const int numColsX = 5;
        for (int i = 0; i < numColsX; ++i) {
            float tx = -w * 0.46f + (w * 0.92f) * (static_cast<float>(i) / (numColsX - 1));
            for (int sz : {-1, 1}) {
                appendBoxByYRange(local, tx, 0.60f, yWallTop + 0.05f, sz * (d * 0.5f + 0.08f), 0.48f, 0.32f, colColor);
            }
        }
        for (int sx : {-1, 1}) {
            for (int iz = -1; iz <= 1; ++iz) {
                appendBoxByYRange(local, sx * (w * 0.5f + 0.08f), 0.60f, yWallTop + 0.05f, iz * d * 0.32f, 0.32f, 0.48f, colColor);
            }
        }

        // Ô cửa chính ở mặt trước (+Z) và mặt sau (-Z)
        for (int sz : {-1, 1}) {
            appendBoxByYRange(local, 0.0f, 0.65f, 0.65f + wallH * 0.72f, sz * (d * 0.5f + 0.04f), 3.6f, 0.18f, doorCol);
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
     * @brief Dựng Quảng trường Ngôi sao & 4 tháp chòi nhọn Angkor (#19 - ANGKOR_STAR_PLAZA_POS)
     */
    static void appendAngkorStarPlazaTowers(
        Geometry::PolygonMesh& target,
        const MathCG::Vec3& centerPos
    ) {
        const float cx = centerPos.x;
        const float cz = centerPos.z;
        const MathCG::ColorRGB stoneBase(0.55f, 0.49f, 0.41f);
        const MathCG::ColorRGB sandstone(0.68f, 0.58f, 0.44f);
        const MathCG::ColorRGB darkStone(0.42f, 0.36f, 0.30f);
        const MathCG::ColorRGB goldTip(0.92f, 0.74f, 0.22f);

        // Hàm nội bộ dựng 1 tháp chòi nhọn Angkor (Prang Khmer) nhiều tầng thuôn nhọn liền khối
        auto buildAngkorTower = [&](float tx, float tz, float baseS, float totalScale) {
            float y0 = 0.0f;
            // Bệ tháp đá
            appendBoxByYRange(target, tx, y0, y0 + 0.65f * totalScale, tz, baseS + 1.0f, baseS + 1.0f, darkStone);
            // Thân chòi vuông + 4 trụ góc + cửa thông 4 hướng (y = 0.60 .. 3.80m)
            float h1 = 3.80f * totalScale;
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    appendBoxByYRange(
                        target,
                        tx + sx * (baseS * 0.36f),
                        0.60f * totalScale, h1,
                        tz + sz * (baseS * 0.36f),
                        baseS * 0.28f, baseS * 0.28f, sandstone
                    );
                }
            }
            // Lõi đá giữa tháp
            appendBoxByYRange(target, tx, 0.60f * totalScale, h1, tz, baseS * 0.52f, baseS * 0.52f, darkStone);
            // 3 tầng cổ tháp thu nhỏ dần kiểu kiến trúc Angkor
            float curY = h1 - 0.05f;
            float curS = baseS + 0.4f;
            for (int tier = 0; tier < 3; ++tier) {
                float th = (1.05f - tier * 0.15f) * totalScale;
                appendBoxByYRange(target, tx, curY, curY + th, tz, curS, curS, (tier % 2 == 0) ? sandstone : stoneBase);
                curY += th - 0.04f;
                curS *= 0.72f;
            }
            // Chóp tháp bút sen nhọn trên đỉnh (createPyramid)
            target.append(Geometry::PolygonMesh::createPyramid(
                tx, curY, tz,
                curS * 1.05f, curS * 1.05f, 3.2f * totalScale, sandstone
            ));
            target.append(Geometry::PolygonMesh::createPyramid(
                tx, curY + 3.05f * totalScale, tz,
                0.55f, 0.55f, 1.15f * totalScale, goldTip
            ));
        };

        // 4 tháp chòi nhọn Angkor ở 4 góc quảng trường ngôi sao (±7.8m, ±7.8m)
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                buildAngkorTower(cx + sx * 7.8f, cz + sz * 7.8f, 4.4f, 1.0f);
            }
        }
        // Đài tháp tâm ngôi sao ở chính giữa quảng trường
        buildAngkorTower(cx, cz, 3.4f, 0.82f);
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
        {
            const float ax = ParkLayout::ANGKOR_STAR_PLAZA_POS.x;
            const float az = ParkLayout::ANGKOR_STAR_PLAZA_POS.z;
            const float ay = 0.044f;
            const float hs = 11.8f;
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
                float r = (i % 2 == 0) ? 11.0f : 4.5f;
                glVertex3f(ax + r * std::cos(a), ay + 0.004f, az - r * std::sin(a));
            }
            glEnd();
        }
    }

    /**
     * @brief Vẽ HAI MÀN HÌNH TRÒN phát sáng lớn và màn hình sân khấu dưới chân Sun Wheel
     */
    void drawStageCircularScreensAndLights(bool isNightOrSunset) const {
        const float stZ     = ParkLayout::SUN_WHEEL_STAGE_POS.z; // -17.7m
        const float frontZ  = stZ + ParkLayout::SUN_WHEEL_STAGE_DEPTH * 0.5f + 0.08f; // -13.12m
        const float screenR = ParkLayout::SUN_WHEEL_SCREEN_RADIUS; // 3.1m

        // 1. Hai Màn hình Tròn lớn ở hai khối nhà cánh (cx = ±19.75m, cy = 4.65m)
        for (int side : {-1, 1}) {
            float sx = side * 19.75f;
            float sy = 4.65f;

            glPushMatrix();
            glTranslatef(sx, sy, frontZ);

            // Vành khuyên viền mạ vàng quanh màn hình tròn
            glColor3f(0.92f, 0.74f, 0.20f);
            ParametricShapes::drawToroid(screenR + 0.12f, 0.22f, 28, 8, false);

            // Mặt đĩa màn hình tròn phát sáng rực rỡ vào ban đêm (giống hệt 3 ảnh mẫu)
            if (isNightOrSunset) {
                GLfloat emit[4] = { 1.0f, 0.90f, 0.78f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(1.0f, 0.94f, 0.85f);
            } else {
                glColor3f(0.85f, 0.90f, 0.96f);
            }
            ParametricShapes::drawDisk(screenR, 28, 0.0f, false);

            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }
            glPopMatrix();
        }

        // 2. Màn hình LED chính giữa Sân khấu Sun Wheel (X thuộc [-9.5, +9.5], Y thuộc [1.6, 7.2], Z = -14.0m)
        if (isNightOrSunset) {
            GLfloat stageEmit[4] = { 0.55f, 0.78f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, stageEmit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, stageEmit);
            glColor3f(0.65f, 0.85f, 1.0f);
        } else {
            glColor3f(0.20f, 0.35f, 0.55f);
        }
        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-9.5f, 1.55f, stZ + 3.72f);
        glVertex3f( 9.5f, 1.55f, stZ + 3.72f);
        glVertex3f( 9.5f, 7.25f, stZ + 3.72f);
        glVertex3f(-9.5f, 7.25f, stZ + 3.72f);
        glEnd();

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
