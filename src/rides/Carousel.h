#ifndef CAROUSEL_H
#define CAROUSEL_H

/**
 * @file Carousel.h
 * @brief Chương 4 & 5: Mô phỏng Đu Quay Ngựa Gỗ Lễ Hội (Festival Carousel tại Asia Park)
 *
 * Triển khai:
 *  - Thông số thực tế của Đu Quay Lễ Hội (Festival Carousel):
 *      * Bán kính R = 8.5m, Chiều cao H = 8.8m
 *      * Vị trí: (45.0, 0.0, 48.0) tại Quảng trường Lễ hội phía Đông Nam
 *      * Các chú ngựa gỗ phi nước đại xếp trên 2 vòng đồng tâm (Vòng ngoài R=6.2m, Vòng trong R=4.2m)
 *  - Chương 4 Mô hình hóa phân cấp (Cấu trúc phân cấp biến đổi 2 cấp):
 *      * Cấp 1 (Cha - Parent): Sàn gỗ tròn xoay, trụ lõi gương trung tâm
 *        và mái vòm hình nón lễ hội cùng quay quanh trục Y thế giới theo góc gamma = carouselAngleDeg:
 *        M_parent = T(pos) * R_y(gamma)
 *      * Cấp 2 (Con - Child): Các cột đồng thau và các chú ngựa kế thừa góc quay cha gamma,
 *        cộng thêm dao động điều hòa thẳng đứng độc lập dọc theo từng cột:
 *        y_i(t) = y_base + A * sin(omega * t + phi_i)
 *        với pha lệch nhau phi_i = i * (2*pi / N) tạo nên hiệu ứng lượn sóng phi nước đại sống động!
 *  - Chương 5 Mặt cong tham số:
 *      * Mái vòm hình nón lớn: ParametricShapes::drawConeY
 *      * Các cột đồng thau thẳng đứng: ParametricShapes::drawCylinderY
 *      * Vòng viền mạ vàng: ParametricShapes::drawToroidY
 *      * Quả cầu trang trí trên đỉnh mái: ParametricShapes::drawSphere
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

class Carousel {
public:
    // Vị trí dùng tạm từ ứng viên mái nón kẻ sọc 16.03923°N, 108.22689°E [Chưa xác minh]
    MathCG::Vec3 position       = ParkLayout::CAROUSEL_POS_UNVERIFIED; // (-159.4, 0.0, +102.5)
    float        platformRadius = 8.5f;   // ĐK 17.0m theo vệ tinh (tỷ lệ ~0.71 so với bản vẽ BP10 ĐK 24.0m)
    float        canopyHeight   = 10.0f;  // Đợt 8b: Tổng chiều cao ~10.0m (14.0m * 0.71)
    float        rotationSpeed  = 35.0f;  // độ/giây
    float        currentAngleDeg= 0.0f;   // gamma: góc quay của sàn đu quay
    float        oscillationPhase= 0.0f;  // omega * t: pha nhấp nhô của ngựa gỗ
    int          numHorses      = 12;     // 12 chú ngựa gỗ tầng 1
    int          numUpperHorses = 8;      // Đợt 8b: 8 chú ngựa gỗ tầng 2 (đu quay 2 tầng kiểu Venice BP10)

    Carousel() = default;

    /**
     * @brief Cập nhật bộ đếm thời gian mô phỏng cho cả chuyển động quay và nhịp nhấp nhô của ngựa
     */
    void update(float dt) {
        currentAngleDeg  = std::fmod(currentAngleDeg + rotationSpeed * dt, 360.0f);
        oscillationPhase = std::fmod(oscillationPhase + 3.2f * dt, MathCG::TWO_PI);
    }

    /**
     * @brief Lấy tọa độ thế giới của yên ngựa phục vụ camera góc nhìn thứ nhất trên lưng ngựa
     */
    MathCG::Vec3 getHorseWorldPosition(int horseIndex = 0) const {
        if (numHorses <= 0) return position;
        float horseAngleDeg = currentAngleDeg + (360.0f / numHorses) * horseIndex;
        float rad = horseAngleDeg * MathCG::DEG2RAD;
        float ringR = (horseIndex % 2 == 0) ? 6.2f : 4.2f;
        float phaseOffset = (MathCG::TWO_PI / numHorses) * horseIndex;
        float horseY = 1.35f + 0.45f * std::sin(oscillationPhase + phaseOffset);

        return MathCG::Vec3(
            position.x + ringR * std::cos(rad),
            position.y + horseY + 0.85f,
            position.z + ringR * std::sin(rad)
        );
    }

    /**
     * @brief Vẽ toàn bộ tổ hợp Đu Quay Ngựa Gỗ Lễ Hội 2 tầng kiểu Venice (Đợt 8b: thu tỷ lệ ~0.71 từ BP10,
     *        giữ ĐK ~17m tại (-159.4, 0, +102.5), tổng cao ~10.0m, có cầu thang lên tầng 2, hàng rào & quầy vé trong R <= 8.8m)
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        if (!showNormals) {
            ensureDisplayLists();
        }

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // ====================================================================
        // 1. BỆ NỀN TRÒN TĨNH, HÀNG RÀO BẢO VỆ & QUẦY VÉ MINI (GÓI TRONG R <= 8.80M)
        // ====================================================================
        if (!showNormals && staticBaseList_ != 0) {
            glCallList(staticBaseList_);
        } else {
            drawStaticBaseAndTicketBooth(showNormals);
        }

        // ====================================================================
        // 2. CẤP 1 (CHA - PARENT): KHỐI ĐU QUAY 2 TẦNG KIỂU VENICE QUAY QUANH TRỤC Y
        // ====================================================================
        glPushMatrix();
        glRotatef(currentAngleDeg, 0.0f, 1.0f, 0.0f);

        if (!showNormals && rotatingStructureList_ != 0) {
            glCallList(rotatingStructureList_);
        } else {
            drawRotatingTwoTierStructure(showNormals);
        }

        // Dải đèn LED vàng ấm quanh 2 tầng diềm mái (tầng 1 y = 3.85m, tầng 2 y = 7.20m) vào ban đêm / hoàng hôn
        if (isNightOrSunset) {
            GLfloat emit[4] = { 1.0f, 0.90f, 0.45f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
            glColor3f(1.0f, 0.92f, 0.50f);
            for (int i = 0; i < 28; ++i) {
                float u = (static_cast<float>(i) / 28) * MathCG::TWO_PI;
                glPushMatrix();
                glTranslatef((platformRadius + 0.12f) * std::cos(u), 3.82f, (platformRadius + 0.12f) * std::sin(u));
                ParametricShapes::drawSphere(0.11f, 8, 6, false);
                glPopMatrix();
            }
            for (int i = 0; i < 20; ++i) {
                float u = (static_cast<float>(i) / 20) * MathCG::TWO_PI;
                glPushMatrix();
                glTranslatef(6.52f * std::cos(u), 7.18f, 6.52f * std::sin(u));
                ParametricShapes::drawSphere(0.10f, 8, 6, false);
                glPopMatrix();
            }
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            LightingSystem::resetMaterial();
        }

        // ====================================================================
        // 3. CẤP 2 (CON - CHILDREN): 12 NGỰA GỖ TẦNG 1 + 8 NGỰA GỖ TẦNG 2
        // ====================================================================
        // 3.1. 12 Ngựa gỗ tầng 1 (2 vòng đồng tâm R = 6.15m và R = 4.35m, sàn y = 0.55m -> trần tầng 1 y = 3.78m)
        for (int i = 0; i < numHorses; ++i) {
            float angleDeg    = (360.0f / numHorses) * i;
            float ringR       = (i % 2 == 0) ? 6.15f : 4.35f;
            float phaseOffset = (MathCG::TWO_PI / numHorses) * i;
            float horseY      = 1.25f + 0.36f * std::sin(oscillationPhase + phaseOffset);

            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(ringR, 0.55f, 0.0f);

            glColor3f(0.92f, 0.78f, 0.22f);
            ParametricShapes::drawCylinderY(0.062f, 3.25f, 10, 2, true, false);

            glTranslatef(0.0f, horseY - 0.55f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            if (!showNormals && horseLists_[i % 3] != 0) {
                glCallList(horseLists_[i % 3]);
            } else {
                drawCarouselHorse(i, isNightOrSunset);
            }
            glPopMatrix();
        }

        // 3.2. 8 Ngựa gỗ tầng 2 (vòng R = 4.55m trên sàn tầng 2 y = 4.02m -> trần tầng 2 y = 7.12m, thu tỷ lệ 0.82)
        for (int i = 0; i < numUpperHorses; ++i) {
            float angleDeg    = (360.0f / numUpperHorses) * i + 22.5f;
            float ringR       = 4.55f;
            float phaseOffset = (MathCG::TWO_PI / numUpperHorses) * i + MathCG::PI;
            float horseY      = 4.65f + 0.28f * std::sin(oscillationPhase + phaseOffset);

            glPushMatrix();
            glRotatef(angleDeg, 0.0f, 1.0f, 0.0f);
            glTranslatef(ringR, 4.02f, 0.0f);

            glColor3f(0.92f, 0.78f, 0.22f);
            ParametricShapes::drawCylinderY(0.055f, 3.10f, 10, 2, true, false);

            glTranslatef(0.0f, horseY - 4.02f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.82f, 0.82f, 0.82f);
            if (!showNormals && horseLists_[(i + 1) % 3] != 0) {
                glCallList(horseLists_[(i + 1) % 3]);
            } else {
                drawCarouselHorse(i + 1, isNightOrSunset);
            }
            glPopMatrix();
        }

        glPopMatrix(); // Kết thúc khối Cấp 1 (Cha)
        glPopMatrix();
    }

private:
    mutable GLuint staticBaseList_        = 0;
    mutable GLuint rotatingStructureList_ = 0;
    mutable GLuint horseLists_[3]         = { 0, 0, 0 };

    void ensureDisplayLists() const {
        if (staticBaseList_ == 0) {
            staticBaseList_ = glGenLists(1);
            if (staticBaseList_ != 0) {
                glNewList(staticBaseList_, GL_COMPILE);
                drawStaticBaseAndTicketBooth(false);
                glEndList();
            }
        }
        if (rotatingStructureList_ == 0) {
            rotatingStructureList_ = glGenLists(1);
            if (rotatingStructureList_ != 0) {
                glNewList(rotatingStructureList_, GL_COMPILE);
                drawRotatingTwoTierStructure(false);
                glEndList();
            }
        }
        for (int c = 0; c < 3; ++c) {
            if (horseLists_[c] == 0) {
                horseLists_[c] = glGenLists(1);
                if (horseLists_[c] != 0) {
                    glNewList(horseLists_[c], GL_COMPILE);
                    drawCarouselHorse(c, false);
                    glEndList();
                }
            }
        }
    }

    /**
     * @brief Phần bệ tĩnh, hàng rào bảo vệ tròn và quầy vé kiểu Venice gói gọn trong bán kính R <= 8.80m
     */
    void drawStaticBaseAndTicketBooth(bool showNormals) const {
        namespace MB = Geometry::MeshBuilders;
        glColor3f(0.50f, 0.52f, 0.56f);
        ParametricShapes::drawCylinderY(platformRadius + 0.25f, 0.22f, 32, 2, true, showNormals);
        glColor3f(0.62f, 0.58f, 0.52f);
        ParametricShapes::drawCylinderY(platformRadius + 0.08f, 0.35f, 32, 2, true, false);
        glColor3f(0.88f, 0.72f, 0.18f);
        ParametricShapes::drawToroidY(platformRadius + 0.26f, 0.08f, 32, 6, false);

        // Hàng rào bảo vệ quanh bệ (R = 8.68m, chừa cổng vào phía Bắc-Đông và vùng quầy vé)
        Geometry::PolygonMesh fenceAndBooth("CarouselFenceAndTicketBooth");
        const MathCG::ColorRGB goldFence(0.90f, 0.74f, 0.20f);
        const MathCG::ColorRGB boothCream(0.93f, 0.88f, 0.76f);
        const MathCG::ColorRGB boothRed(0.82f, 0.18f, 0.14f);
        const MathCG::ColorRGB boothTrim(0.36f, 0.22f, 0.14f);

        constexpr int kFenceSegs = 28;
        const float fenceR = platformRadius + 0.18f; // 8.68m
        for (int i = 0; i < kFenceSegs; ++i) {
            float deg0 = -180.0f + (360.0f / kFenceSegs) * i;
            float deg1 = deg0 + (360.0f / kFenceSegs);
            float mid  = 0.5f * (deg0 + deg1);
            // Chừa cổng vào phía Bắc (-90 độ) và góc đặt quầy vé phía Đông-Nam (+35..+62 độ)
            if (std::fabs(mid + 90.0f) < 22.0f || (mid > 32.0f && mid < 64.0f)) continue;
            float a0 = deg0 * MathCG::DEG2RAD;
            float a1 = deg1 * MathCG::DEG2RAD;
            float x0 = fenceR * std::cos(a0), z0 = fenceR * std::sin(a0);
            float x1 = fenceR * std::cos(a1), z1 = fenceR * std::sin(a1);
            fenceAndBooth.append(Geometry::PolygonMesh::createPrismY(x0, 0.22f, z0, 0.08f, 0.78f, 6, goldFence));
            MB::appendOrientedPolygon(fenceAndBooth, {
                MathCG::Vec3(x0, 0.86f, z0), MathCG::Vec3(x1, 0.86f, z1),
                MathCG::Vec3(x1, 0.96f, z1), MathCG::Vec3(x0, 0.96f, z0)
            }, goldFence, MathCG::Vec3(std::cos(mid * MathCG::DEG2RAD), 0.2f, std::sin(mid * MathCG::DEG2RAD)));
        }

        // Quầy vé nhỏ kiểu Venice (quầy vé BP10) đặt ở góc Đông-Nam sân lát ngoài vòng quay (góc 45 độ, R_center = 9.35m,
        // cách mép sàn quay R=8.48m là 0.25m, nằm hoàn toàn phía Nam nên cách PATH_WEST_SPINE phía Bắc > 19m)
        {
            const float ba = 45.0f * MathCG::DEG2RAD;
            const float bx = 9.35f * std::cos(ba);
            const float bz = 9.35f * std::sin(ba);
            fenceAndBooth.append(Geometry::PolygonMesh::createPrismY(bx, 0.00f, bz, 0.62f, 0.24f, 8, boothTrim));
            fenceAndBooth.append(Geometry::PolygonMesh::createPrismY(bx, 0.24f, bz, 0.54f, 1.05f, 8, boothCream));
            fenceAndBooth.append(Geometry::PolygonMesh::createPrismY(bx, 1.29f, bz, 0.56f, 0.62f, 8, boothTrim)); // ô kính quầy vé
            fenceAndBooth.append(Geometry::PolygonMesh::createPrismY(bx, 1.91f, bz, 0.62f, 0.16f, 8, goldFence));
            fenceAndBooth.append(Geometry::PolygonMesh::createCurvedAsianRoof(bx, 2.07f, bz, 1.35f, 1.35f, 0.68f, 0.22f, boothRed));
            fenceAndBooth.append(Geometry::PolygonMesh::createPyramid(bx, 2.72f, bz, 0.20f, 0.20f, 0.32f, goldFence));
        }

        fenceAndBooth.computeNormals();
        fenceAndBooth.draw(true, showNormals);
    }

    /**
     * @brief Cấu trúc 2 tầng kiểu Venice đang quay: Sàn tầng 1, Trụ lõi 2 tầng, 2 Cầu thang cong lên tầng 2,
     *        Sàn & Lan can tầng 2 (y = 4.02m), Mái hiên tầng 1, Hàng cột ngoại vi và Mái vòm vương miện tầng 2 (đỉnh y = 10.0m)
     */
    void drawRotatingTwoTierStructure(bool showNormals) const {
        namespace MB = Geometry::MeshBuilders;

        // 1. Sàn gỗ tầng 1 (R = 8.35m, y = 0.0 .. 0.55m) & Trụ lõi chạm khắc trung tâm tầng 1 + tầng 2
        glColor3f(0.68f, 0.42f, 0.22f);
        ParametricShapes::drawCylinderY(platformRadius - 0.10f, 0.55f, 32, 2, true, showNormals);
        glColor3f(0.95f, 0.80f, 0.15f);
        ParametricShapes::drawToroidY(platformRadius - 0.06f, 0.10f, 32, 8, false);

        // Lõi trung tâm tầng 1 (R = 1.95m, y = 0.55 .. 3.80m) & tầng 2 (R = 1.45m, y = 4.00 .. 7.15m)
        glColor3f(0.90f, 0.86f, 0.78f);
        ParametricShapes::drawCylinderY(1.95f, 3.85f, 20, 4, true, showNormals);
        glPushMatrix();
        glTranslatef(0.0f, 3.85f, 0.0f);
        ParametricShapes::drawCylinderY(1.42f, 3.35f, 16, 4, true, showNormals);
        glPopMatrix();

        glColor3f(0.94f, 0.76f, 0.16f);
        const float coreRingsY[] = { 1.35f, 2.55f, 3.65f, 4.85f, 6.05f };
        for (float ry : coreRingsY) {
            glPushMatrix();
            glTranslatef(0.0f, ry, 0.0f);
            ParametricShapes::drawToroidY((ry < 3.8f) ? 2.00f : 1.46f, 0.10f, 20, 6, false);
            glPopMatrix();
        }

        // 2. Hai cầu thang cong đối xứng dẫn từ sàn tầng 1 (y = 0.55m) lên sàn tầng 2 (y = 4.02m) ôm quanh lõi trung tâm
        //    cùng 12 cột ngoại vi tầng 1, lan can tầng 2 và 10 cột ngoại vi tầng 2
        Geometry::PolygonMesh deckMesh("CarouselTwoTierDeckAndStairs");
        const MathCG::ColorRGB woodStep(0.64f, 0.38f, 0.18f);
        const MathCG::ColorRGB goldTrim(0.95f, 0.78f, 0.16f);
        const MathCG::ColorRGB creamPanel(0.94f, 0.89f, 0.78f);
        const MathCG::ColorRGB crimsonRed(0.86f, 0.16f, 0.14f);

        constexpr int kSteps = 11;
        const float rInner = 2.02f, rOuter = 3.08f;
        for (int flight = 0; flight < 2; ++flight) {
            float baseDeg = flight * 180.0f;
            for (int s = 0; s < kSteps; ++s) {
                float u0 = static_cast<float>(s) / kSteps;
                float u1 = static_cast<float>(s + 1) / kSteps;
                float a0 = (baseDeg + u0 * 105.0f) * MathCG::DEG2RAD;
                float a1 = (baseDeg + u1 * 105.0f) * MathCG::DEG2RAD;
                float yTop = 0.55f + (3.47f * static_cast<float>(s + 1) / kSteps);
                float yBot = std::max(0.55f, yTop - 0.45f);
                float xm = 0.5f * (rInner + rOuter) * std::cos(0.5f * (a0 + a1));
                float zm = 0.5f * (rInner + rOuter) * std::sin(0.5f * (a0 + a1));
                MB::appendBoxByYRange(deckMesh, xm, yBot, yTop, zm, 0.95f, 0.55f, (s % 2 == 0) ? woodStep : goldTrim);
                // Tay vịn cột nhỏ mép ngoài cầu thang
                float xo = rOuter * std::cos(a1), zo = rOuter * std::sin(a1);
                deckMesh.append(Geometry::PolygonMesh::createPrismY(xo, yTop, zo, 0.055f, 0.68f, 6, goldTrim));
            }
        }

        // 12 Cột hoa văn ngoại vi tầng 1 (R = 7.95m, y = 0.55 .. 3.78m) & 10 cột ngoại vi tầng 2 (R = 5.95m, y = 4.02 .. 7.12m)
        for (int i = 0; i < 12; ++i) {
            float a = (static_cast<float>(i) / 12) * MathCG::TWO_PI;
            float px = 7.95f * std::cos(a), pz = 7.95f * std::sin(a);
            deckMesh.append(Geometry::PolygonMesh::createPrismY(px, 0.55f, pz, 0.11f, 3.23f, 8, goldTrim));
            deckMesh.append(Geometry::PolygonMesh::createPrismY(px, 0.55f, pz, 0.18f, 0.35f, 8, crimsonRed));
        }
        for (int i = 0; i < 10; ++i) {
            float a = (static_cast<float>(i) / 10) * MathCG::TWO_PI;
            float px = 5.95f * std::cos(a), pz = 5.95f * std::sin(a);
            deckMesh.append(Geometry::PolygonMesh::createPrismY(px, 4.02f, pz, 0.10f, 3.10f, 8, goldTrim));
            deckMesh.append(Geometry::PolygonMesh::createPrismY(px, 4.02f, pz, 0.16f, 0.30f, 8, crimsonRed));
        }

        // Lan can con tiện quanh mép sàn tầng 2 (R = 6.18m, y = 4.02 .. 4.78m)
        constexpr int kUpperBalusters = 28;
        const float balR = 6.18f;
        for (int i = 0; i < kUpperBalusters; ++i) {
            float a0 = (static_cast<float>(i) / kUpperBalusters) * MathCG::TWO_PI;
            float a1 = (static_cast<float>(i + 1) / kUpperBalusters) * MathCG::TWO_PI;
            float x0 = balR * std::cos(a0), z0 = balR * std::sin(a0);
            float x1 = balR * std::cos(a1), z1 = balR * std::sin(a1);
            deckMesh.append(Geometry::PolygonMesh::createPrismY(x0, 4.02f, z0, 0.065f, 0.72f, 6, goldTrim));
            MB::appendOrientedPolygon(deckMesh, {
                MathCG::Vec3(x0, 4.66f, z0), MathCG::Vec3(x1, 4.66f, z1),
                MathCG::Vec3(x1, 4.78f, z1), MathCG::Vec3(x0, 4.78f, z0)
            }, ((i % 2 == 0) ? goldTrim : creamPanel), MathCG::Vec3(std::cos(0.5f * (a0 + a1)), 0.2f, std::sin(0.5f * (a0 + a1))));
        }

        deckMesh.computeNormals();
        deckMesh.draw(true, showNormals);

        // 3. Sàn tầng 2 & Diềm mái hiên tầng 1 (y = 3.72 .. 4.18m, diềm ngoài R = 8.48m, sàn trên R = 6.38m)
        glPushMatrix();
        glTranslatef(0.0f, 3.72f, 0.0f);
        glColor3f(0.94f, 0.76f, 0.16f);
        ParametricShapes::drawCylinderY(platformRadius - 0.02f, 0.32f, 32, 2, true, false);
        glColor3f(0.86f, 0.16f, 0.14f);
        glTranslatef(0.0f, 0.30f, 0.0f);
        // Vành mái nghiêng chuyển tiếp từ hiên tầng 1 (R = 8.48m) thu vào mép sàn tầng 2 (R = 6.38m)
        ParametricShapes::drawConeY(platformRadius - 0.02f, 6.38f, 0.42f, 32, 2, false, showNormals);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 3.75f, 0.0f);
        glColor3f(0.66f, 0.40f, 0.20f);
        ParametricShapes::drawCylinderY(6.38f, 0.28f, 28, 2, true, showNormals);
        glPopMatrix();

        // 4. Hệ mái vòm tầng 2 kiểu Venice & Đỉnh vương miện (y = 7.08 .. 10.00m)
        glPushMatrix();
        glTranslatef(0.0f, 7.08f, 0.0f);

        // Cổ diềm hoa văn tầng 2 (y = 7.08 .. 7.52m, R = 6.45m)
        glColor3f(0.95f, 0.76f, 0.15f);
        ParametricShapes::drawCylinderY(6.45f, 0.44f, 28, 2, true, false);
        glColor3f(0.94f, 0.90f, 0.80f);
        ParametricShapes::drawToroidY(6.48f, 0.09f, 32, 6, false);

        // Mái vòm nón chính tầng 2 (y = 7.52 .. 9.02m, R = 6.45m -> 1.15m)
        glTranslatef(0.0f, 0.44f, 0.0f);
        glColor3f(0.88f, 0.18f, 0.14f);
        ParametricShapes::drawConeY(6.45f, 1.15f, 1.50f, 28, 4, true, showNormals);

        // Lầu đèn vương miện (cupola lantern) trên đỉnh mái (y = 9.02 .. 9.44m)
        glTranslatef(0.0f, 1.50f, 0.0f);
        glColor3f(0.95f, 0.88f, 0.72f);
        ParametricShapes::drawCylinderY(1.05f, 0.42f, 16, 2, true, false);

        // Chóp nón vàng & quả cầu đỉnh tại y = 10.00m
        glTranslatef(0.0f, 0.42f, 0.0f);
        glColor3f(0.98f, 0.82f, 0.18f);
        ParametricShapes::drawConeY(1.12f, 0.0f, 0.36f, 16, 2, true, false);
        glTranslatef(0.0f, 0.38f, 0.0f);
        ParametricShapes::drawSphere(0.22f, 14, 10, false); // đỉnh y = 7.08 + 0.44 + 1.50 + 0.42 + 0.38 + 0.22 = 10.04m (~10.0m)

        glPopMatrix();
    }
    /**
     * @brief Mô hình 3D chi tiết của Ngựa gỗ với yên cương hoa văn, bờm và tư thế phi nước đại
     */
    void drawCarouselHorse(int index, bool isNightOrSunset) const {
        (void)isNightOrSunset;

        // Màu lông ngựa: xen kẽ giữa Trắng tuyết, Nâu hạt dẻ và Đen tuyền
        MathCG::ColorRGB coatCol;
        if (index % 3 == 0)      coatCol = MathCG::ColorRGB(0.95f, 0.95f, 0.97f); // Trắng
        else if (index % 3 == 1) coatCol = MathCG::ColorRGB(0.55f, 0.28f, 0.16f); // Nâu hạt dẻ
        else                     coatCol = MathCG::ColorRGB(0.18f, 0.18f, 0.20f); // Đen

        LightingSystem::setMaterial(0.55f, 32.0f);

        // 1. Thân mình ngựa
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        drawBox(0.0f, 0.0f, 0.0f, 0.55f, 0.45f, 1.35f);

        // 2. Phần mông / thân sau
        drawBox(0.0f, 0.06f, -0.65f, 0.50f, 0.42f, 0.45f);

        // 3. Ngực & vai trước
        drawBox(0.0f, 0.08f, 0.62f, 0.52f, 0.48f, 0.45f);

        // 4. Cổ ngựa uốn cong
        glPushMatrix();
        glTranslatef(0.0f, 0.48f, 0.75f);
        glRotatef(38.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.32f, 0.65f, 0.38f);
        glPopMatrix();

        // 5. Đầu & Mõm ngựa
        glPushMatrix();
        glTranslatef(0.0f, 0.85f, 0.98f);
        glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.0f, 0.0f, 0.30f, 0.32f, 0.42f); // Phần đầu
        // Phần mõm thon gọn
        glTranslatef(0.0f, -0.10f, 0.26f);
        drawBox(0.0f, 0.0f, 0.0f, 0.22f, 0.22f, 0.28f);
        // Hai tai ngựa
        glColor3f(coatCol.r * 0.9f, coatCol.g * 0.9f, coatCol.b * 0.9f);
        drawBox(-0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        drawBox( 0.10f, 0.26f, -0.15f, 0.08f, 0.18f, 0.08f);
        glPopMatrix();

        // 6. Bờm ngựa màu vàng óng
        glColor3f(0.96f, 0.78f, 0.15f); // Bờm vàng
        glPushMatrix();
        glTranslatef(0.0f, 0.62f, 0.65f);
        glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, 0.18f, -0.12f, 0.12f, 0.55f, 0.18f);
        glPopMatrix();

        // 7. Yên ngựa mạ vàng & Thảm lót yên màu đỏ thẫm
        // Thảm lót yên màu đỏ thẫm
        glColor3f(0.85f, 0.12f, 0.14f);
        drawBox(0.0f, 0.18f, 0.0f, 0.58f, 0.14f, 0.75f);
        // Yên ngựa mạ vàng
        glColor3f(0.96f, 0.80f, 0.18f);
        drawBox(0.0f, 0.26f, 0.02f, 0.48f, 0.12f, 0.52f);
        // Gờ sau & mỏm trước của yên ngựa
        drawBox(0.0f, 0.35f, -0.20f, 0.42f, 0.16f, 0.10f);
        drawBox(0.0f, 0.35f,  0.22f, 0.38f, 0.14f, 0.09f);

        // 8. Cặp chân trước đang phi nước đại (vươn về phía trước)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.20f, -0.18f, 0.60f);
            glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // Vươn ra trước
            drawBox(0.0f, -0.22f, 0.0f, 0.12f, 0.48f, 0.14f);
            glTranslatef(0.0f, -0.42f, 0.0f);
            glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.10f, 0.42f, 0.11f);
            // Móng ngựa mạ vàng
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.12f, 0.10f, 0.14f);
            glPopMatrix();
        }

        // 9. Cặp chân sau đang đạp bật nhảy (duỗi về phía sau)
        glColor3f(coatCol.r, coatCol.g, coatCol.b);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.19f, -0.16f, -0.62f);
            glRotatef(-35.0f, 1.0f, 0.0f, 0.0f); // Duỗi ra sau
            drawBox(0.0f, -0.22f, 0.0f, 0.14f, 0.50f, 0.16f);
            glTranslatef(0.0f, -0.46f, 0.0f);
            glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
            drawBox(0.0f, -0.18f, 0.0f, 0.11f, 0.42f, 0.12f);
            // Móng ngựa mạ vàng
            glColor3f(0.92f, 0.76f, 0.18f);
            drawBox(0.0f, -0.40f, 0.0f, 0.13f, 0.10f, 0.15f);
            glPopMatrix();
        }

        // 10. Đuôi ngựa màu vàng tung bay
        glColor3f(0.96f, 0.78f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, 0.10f, -0.85f);
        glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
        drawBox(0.0f, -0.25f, 0.0f, 0.12f, 0.55f, 0.14f);
        glPopMatrix();

        LightingSystem::resetMaterial();
    }

    static void drawBox(float cx, float cy, float cz, float sx, float sy, float sz) {
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

#endif // CAROUSEL_H
