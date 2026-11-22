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
    ParkArchitecture() {
        buildStaticMeshes();
    }

    /**
     * @brief Vẽ toàn bộ Quảng trường lát gạch, Sân khấu chân Sun Wheel và các Chòi mái ngói Đợt 4
     */
    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        // 1. Mặt phẳng Quảng trường Trung tâm lát gạch (nâng y = 0.028m tránh Z-fighting)
        drawTiledCentralPlaza();

        // 2. Các khối kiến trúc lưới đa giác tĩnh (dùng Display List khi không bật debug normals)
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
};

} // namespace AsiaParkRides

#endif // PARK_ARCHITECTURE_H
