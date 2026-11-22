#ifndef NGO_MON_GATE_H
#define NGO_MON_GATE_H

/**
 * @file NgoMonGate.h
 * @brief Chương 4 & 6 (Đợt 4): Mô phỏng Kiến trúc Cổng Chính Công viên Châu Á (Asia Park):
 *        Cổng Ngọ Môn 5 lầu nhiều tầng mái cong, Bậc thềm, Hai Cánh Tường Thành có răng cưa & cổng vòm phụ,
 *        Hai Hồ Sen trước cổng, Sân trước lát đá, và Tháp Đồng Hồ Trung Tâm tách riêng (mốc 2019).
 *
 * Nguyên tắc hình học Đợt 4:
 *  - Mọi bộ phận đều LIỀN KHỐI 100%: mỗi tầng trên có chân đế/cột/tường cắm khít vào mặt đỉnh của tầng dưới,
 *    mái cong 4 dốc (createCurvedAsianRoof) có cổ diềm dưới ôm khít xà đỡ và tường, không hở, không lơ lửng.
 *  - Sử dụng OpenGL Display List cho các lưới đa giác tĩnh để duy trì FPS cao.
 *  - Toàn bộ hằng số vị trí, kích thước và tỷ lệ đọc từ ParkLayout.h.
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
    MathCG::Vec3 position        = ParkLayout::NGO_MON_GATE_POS;    // (+16.1, 0.0, +139.1)
    float        gateScale       = ParkLayout::NGO_MON_GATE_SCALE;  // 0.76f
    MathCG::Vec3 clockTowerPos   = ParkLayout::CLOCK_TOWER_POS;     // (+10.7, 0.0, +61.6)
    float        clockTowerScale = ParkLayout::CLOCK_TOWER_SCALE;   // 1.00f

    float        width           = 46.0f;
    float        depth           = 15.0f;
    float        bastionHeight   =  6.8f;
    float        towerHeight     = 27.5f;

    NgoMonGate() {
        buildGateAndClockMeshes();
    }

    /**
     * @brief Vẽ toàn bộ tổ hợp Cổng Ngọ Môn + 2 Cánh Tường Thành + Hồ Sen + Sân Trước & Tháp Đồng Hồ tách riêng
     */
    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        // 1. Vẽ Hai Hồ Sen trước cổng & Sân trước lát đá (trong hệ tọa độ thế giới 1:1)
        drawForecourtAndLotusPonds(isNightOrSunset);

        // 2. Vẽ Cổng Thành Ngọ Môn + Hai Cánh Tường Thành Răng Cưa tại (+16.1, 0.0, +139.1), scale = 0.76
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(gateScale, gateScale, gateScale);

        if (!showNormals) {
            ensureDisplayLists();
            if (smoothShading && gateSmoothList != 0) {
                glCallList(gateSmoothList);
            } else if (!smoothShading && gateFlatList != 0) {
                glCallList(gateFlatList);
            } else {
                gateMesh.draw(smoothShading, false);
            }
        } else {
            gateMesh.draw(smoothShading, true);
        }

        // Vẽ các ô cửa sổ tròn phát sáng trên mặt tiền tầng 2 và dàn đèn lồng đỏ cung đình dưới hiên mái
        drawTier2RoundWindows(isNightOrSunset);
        drawLanterns(isNightOrSunset, showNormals);

        glPopMatrix();

        // 3. Vẽ Tháp Đồng Hồ Trung Tâm tách riêng tại (+10.7, 0.0, +61.6), scale = 1.0
        glPushMatrix();
        glTranslatef(clockTowerPos.x, clockTowerPos.y, clockTowerPos.z);
        glScalef(clockTowerScale, clockTowerScale, clockTowerScale);

        if (!showNormals) {
            ensureDisplayLists();
            if (smoothShading && clockSmoothList != 0) {
                glCallList(clockSmoothList);
            } else if (!smoothShading && clockFlatList != 0) {
                glCallList(clockFlatList);
            } else {
                clockTowerMesh.draw(smoothShading, false);
            }
        } else {
            clockTowerMesh.draw(smoothShading, true);
        }

        drawClockFaces(isNightOrSunset);

        glPopMatrix();
    }

