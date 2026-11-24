#ifndef PIRATE_SHIP_H
#define PIRATE_SHIP_H

/**
 * @file PirateShip.h
 * @brief Swinging Pirate Ship Ride (Thuyền Cướp Biển) - Chapter 4 Hierarchical Animation
 *
 * Cấu trúc liền khối hoàn chỉnh (đã sửa lỗi rời rạc các chi tiết mũi/lái/tay treo/khung chữ A):
 *  - Bệ sàn đón khách có bậc thang & lan can hai bên hành lang lên thuyền.
 *  - Hai khung tháp chữ A (A-frame) đặt tại Z = ±2.25 m, mỗi khung gồm 2 chân nghiêng
 *    tụ đỉnh chính xác vào ổ trục ngang tại (0, 10.5 m, ±2.25 m) kèm thanh giằng ngang.
 *  - Trục xoay ngang (axle) nối liền hai đỉnh khung chữ A dọc trục Z.
 *  - Cụm con lắc (pendulum group) lắc quanh trục Z (mũi - lái văng theo phương X):
 *    + 4 thanh tay treo nghiêng + trụ treo giữa nối trực tiếp từ ổ trục (y = 10.5 m) xuống mạn thuyền.
 *    + Thân thuyền gỗ liền khối dọc trục X (lái -8.0..-4.5 m, khoang giữa -4.5..+4.5 m, mũi +4.5..+8.0 m,
 *      đầu rồng/chóp mũi +8.0..+8.9 m) dùng chung tiết diện khớp tuyệt đối 100% đỉnh giữa các đốt.
 *    + Viền mạn vàng, băng ghế gỗ, cột buồm trung tâm + xà ngang + cờ hiệu và hành khách ngồi trên ghế.
 */

#include <GL/freeglut.h>
#include <cmath>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class PirateShip {
public:
    MathCG::Vec3 position = ParkLayout::PIRATE_SHIP_TARGET_POS;
    float swingPhase     = 0.0f;
    float swingAmplitude = 48.0f; // Biên độ góc lắc tối đa (độ)
    float swingPeriod    = 6.0f;  // Chu kỳ lắc (giây)
    float pendulumAngle  = 0.0f;  // Góc lắc hiện tại quanh trục Z

    PirateShip() {
        buildMeshes();
    }

    ~PirateShip() {
        if (staticListDay_ != 0)   glDeleteLists(staticListDay_, 1);
        if (staticListNight_ != 0) glDeleteLists(staticListNight_, 1);
        if (boatListDay_ != 0)     glDeleteLists(boatListDay_, 1);
        if (boatListNight_ != 0)   glDeleteLists(boatListNight_, 1);
    }

    void update(float dt) {
        swingPhase    = std::fmod(swingPhase + (MathCG::TWO_PI / swingPeriod) * dt, MathCG::TWO_PI);
        pendulumAngle = swingAmplitude * std::sin(swingPhase);
    }

    void draw(bool isNightOrSunset, bool showNormals) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        if (!showNormals) {
            GLuint& sList = isNightOrSunset ? staticListNight_ : staticListDay_;
            if (sList == 0) {
                sList = glGenLists(1);
                if (sList != 0) {
                    glNewList(sList, GL_COMPILE);
                    drawStaticStructureDirect(isNightOrSunset, false);
                    glEndList();
                }
            }
            if (sList != 0) glCallList(sList);
            else drawStaticStructureDirect(isNightOrSunset, false);

            // Cụm thuyền & tay treo lắc quanh trục Z tại ổ trục (0, 10.5, 0)
            glPushMatrix();
            glTranslatef(0.0f, 10.5f, 0.0f);
            glRotatef(pendulumAngle, 0.0f, 0.0f, 1.0f);

            GLuint& bList = isNightOrSunset ? boatListNight_ : boatListDay_;
            if (bList == 0) {
                bList = glGenLists(1);
                if (bList != 0) {
                    glNewList(bList, GL_COMPILE);
                    drawSwingingBoatDirect(isNightOrSunset, false);
                    glEndList();
                }
            }
            if (bList != 0) glCallList(bList);
            else drawSwingingBoatDirect(isNightOrSunset, false);

            glPopMatrix();
        } else {
            drawStaticStructureDirect(isNightOrSunset, true);
            glPushMatrix();
            glTranslatef(0.0f, 10.5f, 0.0f);
            glRotatef(pendulumAngle, 0.0f, 0.0f, 1.0f);
            drawSwingingBoatDirect(isNightOrSunset, true);
            glPopMatrix();
        }

        glPopMatrix();
    }

