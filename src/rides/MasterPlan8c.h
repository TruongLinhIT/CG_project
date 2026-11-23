#ifndef MASTER_PLAN_8C_H
#define MASTER_PLAN_8C_H

/**
 * @file MasterPlan8c.h
 * @brief Đợt 8c: Мо-đun Quy Hoạch Tổng Thể Nửa Phía Đông & Bờ Đông Hồ Nước (8 công trình từ bộ bản vẽ docs/img/ref/):
 *        1. BP6  (1795453362382_image.png): Nhà hàng Panda (Panda Restaurant) — 26.0 x 18.0 x 14.0m
 *        2. BP7  (1795453365678_image.png): Tượng & Hồ phun nước Merlion (Phân khu Singapore) — ĐK hồ 28.0m, cao 10.0m
 *        3. BP8  (1795453369127_image.png): Thuyền Rồng Hoàng Gia (Dragon Boat) nổi trên mặt hồ — 22.0 x 7.0 x 11.5m
 *        4. BP9  (1795453378911_image.png): Tàu lượn Thuyền Rồng Đu Đưa "The Flying Kirins" trên cầu cảng — 18.0 x 12.0 x 15.0m
 *                                           (Tách biệt hoàn toàn với PirateShip.h ở khu Tây theo Quyết định 3)
 *        5. BP13 (1795453392187_image.png): Sân khấu nổi & Khán đài bán nguyệt Symphony of River — 42.0 x 20.0 x 12.0m
 *        6. BP14 (1795453395295_image.png): Nhà hát Âu Cơ (Múa rối nước Việt) — 24.0 x 18.0 x 12.0m
 *        7. BP16 (1795453400510_image.png): Đảo Khủng Long (Dino Island: Cổng gỗ, Núi lửa 10m, T-Rex, Brachiosaurus, Triceratops) — 28.0 x 22.0 x 10.0m
 *        8. BP17 (1795453403998_image.png): Vườn Trúc Nhật Bản (Bamboo Garden: Cổng Torii, Suối Koi, Cầu cong, Trà thất) — 24.0 x 20.0 x 9.0m
 *
 * Nguyên tắc kỹ thuật:
 *  - Gắn nhãn rõ: "Quy hoạch tổng thể, không phải hiện trạng 2019", có phím bật/tắt [K] và mục menu chuột phải.
 *  - Kích thước đã được chuẩn hóa & lọc bỏ mọi số liệu tự mâu thuẫn trên bản vẽ AI.
 *  - KHÔNG dùng glutSolid*; tự tính pháp tuyến từng mặt bằng tích có hướng (computeNormals) và đóng gói toàn bộ
 *    phần kiến trúc tĩnh vào OpenGL Display Lists (smoothList_ / flatList_) để duy trì FPS cao.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"
#include "geometry/MeshBuilders.h"

namespace AsiaParkRides {

class MasterPlan8c {
public:
    bool  enabled        = true;  // Bật mặc định để người dùng thấy đủ 100% công trình từ img/ref (nhấn [K] để bật/tắt so sánh 2019)
    float animTime       = 0.0f;
    float kirinSwingDeg  = 0.0f;

    MasterPlan8c() {
        buildAllStaticMeshes();
    }

    void update(float dt) {
        animTime = std::fmod(animTime + dt, 120.0f);
        // Dao động con lắc đơn cho thuyền rồng đôi The Flying Kirins (biên độ 58 độ, chu kỳ 5.4s)
        kirinSwingDeg = 58.0f * std::sin((MathCG::TWO_PI / 5.4f) * animTime);
    }

    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        if (!enabled) return;

        ensureDisplayLists();

        // 1. Vẽ toàn bộ khối kiến trúc tĩnh của 8 công trình + lối đi khu Đông từ Display List
        if (!showNormals) {
            GLuint listId = smoothShading ? smoothList_ : flatList_;
            if (listId != 0) {
                glCallList(listId);
            } else {
                staticMesh_.draw(smoothShading, false);
            }
        } else {
            staticMesh_.draw(smoothShading, true);
        }

        // 2. Vẽ các phần động & hiệu ứng phát sáng ban đêm (Tia nước phun Merlion, Thuyền Rồng đu đưa The Flying Kirins, Khói núi lửa & Đèn sân khấu)
        drawDynamicAndEmissiveElements(isNightOrSunset, showNormals, smoothShading);
    }

private:
    Geometry::PolygonMesh staticMesh_;
    Geometry::PolygonMesh kirinBoatMesh_;
    mutable GLuint smoothList_    = 0;
    mutable GLuint flatList_      = 0;
    mutable GLuint kirinBoatList_ = 0;

    void ensureDisplayLists() const {
        if (smoothList_ == 0) {
            smoothList_ = glGenLists(1);
            if (smoothList_ != 0) {
                glNewList(smoothList_, GL_COMPILE);
                drawEastMasterPlanPathsAndCourts();
                staticMesh_.draw(true, false);
                glEndList();
            }
        }
        if (flatList_ == 0) {
            flatList_ = glGenLists(1);
            if (flatList_ != 0) {
                glNewList(flatList_, GL_COMPILE);
                drawEastMasterPlanPathsAndCourts();
                staticMesh_.draw(false, false);
                glEndList();
            }
        }
        if (kirinBoatList_ == 0) {
            kirinBoatList_ = glGenLists(1);
            if (kirinBoatList_ != 0) {
                glNewList(kirinBoatList_, GL_COMPILE);
                kirinBoatMesh_.draw(true, false);
                glEndList();
            }
        }
    }

    static void drawFlatStrip(
        const ParkLayout::PointXZ* pts,
        int count,
        float width,
        float y,
        float r, float g, float b
    ) {
        if (!pts || count < 2) return;
        const float halfW = width * 0.5f;
        glColor3f(r, g, b);
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (int i = 0; i < count - 1; ++i) {
            float dx = pts[i + 1].x - pts[i].x;
            float dz = pts[i + 1].z - pts[i].z;
            float len = std::sqrt(dx * dx + dz * dz);
            if (len < 1e-4f) continue;
            float nx = -dz / len * halfW;
            float nz =  dx / len * halfW;
            glBegin(GL_QUADS);
            glVertex3f(pts[i].x     - nx, y, pts[i].z     - nz);
            glVertex3f(pts[i].x     + nx, y, pts[i].z     + nz);
            glVertex3f(pts[i + 1].x + nx, y, pts[i + 1].z + nz);
            glVertex3f(pts[i + 1].x - nx, y, pts[i + 1].z - nz);
            glEnd();
        }
    }

    static void drawEastMasterPlanPathsAndCourts() {
        // Các tuyến đường dạo bộ lát đá nối sang khu Quy hoạch Tổng thể phía Đông
        drawFlatStrip(
            ParkLayout::PATH_EAST_MASTERPLAN_MAIN,
            ParkLayout::NUM_PATH_EAST_MAIN_PTS,
            ParkLayout::PATH_EAST_MASTERPLAN_WIDTH,
            0.036f,
            0.72f, 0.67f, 0.58f
        );
        drawFlatStrip(
            ParkLayout::PATH_EAST_MASTERPLAN_NORTH,
            ParkLayout::NUM_PATH_EAST_NORTH_SPUR_PTS,
            4.0f,
            0.035f,
            0.68f, 0.63f, 0.55f
        );

        // Sân lát đá dưới các công trình khu Đông
        struct EastCourt {
            float cx, cz, w, d;
            float r, g, b;
        };
        const EastCourt courts[5] = {
            { ParkLayout::W8C_PANDA_RESTAURANT_POS.x, ParkLayout::W8C_PANDA_RESTAURANT_POS.z, 34.0f, 26.0f, 0.66f, 0.61f, 0.54f },
            { ParkLayout::W8C_AU_CO_THEATER_POS.x,    ParkLayout::W8C_AU_CO_THEATER_POS.z,    30.0f, 24.0f, 0.64f, 0.59f, 0.52f },
            { ParkLayout::W8C_BAMBOO_GARDEN_POS.x,    ParkLayout::W8C_BAMBOO_GARDEN_POS.z,    26.0f, 22.0f, 0.58f, 0.56f, 0.49f },
            { ParkLayout::W8C_DINO_ISLAND_POS.x,      ParkLayout::W8C_DINO_ISLAND_POS.z,      30.0f, 24.0f, 0.55f, 0.50f, 0.43f },
            { ParkLayout::W8C_SYMPHONY_STAGE_POS.x + 4.0f, ParkLayout::W8C_SYMPHONY_STAGE_POS.z, 26.0f, 44.0f, 0.65f, 0.61f, 0.55f }
        };
        glNormal3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        for (const auto& c : courts) {
            float hw = c.w * 0.5f, hd = c.d * 0.5f;
            glColor3f(c.r, c.g, c.b);
            glVertex3f(c.cx - hw, 0.023f, c.cz - hd);
            glVertex3f(c.cx - hw, 0.023f, c.cz + hd);
            glVertex3f(c.cx + hw, 0.023f, c.cz + hd);
            glVertex3f(c.cx + hw, 0.023f, c.cz - hd);
        }
        glEnd();
    }

    // ========================================================================
    // 1. NHÀ HÀNG PANDA (BP6 - 1795453362382_image.png): 26.0 (W) x 18.0 (D) x 14.0 (H) m
    //    Kiến trúc Trung Hoa cổ điển 2 tầng mái ngói lưu ly cong, hệ đấu củng, lan can gỗ tầng 2,
    //    bệ đá cao 1.2m có bậc thềm chính và 2 cầu thang nghiêng hai bên hông, tượng đôi rồng chầu trên bờ nóc.
    // ========================================================================
    static void appendPandaRestaurant(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB stoneBase (0.58f, 0.56f, 0.52f);
        const MathCG::ColorRGB stoneLight(0.74f, 0.71f, 0.66f);
        const MathCG::ColorRGB redCol    (0.76f, 0.18f, 0.14f);
        const MathCG::ColorRGB creamWall (0.93f, 0.88f, 0.76f);
        const MathCG::ColorRGB woodDark  (0.42f, 0.24f, 0.14f);
        const MathCG::ColorRGB goldRoof  (0.86f, 0.56f, 0.14f);
        const MathCG::ColorRGB brightGold(0.96f, 0.80f, 0.22f);
        const MathCG::ColorRGB bambooGrn (0.24f, 0.58f, 0.26f);

        // 1.1. Bệ đá tam cấp (26.0 x 18.0 m, y = 0.0 .. 1.25 m) + cầu thang chính mặt Nam (+Z) & Bắc (-Z) + 2 thang bên Đông/Tây
        MB::appendBoxByYRange(local, 0.0f, 0.00f, 0.45f, 0.0f, 26.0f, 18.0f, stoneBase);
        MB::appendBoxByYRange(local, 0.0f, 0.42f, 1.25f, 0.0f, 24.4f, 16.4f, stoneLight);
        MB::appendStaircase(local,   0.0f,  8.2f,  0,  1, 0.0f, 5.2f, 2.4f, 1.25f, 6, stoneLight, stoneBase, 0.30f);
        MB::appendStaircase(local,   0.0f, -8.2f,  0, -1, 0.0f, 4.4f, 2.2f, 1.25f, 6, stoneLight, stoneBase, 0.30f);
        MB::appendStaircase(local,  12.2f,  0.0f,  1,  0, 0.0f, 3.6f, 2.2f, 1.25f, 6, stoneLight, stoneBase, 0.30f);
        MB::appendStaircase(local, -12.2f,  0.0f, -1,  0, 0.0f, 3.6f, 2.2f, 1.25f, 6, stoneLight, stoneBase, 0.30f);

        // Lan can đá quanh hiên tầng 1 (y = 1.25 .. 2.05 m)
        for (int sz : {-1, 1}) {
            MB::appendBoxByYRange(local, -7.8f, 1.25f, 1.98f, sz * 7.85f, 7.6f, 0.28f, stoneBase);
            MB::appendBoxByYRange(local,  7.8f, 1.25f, 1.98f, sz * 7.85f, 7.6f, 0.28f, stoneBase);
        }

        // 1.2. Thân tầng 1 (21.6 x 13.6 m, y = 1.22 .. 5.25 m) + hệ 16 cột trụ tròn/vuông sơn đỏ + khung cửa gỗ mắt cáo
        MB::appendBoxByYRange(local, 0.0f, 1.22f, 5.25f, 0.0f, 20.2f, 12.4f, creamWall);
        const float colX[6] = { -10.8f, -6.5f, -2.4f, 2.4f, 6.5f, 10.8f };
        for (float cx : colX) {
            for (int sz : {-1, 1}) {
                MB::appendBoxByYRange(local, cx, 1.22f, 5.30f, sz * 6.8f, 0.48f, 0.48f, redCol);
            }
        }
        // Cửa đại sảnh & 4 ô cửa sổ mắt cáo tầng 1
        for (int sz : {-1, 1}) {
            MB::appendBoxByYRange(local,  0.0f, 1.25f, 4.55f, sz * 6.26f, 3.8f, 0.18f, woodDark);
            MB::appendBoxByYRange(local, -4.4f, 1.85f, 4.35f, sz * 6.26f, 2.8f, 0.16f, woodDark);
            MB::appendBoxByYRange(local,  4.4f, 1.85f, 4.35f, sz * 6.26f, 2.8f, 0.16f, woodDark);
            MB::appendBoxByYRange(local, -8.6f, 1.85f, 4.35f, sz * 6.26f, 2.6f, 0.16f, woodDark);
            MB::appendBoxByYRange(local,  8.6f, 1.85f, 4.35f, sz * 6.26f, 2.6f, 0.16f, woodDark);
        }

        // 1.3. Hệ đấu củng (bracket complex) & Mái ngói cong tầng 1 (y = 5.15 .. 7.65 m)
        MB::appendBoxByYRange(local, 0.0f, 5.10f, 5.48f, 0.0f, 22.8f, 14.8f, woodDark);
        MB::appendBoxByYRange(local, 0.0f, 5.35f, 5.62f, 0.0f, 23.6f, 15.6f, brightGold);
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 5.55f, 0.0f, 25.6f, 17.6f, 2.25f, 2.1f, goldRoof
        ));

        // 1.4. Cổ lầu & Ban công gỗ tầng 2 (16.4 x 10.4 m, y = 6.85 .. 10.35 m)
        MB::appendBoxByYRange(local, 0.0f, 6.85f, 10.30f, 0.0f, 15.6f, 9.6f, creamWall);
        // Lan can ban công tầng 2 + cột đỏ tầng 2
        MB::appendBoxByYRange(local, 0.0f, 7.20f, 7.95f, 0.0f, 17.2f, 11.2f, woodDark);
        const float col2X[4] = { -7.6f, -2.5f, 2.5f, 7.6f };
        for (float cx : col2X) {
            for (int sz : {-1, 1}) {
                MB::appendBoxByYRange(local, cx, 7.20f, 10.35f, sz * 4.85f, 0.42f, 0.42f, redCol);
            }
        }
        // Biển hiệu mạ vàng giữa tầng 2
        MB::appendBoxByYRange(local, 0.0f, 9.35f, 10.15f, 5.05f, 4.2f, 0.22f, brightGold);

        // 1.5. Đấu củng tầng 2 & Mái ngói lưu ly tầng 2 (y = 10.25 .. 13.55 m) + bờ nóc rồng chầu đạt đỉnh 14.0m
        MB::appendBoxByYRange(local, 0.0f, 10.20f, 10.55f, 0.0f, 17.2f, 11.2f, woodDark);
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 10.50f, 0.0f, 19.8f, 13.6f, 2.85f, 1.95f, goldRoof
        ));
        // Bờ nóc chính + bảo châu ở giữa + đôi rồng vàng chầu hai đầu bờ nóc (y = 13.20 .. 14.00 m)
        MB::appendBoxByYRange(local, 0.0f, 13.15f, 13.52f, 0.0f, 9.6f, 0.42f, brightGold);
        local.append(Geometry::PolygonMesh::createPyramid(0.0f, 13.52f, 0.0f, 0.65f, 0.65f, 0.52f, brightGold));
        for (int sx : {-1, 1}) {
            MB::appendBoxByYRange(local, sx * 4.4f, 13.45f, 13.95f, 0.0f, 1.15f, 0.38f, brightGold);
            MB::appendBoxByYRange(local, sx * 5.1f, 13.25f, 13.75f, 0.0f, 0.55f, 0.34f, redCol);
        }

        // Tiểu cảnh 2 bồn trúc xanh + tượng Gấu Trúc Panda trắng-đen đón khách trước bậc thềm Nam (+Z)
        for (int sx : {-1, 1}) {
            float px = sx * 4.2f, pz = 9.6f;
            MB::appendBoxByYRange(local, px, 0.00f, 0.55f, pz, 1.6f, 1.6f, stoneBase);
            // Thân gấu trúc trắng + chân/tai/đai vai đen
            MB::appendBoxByYRange(local, px, 0.55f, 1.45f, pz, 0.92f, 0.78f, MathCG::ColorRGB(0.95f, 0.95f, 0.96f));
            MB::appendBoxByYRange(local, px, 1.15f, 1.42f, pz, 0.96f, 0.82f, MathCG::ColorRGB(0.12f, 0.12f, 0.14f));
            MB::appendBoxByYRange(local, px, 1.45f, 2.02f, pz + 0.06f, 0.72f, 0.68f, MathCG::ColorRGB(0.96f, 0.96f, 0.96f));
            MB::appendBoxByYRange(local, px - 0.28f, 1.98f, 2.18f, pz + 0.06f, 0.20f, 0.18f, MathCG::ColorRGB(0.12f, 0.12f, 0.14f));
            MB::appendBoxByYRange(local, px + 0.28f, 1.98f, 2.18f, pz + 0.06f, 0.20f, 0.18f, MathCG::ColorRGB(0.12f, 0.12f, 0.14f));
            // Khóm trúc xanh cạnh tượng Panda
            local.append(Geometry::PolygonMesh::createPrismY(px + sx * 1.4f, 0.0f, pz - 0.4f, 0.12f, 3.8f, 6, bambooGrn));
        }

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_PANDA_RESTAURANT_POS,
            ParkLayout::W8C_PANDA_RESTAURANT_ROT_Y_DEG
        );
    }

    // ========================================================================
    // 2. TƯỢNG & HỒ PHUN NƯỚC MERLION (BP7 - 1795453365678_image.png): ĐK hồ 28.0m (R = 14.0m), tổng cao 10.0m
    //    Hồ nước tròn giật cấp đồng tâm (y = 0 .. 2.0m), bệ sóng biển (y = 2.0 .. 3.2m),
    //    tượng Ngư Sư Merlion trắng ngà (thân cá uốn cong có vẩy + bờm sư tử điêu khắc, y = 3.2 .. 10.0m),
    //    vòng vòi phun nước đồng tâm quanh hồ và bậc tam cấp phía Đông-Nam.
    // ========================================================================
    static void appendMerlionStatueAndPool(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB poolStone  (0.64f, 0.62f, 0.58f);
        const MathCG::ColorRGB poolTrim   (0.78f, 0.76f, 0.72f);
        const MathCG::ColorRGB waterBlue  (0.14f, 0.58f, 0.78f);
        const MathCG::ColorRGB waveStone  (0.76f, 0.82f, 0.86f);
        const MathCG::ColorRGB ivoryWhite (0.95f, 0.94f, 0.90f);
        const MathCG::ColorRGB ivoryShade (0.86f, 0.84f, 0.79f);
        const MathCG::ColorRGB nozzleGold (0.88f, 0.72f, 0.22f);

        // 2.1. Thành hồ tròn đồng tâm (R_outer = 14.0m, ĐK = 28.0m) + mặt nước xanh ngọc + bậc thềm dẫn lên
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 0.00f, 0.0f, 14.0f, 0.55f, 24, poolStone));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 0.55f, 0.0f, 13.3f, 0.35f, 24, poolTrim));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 0.88f, 0.0f, 12.6f, 0.08f, 24, waterBlue));
        // Tầng hồ phun trong (R = 6.2m, y = 0.90 .. 2.00 m)
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 0.90f, 0.0f, 6.2f, 0.65f, 20, poolStone));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 1.55f, 0.0f, 4.6f, 0.45f, 20, poolTrim));
        // Cầu thang vòng cung phía Nam (+Z)
        MB::appendStaircase(local, 0.0f, 13.2f, 0, 1, 0.0f, 4.6f, 1.8f, 0.90f, 5, poolTrim, poolStone, 0.22f);

        // 12 trụ vòi phun nước bằng đồng quanh lòng hồ (R = 8.4m và R = 5.3m)
        for (int i = 0; i < 12; ++i) {
            float a = (static_cast<float>(i) / 12.0f) * MathCG::TWO_PI;
            local.append(Geometry::PolygonMesh::createPrismY(8.4f * std::cos(a), 0.92f, 8.4f * std::sin(a), 0.18f, 0.42f, 8, nozzleGold));
        }

        // 2.2. Bệ sóng nước cuộn dưới chân tượng (y = 2.00 .. 3.20 m, R = 2.8m -> 2.1m)
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 2.00f, 0.0f, 2.85f, 0.48f, 16, waveStone));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 2.45f, 0.0f, 2.45f, 0.42f, 16, ivoryShade));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 2.84f, 0.0f, 2.05f, 0.38f, 16, waveStone));

        // 2.3. Thân cá uốn cong hình chữ S & đuôi cá phía sau + đầu & bờm sư tử uy nghi hướng về phía Nam (+Z) (y = 3.20 .. 10.00 m)
        constexpr int kSegments = 8;
        const float segY[kSegments]  = { 3.20f, 4.05f, 4.95f, 5.85f, 6.75f, 7.65f, 8.55f, 9.35f };
        const float segZ[kSegments]  = { -0.25f,-0.42f,-0.32f,-0.10f, 0.12f, 0.25f, 0.30f, 0.22f };
        const float segRX[kSegments] = { 1.48f, 1.32f, 1.24f, 1.30f, 1.44f, 1.56f, 1.52f, 1.28f };
        const float segRZ[kSegments] = { 1.55f, 1.38f, 1.30f, 1.36f, 1.52f, 1.68f, 1.65f, 1.35f };
        constexpr int kSides = 14;

        for (int r = 0; r < kSegments - 1; ++r) {
            for (int s = 0; s < kSides; ++s) {
                int sNext = (s + 1) % kSides;
                float a0 = (static_cast<float>(s)     / kSides) * MathCG::TWO_PI;
                float a1 = (static_cast<float>(sNext) / kSides) * MathCG::TWO_PI;
                float aMid = 0.5f * (a0 + a1);
                const MathCG::ColorRGB& col = ((r + s) % 2 == 0) ? ivoryWhite : ivoryShade;
                MathCG::Vec3 p00(segRX[r]     * std::cos(a0), segY[r],     segZ[r]     + segRZ[r]     * std::sin(a0));
                MathCG::Vec3 p01(segRX[r]     * std::cos(a1), segY[r],     segZ[r]     + segRZ[r]     * std::sin(a1));
                MathCG::Vec3 p11(segRX[r + 1] * std::cos(a1), segY[r + 1], segZ[r + 1] + segRZ[r + 1] * std::sin(a1));
                MathCG::Vec3 p10(segRX[r + 1] * std::cos(a0), segY[r + 1], segZ[r + 1] + segRZ[r + 1] * std::sin(a0));
                MB::appendOrientedPolygon(local, { p00, p01, p11, p10 }, col, MathCG::Vec3(std::cos(aMid), 0.0f, std::sin(aMid)));
            }
        }
        // Đỉnh đầu bờm sư tử khép kín tại y = 10.00 m
        for (int s = 0; s < kSides; ++s) {
            int sNext = (s + 1) % kSides;
            float a0 = (static_cast<float>(s)     / kSides) * MathCG::TWO_PI;
            float a1 = (static_cast<float>(sNext) / kSides) * MathCG::TWO_PI;
            MathCG::Vec3 p0(segRX[kSegments - 1] * std::cos(a0), segY[kSegments - 1], segZ[kSegments - 1] + segRZ[kSegments - 1] * std::sin(a0));
            MathCG::Vec3 p1(segRX[kSegments - 1] * std::cos(a1), segY[kSegments - 1], segZ[kSegments - 1] + segRZ[kSegments - 1] * std::sin(a1));
            MathCG::Vec3 apex(0.0f, 10.00f, 0.15f);
            MB::appendOrientedPolygon(local, { p0, p1, apex }, ivoryWhite, MathCG::Vec3(0.0f, 1.0f, 0.0f));
        }

        // Vây đuôi cá uốn lên phía sau lưng (-Z, y = 3.2 .. 5.4m) + hai chân trước sư tử + mõm sư tử phun nước hướng +Z (y = 8.1 .. 9.2m)
        MB::appendBoxByYRange(local, 0.0f, 3.20f, 5.10f, -1.55f, 1.35f, 0.65f, ivoryShade);
        MB::appendBoxByYRange(local, -0.95f, 6.20f, 7.45f, 1.35f, 0.55f, 0.85f, ivoryWhite);
        MB::appendBoxByYRange(local,  0.95f, 6.20f, 7.45f, 1.35f, 0.55f, 0.85f, ivoryWhite);
        // Mõm & hàm sư tử mở nhẹ ở cao độ y = 8.35m, z = +1.95m (nơi phóng tia nước parabol xuống hồ)
        MB::appendBoxByYRange(local, 0.0f, 8.35f, 9.05f, 1.75f, 1.25f, 0.95f, ivoryWhite);
        MB::appendBoxByYRange(local, 0.0f, 7.95f, 8.28f, 1.65f, 1.05f, 0.80f, ivoryShade);

        MB::appendRotatedTranslatedMesh(target, std::move(local), ParkLayout::W8C_MERLION_POOL_POS, 0.0f);
    }

    // ========================================================================
    // 3. THUYỀN RỒNG HOÀNG GIA (BP8 - 1795453369127_image.png): 22.0 (L) x 7.0 (W) x 11.5 (H) m
    //    Đặt nổi trên mặt hồ phía Đông Thủy đình: thân thuyền chạm vẩy rồng uốn cong hai đầu,
    //    đầu rồng chạm trổ uy nghiêm ở mũi thuyền (-X), đuôi rồng vươn cao ở lái thuyền (+X),
    //    lầu đình 2 tầng mái ngói lưu ly vàng-đỏ ở khoang giữa thuyền.
    // ========================================================================
    static void appendDragonBoat(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB hullGold  (0.86f, 0.64f, 0.16f);
        const MathCG::ColorRGB hullRed   (0.76f, 0.18f, 0.12f);
        const MathCG::ColorRGB deckWood  (0.54f, 0.34f, 0.18f);
        const MathCG::ColorRGB brightGold(0.96f, 0.82f, 0.22f);
        const MathCG::ColorRGB hornWhite (0.95f, 0.93f, 0.86f);

        // 3.1. Thân thuyền rồng dài 22.0m (X từ -11.0m tới +11.0m), rộng 7.0m ở khoang giữa, thu nhọn dần về mũi & lái
        constexpr int kHullSegs = 9;
        const float hx[kHullSegs]  = { -9.2f, -7.2f, -4.5f, -1.8f,  0.0f,  1.8f,  4.5f,  7.2f,  9.2f };
        const float hw[kHullSegs]  = {  1.3f,  2.3f,  3.3f,  3.5f,  3.5f,  3.5f,  3.3f,  2.2f,  1.2f };
        const float hy0[kHullSegs] = {  0.9f,  0.3f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.3f,  0.9f };
        const float hy1[kHullSegs] = {  2.5f,  1.9f,  1.65f, 1.60f, 1.60f, 1.60f, 1.65f, 1.9f,  2.5f };

        for (int i = 0; i < kHullSegs - 1; ++i) {
            float cx = 0.5f * (hx[i] + hx[i + 1]);
            float sx = hx[i + 1] - hx[i] + 0.08f;
            float sz = (hw[i] + hw[i + 1]);
            float yb = 0.5f * (hy0[i] + hy0[i + 1]);
            float yt = 0.5f * (hy1[i] + hy1[i + 1]);
            MB::appendBoxByYRange(local, cx, yb, yt, 0.0f, sx, sz, (i % 2 == 0) ? hullGold : hullRed);
            MB::appendBoxByYRange(local, cx, yt - 0.12f, yt + 0.14f, 0.0f, sx, sz + 0.25f, deckWood);
        }

        // 3.2. Cổ & Đầu Rồng Hoàng Gia ở mũi thuyền phía Tây (-X = -9.0 .. -11.2m, vươn cao lên y = 6.8m)
        MB::appendBoxByYRange(local, -9.4f, 2.2f, 4.4f, 0.0f, 1.6f, 1.85f, hullGold);
        MB::appendBoxByYRange(local,-10.1f, 3.8f, 5.8f, 0.0f, 1.8f, 2.05f, brightGold);
        // Hàm trên, hàm dưới, bờm đỏ & cặp sừng rồng vươn ra sau
        MB::appendBoxByYRange(local,-11.0f, 4.9f, 5.65f, 0.0f, 1.45f, 1.65f, brightGold);
        MB::appendBoxByYRange(local,-10.9f, 4.2f, 4.65f, 0.0f, 1.30f, 1.40f, hullRed);
        for (int sz : {-1, 1}) {
            MB::appendBoxByYRange(local, -9.6f, 5.75f, 6.85f, sz * 0.65f, 1.25f, 0.24f, hornWhite);
            MB::appendBoxByYRange(local, -9.2f, 4.00f, 5.60f, sz * 1.05f, 0.95f, 0.28f, hullRed);
        }

        // 3.3. Đuôi Rồng uốn lượn hình ngọn lửa ở lái thuyền phía Đông (+X = +9.0 .. +11.2m, cao lên y = 6.4m)
        MB::appendBoxByYRange(local,  9.4f, 2.2f, 4.2f, 0.0f, 1.5f, 1.55f, hullGold);
        MB::appendBoxByYRange(local, 10.2f, 3.8f, 5.6f, 0.0f, 1.5f, 1.25f, hullRed);
        local.append(Geometry::PolygonMesh::createPyramid(10.6f, 5.5f, 0.0f, 1.4f, 1.0f, 1.35f, brightGold));

        // 3.4. Lầu Hoàng Gia 2 tầng mái ngói cong giữa thuyền (X thuộc [-4.8, +4.8], rộng 5.8m, cao từ sàn 1.65m lên đỉnh 11.45m)
        // Lan can boong thuyền + 10 cột trụ đỏ tầng 1
        MB::appendBoxByYRange(local, 0.0f, 1.65f, 2.35f,  2.75f, 9.6f, 0.22f, brightGold);
        MB::appendBoxByYRange(local, 0.0f, 1.65f, 2.35f, -2.75f, 9.6f, 0.22f, brightGold);
        const float pxCols[5] = { -4.4f, -2.2f, 0.0f, 2.2f, 4.4f };
        for (float px : pxCols) {
            for (int sz : {-1, 1}) {
                MB::appendBoxByYRange(local, px, 1.65f, 5.45f, sz * 2.55f, 0.36f, 0.36f, hullRed);
            }
        }
        // Khoang vọng cảnh giữa thuyền
        MB::appendBoxByYRange(local, 0.0f, 1.65f, 5.30f, 0.0f, 4.2f, 3.2f, MathCG::ColorRGB(0.92f, 0.86f, 0.72f));
        // Cổ diềm & Mái ngói cong tầng 1 (y = 5.35 .. 7.45 m)
        MB::appendBoxByYRange(local, 0.0f, 5.30f, 5.62f, 0.0f, 10.2f, 5.8f, brightGold);
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 5.58f, 0.0f, 11.4f, 6.8f, 1.95f, 1.65f, hullRed
        ));
        // Cổ lầu tầng 2 & Mái ngói cong tầng 2 (y = 6.95 .. 11.05 m) + bờ nóc vàng
        MB::appendBoxByYRange(local, 0.0f, 6.95f, 8.85f, 0.0f, 6.4f, 3.8f, hullRed);
        MB::appendBoxByYRange(local, 0.0f, 8.75f, 9.05f, 0.0f, 6.8f, 4.2f, brightGold);
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 9.00f, 0.0f, 8.2f, 5.2f, 1.95f, 1.45f, hullGold
        ));
        MB::appendBoxByYRange(local, 0.0f, 10.85f, 11.35f, 0.0f, 3.8f, 0.30f, brightGold);

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_DRAGON_BOAT_POS,
            ParkLayout::W8C_DRAGON_BOAT_ROT_Y_DEG
        );
    }

    // ========================================================================
    // 4. TRÒ CHƠI THUYỀN RỒNG ĐU ĐƯA "THE FLYING KIRINS" (BP9 - 1795453378911_image.png): 18.0 x 12.0 x 15.0 m
    //    Đặt trên cầu cảng ven bờ Đông hồ Bắc (tách biệt hoàn toàn với PirateShip.h của thành viên nhóm theo Quyết định 3):
    //    - Phần tĩnh: Bến cầu cảng gỗ trên cọc bê tông (18.0 x 12.0 m), lan can xếp hàng, cầu thang đôi hai đầu,
    //                 4 chân trụ thép chữ A nghiêng chụm lên ổ trục cơ khí ở cao độ y = 14.5m (đỉnh 15.0m).
    //    - Phần động (kirinBoatMesh_): Thuyền chạm hai đầu Kỳ Lân / Rồng vàng có cột buồm ở giữa, lắc lư quanh trục y = 14.5m.
    // ========================================================================
    void appendFlyingKirinsStaticAndBuildBoat(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB pierConcrete(0.56f, 0.55f, 0.52f);
        const MathCG::ColorRGB pierWood    (0.54f, 0.36f, 0.20f);
        const MathCG::ColorRGB steelTeal   (0.18f, 0.46f, 0.56f);
        const MathCG::ColorRGB goldTrim    (0.94f, 0.78f, 0.20f);
        const MathCG::ColorRGB redAccent   (0.80f, 0.20f, 0.14f);

        // 4.1. Bến cầu cảng & Sàn đón khách (18.0 x 12.0 m, y = 0.0 .. 1.45 m) + 8 trụ móng bê tông
        for (int ix = -1; ix <= 1; ++ix) {
            for (int iz : {-1, 1}) {
                MB::appendBoxByYRange(local, ix * 7.2f, -0.35f, 1.10f, iz * 4.8f, 1.2f, 1.2f, pierConcrete);
            }
        }
        MB::appendBoxByYRange(local, 0.0f, 1.05f, 1.45f, 0.0f, 18.0f, 12.0f, pierWood);
        // Hai bục nhà ga đón khách hai bên hông thuyền (z = ±3.8m, y = 1.45 .. 2.35 m) + cầu thang lên xuống
        for (int sz : {-1, 1}) {
            MB::appendBoxByYRange(local, 0.0f, 1.45f, 2.35f, sz * 3.85f, 11.2f, 2.8f, pierConcrete);
            MB::appendBoxByYRange(local, 0.0f, 2.35f, 3.05f, sz * 5.10f, 11.2f, 0.18f, goldTrim);
        }
        MB::appendStaircase(local, 0.0f, 6.0f, 0, 1, 0.0f, 3.6f, 2.2f, 1.45f, 6, pierConcrete, goldTrim, 0.25f);

        // 4.2. Khung giá đỡ hình chữ A lớn (4 chân trụ thép chụm từ 4 móng (±6.8m, ±4.4m) lên ổ trục trung tâm y = 14.5m)
        const MathCG::Vec3 topHubL(0.0f, 14.5f, -2.55f);
        const MathCG::Vec3 topHubR(0.0f, 14.5f,  2.55f);
        const MathCG::Vec3 feet[4] = {
            MathCG::Vec3(-6.8f, 1.45f, -4.35f),
            MathCG::Vec3( 6.8f, 1.45f, -4.35f),
            MathCG::Vec3(-6.8f, 1.45f,  4.35f),
            MathCG::Vec3( 6.8f, 1.45f,  4.35f)
        };
        for (int i = 0; i < 4; ++i) {
            MB::appendBoxByYRange(local, feet[i].x, 1.40f, 2.35f, feet[i].z, 1.65f, 1.65f, pierConcrete);
            const MathCG::Vec3& topP = (i < 2) ? topHubL : topHubR;
            // Chia mỗi chân chữ A thành 8 đốt hộp thép nghiêng liền khối
            constexpr int kSegs = 8;
            for (int s = 0; s < kSegs; ++s) {
                float t0 = static_cast<float>(s) / kSegs;
                float t1 = static_cast<float>(s + 1) / kSegs;
                float tm = 0.5f * (t0 + t1);
                MathCG::Vec3 pm = feet[i] * (1.0f - tm) + topP * tm;
                float hSeg = (topP.y - feet[i].y) / kSegs + 0.35f;
                MB::appendBoxByYRange(local, pm.x, pm.y - hSeg * 0.5f, pm.y + hSeg * 0.5f, pm.z, 0.68f, 0.68f, steelTeal);
            }
        }
        // Xà ngang giằng chữ A ở cao độ y = 9.8m và cụm ổ trục cơ khí (Mechanical Hub) ở y = 14.0 .. 15.0m
        MB::appendBoxByYRange(local, 0.0f,  9.55f, 10.05f, -3.15f, 6.8f, 0.45f, steelTeal);
        MB::appendBoxByYRange(local, 0.0f,  9.55f, 10.05f,  3.15f, 6.8f, 0.45f, steelTeal);
        MB::appendBoxByYRange(local, 0.0f, 14.05f, 15.00f,  0.00f, 2.2f, 5.80f, redAccent);
        MB::appendBoxByYRange(local, 0.0f, 14.20f, 14.85f,  0.00f, 2.5f, 6.10f, goldTrim);

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_FLYING_KIRINS_POS,
            ParkLayout::W8C_FLYING_KIRINS_ROT_Y_DEG
        );

        // 4.3. Xây dựng lưới động của Thuyền Kỳ Lân (kirinBoatMesh_) trong hệ tọa độ tâm quay tại (0, 0, 0) (ứng với y = 14.5m)
        kirinBoatMesh_.clear();
        // Hai thanh treo cứng từ trục (y = 0) xuống thân thuyền (y = -10.2m)
        MB::appendBoxByYRange(kirinBoatMesh_, 0.0f, -10.2f, 0.0f, -1.55f, 0.32f, 0.28f, goldTrim);
        MB::appendBoxByYRange(kirinBoatMesh_, 0.0f, -10.2f, 0.0f,  1.55f, 0.32f, 0.28f, goldTrim);
        // Cột buồm & xà buồm chính giữa thuyền (y = -10.2 .. -4.2m)
        MB::appendBoxByYRange(kirinBoatMesh_, 0.0f, -10.2f, -4.2f,  0.0f, 0.36f, 0.36f, pierWood);
        MB::appendBoxByYRange(kirinBoatMesh_, 0.0f,  -6.2f, -5.9f,  0.0f, 0.28f, 3.20f, goldTrim);

        // Thân thuyền cong hình bán nguyệt dài 12.6m (X từ -6.3m tới +6.3m), hai đầu chạm đầu Kỳ Lân/Rồng đối xứng
        constexpr int kBoatSegs = 7;
        const float bx[kBoatSegs]  = { -5.4f, -3.6f, -1.8f,  0.0f,  1.8f,  3.6f,  5.4f };
        const float by0[kBoatSegs] = { -9.5f,-10.6f,-11.2f,-11.4f,-11.2f,-10.6f, -9.5f };
        const float by1[kBoatSegs] = { -7.8f, -9.1f, -9.7f, -9.9f, -9.7f, -9.1f, -7.8f };
        const float bw[kBoatSegs]  = {  1.6f,  2.4f,  2.8f,  2.9f,  2.8f,  2.4f,  1.6f };
        for (int i = 0; i < kBoatSegs - 1; ++i) {
            float cx = 0.5f * (bx[i] + bx[i + 1]);
            float sx = bx[i + 1] - bx[i] + 0.06f;
            float sz = 0.5f * (bw[i] + bw[i + 1]);
            float yb = 0.5f * (by0[i] + by0[i + 1]);
            float yt = 0.5f * (by1[i] + by1[i + 1]);
            MB::appendBoxByYRange(kirinBoatMesh_, cx, yb, yt, 0.0f, sx, sz, (i % 2 == 0) ? redAccent : pierWood);
            MB::appendBoxByYRange(kirinBoatMesh_, cx, yt - 0.10f, yt + 0.15f, 0.0f, sx, sz + 0.18f, goldTrim);
        }
        // Hai đầu Kỳ Lân Vàng đối xứng ở hai mũi thuyền (x = ±6.1m, y = -8.2 .. -5.8m)
        for (int sx : {-1, 1}) {
            MB::appendBoxByYRange(kirinBoatMesh_, sx * 5.85f, -8.2f, -6.3f, 0.0f, 1.15f, 1.35f, goldTrim);
            MB::appendBoxByYRange(kirinBoatMesh_, sx * 6.45f, -7.1f, -5.8f, 0.0f, 1.05f, 1.15f, redAccent);
            MB::appendBoxByYRange(kirinBoatMesh_, sx * 5.65f, -6.3f, -5.4f, 0.0f, 0.65f, 0.85f, MathCG::ColorRGB(0.96f, 0.94f, 0.88f));
        }
        kirinBoatMesh_.computeNormals();
    }

    // ========================================================================
    // 5. SÂN KHẤU NỔI & KHÁN ĐÀI BÁN NGUYỆT SYMPHONY OF RIVER (BP13 - 1795453392187_image.png): 42.0 x 20.0 x 12.0 m
    //    Sàn sân khấu bán nguyệt nhô ra mặt nước, cổng vòm bán nguyệt kết cấu giàn thép cao 12.0m ở tâm sân khấu,
    //    khán đài bậc thang hình quạt (amphitheater tiers) và 2 vọng lầu bát giác 2 tầng mái cong ở hai đầu cánh (±16.5m).
    // ========================================================================
    static void appendSymphonyOfRiverStage(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB deckStone  (0.68f, 0.65f, 0.59f);
        const MathCG::ColorRGB seatTierCol(0.78f, 0.74f, 0.66f);
        const MathCG::ColorRGB stageFloor (0.46f, 0.30f, 0.18f);
        const MathCG::ColorRGB archGold   (0.95f, 0.78f, 0.22f);
        const MathCG::ColorRGB archTeal   (0.16f, 0.54f, 0.68f);
        const MathCG::ColorRGB redPillar  (0.76f, 0.18f, 0.14f);
        const MathCG::ColorRGB roofTile   (0.16f, 0.48f, 0.64f);

        // 5.1. Bệ sàn ven hồ & đường dạo (42.0 x 20.0 m, y = 0.0 .. 1.15 m) + hàng cọc đỡ dưới mặt nước
        MB::appendBoxByYRange(local, 0.0f, -0.30f, 1.15f, 0.0f, 42.0f, 18.5f, deckStone);
        // Sàn biểu diễn bán nguyệt nhô về phía mặt nước (+Z = +5.0 .. +10.2m)
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 0.00f, 5.2f, 9.6f, 1.28f, 20, stageFloor));
        local.append(Geometry::PolygonMesh::createPrismY(0.0f, 1.25f, 5.2f, 9.8f, 0.12f, 20, archGold));

        // 5.2. Khán đài bậc thang hình bán nguyệt (6 bậc vòng cung từ R = 8.5m ra R = 17.5m, cao dần từ y = 1.2m lên y = 4.8m)
        constexpr int kTiers = 6;
        constexpr int kArcSegs = 16;
        for (int t = 0; t < kTiers; ++t) {
            float rIn  = 8.2f + t * 1.45f;
            float rOut = rIn + 1.50f;
            float yBot = 1.15f + t * 0.55f;
            float yTop = yBot + 0.62f;
            const MathCG::ColorRGB& tc = (t % 2 == 0) ? seatTierCol : deckStone;
            for (int s = 0; s < kArcSegs; ++s) {
                // Cung tròn phía sau sân khấu (góc từ Pi + 0.22 tới 2*Pi - 0.22, tức phía -Z)
                float a0 = MathCG::PI + 0.20f + (static_cast<float>(s)     / kArcSegs) * (MathCG::PI - 0.40f);
                float a1 = MathCG::PI + 0.20f + (static_cast<float>(s + 1) / kArcSegs) * (MathCG::PI - 0.40f);
                float am = 0.5f * (a0 + a1);
                float rMid = 0.5f * (rIn + rOut);
                MB::appendBoxByYRange(
                    local,
                    rMid * std::cos(am), yBot, yTop, 3.5f + rMid * std::sin(am),
                    1.85f, 1.55f, tc
                );
            }
        }
        // Tường hậu đài phía sau khán đài (Z = -8.5m, y = 1.15 .. 4.85m, rộng 42.0m)
        MB::appendBoxByYRange(local, 0.0f, 1.15f, 4.85f, -8.2f, 41.6f, 2.2f, deckStone);

        // 5.3. Cổng Chào Bán Nguyệt Trang Trí Sân Khấu (bán kính ngoài R = 10.8m, cao từ y = 1.25m lên đỉnh y = 12.05m)
        constexpr int kArchSegs = 18;
        for (int s = 0; s < kArchSegs; ++s) {
            float a0 = (static_cast<float>(s)     / kArchSegs) * MathCG::PI;
            float a1 = (static_cast<float>(s + 1) / kArchSegs) * MathCG::PI;
            float am = 0.5f * (a0 + a1);
            // Vòm giàn thép ngoài (R = 10.2m) & vòm ánh sáng vàng trong (R = 8.8m)
            float xOut = 10.2f * std::cos(am);
            float yOut = 1.25f + 10.2f * std::sin(am);
            MB::appendBoxByYRange(local, xOut, yOut - 0.55f, yOut + 0.55f, 2.4f, 2.05f, 0.95f, (s % 2 == 0) ? archGold : archTeal);

            float xIn = 8.6f * std::cos(am);
            float yIn = 1.25f + 8.6f * std::sin(am);
            MB::appendBoxByYRange(local, xIn, yIn - 0.42f, yIn + 0.42f, 2.5f, 1.75f, 0.75f, archGold);

            // Thanh giằng hướng tâm nối 2 vành vòm
            float xMid = 9.4f * std::cos(a0);
            float yMid = 1.25f + 9.4f * std::sin(a0);
            MB::appendBoxByYRange(local, xMid, yMid - 0.35f, yMid + 0.35f, 2.45f, 0.85f, 0.85f, archTeal);
        }

        // 5.4. Hai Vọng Lầu Nhà Mái Vòm Bát Giác ở hai đầu cánh khán đài (cx = ±16.8m, cz = -5.2m, cao từ y = 4.8m lên 11.4m)
        for (int sx : {-1, 1}) {
            float px = sx * 16.8f;
            float pz = -5.2f;
            MB::appendBoxByYRange(local, px, 1.15f, 4.85f, pz, 5.8f, 5.8f, deckStone);
            // 8 cột trụ đỏ quanh vọng lầu
            for (int c = 0; c < 8; ++c) {
                float ca = (22.5f + c * 45.0f) * MathCG::DEG2RAD;
                MB::appendBoxByYRange(local, px + 2.25f * std::cos(ca), 4.85f, 7.65f, pz + 2.25f * std::sin(ca), 0.34f, 0.34f, redPillar);
            }
            // Mái ngói cong 2 tầng của vọng lầu
            MB::appendOctagonalPrism(local, px, 7.55f, 7.85f, pz, 2.55f, archGold);
            MB::appendOctagonalRoof(local,  px, 7.82f, pz, 3.25f, 1.55f, 1.65f, roofTile, 0.28f);
            MB::appendOctagonalPrism(local, px, 9.20f, 9.85f, pz, 1.55f, redPillar);
            MB::appendOctagonalRoof(local,  px, 9.82f, pz, 2.25f, 1.45f, 0.0f,  roofTile, 0.24f);
            local.append(Geometry::PolygonMesh::createPyramid(px, 11.25f, pz, 0.45f, 0.45f, 0.55f, archGold));
        }

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_SYMPHONY_STAGE_POS,
            ParkLayout::W8C_SYMPHONY_STAGE_ROT_Y_DEG
        );
    }

    // ========================================================================
    // 6. NHÀ HÁT ÂU CƠ — MÚA RỐI NƯỚC VIỆT (BP14 - 1795453395295_image.png): 24.0 (W) x 18.0 (D) x 12.0 (H) m
    //    Khối sảnh giữa cao 12.0m có cổng vòm đón khách mái uốn cong mềm mại, hai khối cánh bậc thang
    //    thấp dần (cao 9.2m và 6.8m), hệ lam trang trí dọc mặt đứng và bậc thềm tam cấp phía trước.
    // ========================================================================
    static void appendAuCoTheater(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB plinthCol (0.56f, 0.54f, 0.50f);
        const MathCG::ColorRGB wallCream (0.94f, 0.89f, 0.78f);
        const MathCG::ColorRGB redWood   (0.74f, 0.18f, 0.14f);
        const MathCG::ColorRGB louverWood(0.52f, 0.32f, 0.18f);
        const MathCG::ColorRGB roofRed   (0.80f, 0.24f, 0.14f);
        const MathCG::ColorRGB goldCol   (0.95f, 0.78f, 0.20f);
        const MathCG::ColorRGB waterPool (0.14f, 0.54f, 0.68f);

        // 6.1. Bệ đá (24.0 x 18.0 m, y = 0.0 .. 1.10 m) + bậc thềm chính mặt Nam (+Z) + hồ nước biểu diễn rối nước trước cổng
        MB::appendBoxByYRange(local, 0.0f, 0.00f, 0.45f, 0.0f, 24.0f, 18.0f, plinthCol);
        MB::appendBoxByYRange(local, 0.0f, 0.42f, 1.10f, 0.0f, 22.4f, 16.4f, MathCG::ColorRGB(0.72f, 0.69f, 0.64f));
        MB::appendStaircase(local, 0.0f,  8.2f, 0,  1, 0.0f, 6.4f, 2.4f, 1.10f, 6, wallCream, plinthCol, 0.28f);
        MB::appendStaircase(local, 0.0f, -8.2f, 0, -1, 0.0f, 5.2f, 2.0f, 1.10f, 6, wallCream, plinthCol, 0.28f);

        // 6.2. Khối trung tâm cao 12.0m (rộng 10.6m, sâu 14.2m) + 2 khối cánh giữa (rộng 17.8m) + 2 hiên cánh ngoài (rộng 22.4m)
        MB::appendBoxByYRange(local, 0.0f, 1.08f, 8.85f, 0.0f, 10.4f, 13.6f, wallCream);
        MB::appendBoxByYRange(local, 0.0f, 1.08f, 6.55f, 0.0f, 17.6f, 12.2f, wallCream);
        MB::appendBoxByYRange(local, 0.0f, 1.08f, 4.65f, 0.0f, 21.8f, 10.4f, wallCream);

        // Hệ lam gỗ dọc mặt tiền (panel screen / vertical louvers)
        for (int sx : {-1, 1}) {
            for (int k = 0; k < 5; ++k) {
                float lx = sx * (5.6f + k * 0.62f);
                MB::appendBoxByYRange(local, lx, 1.35f, 6.20f, 6.15f, 0.18f, 0.22f, louverWood);
            }
            for (int k = 0; k < 4; ++k) {
                float lx = sx * (9.2f + k * 0.45f);
                MB::appendBoxByYRange(local, lx, 1.35f, 4.40f, 5.25f, 0.16f, 0.20f, louverWood);
            }
        }

        // 6.3. Cổng vòm bán nguyệt đón khách ở chính giữa mặt tiền (+Z = +6.85m, R = 2.8m) + mái đón uốn cong
        auto frontArch = Geometry::PolygonMesh::createArchVault(
            0.0f, 1.10f, 6.55f, 7.35f,
            2.45f, 3.35f, 14,
            redWood
        );
        local.append(frontArch);
        // Mặt nước hồ thủy đình rối nước ngay dưới vòm cổng
        MB::appendBoxByYRange(local, 0.0f, 1.05f, 1.16f, 6.95f, 4.6f, 1.4f, waterPool);
        // Mái đón cong trên cổng vòm (y = 4.75 .. 6.10 m)
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 4.75f, 7.15f, 7.6f, 3.2f, 1.35f, 1.15f, roofRed
        ));

        // 6.4. Ba tầng mái ngói đỏ đầu đao cong vút giật cấp từ hai cánh lên đỉnh trung tâm 12.0m
        // Mái hiên hai cánh ngoài (y = 4.60 .. 6.25 m)
        for (int sx : {-1, 1}) {
            local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                sx * 9.2f, 4.60f, 0.0f, 6.2f, 11.6f, 1.55f, 1.25f, roofRed
            ));
            // Mái khối cánh giữa (y = 6.50 .. 8.45 m)
            local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                sx * 6.2f, 6.50f, 0.0f, 7.8f, 13.6f, 1.85f, 1.45f, roofRed
            ));
        }
        // Mái chính khối trung tâm + cổ lầu đỉnh cao 12.0m
        MB::appendBoxByYRange(local, 0.0f, 8.75f, 9.10f, 0.0f, 11.2f, 14.2f, goldCol);
        local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, 9.05f, 0.0f, 12.6f, 15.6f, 2.65f, 1.85f, roofRed
        ));
        MB::appendBoxByYRange(local, 0.0f, 11.55f, 12.00f, 0.0f, 5.8f, 0.36f, goldCol);

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_AU_CO_THEATER_POS,
            ParkLayout::W8C_AU_CO_THEATER_ROT_Y_DEG
        );
    }

    // ========================================================================
    // 7. ĐẢO KHỦNG LONG — DINO ISLAND (BP16 - 1795453400510_image.png): 28.0 (W) x 22.0 (D) x 10.0 (H) m
    //    Cổng chào Đảo Khủng Long bằng trụ đá/gỗ cao 6.2m ở phía Tây-Nam,
    //    Núi lửa giả tạo cao 10.0m ở phía Đông-Bắc có miệng núi lửa đỏ rực,
    //    3 mô hình khủng long 3D đa giác: Khủng long cổ dài Brachiosaurus (cao 8.8m),
    //    Khủng long bạo chúa T-Rex (cao 5.4m) và Triceratops 3 sừng kèm hàng cây cọ tiền sử.
    // ========================================================================
    static void appendDinoIsland(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB rockDark  (0.36f, 0.31f, 0.26f);
        const MathCG::ColorRGB rockMid   (0.48f, 0.41f, 0.34f);
        const MathCG::ColorRGB rockLight (0.58f, 0.50f, 0.41f);
        const MathCG::ColorRGB lavaOrange(0.96f, 0.34f, 0.10f);
        const MathCG::ColorRGB gateWood  (0.46f, 0.28f, 0.14f);
        const MathCG::ColorRGB signGold  (0.92f, 0.74f, 0.20f);
        const MathCG::ColorRGB brachiCol (0.34f, 0.62f, 0.38f);
        const MathCG::ColorRGB trexCol   (0.68f, 0.36f, 0.22f);
        const MathCG::ColorRGB triceCol  (0.42f, 0.54f, 0.64f);
        const MathCG::ColorRGB palmTrunk (0.48f, 0.34f, 0.20f);
        const MathCG::ColorRGB palmLeaf  (0.18f, 0.56f, 0.22f);

        // 7.1. Bệ đảo đá giật cấp (28.0 x 22.0 m) + hàng rào gỗ bảo vệ
        MB::appendBoxByYRange(local, 0.0f, 0.00f, 0.45f, 0.0f, 27.6f, 21.6f, rockDark);
        MB::appendBoxByYRange(local, 2.5f, 0.42f, 1.35f, -1.2f, 21.0f, 16.5f, rockMid);

        // 7.2. Cổng chào Đảo Khủng Long (ở góc Tây-Nam: cx = -7.8m, cz = +7.2m, rộng 6.8m, cao 6.2m)
        for (int sx : {-1, 1}) {
            MB::appendBoxByYRange(local, -7.8f + sx * 2.8f, 0.40f, 6.10f, 7.2f, 1.35f, 1.35f, gateWood);
            MB::appendBoxByYRange(local, -7.8f + sx * 2.8f, 0.40f, 2.20f, 7.2f, 1.65f, 1.65f, rockLight);
            // Đuốc lửa trên đỉnh 2 trụ cổng
            local.append(Geometry::PolygonMesh::createPyramid(-7.8f + sx * 2.8f, 6.10f, 7.2f, 0.75f, 0.75f, 0.65f, lavaOrange));
        }
        // Đà ngang bảng tên "DINO ISLAND" uốn vòm trên cổng
        MB::appendBoxByYRange(local, -7.8f, 4.65f, 5.85f, 7.2f, 7.2f, 0.85f, gateWood);
        MB::appendBoxByYRange(local, -7.8f, 4.85f, 5.65f, 7.68f, 5.4f, 0.16f, signGold);

        // 7.3. Núi lửa giả tạo cao 10.0m (ở phía Đông-Bắc đảo: cx = +4.2m, cz = -2.2m, ĐK chân 14.5m -> miệng núi lửa R = 2.1m ở y = 10.0m)
        constexpr int kVolRings = 5;
        const float volY[kVolRings] = { 1.20f, 3.40f, 5.80f, 8.10f, 10.00f };
        const float volR[kVolRings] = { 7.20f, 5.60f, 4.10f, 2.90f,  1.95f };
        constexpr int kVolSides = 12;
        for (int r = 0; r < kVolRings - 1; ++r) {
            for (int s = 0; s < kVolSides; ++s) {
                int sNext = (s + 1) % kVolSides;
                float a0 = (static_cast<float>(s)     / kVolSides) * MathCG::TWO_PI;
                float a1 = (static_cast<float>(sNext) / kVolSides) * MathCG::TWO_PI;
                float aMid = 0.5f * (a0 + a1);
                // Tạo gân sườn núi lửa gồ ghề
                float jitter0 = (s % 2 == 0) ? 1.04f : 0.94f;
                float jitter1 = (sNext % 2 == 0) ? 1.04f : 0.94f;
                const MathCG::ColorRGB& vc = (r == kVolRings - 2 && s % 3 == 0) ? lavaOrange : ((r + s) % 2 == 0 ? rockMid : rockDark);
                MathCG::Vec3 p00(4.2f + volR[r]     * jitter0 * std::cos(a0), volY[r],     -2.2f + volR[r]     * jitter0 * std::sin(a0));
                MathCG::Vec3 p01(4.2f + volR[r]     * jitter1 * std::cos(a1), volY[r],     -2.2f + volR[r]     * jitter1 * std::sin(a1));
                MathCG::Vec3 p11(4.2f + volR[r + 1] * jitter1 * std::cos(a1), volY[r + 1], -2.2f + volR[r + 1] * jitter1 * std::sin(a1));
                MathCG::Vec3 p10(4.2f + volR[r + 1] * jitter0 * std::cos(a0), volY[r + 1], -2.2f + volR[r + 1] * jitter0 * std::sin(a0));
                MB::appendOrientedPolygon(local, { p00, p01, p11, p10 }, vc, MathCG::Vec3(std::cos(aMid), 0.3f, std::sin(aMid)));
            }
        }
        // Hồ dung nham đỏ cam trong miệng núi lửa tại y = 9.85m
        local.append(Geometry::PolygonMesh::createPrismY(4.2f, 9.65f, -2.2f, 1.85f, 0.35f, 12, lavaOrange));

        // 7.4. Mô hình Khủng Long Cổ Dài Brachiosaurus (đứng cạnh sườn Tây núi lửa: cx = -2.6m, cz = +1.2m, vươn cổ cao tới y = 8.95m)
        {
            const float bx = -2.6f, bz = 1.2f;
            // 4 chân trụ lớn
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    MB::appendBoxByYRange(local, bx + sx * 1.15f, 1.30f, 3.25f, bz + sz * 0.55f, 0.58f, 0.58f, brachiCol);
                }
            }
            // Thân mình + đuôi vươn về phía Tây (-X)
            MB::appendBoxByYRange(local, bx,        3.05f, 4.45f, bz, 3.10f, 1.65f, brachiCol);
            MB::appendBoxByYRange(local, bx - 2.2f, 2.65f, 3.55f, bz, 1.85f, 0.95f, brachiCol);
            MB::appendBoxByYRange(local, bx - 3.6f, 2.25f, 2.85f, bz, 1.45f, 0.55f, brachiCol);
            // Cổ dài 4 đốt vươn cao lên y = 8.95m về phía Đông (+X) + đầu nhỏ trên đỉnh
            MB::appendBoxByYRange(local, bx + 1.35f, 4.20f, 5.65f, bz, 0.85f, 0.82f, brachiCol);
            MB::appendBoxByYRange(local, bx + 1.85f, 5.45f, 6.95f, bz, 0.72f, 0.70f, brachiCol);
            MB::appendBoxByYRange(local, bx + 2.25f, 6.75f, 8.35f, bz, 0.62f, 0.60f, brachiCol);
            MB::appendBoxByYRange(local, bx + 2.75f, 8.15f, 8.95f, bz, 1.15f, 0.68f, brachiCol);
        }

        // 7.5. Mô hình Khủng Long Bạo Chúa T-Rex (đứng trên bậc đá phía Đông: cx = +9.4m, cz = +3.2m, cao tới y = 6.2m)
        {
            const float tx = 9.4f, tz = 3.2f;
            MB::appendBoxByYRange(local, tx, 1.30f, 2.20f, tz, 4.2f, 3.2f, rockLight);
            // Hai chân sau khỏe + thân nghiêng + đuôi dài + đầu lớn hàm răng mở về phía Tây (-X)
            MB::appendBoxByYRange(local, tx + 0.3f, 2.20f, 4.05f, tz - 0.55f, 0.75f, 0.65f, trexCol);
            MB::appendBoxByYRange(local, tx + 0.3f, 2.20f, 4.05f, tz + 0.55f, 0.75f, 0.65f, trexCol);
            MB::appendBoxByYRange(local, tx,        3.85f, 5.15f, tz,         2.45f, 1.45f, trexCol);
            MB::appendBoxByYRange(local, tx + 2.1f, 3.55f, 4.35f, tz,         2.20f, 0.85f, trexCol);
            MB::appendBoxByYRange(local, tx - 1.3f, 4.85f, 6.15f, tz,         1.65f, 1.25f, trexCol);
            MB::appendBoxByYRange(local, tx - 1.8f, 4.45f, 4.75f, tz,         1.35f, 1.05f, rockLight);
        }

        // 7.6. Mô hình Khủng Long 3 Sừng Triceratops (ở góc Đông-Nam: cx = +3.8m, cz = +7.0m) + 4 cây cọ tiền sử quanh đảo
        {
            const float cx = 3.8f, cz = 7.0f;
            MB::appendBoxByYRange(local, cx,        0.45f, 2.35f, cz, 2.85f, 1.55f, triceCol);
            MB::appendBoxByYRange(local, cx - 1.5f, 1.15f, 2.95f, cz, 0.45f, 2.15f, signGold); // diềm cổ
            MB::appendBoxByYRange(local, cx - 2.1f, 0.95f, 2.25f, cz, 1.15f, 1.15f, triceCol);
        }
        const float palmCoords[4][2] = { {-11.5f, 5.5f}, {-10.8f, -6.5f}, {11.2f, -6.8f}, {11.5f, 6.8f} };
        for (int i = 0; i < 4; ++i) {
            float px = palmCoords[i][0], pz = palmCoords[i][1];
            local.append(Geometry::PolygonMesh::createPrismY(px, 0.45f, pz, 0.26f, 4.2f, 8, palmTrunk));
            local.append(Geometry::PolygonMesh::createPyramid(px, 4.55f, pz, 2.8f, 2.8f, 1.45f, palmLeaf));
        }

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_DINO_ISLAND_POS,
            ParkLayout::W8C_DINO_ISLAND_ROT_Y_DEG
        );
    }

    // ========================================================================
    // 8. VƯỜN TRÚC NHẬT BẢN — BAMBOO GARDEN (BP17 - 1795453403998_image.png): 24.0 (W) x 20.0 (D) x 9.0 (H) m
    //    Cổng thần đạo Torii sơn đỏ cao 6.8m ở phía Tây-Nam, dòng suối cá Koi uốn lượn ở trung tâm,
    //    Cầu gỗ cong sơn đỏ (Taiko-bashi) bắc qua suối, Nhà trà đạo Nhật Bản (Chashitsu) mái cong cao 6.5m
    //    ở phía Đông, đèn đá Tōrō và 4 rặng trúc xanh cao 9.0m ở 4 góc vườn.
    // ========================================================================
    static void appendBambooGarden(Geometry::PolygonMesh& target) {
        namespace MB = Geometry::MeshBuilders;
        Geometry::PolygonMesh local;

        const MathCG::ColorRGB gardenBase (0.58f, 0.56f, 0.49f);
        const MathCG::ColorRGB pebblePath (0.76f, 0.73f, 0.66f);
        const MathCG::ColorRGB koiStream  (0.14f, 0.52f, 0.68f);
        const MathCG::ColorRGB koiOrange  (0.96f, 0.42f, 0.12f);
        const MathCG::ColorRGB toriiRed   (0.84f, 0.16f, 0.12f);
        const MathCG::ColorRGB toriiBlack (0.16f, 0.16f, 0.18f);
        const MathCG::ColorRGB teaWall    (0.93f, 0.89f, 0.78f);
        const MathCG::ColorRGB teaWood    (0.46f, 0.30f, 0.18f);
        const MathCG::ColorRGB slateRoof  (0.26f, 0.30f, 0.34f);
        const MathCG::ColorRGB stoneLantern(0.68f, 0.66f, 0.62f);
        const MathCG::ColorRGB bambooStalk(0.24f, 0.62f, 0.26f);
        const MathCG::ColorRGB bambooLeaf (0.16f, 0.48f, 0.20f);

        // 8.1. Nền vườn & Dòng suối cá Koi ở giữa (24.0 x 20.0 m) + đá cuội ven suối
        MB::appendBoxByYRange(local, 0.0f, 0.00f, 0.18f, 0.0f, 23.6f, 19.6f, gardenBase);
        MB::appendBoxByYRange(local, -0.5f, 0.16f, 0.26f, 0.0f, 8.8f, 15.2f, koiStream);
        // Đàn cá Koi cam-trắng bơi dưới mặt suối
        const float koiPos[5][2] = { {-1.2f, -3.5f}, {0.6f, -1.8f}, {-0.8f, 2.4f}, {1.1f, 4.2f}, {-1.5f, 5.2f} };
        for (int i = 0; i < 5; ++i) {
            MB::appendBoxByYRange(local, koiPos[i][0], 0.22f, 0.30f, koiPos[i][1], 0.35f, 0.75f, koiOrange);
        }

        // 8.2. Cổng Torii Nhật Bản cao 6.8m (ở phía Tây vườn: cx = -6.8m, cz = +4.5m, rộng 5.2m)
        {
            const float tx = -6.8f, tz = 4.5f;
            for (int sx : {-1, 1}) {
                // Chân trụ đá đen + cột trụ tròn đỏ cao 6.4m
                local.append(Geometry::PolygonMesh::createPrismY(tx + sx * 1.75f, 0.18f, tz, 0.36f, 0.65f, 10, toriiBlack));
                local.append(Geometry::PolygonMesh::createPrismY(tx + sx * 1.75f, 0.80f, tz, 0.28f, 5.55f, 12, toriiRed));
            }
            // Xà ngang dưới (Nuki, y = 5.10 .. 5.42m) & Xà ngang trên cong hai đầu (Kasagi, y = 6.20 .. 6.80m)
            MB::appendBoxByYRange(local, tx, 5.10f, 5.42f, tz, 4.8f, 0.34f, toriiRed);
            MB::appendBoxByYRange(local, tx, 5.42f, 6.22f, tz, 0.42f, 0.36f, toriiRed);
            MB::appendBoxByYRange(local, tx, 6.18f, 6.58f, tz, 5.6f, 0.52f, toriiRed);
            MB::appendBoxByYRange(local, tx, 6.52f, 6.78f, tz, 6.0f, 0.58f, toriiBlack);
        }

        // 8.3. Cầu gỗ cong Nhật Bản (Taiko-bashi) bắc ngang qua dòng suối Koi (X từ -4.2m sang +3.2m tại Z = 0.0m)
        constexpr int kBridgeSegs = 7;
        for (int i = 0; i < kBridgeSegs; ++i) {
            float t = (static_cast<float>(i) + 0.5f) / kBridgeSegs;
            float bx = -4.0f + 7.2f * t;
            float archY = 0.25f + 1.15f * std::sin(t * MathCG::PI);
            MB::appendBoxByYRange(local, bx, archY, archY + 0.22f, 0.0f, 1.12f, 2.4f, pebblePath);
            // Lan can đỏ hai bên thành cầu cong
            for (int sz : {-1, 1}) {
                MB::appendBoxByYRange(local, bx, archY + 0.22f, archY + 0.92f, sz * 1.12f, 0.16f, 0.16f, toriiRed);
                MB::appendBoxByYRange(local, bx, archY + 0.82f, archY + 0.98f, sz * 1.12f, 1.15f, 0.18f, toriiRed);
            }
        }

        // 8.4. Nhà Trà Đạo Nhật Bản (Chashitsu) ở phía Đông suối (cx = +6.6m, cz = -1.2m, 5.8 x 5.8m, cao 6.5m)
        {
            const float hx = 6.6f, hz = -1.2f;
            MB::appendBoxByYRange(local, hx, 0.18f, 0.85f, hz, 6.2f, 6.2f, stoneLantern);
            MB::appendBoxByYRange(local, hx, 0.82f, 1.05f, hz, 6.6f, 6.6f, teaWood); // hiên gỗ Engawa
            MB::appendBoxByYRange(local, hx, 1.05f, 3.85f, hz, 4.4f, 4.4f, teaWall); // vách giấy Shoji
            for (int sx : {-1, 1}) {
                for (int sz : {-1, 1}) {
                    MB::appendBoxByYRange(local, hx + sx * 2.75f, 1.05f, 3.95f, hz + sz * 2.75f, 0.28f, 0.28f, teaWood);
                }
            }
            MB::appendBoxByYRange(local, hx, 3.82f, 4.08f, hz, 6.2f, 6.2f, teaWood);
            local.append(Geometry::PolygonMesh::createCurvedAsianRoof(
                hx, 4.05f, hz, 7.4f, 7.4f, 2.25f, 1.65f, slateRoof
            ));
            local.append(Geometry::PolygonMesh::createPyramid(hx, 6.25f, hz, 0.48f, 0.48f, 0.45f, toriiRed));
        }

        // 8.5. Hai đèn đá Tōrō hai bên đầu cầu + 4 rặng trúc xanh cao 9.0m ở 4 góc vườn
        for (int sz : {-1, 1}) {
            float lx = -4.4f, lz = sz * 2.2f;
            MB::appendBoxByYRange(local, lx, 0.18f, 1.65f, lz, 0.32f, 0.32f, stoneLantern);
            MB::appendBoxByYRange(local, lx, 1.65f, 2.15f, lz, 0.58f, 0.58f, teaWall);
            local.append(Geometry::PolygonMesh::createPyramid(lx, 2.15f, lz, 0.92f, 0.92f, 0.45f, stoneLantern));
        }

        const float groveCorners[4][2] = { {-9.6f, -7.4f}, {-9.6f, 7.4f}, {9.6f, -7.4f}, {9.6f, 7.4f} };
        const float stalkOffsets[5][3] = {
            { 0.0f,  0.0f, 8.8f }, { -0.85f,  0.65f, 7.9f }, { 0.85f, -0.65f, 8.4f },
            { 0.75f, 0.75f, 7.4f }, { -0.75f, -0.75f, 9.0f }
        };
        for (int g = 0; g < 4; ++g) {
            float gx = groveCorners[g][0], gz = groveCorners[g][1];
            for (int s = 0; s < 5; ++s) {
                float sx = gx + stalkOffsets[s][0];
                float sz = gz + stalkOffsets[s][1];
                float sh = stalkOffsets[s][2];
                local.append(Geometry::PolygonMesh::createPrismY(sx, 0.18f, sz, 0.14f, sh, 6, bambooStalk));
                local.append(Geometry::PolygonMesh::createPyramid(sx, sh * 0.72f, sz, 1.45f, 1.45f, sh * 0.30f, bambooLeaf));
            }
        }

        MB::appendRotatedTranslatedMesh(
            target, std::move(local),
            ParkLayout::W8C_BAMBOO_GARDEN_POS,
            ParkLayout::W8C_BAMBOO_GARDEN_ROT_Y_DEG
        );
    }

    void buildAllStaticMeshes() {
        staticMesh_.clear();
        staticMesh_.name = "Wave8cMasterPlanMesh";

        appendPandaRestaurant(staticMesh_);
        appendMerlionStatueAndPool(staticMesh_);
        appendDragonBoat(staticMesh_);
        appendFlyingKirinsStaticAndBuildBoat(staticMesh_);
        appendSymphonyOfRiverStage(staticMesh_);
        appendAuCoTheater(staticMesh_);
        appendDinoIsland(staticMesh_);
        appendBambooGarden(staticMesh_);

        staticMesh_.computeNormals();
    }

    void drawDynamicAndEmissiveElements(bool isNightOrSunset, bool showNormals, bool smoothShading) const {
        // 1. Thuyền Rồng Đu Đưa "The Flying Kirins" lắc lư quanh ổ trục y = 14.5m
        glPushMatrix();
        glTranslatef(
            ParkLayout::W8C_FLYING_KIRINS_POS.x,
            ParkLayout::W8C_FLYING_KIRINS_POS.y + 14.5f,
            ParkLayout::W8C_FLYING_KIRINS_POS.z
        );
        glRotatef(ParkLayout::W8C_FLYING_KIRINS_ROT_Y_DEG, 0.0f, 1.0f, 0.0f);
        glRotatef(kirinSwingDeg, 0.0f, 0.0f, 1.0f);
        if (!showNormals && kirinBoatList_ != 0) {
            glCallList(kirinBoatList_);
        } else {
            kirinBoatMesh_.draw(smoothShading, showNormals);
        }
        glPopMatrix();

        // 2. Tia nước parabol phun ra từ miệng tượng Merlion xuống lòng hồ + 12 tia nước vòi phun đồng tâm
        {
            GLboolean wasLit = glIsEnabled(GL_LIGHTING);
            glDisable(GL_LIGHTING);
            glColor3f(0.78f, 0.94f, 1.00f);
            glLineWidth(2.4f);

            const MathCG::Vec3 mp = ParkLayout::W8C_MERLION_POOL_POS;
            const float pulse = 0.18f * std::sin(animTime * 4.5f);

            // Chùm 3 tia nước chính từ miệng sư tử Merlion (y = 8.55m, z = +2.15m) phóng ra z = +10.2m, y = 0.96m
            for (int j = -1; j <= 1; ++j) {
                glBegin(GL_LINE_STRIP);
                for (int k = 0; k <= 16; ++k) {
                    float t = static_cast<float>(k) / 16.0f;
                    float x = mp.x + j * 0.22f * (1.0f + t);
                    float z = mp.z + 2.15f + (8.1f + pulse) * t;
                    float y = 8.55f * (1.0f - t * t) + 0.96f * (t * t) + 1.15f * t * (1.0f - t);
                    glVertex3f(x, y, z);
                }
                glEnd();
            }

            // 12 tia vòi phun xung quanh hồ hướng vào tâm
            glLineWidth(1.6f);
            for (int i = 0; i < 12; ++i) {
                float a = (static_cast<float>(i) / 12.0f) * MathCG::TWO_PI;
                float c = std::cos(a), s = std::sin(a);
                glBegin(GL_LINE_STRIP);
                for (int k = 0; k <= 10; ++k) {
                    float t = static_cast<float>(k) / 10.0f;
                    float r = 8.4f - 3.6f * t;
                    float y = 1.32f + (2.65f + 0.25f * std::sin(animTime * 5.0f + i)) * 4.0f * t * (1.0f - t);
                    glVertex3f(mp.x + r * c, y, mp.z + r * s);
                }
                glEnd();
            }
            glLineWidth(1.0f);
            if (wasLit) glEnable(GL_LIGHTING);
        }

        // 3. Hiệu ứng đèn phát sáng ban đêm (GL_EMISSION, không dùng thêm GL_LIGHTi) cho Cổng vòm Symphony of River,
        //    Miệng núi lửa Dino Island và Đèn lồng Nhà hàng Panda / Thuyền Rồng
        if (isNightOrSunset) {
            GLfloat goldEmit[4] = { 1.0f, 0.85f, 0.35f, 1.0f };
            GLfloat lavaEmit[4] = { 1.0f, 0.32f, 0.08f, 1.0f };
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

            // Quả cầu lửa dung nham nhấp nhô trên đỉnh miệng núi lửa Dino Island
            glPushMatrix();
            glTranslatef(
                ParkLayout::W8C_DINO_ISLAND_POS.x + 4.2f,
                10.15f + 0.25f * std::sin(animTime * 3.2f),
                ParkLayout::W8C_DINO_ISLAND_POS.z - 2.2f
            );
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, lavaEmit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, lavaEmit);
            glColor3f(1.0f, 0.35f, 0.10f);
            ParametricShapes::drawSphere(1.15f, 12, 10, false);
            glPopMatrix();

            // Dải đèn LED vàng rực dọc Cổng vòm bán nguyệt Symphony of River
            glPushMatrix();
            glTranslatef(
                ParkLayout::W8C_SYMPHONY_STAGE_POS.x,
                ParkLayout::W8C_SYMPHONY_STAGE_POS.y,
                ParkLayout::W8C_SYMPHONY_STAGE_POS.z
            );
            glRotatef(ParkLayout::W8C_SYMPHONY_STAGE_ROT_Y_DEG, 0.0f, 1.0f, 0.0f);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, goldEmit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, goldEmit);
            glColor3f(1.0f, 0.88f, 0.42f);
            for (int i = 0; i <= 14; ++i) {
                float a = (static_cast<float>(i) / 14.0f) * MathCG::PI;
                glPushMatrix();
                glTranslatef(9.4f * std::cos(a), 1.25f + 9.4f * std::sin(a), 2.95f);
                ParametricShapes::drawSphere(0.24f, 8, 6, false);
                glPopMatrix();
            }
            glPopMatrix();

            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
    }
};

} // namespace AsiaParkRides

#endif // MASTER_PLAN_8C_H