private:
    mutable Geometry::PolygonMesh gateMesh;
    mutable Geometry::PolygonMesh clockTowerMesh;
    mutable GLuint gateSmoothList  = 0;
    mutable GLuint gateFlatList    = 0;
    mutable GLuint clockSmoothList = 0;
    mutable GLuint clockFlatList   = 0;

    void ensureDisplayLists() const {
        if (gateSmoothList == 0) {
            gateSmoothList = glGenLists(1);
            if (gateSmoothList != 0) {
                glNewList(gateSmoothList, GL_COMPILE);
                gateMesh.draw(true, false);
                glEndList();
            }
        }
        if (gateFlatList == 0) {
            gateFlatList = glGenLists(1);
            if (gateFlatList != 0) {
                glNewList(gateFlatList, GL_COMPILE);
                gateMesh.draw(false, false);
                glEndList();
            }
        }
        if (clockSmoothList == 0) {
            clockSmoothList = glGenLists(1);
            if (clockSmoothList != 0) {
                glNewList(clockSmoothList, GL_COMPILE);
                clockTowerMesh.draw(true, false);
                glEndList();
            }
        }
        if (clockFlatList == 0) {
            clockFlatList = glGenLists(1);
            if (clockFlatList != 0) {
                glNewList(clockFlatList, GL_COMPILE);
                clockTowerMesh.draw(false, false);
                glEndList();
            }
        }
    }

    /**
     * @brief Hàm tiện ích thêm một hình hộp theo biên độ cao [yBottom, yTop] để đảm bảo
     *        các tầng kiến trúc luôn tiếp xúc/chồng khít liền khối 100%, không bao giờ lơ lửng.
     */
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
     * @brief Khởi tạo toàn bộ lưới đa giác (PolygonMesh) của Cổng Ngọ Môn + 2 Cánh Tường Thành & Tháp Đồng Hồ
     */
    void buildGateAndClockMeshes() {
        gateMesh.clear();
        gateMesh.name = "NgoMonImperialGateAndWingWalls";

        MathCG::ColorRGB plinthCol(0.46f, 0.45f, 0.44f);      // Đá tảng chân móng
        MathCG::ColorRGB stoneCol(0.58f, 0.56f, 0.54f);       // Đá xám xây thành tầng 1 & cánh tường
        MathCG::ColorRGB lightStoneCol(0.84f, 0.82f, 0.76f);  // Phào chỉ đá sáng & bổ trụ tường thành
        MathCG::ColorRGB warmWallCol(0.76f, 0.72f, 0.52f);    // Tường đá vàng-xanh rêu tầng 2 (theo ảnh đêm)
        MathCG::ColorRGB redPillarCol(0.76f, 0.16f, 0.14f);   // Cột & tường gỗ sơn son đỏ cung đình
        MathCG::ColorRGB darkRoofCol(0.46f, 0.25f, 0.14f);    // Ngói nâu sẫm cổ kính
        MathCG::ColorRGB goldRoofCol(0.88f, 0.58f, 0.14f);    // Ngói hoàng lưu ly vàng-nâu ấm
        MathCG::ColorRGB darkTrimCol(0.24f, 0.20f, 0.18f);    // Hệ dầm xà đỡ mái
        MathCG::ColorRGB balustradeCol(0.88f, 0.88f, 0.90f);  // Lan can đá trắng

        const float hZ      = depth * 0.5f; // 7.5m
        const float centerW = 24.0f;
        const float wingW   = (width - centerW) * 0.5f; // 11.0m mỗi bên thân cổng chính

        // ====================================================================
        // 1. BỆ MÓNG ĐÁ & BẬC THỀM TAM CẤP PHÍA NAM (+Z)
        // ====================================================================
        // Bệ móng dưới hai cánh thân cổng (để trống lối đi thông suốt qua 3 vòm cổng giữa)
        appendBoxByYRange(gateMesh, -16.25f, 0.0f, 0.45f, 0.0f, 15.5f, depth + 1.6f, plinthCol);
        appendBoxByYRange(gateMesh,  16.25f, 0.0f, 0.45f, 0.0f, 15.5f, depth + 1.6f, plinthCol);
        // Bệ đá dưới 4 trụ cổng vòm
        appendBoxByYRange(gateMesh, -9.25f, 0.0f, 0.45f, 0.0f, 2.7f, depth + 1.2f, plinthCol);
        appendBoxByYRange(gateMesh, -3.00f, 0.0f, 0.45f, 0.0f, 2.2f, depth + 1.2f, plinthCol);
        appendBoxByYRange(gateMesh,  3.00f, 0.0f, 0.45f, 0.0f, 2.2f, depth + 1.2f, plinthCol);
        appendBoxByYRange(gateMesh,  9.25f, 0.0f, 0.45f, 0.0f, 2.7f, depth + 1.2f, plinthCol);

        // Thềm dẫn & bậc tam cấp đá cùng hai thành lan can rồng dẫn vào mặt tiền phía Nam (+Z)
        appendBoxByYRange(gateMesh, 0.0f, 0.0f, 0.16f, hZ + 3.2f, 22.0f, 6.4f, lightStoneCol);
        for (int s : {-1, 1}) {
            // Thành bậc thềm hai bên lối vào chính
            appendBoxByYRange(gateMesh, s * 10.6f, 0.0f, 0.95f, hZ + 3.0f, 0.75f, 6.0f, balustradeCol);
            appendBoxByYRange(gateMesh, s *  3.1f, 0.0f, 0.85f, hZ + 2.8f, 0.55f, 5.2f, balustradeCol);
        }

        // ====================================================================
        // 2. THÂN ĐÀI THÀNH TẦNG 1, 3 CỔNG VÒM CUỐN & MÁI HIÊN TIỀN SẢNH (y = 0.0 .. 6.80m)
        // ====================================================================
        // 4 trụ đá đỡ 3 vòm cổng (y = 0.40 .. 5.25m, chồng 0.05m vào bệ dưới và dầm trên)
        appendBoxByYRange(gateMesh, -9.25f, 0.40f, 5.25f, 0.0f, 2.5f, depth, stoneCol);
        appendBoxByYRange(gateMesh, -3.00f, 0.40f, 5.25f, 0.0f, 2.0f, depth, stoneCol);
        appendBoxByYRange(gateMesh,  3.00f, 0.40f, 5.25f, 0.0f, 2.0f, depth, stoneCol);
        appendBoxByYRange(gateMesh,  9.25f, 0.40f, 5.25f, 0.0f, 2.5f, depth, stoneCol);

        // Khối tường đặc phía trên 3 vòm cổng (y = 5.15 .. 6.80m)
        appendBoxByYRange(gateMesh, 0.0f, 5.15f, bastionHeight, 0.0f, centerW, depth, stoneCol);

        // 3 vòm cuốn xuyên suốt từ -hZ đến +hZ, kèm tường lấp góc spandrel giữa lưng vòm và dầm trên
        auto centralVault = Geometry::PolygonMesh::createArchVault(
            0.0f, 2.80f, -hZ, hZ, 1.98f, 2.55f, 14, lightStoneCol
        );
        auto leftVault = Geometry::PolygonMesh::createArchVault(
            -6.0f, 2.40f, -hZ, hZ, 1.40f, 2.82f, 12, lightStoneCol
        );
        auto rightVault = Geometry::PolygonMesh::createArchVault(
             6.0f, 2.40f, -hZ, hZ, 1.40f, 2.82f, 12, lightStoneCol
        );
        gateMesh.append(centralVault);
        gateMesh.append(leftVault);
        gateMesh.append(rightVault);

        // Khối lấp tường phía trên hai vòm phụ (y = 3.80 .. 5.20m) để mặt tường không bị hở lỗ phía trên vòm phụ
        appendBoxByYRange(gateMesh, -6.0f, 3.80f, 5.25f, 0.0f, 4.0f, depth - 0.1f, stoneCol);
        appendBoxByYRange(gateMesh,  6.0f, 3.80f, 5.25f, 0.0f, 4.0f, depth - 0.1f, stoneCol);
        appendBoxByYRange(gateMesh,  0.0f, 4.30f, 5.25f, 0.0f, 4.0f, depth - 0.1f, stoneCol);

        // Hai khối cánh thân cổng chính (x = -23..-10.5 và +10.5..+23.0, y = 0.40 .. 6.80m)
        for (int side : {-1, 1}) {
            float wingCX = side * (centerW * 0.5f + wingW * 0.5f); // ±17.5m
            appendBoxByYRange(gateMesh, wingCX, 0.40f, bastionHeight, 0.0f, wingW, depth, stoneCol);

            // Viền khung phào chỉ vàng kem và 2 trụ bổ trụ mặt tiền tầng 1
            appendBoxByYRange(gateMesh, side * 12.2f, 0.40f, bastionHeight, hZ + 0.18f, 1.2f, 0.40f, lightStoneCol);
            appendBoxByYRange(gateMesh, side * 21.8f, 0.40f, bastionHeight, hZ + 0.18f, 1.4f, 0.40f, lightStoneCol);
            // Ô cửa vòm giả trang trí trên mặt tiền hai cánh thân cổng (giống ảnh đêm 2)
            appendBoxByYRange(gateMesh, wingCX, 0.45f, 4.40f, hZ + 0.12f, 3.4f, 0.25f, darkTrimCol);
            appendBoxByYRange(gateMesh, wingCX, 4.35f, 4.75f, hZ + 0.16f, 4.0f, 0.32f, lightStoneCol);
        }

        // Mái hiên Tiền sảnh (Cổ lâu đón khách) nhô ra phía trước cửa vòm chính (z = hZ .. hZ + 3.2m):
        // 4 cột trụ đỡ trực tiếp dầm hiên tại y = 4.10m và mái ngói cong tại y = 4.10 .. 5.85m
        const float porchZ = hZ + 1.65f;
        const float porchColX[4] = { -4.4f, -2.3f, 2.3f, 4.4f };
        for (float px : porchColX) {
            appendBoxByYRange(gateMesh, px, 0.15f, 0.65f, hZ + 2.85f, 0.68f, 0.68f, plinthCol);
            gateMesh.append(Geometry::PolygonMesh::createPrismY(px, 0.60f, hZ + 2.85f, 0.25f, 3.55f, 12, lightStoneCol));
        }
        // Dầm xà ngang nối 4 cột tiền sảnh thẳng vào mặt tường chính (y = 4.05 .. 4.45m)
        appendBoxByYRange(gateMesh, 0.0f, 4.05f, 4.45f, porchZ, 10.2f, 3.4f, darkTrimCol);
        // Mái ngói cong tiền sảnh đặt trực tiếp trên dầm xà tại y = 4.42m
        auto porchRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 4.42f, porchZ,
            10.4f, 3.5f, 1.55f, 1.4f,
            darkRoofCol
        );
        gateMesh.append(porchRoof);

        // Hai Lầu Vọng Cảnh Góc ở hai đầu mặt tiền tầng 1 (cx = ±19.2m, cz = hZ - 1.2m):
        // Mọc trực tiếp từ mặt sàn đài thành y = 6.80m, có 4 cột đỡ mái cong tại y = 9.60m
        for (int side : {-1, 1}) {
            float tcX = side * 19.2f;
            float tcZ = hZ - 1.2f;
            appendBoxByYRange(gateMesh, tcX, bastionHeight - 0.10f, bastionHeight + 0.45f, tcZ, 5.2f, 4.8f, lightStoneCol);
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    gateMesh.append(Geometry::PolygonMesh::createPrismY(
                        tcX + sx * 1.95f, bastionHeight + 0.40f, tcZ + sz * 1.75f,
                        0.22f, 2.65f, 10, redPillarCol
                    ));
                }
            }
            // Lõi tường lửng và xà đỡ mái lầu góc
            appendBoxByYRange(gateMesh, tcX, bastionHeight + 0.40f, bastionHeight + 1.35f, tcZ, 4.2f, 3.8f, warmWallCol);
            appendBoxByYRange(gateMesh, tcX, bastionHeight + 2.95f, bastionHeight + 3.30f, tcZ, 4.6f, 4.2f, darkTrimCol);
            auto cornerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
                tcX, bastionHeight + 3.28f, tcZ,
                4.8f, 4.4f, 1.65f, 1.3f,
                goldRoofCol
            );
            gateMesh.append(cornerRoof);
        }

        // ====================================================================
        // 3. SÀN SÂN THƯỢNG, LAN CAN ĐÁ & KHỐI THÀNH TẦNG 2 (y = 6.80 .. 12.60m)
        // ====================================================================
        // Mặt sàn sân thượng trên đài thành tầng 1 (y = 6.75 .. 7.10m)
        appendBoxByYRange(gateMesh, 0.0f, bastionHeight - 0.05f, bastionHeight + 0.30f, 0.0f, width + 0.8f, depth + 0.8f, lightStoneCol);

        // Tường lan can đá trắng bao quanh chu vi đài thành tầng 1 (y = 7.05 .. 7.90m)
        appendBoxByYRange(gateMesh, 0.0f, bastionHeight + 0.25f, bastionHeight + 1.10f,  hZ + 0.22f, width + 0.5f, 0.32f, balustradeCol);
        appendBoxByYRange(gateMesh, 0.0f, bastionHeight + 0.25f, bastionHeight + 1.10f, -hZ - 0.22f, width + 0.5f, 0.32f, balustradeCol);
        appendBoxByYRange(gateMesh, -width * 0.5f - 0.22f, bastionHeight + 0.25f, bastionHeight + 1.10f, 0.0f, 0.32f, depth + 0.5f, balustradeCol);
        appendBoxByYRange(gateMesh,  width * 0.5f + 0.22f, bastionHeight + 0.25f, bastionHeight + 1.10f, 0.0f, 0.32f, depth + 0.5f, balustradeCol);

        // Khối thân thành Tầng 2 đặt trực tiếp trên sàn tầng 1 (y = 7.00 .. 12.60m)
        const float tier2Y0 = bastionHeight + 0.20f; // 7.00m
        const float tier2Y1 = 12.60f;
        const float tier2W  = 31.6f;
        const float tier2D  = 11.4f;
        const float tier2HZ = tier2D * 0.5f; // 5.70m

        appendBoxByYRange(gateMesh, 0.0f, tier2Y0, tier2Y1, 0.0f, tier2W, tier2D, warmWallCol);

        // Các trụ bổ trụ đứng màu kem sáng chia mặt tiền tầng 2 thành 3 khoang (giống ảnh đêm 2)
        const float t2PilasterX[] = { -15.6f, -6.2f, 6.2f, 15.6f };
        for (float px : t2PilasterX) {
            appendBoxByYRange(gateMesh, px, tier2Y0, tier2Y1 + 0.20f,  tier2HZ + 0.16f, 0.95f, 0.36f, lightStoneCol);
            appendBoxByYRange(gateMesh, px, tier2Y0, tier2Y1 + 0.20f, -tier2HZ - 0.16f, 0.95f, 0.36f, lightStoneCol);
        }
        // Dầm phân tầng ngang giữa tầng 2 (y = 9.75 .. 10.05m)
        appendBoxByYRange(gateMesh, 0.0f, 9.75f, 10.05f, 0.0f, tier2W + 0.5f, tier2D + 0.5f, lightStoneCol);

        // Cầu thang đôi hình chữ A đối xứng ở chính giữa mặt tiền tầng 2 dẫn lên Lầu Vọng Cảnh Giữa
        for (int side : {-1, 1}) {
            for (int step = 0; step < 6; ++step) {
                float frac = static_cast<float>(step) / 5.0f;
                float sx = side * (5.2f - frac * 2.4f);
                float syTop = tier2Y0 + 0.45f + frac * 2.35f;
                appendBoxByYRange(gateMesh, sx, tier2Y0, syTop, tier2HZ + 0.55f, 0.85f, 1.15f, lightStoneCol);
            }
        }
        // Sàn & Lầu Vọng Cảnh Giữa mặt tiền tầng 2 (y = 9.40 .. 12.55m, nhô ra tại z = tier2HZ + 0.85m)
        const float midPavZ = tier2HZ + 0.90f;
        appendBoxByYRange(gateMesh, 0.0f, 9.35f, 9.80f, midPavZ, 6.4f, 2.2f, lightStoneCol);
        // Khung cửa đỏ rực bên trong lầu giữa tầng 2
        appendBoxByYRange(gateMesh, 0.0f, 9.80f, 11.75f, tier2HZ + 0.15f, 4.2f, 0.45f, redPillarCol);
        for (int sx : {-1, 1}) {
            gateMesh.append(Geometry::PolygonMesh::createPrismY(
                sx * 2.55f, 9.75f, midPavZ + 0.75f, 0.18f, 2.05f, 10, redPillarCol
            ));
        }
        appendBoxByYRange(gateMesh, 0.0f, 11.70f, 12.02f, midPavZ, 5.8f, 2.1f, darkTrimCol);
        auto midBalconyRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 12.00f, midPavZ,
            6.0f, 2.3f, 1.35f, 1.1f,
            darkRoofCol
        );
        gateMesh.append(midBalconyRoof);

        // Gờ sàn thượng đỉnh tầng 2 & lan can đá trắng bao quanh Lầu Ngũ Phụng (y = 12.55 .. 13.35m)
        appendBoxByYRange(gateMesh, 0.0f, tier2Y1 - 0.05f, tier2Y1 + 0.30f, 0.0f, tier2W + 0.8f, tier2D + 0.8f, lightStoneCol);
        appendBoxByYRange(gateMesh, 0.0f, tier2Y1 + 0.25f, tier2Y1 + 0.95f,  tier2HZ + 0.22f, tier2W + 0.5f, 0.28f, balustradeCol);
        appendBoxByYRange(gateMesh, 0.0f, tier2Y1 + 0.25f, tier2Y1 + 0.95f, -tier2HZ - 0.22f, tier2W + 0.5f, 0.28f, balustradeCol);
        appendBoxByYRange(gateMesh, -tier2W * 0.5f - 0.22f, tier2Y1 + 0.25f, tier2Y1 + 0.95f, 0.0f, 0.28f, tier2D + 0.5f, balustradeCol);
        appendBoxByYRange(gateMesh,  tier2W * 0.5f + 0.22f, tier2Y1 + 0.25f, tier2Y1 + 0.95f, 0.0f, 0.28f, tier2D + 0.5f, balustradeCol);

        // ====================================================================
        // 4. LẦU NGŨ PHỤNG (HAI TẦNG MÁI NGÓI CONG CHÍNH + HAI LẦU PHỤ BÊN, y = 12.80 .. 22.80m)
        //    Đặt trực tiếp trên sàn đỉnh tầng 2 (y = 12.80m), hoàn toàn liền khối!
        // ====================================================================
        const float pavY0 = tier2Y1 + 0.20f; // 12.80m
        const float pavW  = 19.6f;
        const float pavD  =  8.2f;

        // Lõi phòng gỗ sơn son đỏ tầng dưới Lầu Ngũ Phụng (y = 12.80 .. 16.35m)
        appendBoxByYRange(gateMesh, 0.0f, pavY0, pavY0 + 3.55f, 0.0f, pavW - 1.6f, pavD - 1.4f, redPillarCol);

        // Hàng cột gỗ sơn son chạy quanh hiên tầng dưới (y = 12.80 .. 16.30m)
        const int colCols = 6;
        for (int c = 0; c < colCols; ++c) {
            float frac = static_cast<float>(c) / (colCols - 1);
            float px = -pavW * 0.48f + frac * pavW * 0.96f;
            for (int s : {-1, 1}) {
                gateMesh.append(Geometry::PolygonMesh::createPrismY(
                    px, pavY0, s * (pavD * 0.46f), 0.24f, 3.50f, 10, redPillarCol
                ));
            }
        }

        // Hệ dầm xà đỡ mái tầng dưới (y = 16.20 .. 16.58m)
        appendBoxByYRange(gateMesh, 0.0f, pavY0 + 3.40f, pavY0 + 3.78f, 0.0f, pavW + 0.8f, pavD + 0.8f, darkTrimCol);

        // Mái đao cong tầng dưới của Lầu Ngũ Phụng đặt khít trên xà đỡ tại y = 16.55m (cao 2.15m -> đỉnh tại 18.70m)
        const float lowerRoofY = pavY0 + 3.75f; // 16.55m
        auto lowerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, lowerRoofY, 0.0f,
            pavW + 1.2f, pavD + 1.2f,
            2.15f, 2.4f,
            goldRoofCol
        );
        gateMesh.append(lowerRoof);

        // Cổ lầu tầng trên của Lầu Ngũ Phụng: mọc liền khối từ y = 17.60m (chồng sâu 1.10m vào trong mái dưới) lên tới y = 20.30m!
        const float upPavY0 = lowerRoofY + 1.05f; // 17.60m (thấp hơn đỉnh mái dưới 18.70m -> không bao giờ hở!)
        const float upPavY1 = 20.30f;
        const float upPavW  = 13.8f;
        const float upPavD  =  5.8f;

        appendBoxByYRange(gateMesh, 0.0f, upPavY0, upPavY1, 0.0f, upPavW, upPavD, redPillarCol);
        // Các cột góc và xà đỡ mái tầng đỉnh (y = 20.15 .. 20.48m)
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                gateMesh.append(Geometry::PolygonMesh::createPrismY(
                    sx * (upPavW * 0.48f), upPavY0, sz * (upPavD * 0.48f),
                    0.22f, upPavY1 - upPavY0, 10, lightStoneCol
                ));
            }
        }
        appendBoxByYRange(gateMesh, 0.0f, upPavY1 - 0.15f, upPavY1 + 0.18f, 0.0f, upPavW + 0.7f, upPavD + 0.7f, darkTrimCol);

        // Mái đao cong tầng đỉnh Lầu Ngũ Phụng đặt trực tiếp tại y = 20.45m (cao 2.25m -> đỉnh 22.70m)
        auto upperRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, upPavY1 + 0.15f, 0.0f,
            upPavW + 1.0f, upPavD + 1.0f,
            2.25f, 2.2f,
            goldRoofCol
        );
        gateMesh.append(upperRoof);

        // Hai lầu phụ hai bên trên sàn tầng 2 (cx = ±12.8m, y = 12.80 .. 16.80m) tạo đủ 5 chóp lầu Ngũ Phụng
        for (int side : {-1, 1}) {
            float spX = side * 12.8f;
            appendBoxByYRange(gateMesh, spX, pavY0, pavY0 + 2.45f, 0.0f, 3.6f, 4.2f, redPillarCol);
            appendBoxByYRange(gateMesh, spX, pavY0 + 2.35f, pavY0 + 2.65f, 0.0f, 4.0f, 4.6f, darkTrimCol);
            auto sideTurretRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
                spX, pavY0 + 2.62f, 0.0f,
                4.2f, 4.8f, 1.55f, 1.2f,
                goldRoofCol
            );
            gateMesh.append(sideTurretRoof);
        }

        // ====================================================================
        // 5. HAI CÁNH TƯỜNG THÀNH ĐÁ CÓ RĂNG CƯA & CỔNG VÒM PHỤ KÉO DÀI HAI BÊN
        // ====================================================================
        const float wingStartLocalX = width * 0.5f - 0.2f; // 22.8m (chồng khít 0.2m vào hông đài thành chính)
        const float wingEndLocalX   = wingStartLocalX + ParkLayout::NGO_MON_WING_SPAN_LOCAL; // 80.8m
        const float archCenterLocalX= ParkLayout::NGO_MON_SIDE_ARCH_X_LOCAL; // 42.0m
        const float archHalfOpen    = 2.2f;
        const float wDepth          = ParkLayout::NGO_MON_WING_DEPTH_LOCAL;  // 4.2m
        const float wHeight         = ParkLayout::NGO_MON_WING_HEIGHT_LOCAL; // 6.4m
        const float wZ              = 1.2f; // Đặt mặt tiền cánh tường thành đồng phẳng/liền lạc với thân cổng

        for (int side : {-1, 1}) {
            float s = static_cast<float>(side);

            // Đoạn tường thành trong: từ hông cổng chính (22.8m) tới mép trong cổng vòm phụ (39.8m)
            float xIn0 = wingStartLocalX;
            float xIn1 = archCenterLocalX - archHalfOpen;
            float cxIn = s * 0.5f * (xIn0 + xIn1);
            float lenIn = xIn1 - xIn0;
            appendBoxByYRange(gateMesh, cxIn, 0.0f, 0.45f, wZ, lenIn, wDepth + 0.8f, plinthCol);
            appendBoxByYRange(gateMesh, cxIn, 0.40f, wHeight, wZ, lenIn, wDepth, stoneCol);

            // Đoạn tường thành ngoài: từ mép ngoài cổng vòm phụ (44.2m) tới hết cánh tường (80.8m)
            float xOut0 = archCenterLocalX + archHalfOpen;
            float xOut1 = wingEndLocalX;
            float cxOut = s * 0.5f * (xOut0 + xOut1);
            float lenOut = xOut1 - xOut0;
            appendBoxByYRange(gateMesh, cxOut, 0.0f, 0.45f, wZ, lenOut, wDepth + 0.8f, plinthCol);
            appendBoxByYRange(gateMesh, cxOut, 0.40f, wHeight, wZ, lenOut, wDepth, stoneCol);

            // Cổng vòm phụ trên cánh tường thành (tại x = ±42.0m):
            // Khung vòm đá trắng nổi bật ở mặt tiền (như 2 cổng vòm trắng hai bên tường thành trong ảnh đêm 2)
            float archX = s * archCenterLocalX;
            appendBoxByYRange(gateMesh, archX - s * (archHalfOpen + 0.9f), 0.0f, wHeight + 0.25f, wZ, 1.8f, wDepth + 0.6f, lightStoneCol);
            appendBoxByYRange(gateMesh, archX + s * (archHalfOpen + 0.9f), 0.0f, wHeight + 0.25f, wZ, 1.8f, wDepth + 0.6f, lightStoneCol);
            // Khối tường đá phía trên vòm phụ (y = 4.40 .. 6.45m)
            appendBoxByYRange(gateMesh, archX, 4.40f, wHeight + 0.25f, wZ, archHalfOpen * 2.0f + 0.4f, wDepth + 0.5f, lightStoneCol);
            auto sideArch = Geometry::PolygonMesh::createArchVault(
                archX, 2.35f, wZ - wDepth * 0.5f - 0.25f, wZ + wDepth * 0.5f + 0.25f,
                archHalfOpen - 0.15f, archHalfOpen + 0.55f, 12, balustradeCol
            );
            gateMesh.append(sideArch);

            // Gờ đá đỉnh tường thành chạy suốt từ 22.8m đến 80.8m (y = 6.35 .. 6.65m)
            float fullCX  = s * 0.5f * (wingStartLocalX + wingEndLocalX);
            float fullLen = wingEndLocalX - wingStartLocalX;
            appendBoxByYRange(gateMesh, fullCX, wHeight - 0.05f, wHeight + 0.25f, wZ, fullLen + 0.4f, wDepth + 0.55f, lightStoneCol);

            // Các bổ trụ khung đá trắng (pilasters) dọc mặt tiền cánh tường thành (cách nhau 8.5m, giống ảnh đêm 1, 2, 3)
            for (float px = wingStartLocalX + 5.0f; px <= wingEndLocalX - 2.0f; px += 8.8f) {
                if (std::fabs(px - archCenterLocalX) < 4.2f) continue;
                appendBoxByYRange(gateMesh, s * px, 0.45f, wHeight, wZ + wDepth * 0.5f + 0.15f, 2.1f, 0.35f, lightStoneCol);
                appendBoxByYRange(gateMesh, s * px, 1.10f, wHeight - 0.70f, wZ + wDepth * 0.5f + 0.22f, 1.3f, 0.25f, stoneCol);
            }

            // Hàng răng cưa thành lũy (crenelations / battlements) dọc bờ mặt tiền và mặt hậu đỉnh tường thành (y = 6.60 .. 7.45m)
            for (float mx = wingStartLocalX + 1.2f; mx <= wingEndLocalX - 1.2f; mx += 2.8f) {
                // Răng cưa mặt tiền (+Z)
                appendBoxByYRange(gateMesh, s * mx, wHeight + 0.20f, wHeight + 1.05f, wZ + wDepth * 0.5f - 0.25f, 1.55f, 0.65f, lightStoneCol);
                // Lan can thấp mặt hậu (-Z)
                appendBoxByYRange(gateMesh, s * mx, wHeight + 0.20f, wHeight + 0.75f, wZ - wDepth * 0.5f + 0.25f, 1.55f, 0.45f, stoneCol);
            }

            // Trụ tháp canh kết thúc ở hai đầu mút ngoài cùng của cánh tường thành (x = ±81.5m)
            appendBoxByYRange(gateMesh, s * (wingEndLocalX + 1.4f), 0.0f, wHeight + 1.35f, wZ, 3.6f, wDepth + 1.2f, lightStoneCol);
        }

        gateMesh.computeNormals();

        // ====================================================================
        // 6. THÁP ĐỒNG HỒ TRUNG TÂM TÁCH RIÊNG (clockTowerMesh tại (+10.7, 0.0, +61.6), scale 1.0)
        //    Đảm bảo liền khối 100% từ mặt đất y = 0 lên đỉnh chóp H = 27.5m
        // ====================================================================
        clockTowerMesh.clear();
        clockTowerMesh.name = "CentralClockTower2019";

        const float ctBaseW = 8.4f;
        const float ctMidW  = 7.2f;
        const float ctTopW  = 6.4f;

        // Tầng đế đá (y = 0.0 .. 1.2m) & Thân dưới Tháp Đồng Hồ (y = 1.15 .. 10.5m)
        appendBoxByYRange(clockTowerMesh, 0.0f, 0.00f,  1.20f, 0.0f, ctBaseW + 1.4f, ctBaseW + 1.4f, plinthCol);
        appendBoxByYRange(clockTowerMesh, 0.0f, 1.15f, 10.50f, 0.0f, ctBaseW,        ctBaseW,        stoneCol);
        // Gờ mái phân tầng giữa (y = 10.45 .. 11.10m) & Mái diềm tầng giữa
        appendBoxByYRange(clockTowerMesh, 0.0f, 10.45f, 11.10f, 0.0f, ctBaseW + 1.0f, ctBaseW + 1.0f, lightStoneCol);
        // Thân giữa Tháp Đồng Hồ (y = 11.00 .. 17.20m)
        appendBoxByYRange(clockTowerMesh, 0.0f, 11.00f, 17.20f, 0.0f, ctMidW, ctMidW, warmWallCol);
        appendBoxByYRange(clockTowerMesh, 0.0f, 17.10f, 17.55f, 0.0f, ctMidW + 0.9f, ctMidW + 0.9f, lightStoneCol);

        // Thân trên nơi gắn 4 mặt đồng hồ (y = 17.50 .. 24.65m)
        const float towerBaseY = 17.15f;
        const float towerBodyH =  7.50f;
        appendBoxByYRange(clockTowerMesh, 0.0f, towerBaseY + 0.30f, towerBaseY + towerBodyH, 0.0f, ctTopW, ctTopW, stoneCol);

        // Gờ phào chỉ và diềm mái mạ vàng trên đỉnh thân tháp (y = 24.55 .. 25.15m)
        appendBoxByYRange(clockTowerMesh, 0.0f, towerBaseY + towerBodyH - 0.10f, towerBaseY + towerBodyH + 0.50f, 0.0f, ctTopW + 1.0f, ctTopW + 1.0f, goldRoofCol);

        // Chóp Mái Tháp Đồng Hồ đao cong 4 phía đặt trực tiếp tại y = 25.10m (cao 2.5m -> đỉnh 27.6m)
        auto towerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, towerBaseY + towerBodyH + 0.45f, 0.0f,
            ctTopW + 1.2f, ctTopW + 1.2f,
            2.50f, 1.8f,
            goldRoofCol
        );
        clockTowerMesh.append(towerRoof);

        // Chóp nhọn mạ vàng gắn liền trên sống nóc đỉnh tháp (y = 27.40 .. 29.80m)
        float peakY = towerBaseY + towerBodyH + 0.45f + 2.35f;
        auto spire = Geometry::PolygonMesh::createPyramid(
            0.0f, peakY, 0.0f,
            1.2f, 1.2f, 2.4f,
            MathCG::ColorRGB(0.98f, 0.85f, 0.20f)
        );
        clockTowerMesh.append(spire);

        clockTowerMesh.computeNormals();
    }

    /**
     * @brief Vẽ Hai Hồ Sen vuông phía Nam Cổng Ngọ Môn & Sân trước lát đá
     */
    void drawForecourtAndLotusPonds(bool isNightOrSunset) const {
        // 1. Sân trước phía Bắc cổng (nối liền Cổng Ngọ Môn vào Quảng trường Trung tâm)
        const auto& fcPos = ParkLayout::GATE_FORECOURT_POS;
        const float fcHW  = ParkLayout::GATE_FORECOURT_WIDTH * 0.5f;
        const float fcHD  = ParkLayout::GATE_FORECOURT_DEPTH * 0.5f;

        glNormal3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.68f, 0.65f, 0.58f);
        glBegin(GL_QUADS);
        glVertex3f(fcPos.x - fcHW, 0.032f, fcPos.z - fcHD);
        glVertex3f(fcPos.x - fcHW, 0.032f, fcPos.z + fcHD);
        glVertex3f(fcPos.x + fcHW, 0.032f, fcPos.z + fcHD);
        glVertex3f(fcPos.x + fcHW, 0.032f, fcPos.z - fcHD);
        glEnd();

        // 2. Hai Hồ Sen vuông phía Nam hai cánh tường thành + lối đi lát đá dẫn qua 2 cổng vòm phụ
        const MathCG::Vec3 pondCenters[2] = {
            ParkLayout::LOTUS_POND_WEST_POS,
            ParkLayout::LOTUS_POND_EAST_POS
        };
        const float pHW = ParkLayout::LOTUS_POND_WIDTH * 0.5f;
        const float pHD = ParkLayout::LOTUS_POND_DEPTH * 0.5f;

        for (int p = 0; p < 2; ++p) {
            const float px = pondCenters[p].x;
            const float pz = pondCenters[p].z;

            // Khung thành hồ đá xám sáng (cao y = 0.38m)
            glColor3f(0.76f, 0.75f, 0.72f);
            drawSimpleBox(px, 0.19f, pz - pHD, pHW * 2.0f + 1.0f, 0.38f, 0.65f);
            drawSimpleBox(px, 0.19f, pz + pHD, pHW * 2.0f + 1.0f, 0.38f, 0.65f);
            drawSimpleBox(px - pHW, 0.19f, pz, 0.65f, 0.38f, pHD * 2.0f);
            drawSimpleBox(px + pHW, 0.19f, pz, 0.65f, 0.38f, pHD * 2.0f);

            // Mặt nước hồ sen (y = 0.16m)
            glColor3f(0.08f, 0.26f, 0.22f);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex3f(px - pHW + 0.3f, 0.16f, pz - pHD + 0.3f);
            glVertex3f(px - pHW + 0.3f, 0.16f, pz + pHD - 0.3f);
            glVertex3f(px + pHW - 0.3f, 0.16f, pz + pHD - 0.3f);
            glVertex3f(px + pHW - 0.3f, 0.16f, pz - pHD + 0.3f);
            glEnd();

            // Các cụm lá sen tròn xanh mướt và búp sen hồng trên mặt hồ (giống ảnh đêm 2)
            for (int r = -1; r <= 1; ++r) {
                for (int c = -3; c <= 3; ++c) {
                    float lx = px + c * 3.1f + ((r % 2 == 0) ? 0.6f : -0.6f);
                    float lz = pz + r * 3.4f;
                    glPushMatrix();
                    glTranslatef(lx, 0.19f, lz);
                    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                    glColor3f(0.22f, 0.58f, 0.20f);
                    ParametricShapes::drawDisk(1.05f, 10, 0.0f, false);
                    glPopMatrix();

                    if ((c + r) % 2 == 0) {
                        glPushMatrix();
                        glTranslatef(lx + 0.25f, 0.30f, lz);
                        if (isNightOrSunset) {
                            GLfloat emit[4] = { 0.85f, 0.45f, 0.55f, 1.0f };
                            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                        }
                        glColor3f(0.96f, 0.58f, 0.68f);
                        ParametricShapes::drawSphere(0.22f, 8, 6, false);
                        if (isNightOrSunset) {
                            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
                        }
                        glPopMatrix();
                    }
                }
            }
        }

        // Lối đi bộ lát đá trắng dẫn qua 2 cổng vòm phụ trên hai cánh tường thành (tại X = 16.1 ± 42.0*0.76 = -15.8 và +48.0)
        for (int side : {-1, 1}) {
            float sideWalkX = position.x + side * (ParkLayout::NGO_MON_SIDE_ARCH_X_LOCAL * gateScale);
            glColor3f(0.76f, 0.74f, 0.70f);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex3f(sideWalkX - 2.2f, 0.035f, position.z - 16.0f);
            glVertex3f(sideWalkX - 2.2f, 0.035f, position.z + 25.0f);
            glVertex3f(sideWalkX + 2.2f, 0.035f, position.z + 25.0f);
            glVertex3f(sideWalkX + 2.2f, 0.035f, position.z - 16.0f);
            glEnd();
        }
    }

    /**
     * @brief Vẽ 4 ô cửa sổ tròn chữ Thọ phát sáng trên mặt tiền tầng 2 của Cổng Ngọ Môn (theo ảnh đêm 2)
     */
    void drawTier2RoundWindows(bool isNightOrSunset) const {
        const float winX[4] = { -11.2f, -10.8f, 10.8f, 11.2f };
        const float winY[4] = {  11.2f,   8.3f,  8.3f, 11.2f };
        const float winZ    = 5.78f;

        for (int i = 0; i < 4; ++i) {
            glPushMatrix();
            glTranslatef(winX[i], winY[i], winZ);

            glColor3f(0.92f, 0.85f, 0.45f);
            ParametricShapes::drawToroid(0.95f, 0.12f, 18, 6, false);

            if (isNightOrSunset) {
                GLfloat emit[4] = { 0.48f, 0.85f, 0.25f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(0.58f, 0.95f, 0.32f);
            } else {
                glColor3f(0.32f, 0.36f, 0.30f);
            }
            ParametricShapes::drawDisk(0.88f, 18, 0.0f, false);
            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }
            glPopMatrix();
        }
    }

    /**
     * @brief Vẽ 4 mặt đồng hồ phát sáng trên bốn phía của Tháp Đồng Hồ Trung Tâm
     */
    void drawClockFaces(bool isNightOrSunset) const {
        float clockY = 21.0f;
        float offset = 3.25f;

        for (int side = 0; side < 4; ++side) {
            glPushMatrix();
            glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, clockY, offset);

            glColor3f(0.95f, 0.78f, 0.15f);
            ParametricShapes::drawToroid(2.10f, 0.15f, 24, 8, false);

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
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(0.85f * std::cos(MathCG::DEG2RAD * 30.0f), 0.85f * std::sin(MathCG::DEG2RAD * 30.0f), 0.03f);
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(1.35f * std::cos(MathCG::DEG2RAD * 60.0f), 1.35f * std::sin(MathCG::DEG2RAD * 60.0f), 0.03f);
            glEnd();
            glLineWidth(1.0f);

            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawSphere(0.15f, 10, 8, false);

            glPopMatrix();
        }
    }

    /**
     * @brief Vẽ dàn đèn lồng đỏ cung đình treo trực tiếp dưới diềm mái Lầu Ngũ Phụng (dây gắn khít vào diềm mái)
     */
    void drawLanterns(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        const float eaveY    = 16.50f; // Đúng cao độ mặt dưới diềm mái tầng 1 Lầu Ngũ Phụng
        const float lanternZ =  5.10f; // Ngay dưới diềm mái đua phía trước

        const float lanternX[] = { -10.0f, -6.0f, -2.0f, 2.0f, 6.0f, 10.0f };
        for (float lx : lanternX) {
            glPushMatrix();
            glTranslatef(lx, eaveY - 0.70f, lanternZ);

            // Dây treo đèn lồng nối từ bầu đèn (y = 0) thẳng lên chạm mặt dưới diềm mái (y = +0.75)
            glColor3f(0.85f, 0.70f, 0.20f);
            ParametricShapes::drawCylinderY(0.03f, 0.75f, 6, 1, true, false);

            if (isNightOrSunset) {
                GLfloat emit[4] = { 1.0f, 0.18f, 0.12f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(1.0f, 0.25f, 0.15f);
            } else {
                glColor3f(0.88f, 0.16f, 0.12f);
            }
            ParametricShapes::drawSphere(0.40f, 12, 10, false);
            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }

            glTranslatef(0.0f, -0.42f, 0.0f);
            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawConeY(0.14f, 0.0f, -0.32f, 8, 1, true, false);

            glPopMatrix();
        }
    }

    static void drawSimpleBox(float cx, float cy, float cz, float sx, float sy, float sz) {
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(cx - hx, cy - hy, cz + hz); glVertex3f(cx + hx, cy - hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz + hz);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(cx + hx, cy - hy, cz - hz); glVertex3f(cx - hx, cy - hy, cz - hz);
        glVertex3f(cx - hx, cy + hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz - hz);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(cx - hx, cy + hy, cz + hz); glVertex3f(cx + hx, cy + hy, cz + hz);
        glVertex3f(cx + hx, cy + hy, cz - hz); glVertex3f(cx - hx, cy + hy, cz - hz);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz); glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy - hy, cz + hz); glVertex3f(cx - hx, cy - hy, cz + hz);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(cx - hx, cy - hy, cz - hz); glVertex3f(cx - hx, cy - hy, cz + hz);
        glVertex3f(cx - hx, cy + hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz - hz);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(cx + hx, cy - hy, cz + hz); glVertex3f(cx + hx, cy - hy, cz - hz);
        glVertex3f(cx + hx, cy + hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz + hz);
        glEnd();
    }
};

} // namespace AsiaParkRides

#endif // NGO_MON_GATE_H