private:
    Geometry::PolygonMesh staticMeshDay_;
    Geometry::PolygonMesh staticMeshNight_;
    Geometry::PolygonMesh boatMeshDay_;
    Geometry::PolygonMesh boatMeshNight_;
    mutable GLuint staticListDay_   = 0;
    mutable GLuint staticListNight_ = 0;
    mutable GLuint boatListDay_     = 0;
    mutable GLuint boatListNight_   = 0;

    // Nối một thanh dầm hộp 3D nghiêng bất kỳ từ điểm A(x0,y0,z0) tới B(x1,y1,z1) với tiết diện (wx, wz)
    static void appendStrutBetween(
        Geometry::PolygonMesh& mesh,
        float x0, float y0, float z0,
        float x1, float y1, float z1,
        float halfW, float halfD,
        const MathCG::ColorRGB& col
    ) {
        int b0 = mesh.addVertex(x0 - halfW, y0, z0 + halfD);
        int b1 = mesh.addVertex(x0 + halfW, y0, z0 + halfD);
        int b2 = mesh.addVertex(x0 + halfW, y0, z0 - halfD);
        int b3 = mesh.addVertex(x0 - halfW, y0, z0 - halfD);

        int t0 = mesh.addVertex(x1 - halfW, y1, z1 + halfD);
        int t1 = mesh.addVertex(x1 + halfW, y1, z1 + halfD);
        int t2 = mesh.addVertex(x1 + halfW, y1, z1 - halfD);
        int t3 = mesh.addVertex(x1 - halfW, y1, z1 - halfD);

        mesh.addQuad(b0, b1, t1, t0, col, true); // Trước (+Z)
        mesh.addQuad(b2, b3, t3, t2, col, true); // Sau (-Z)
        mesh.addQuad(b1, b2, t2, t1, col, true); // Phải (+X)
        mesh.addQuad(b3, b0, t0, t3, col, true); // Trái (-X)
        mesh.addQuad(t0, t1, t2, t3, col, true); // Đỉnh
        mesh.addQuad(b3, b2, b1, b0, col, true); // Đáy
    }

    // Tạo đốt thân thuyền dọc trục X từ tiết diện x0 sang tiết diện x1 (khớp tuyệt đối đỉnh giữa các đốt liên tiếp)
    static void appendHullSectionX(
        Geometry::PolygonMesh& mesh,
        float x0, float yBot0, float yTop0, float wZ0,
        float x1, float yBot1, float yTop1, float wZ1,
        const MathCG::ColorRGB& hullCol,
        const MathCG::ColorRGB& deckCol,
        const MathCG::ColorRGB& trimCol
    ) {
        float hz0 = wZ0 * 0.5f;
        float hz1 = wZ1 * 0.5f;
        // Đáy thuyền thu hẹp nhẹ kiểu chữ V (keel) để thân tàu nổi khối đẹp
        float keelHz0 = hz0 * 0.62f;
        float keelHz1 = hz1 * 0.62f;

        // Thiết diện tại x0 (trái)
        int l0 = mesh.addVertex(x0, yBot0,  keelHz0); // Dưới - Trước (+Z)
        int l1 = mesh.addVertex(x0, yTop0,  hz0);     // Trên - Trước (+Z)
        int l2 = mesh.addVertex(x0, yTop0, -hz0);     // Trên - Sau (-Z)
        int l3 = mesh.addVertex(x0, yBot0, -keelHz0); // Dưới - Sau (-Z)

        // Thiết diện tại x1 (phải)
        int r0 = mesh.addVertex(x1, yBot1,  keelHz1);
        int r1 = mesh.addVertex(x1, yTop1,  hz1);
        int r2 = mesh.addVertex(x1, yTop1, -hz1);
        int r3 = mesh.addVertex(x1, yBot1, -keelHz1);

        mesh.addQuad(l0, r0, r1, l1, hullCol, true); // Mạn trước (+Z)
        mesh.addQuad(r3, l3, l2, r2, hullCol, true); // Mạn sau (-Z)
        mesh.addQuad(l1, r1, r2, l2, deckCol, true); // Mặt boong trên (+Y)
        mesh.addQuad(l3, r3, r0, l0, hullCol, true); // Đáy thuyền (-Y)
        mesh.addQuad(l3, l0, l1, l2, hullCol, true); // Vách đầu x0
        mesh.addQuad(r0, r3, r2, r1, hullCol, true); // Vách đầu x1

        // Gờ mạn vàng (gunwale trim) chạy bám sát mép trên hai bên mạn thuyền
        const float th = 0.14f;
        const float tw = 0.08f;
        for (int s : {-1, 1}) {
            float z0Out = s * (hz0 + tw);
            float z0In  = s * (hz0 - tw);
            float z1Out = s * (hz1 + tw);
            float z1In  = s * (hz1 - tw);
            int g0 = mesh.addVertex(x0, yTop0,      z0Out);
            int g1 = mesh.addVertex(x1, yTop1,      z1Out);
            int g2 = mesh.addVertex(x1, yTop1 + th, z1Out);
            int g3 = mesh.addVertex(x0, yTop0 + th, z0Out);
            int g4 = mesh.addVertex(x0, yTop0,      z0In);
            int g5 = mesh.addVertex(x1, yTop1,      z1In);
            int g6 = mesh.addVertex(x1, yTop1 + th, z1In);
            int g7 = mesh.addVertex(x0, yTop0 + th, z0In);
            if (s > 0) {
                mesh.addQuad(g0, g1, g2, g3, trimCol, true);
                mesh.addQuad(g5, g4, g7, g6, trimCol, true);
                mesh.addQuad(g3, g2, g6, g7, trimCol, true);
            } else {
                mesh.addQuad(g1, g0, g3, g2, trimCol, true);
                mesh.addQuad(g4, g5, g6, g7, trimCol, true);
                mesh.addQuad(g7, g6, g2, g3, trimCol, true);
            }
        }
    }

    static Geometry::PolygonMesh buildStaticMeshForFactor(float n) {
        Geometry::PolygonMesh m("PirateShipStatic");
        const MathCG::ColorRGB steelCol (0.50f * n, 0.53f * n, 0.58f * n);
        const MathCG::ColorRGB darkSteel(0.34f * n, 0.36f * n, 0.40f * n);
        const MathCG::ColorRGB goldCol  (0.86f * n, 0.70f * n, 0.22f * n);
        const MathCG::ColorRGB woodDeck (0.58f * n, 0.40f * n, 0.24f * n);
        const MathCG::ColorRGB stoneCol (0.64f * n, 0.62f * n, 0.58f * n);

        // 1. Sàn gỗ đón khách hai bên mạn thuyền (chừa rãnh giữa Z in [-1.55, +1.55] cho thuyền văng tự do)
        for (int sideZ : {-1, 1}) {
            float zCenter = sideZ * 2.55f;
            Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, 0.03f, 0.75f, zCenter, 9.6f, 1.85f, woodDeck);
            // Bậc tam cấp lên sàn đón ở hai đầu Bắc / Nam
            for (int st = 0; st < 3; ++st) {
                float zStep = sideZ * (3.65f + st * 0.38f);
                float hTop  = 0.75f - st * 0.22f;
                Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, 0.03f, hTop, zStep, 3.2f, 0.40f, stoneCol);
            }
            // Lan can bảo vệ hai đầu sàn đón
            for (int sideX : {-1, 1}) {
                float rx = sideX * 3.2f;
                float rz = sideZ * 3.38f;
                Geometry::PolygonMesh::appendBoxByYRange(m, rx, 0.75f, 1.50f, rz, 2.8f, 0.10f, goldCol);
            }
        }

        // 2. Hai khung tháp chữ A (A-frame) tại Z = ±2.25 m
        //    Chân trụ đặt tại X = ±4.80 m, Y = 0.03 m, hội tụ khít vào ổ trục đỉnh (X = 0.0 m, Y = 10.50 m)
        const float zTowerAbs = 2.25f;
        const float footX     = 4.80f;
        const float apexY     = 10.50f;
        for (int sideZ : {-1, 1}) {
            float zT = sideZ * zTowerAbs;
            for (int legX : {-1, 1}) {
                float fx = legX * footX;
                // Bệ móng bê tông chân trụ chữ A
                Geometry::PolygonMesh::appendBoxByYRange(m, fx, 0.03f, 0.55f, zT, 0.95f, 0.95f, stoneCol);
                // Chân trụ thép nghiêng nối liền từ bệ móng (fx, 0.45, zT) lên đỉnh ổ trục (0.0, apexY, zT)
                appendStrutBetween(m, fx, 0.45f, zT, 0.0f, apexY, zT, 0.24f, 0.24f, steelCol);
            }
            // Thanh giằng ngang chữ A tại cao độ y = 4.60 m (tọa độ X nội suy đúng trên thân chân trụ nghiêng)
            float tBrace = (4.60f - 0.45f) / (apexY - 0.45f);
            float xBrace = footX * (1.0f - tBrace);
            Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, 4.45f, 4.78f, zT, xBrace * 2.0f + 0.30f, 0.34f, darkSteel);

            // Hộp gối đỡ ổ trục đỉnh tháp chữ A + chóp trang trí vàng
            Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, apexY - 0.35f, apexY + 0.45f, zT, 1.15f, 0.82f, darkSteel);
            m.append(Geometry::PolygonMesh::createPyramid(0.0f, apexY + 0.45f, zT, 0.95f, 0.72f, 0.55f, goldCol));
        }

        // 3. Dầm khóa đỉnh và trục xoay ngang (axle) nối liền hai đỉnh tháp từ Z = -2.55 m sang Z = +2.55 m
        Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, apexY - 0.18f, apexY + 0.18f, 0.0f, 0.42f, 5.10f, goldCol);
        Geometry::PolygonMesh::appendBoxByYRange(m, -2.10f, 4.50f, 4.76f, 0.0f, 0.24f, 4.50f, darkSteel);
        Geometry::PolygonMesh::appendBoxByYRange(m,  2.10f, 4.50f, 4.76f, 0.0f, 0.24f, 4.50f, darkSteel);

        m.computeNormals();
        return m;
    }

    static Geometry::PolygonMesh buildBoatMeshForFactor(float n) {
        Geometry::PolygonMesh m("PirateShipBoat");
        const MathCG::ColorRGB hullCol  (0.48f * n, 0.24f * n, 0.09f * n);
        const MathCG::ColorRGB deckCol  (0.62f * n, 0.38f * n, 0.18f * n);
        const MathCG::ColorRGB goldCol  (0.88f * n, 0.72f * n, 0.20f * n);
        const MathCG::ColorRGB redTrim  (0.78f * n, 0.18f * n, 0.14f * n);
        const MathCG::ColorRGB steelArm (0.40f * n, 0.43f * n, 0.48f * n);
        const MathCG::ColorRGB seatCol  (0.72f * n, 0.52f * n, 0.26f * n);

        // Hệ tọa độ cục bộ của cụm con lắc: gốc (0, 0, 0) nằm tại tâm trục xoay ngang (y_world = 10.50 m),
        // đáy thuyền nằm tại y = -7.50 m (tức y_world = 3.00 m khi ở vị trí cân bằng).
        const float hullBaseY = -7.50f;

        // 1. Khớp ống lồng ôm trục xoay tại (0, 0, 0)
        Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, -0.28f, 0.28f, 0.0f, 0.75f, 2.35f, goldCol);

        // 2. Hệ 4 thanh tay treo nghiêng + 1 trụ treo chính giữa nối liền từ ổ trục (0, 0, ±1.00)
        //    xuống gắn chặt vào thành mạn thuyền tại (X = ±2.80, Y = hullBaseY + 1.05, Z = ±1.00)
        for (int sideZ : {-1, 1}) {
            float zArm = sideZ * 0.98f;
            for (int sideX : {-1, 1}) {
                appendStrutBetween(
                    m,
                    sideX * 2.80f, hullBaseY + 0.95f, zArm,
                    0.0f,          0.0f,              zArm,
                    0.13f, 0.11f, steelArm
                );
            }
            // Thanh giằng ngang nối 2 tay treo ở lưng chừng (y = -3.80 m)
            Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, -3.90f, -3.68f, zArm, 2.95f, 0.18f, goldCol);
        }
        // Cột buồm / trụ treo trung tâm nối thẳng từ ổ trục (0, 0, 0) xuống sàn thuyền (0, hullBaseY + 0.40, 0)
        Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, hullBaseY + 0.40f, 0.0f, 0.0f, 0.24f, 0.24f, steelArm);
        // Xà ngang cột buồm và lá cờ hiệu đỏ - vàng gắn liền vào cột trung tâm
        Geometry::PolygonMesh::appendBoxByYRange(m, 0.0f, hullBaseY + 4.10f, hullBaseY + 4.26f, 0.0f, 0.20f, 2.80f, goldCol);
        Geometry::PolygonMesh::appendBoxByYRange(m, 0.75f, hullBaseY + 3.45f, hullBaseY + 4.10f, 0.0f, 1.35f, 0.06f, redTrim);

        // 3. Thân thuyền gỗ liền khối 5 đốt dọc trục X (tất cả đốt kề nhau dùng chung tọa độ tiết diện)
        //    Đốt 1: Đuôi lái nhô cao (stern castle: X = -8.0 -> -6.4)
        appendHullSectionX(m,
            -8.0f, hullBaseY + 0.70f, hullBaseY + 2.05f, 0.36f,
            -6.4f, hullBaseY + 0.25f, hullBaseY + 1.55f, 1.45f,
            hullCol, deckCol, goldCol);
        //    Đốt 2: Thân sau mở rộng (X = -6.4 -> -4.2)
        appendHullSectionX(m,
            -6.4f, hullBaseY + 0.25f, hullBaseY + 1.55f, 1.45f,
            -4.2f, hullBaseY + 0.00f, hullBaseY + 1.20f, 2.20f,
            hullCol, deckCol, goldCol);
        //    Đốt 3: Khoang giữa hành khách (X = -4.2 -> +4.2)
        appendHullSectionX(m,
            -4.2f, hullBaseY + 0.00f, hullBaseY + 1.20f, 2.20f,
             4.2f, hullBaseY + 0.00f, hullBaseY + 1.20f, 2.20f,
            hullCol, deckCol, goldCol);
        //    Đốt 4: Thân trước thuôn mũi (X = +4.2 -> +6.4)
        appendHullSectionX(m,
             4.2f, hullBaseY + 0.00f, hullBaseY + 1.20f, 2.20f,
             6.4f, hullBaseY + 0.25f, hullBaseY + 1.55f, 1.45f,
            hullCol, deckCol, goldCol);
        //    Đốt 5: Mũi thuyền vươn cao (bow prow: X = +6.4 -> +8.0)
        appendHullSectionX(m,
             6.4f, hullBaseY + 0.25f, hullBaseY + 1.55f, 1.45f,
             8.0f, hullBaseY + 0.70f, hullBaseY + 2.05f, 0.36f,
            hullCol, deckCol, goldCol);
        //    Đốt 6: Đầu rồng / tượng mũi thuyền mạ vàng gắn liền vào đỉnh mũi (+8.0 -> +8.95)
        appendHullSectionX(m,
             8.0f,  hullBaseY + 1.35f, hullBaseY + 2.05f, 0.34f,
             8.95f, hullBaseY + 1.85f, hullBaseY + 2.45f, 0.14f,
            goldCol, redTrim, goldCol);
        //    Chóp đuôi lái mạ vàng gắn liền vào đỉnh lái (-8.80 -> -8.0)
        appendHullSectionX(m,
            -8.80f, hullBaseY + 1.80f, hullBaseY + 2.38f, 0.16f,
            -8.0f,  hullBaseY + 1.35f, hullBaseY + 2.05f, 0.34f,
            goldCol, redTrim, goldCol);

        // 4. Các hàng ghế gỗ ngang gắn liền trên mặt boong khoang giữa + hành khách ngồi trên ghế
        static const float seatX[6] = { -4.2f, -2.5f, -0.9f, 0.9f, 2.5f, 4.2f };
        const MathCG::ColorRGB shirtCols[3] = {
            MathCG::ColorRGB(0.85f * n, 0.24f * n, 0.20f * n),
            MathCG::ColorRGB(0.22f * n, 0.56f * n, 0.86f * n),
            MathCG::ColorRGB(0.92f * n, 0.76f * n, 0.22f * n)
        };
        const MathCG::ColorRGB headCol(0.92f * n, 0.78f * n, 0.64f * n);

        for (int i = 0; i < 6; ++i) {
            float sx = seatX[i];
            // Băng ghế gỗ gắn trên boong (y = hullBaseY + 1.15 .. + 1.38)
            Geometry::PolygonMesh::appendBoxByYRange(m, sx, hullBaseY + 1.15f, hullBaseY + 1.38f, 0.0f, 0.45f, 1.85f, seatCol);
            // Hành khách ngồi trên ghế (2 người mỗi băng ghế ở Z = ±0.48 m)
            for (int sideZ : {-1, 1}) {
                float pz = sideZ * 0.48f;
                const MathCG::ColorRGB& sc = shirtCols[(i + (sideZ > 0 ? 1 : 0)) % 3];
                // Thân người gắn trực tiếp từ mặt ghế (hullBaseY + 1.38) lên vai (hullBaseY + 1.96)
                m.append(Geometry::PolygonMesh::createPrismY(sx, hullBaseY + 1.38f, pz, 0.17f, 0.58f, 8, sc));
                // Đầu người gắn liền trên vai (hullBaseY + 1.96 .. + 2.26)
                m.append(Geometry::PolygonMesh::createPrismY(sx, hullBaseY + 1.96f, pz, 0.14f, 0.28f, 8, headCol));
            }
        }

        m.computeNormals();
        return m;
    }

    void buildMeshes() {
        staticMeshDay_   = buildStaticMeshForFactor(1.0f);
        staticMeshNight_ = buildStaticMeshForFactor(0.65f);
        boatMeshDay_     = buildBoatMeshForFactor(1.0f);
        boatMeshNight_   = buildBoatMeshForFactor(0.65f);
    }

    void drawStaticStructureDirect(bool isNightOrSunset, bool showNormals) const {
        if (isNightOrSunset) staticMeshNight_.draw(true, showNormals);
        else                 staticMeshDay_.draw(true, showNormals);
    }

    void drawSwingingBoatDirect(bool isNightOrSunset, bool showNormals) const {
        if (isNightOrSunset) boatMeshNight_.draw(true, showNormals);
        else                 boatMeshDay_.draw(true, showNormals);
    }
};

} // namespace AsiaParkRides

#endif // PIRATE_SHIP_H
