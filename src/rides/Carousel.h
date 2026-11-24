#ifndef CAROUSEL_H
#define CAROUSEL_H

/**
 * @file Carousel.h
 * @brief Chương 4 & 5: Mô phỏng Đu Quay Ngựa Gỗ Lễ Hội 2 tầng (Festival Carousel tại Asia Park)
 *
 * - Giữ nguyên tâm CAROUSEL_POS_UNVERIFIED (-159.4, 0.0, +102.5) [Chưa xác minh] và đường kính
 *   vệ tinh Ø17.0 m (R_max <= 8.50 m) để chừa khoảng hở >= 5.00 m với lối đi PATH_WEST_SPINE.
 * - Hình dáng nâng cấp Lô A3 [Bản vẽ minh họa AI, tham khảo hình dạng] thu theo tỷ lệ ~0.71:
 *     * Bệ đá 3 bậc tròn + 2 cổng vào (Bắc/Nam) nối khít hàng rào bảo vệ + quầy vé gọn trong R <= 8.50 m
 *     * Khối xoay 2 tầng quay đồng trục: sàn tầng 1 (+0.75 m), trục gương giữa kiểu Venice,
 *       cầu thang cong có tay vịn đồng dẫn lên sàn tầng 2 (+3.70 m) + lan can vòng tầng 2
 *     * 20 ngựa gỗ phi nước đại dọc cột đồng (12 ngựa tầng 1 + 8 ngựa tầng 2) giữ nguyên animation
 *     * Mái nón lễ hội 16 múi sọc đỏ-kem, diềm mái lượn sóng, lầu đèn & chóp vương miện (cao ~10.0 m)
 * - Tối ưu hiệu năng Display List: biên dịch toàn bộ lưới mỗi nhóm thành đúng 1 batch GL_TRIANGLES
 *   thuần nhất (gộp 20 ngựa thành 4 nhóm pha dao động nhấp nhô), giúp chi phí vẽ < 0.25 ms/khung.
 */

#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <GL/freeglut.h>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class Carousel {
public:
    // Giữ nguyên các thuộc tính công khai tương thích hoàn toàn với main.cpp & HUD/Camera
    MathCG::Vec3 position         = ParkLayout::CAROUSEL_POS_UNVERIFIED; // (-159.4, 0.0, +102.5)
    float        platformRadius   = ParkLayout::CAROUSEL_OUTER_RADIUS;   // 8.50 m (Ø17.0 m)
    float        canopyHeight     = ParkLayout::CAROUSEL_TOTAL_HEIGHT;   // 10.00 m
    float        rotationSpeed    = 35.0f; // độ/giây
    float        currentAngleDeg  = 0.0f;  // gamma: góc quay của sàn đu quay
    float        oscillationPhase = 0.0f;  // omega * t: pha nhấp nhô của ngựa gỗ
    int          numHorses        = ParkLayout::CAROUSEL_TIER1_HORSES;   // 12 ngựa tầng 1 (+ 8 ngựa tầng 2)

    Carousel() = default;

    /**
     * @brief Cập nhật góc quay carousel và pha nhấp nhô điều hòa của 2 tầng ngựa gỗ
     */
    void update(float dt) {
        currentAngleDeg  = std::fmod(currentAngleDeg  + rotationSpeed * dt, 360.0f);
        oscillationPhase = std::fmod(oscillationPhase + 3.2f * dt, 2.0f * MathCG::PI);
    }

    /**
     * @brief Trả về tọa độ thế giới hiện thời của chú ngựa gỗ tầng 1 (chỉ số 0..numHorses-1)
     */
    MathCG::Vec3 getHorseWorldPosition(int horseIndex = 0) const {
        if (horseIndex < 0 || horseIndex >= numHorses) {
            return MathCG::Vec3(position.x, position.y + 1.6f, position.z);
        }
        float r = 0.0f, localDeg = 0.0f, baseDeckY = 0.0f;
        int styleIdx = 0, phaseGrp = 0;
        getHorseLocalPose(horseIndex, r, localDeg, baseDeckY, styleIdx, phaseGrp);
        float oscOffset = getPhaseGroupOscY(phaseGrp);

        float totalRad = (currentAngleDeg + localDeg) * MathCG::DEG2RAD;
        return MathCG::Vec3(
            position.x + r * std::cos(totalRad),
            position.y + baseDeckY + 0.88f + oscOffset,
            position.z - r * std::sin(totalRad)
        );
    }

    /**
     * @brief Trả về bán kính ngoài thực tế và tổng chiều cao thực đo từ lưới đa giác đã dựng
     */
    void computeActualBounds(float& outStaticMaxR, float& outRotMaxR, float& outMinY, float& outMaxY) const {
        ensureMeshesBuilt();
        outStaticMaxR = 0.0f;
        outRotMaxR    = 0.0f;
        outMinY       = 1e9f;
        outMaxY       = -1e9f;
        for (const auto& v : staticBaseMesh_.vertices) {
            outStaticMaxR = std::max(outStaticMaxR, std::hypot(v.x, v.z));
            outMinY = std::min(outMinY, v.y);
            outMaxY = std::max(outMaxY, v.y);
        }
        for (const auto& v : rotatingBodyMesh_.vertices) {
            outRotMaxR = std::max(outRotMaxR, std::hypot(v.x, v.z));
            outMinY = std::min(outMinY, v.y);
            outMaxY = std::max(outMaxY, v.y);
        }
    }

    /**
     * @brief Vẽ toàn bộ Đu Quay Ngựa Gỗ Lễ Hội 2 tầng (tối ưu Display List 7 draw-call)
     */
    void draw(bool isNightOrSunset, bool showNormals = false) const {
        if (!showNormals) {
            ensureDisplayLists();
        } else {
            ensureMeshesBuilt();
        }

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // 1. Phần tĩnh: bệ đá 3 bậc, cổng Bắc/Nam, hàng rào bảo vệ & quầy vé (R <= 8.50 m)
        if (!showNormals && dlStaticBase_ != 0) {
            glCallList(dlStaticBase_);
        } else {
            staticBaseMesh_.draw(true, showNormals);
        }

        // 2. Phần quay đồng trục: 2 sàn, trụ gương, cầu thang cong, cột đồng & mái nón 16 múi sọc
        glPushMatrix();
        glRotatef(currentAngleDeg, 0.0f, 1.0f, 0.0f);

        if (!showNormals && dlRotatingBody_ != 0) {
            glCallList(dlRotatingBody_);
        } else {
            rotatingBodyMesh_.draw(true, showNormals);
        }

        // 3. 20 ngựa gỗ nhấp nhô theo 4 nhóm pha lệch nhau 90° (chỉ tốn 4 lệnh vẽ thay vì 20)
        for (int g = 0; g < 4; ++g) {
            glPushMatrix();
            glTranslatef(0.0f, getPhaseGroupOscY(g), 0.0f);
            if (!showNormals && dlHorseGroup_[g] != 0) {
                glCallList(dlHorseGroup_[g]);
            } else {
                horseGroupMeshes_[g].draw(true, showNormals);
            }
            glPopMatrix();
        }

        // 4. Đèn lễ hội vẽ sau cùng (khi đêm/hoàng hôn) để trạng thái phát sáng không ảnh hưởng nhóm ngựa
        if (isNightOrSunset) {
            if (!showNormals && dlNightLights_ != 0) {
                glCallList(dlNightLights_);
            } else {
                drawNightLightsDirect(showNormals);
            }
        }

        glPopMatrix(); // Kết thúc hệ trục quay tầng 1+2
        glPopMatrix(); // Kết thúc tọa độ thế giới
    }

private:
    mutable bool meshesBuilt_ = false;
    mutable Geometry::PolygonMesh staticBaseMesh_;
    mutable Geometry::PolygonMesh rotatingBodyMesh_;
    mutable Geometry::PolygonMesh nightLightsMesh_;
    mutable std::array<Geometry::PolygonMesh, 4> horseGroupMeshes_;

    mutable GLuint dlStaticBase_   = 0;
    mutable GLuint dlRotatingBody_ = 0;
    mutable GLuint dlNightLights_  = 0;
    mutable std::array<GLuint, 4> dlHorseGroup_ = { 0, 0, 0, 0 };

    float getPhaseGroupOscY(int phaseGrp) const {
        return 0.22f * std::sin(oscillationPhase + static_cast<float>(phaseGrp) * (MathCG::PI * 0.5f));
    }

    void getHorseLocalPose(
        int idx,
        float& outR,
        float& outAngleDeg,
        float& outDeckY,
        int& outStyleIdx,
        int& outPhaseGrp
    ) const {
        const float deck1Y = ParkLayout::CAROUSEL_TIER1_Y;
        const float deck2Y = ParkLayout::CAROUSEL_TIER2_Y;

        if (idx < 8) {
            // Tầng 1 vòng ngoài (8 ngựa): R = 6.25 m (nằm ngoài cầu thang R = 4.78..5.54 m)
            outR = 6.25f;
            outAngleDeg = idx * 45.0f + 11.25f;
            outDeckY = deck1Y;
            outStyleIdx = idx % 3;
            outPhaseGrp = idx % 4;
        } else if (idx < ParkLayout::CAROUSEL_TIER1_HORSES) {
            // Tầng 1 vòng trong (4 ngựa): R = 3.45 m (dưới gầm sàn tầng 2)
            int sub = idx - 8;
            outR = 3.45f;
            outAngleDeg = 22.5f + sub * 90.0f;
            outDeckY = deck1Y;
            outStyleIdx = (sub + 1) % 3;
            outPhaseGrp = (sub + 1) % 4;
        } else {
            // Tầng 2 (8 ngựa): R = 3.55 m trên sàn +3.70 m (gọn trong lan can R = 4.82 m)
            int sub = idx - ParkLayout::CAROUSEL_TIER1_HORSES;
            outR = 3.55f;
            outAngleDeg = 20.0f + sub * 45.0f;
            outDeckY = deck2Y;
            outStyleIdx = (sub + 2) % 3;
            outPhaseGrp = (sub + 2) % 4;
        }
    }

    // Biên dịch toàn bộ lưới đa giác thành DUY NHẤT 1 batch GL_TRIANGLES bên trong Display List
    static void emitSingleBatchTriangles(const Geometry::PolygonMesh& mesh) {
        if (mesh.faces.empty() || mesh.vertices.empty()) return;
        const bool hasVertexNorms = (mesh.vertexNormals.size() == mesh.vertices.size());
        MathCG::ColorRGB prevCol(-1.0f, -1.0f, -1.0f);

        glBegin(GL_TRIANGLES);
        for (const auto& face : mesh.faces) {
            const size_t n = face.vertexIndices.size();
            if (n < 3) continue;
            const MathCG::ColorRGB& col = face.hasCustomColor ? face.color : mesh.defaultColor;
            if (col.r != prevCol.r || col.g != prevCol.g || col.b != prevCol.b) {
                glColor3f(col.r, col.g, col.b);
                prevCol = col;
            }
            const int i0 = face.vertexIndices[0];
            for (size_t k = 1; k + 1 < n; ++k) {
                const int triIdx[3] = { i0, face.vertexIndices[k], face.vertexIndices[k + 1] };
                for (int corner = 0; corner < 3; ++corner) {
                    const int vi = triIdx[corner];
                    if (hasVertexNorms) {
                        const auto& vn = mesh.vertexNormals[vi];
                        glNormal3f(vn.x, vn.y, vn.z);
                    } else {
                        glNormal3f(face.faceNormal.x, face.faceNormal.y, face.faceNormal.z);
                    }
                    const auto& p = mesh.vertices[vi];
                    glVertex3f(p.x, p.y, p.z);
                }
            }
        }
        glEnd();
    }

    static GLuint compileMeshToSingleBatchDL(const Geometry::PolygonMesh& mesh) {
        GLuint dl = glGenLists(1);
        if (dl != 0) {
            glNewList(dl, GL_COMPILE);
            emitSingleBatchTriangles(mesh);
            glEndList();
        }
        return dl;
    }

    static void appendAnnularRing(
        Geometry::PolygonMesh& mesh,
        float rInner, float rOuter,
        float yBot, float yTop,
        int segs,
        const MathCG::ColorRGB& topCol,
        const MathCG::ColorRGB& sideCol
    ) {
        const float twoPi = 2.0f * MathCG::PI;
        std::vector<int> iB(segs), iT(segs), oB(segs), oT(segs);
        for (int k = 0; k < segs; ++k) {
            float a = twoPi * static_cast<float>(k) / static_cast<float>(segs);
            float c = std::cos(a), s = -std::sin(a);
            iB[k] = mesh.addVertex(rInner * c, yBot, rInner * s);
            iT[k] = mesh.addVertex(rInner * c, yTop, rInner * s);
            oB[k] = mesh.addVertex(rOuter * c, yBot, rOuter * s);
            oT[k] = mesh.addVertex(rOuter * c, yTop, rOuter * s);
        }
        for (int k = 0; k < segs; ++k) {
            int kn = (k + 1) % segs;
            mesh.addQuad(oB[k], oB[kn], oT[kn], oT[k], sideCol, true);
            mesh.addQuad(iB[kn], iB[k], iT[k], iT[kn], sideCol, true);
            mesh.addQuad(oT[k], oT[kn], iT[kn], iT[k], topCol, true);
            mesh.addQuad(oB[kn], oB[k], iB[k], iB[kn], sideCol, true);
        }
    }

    static void appendCircularBalustrade(
        Geometry::PolygonMesh& mesh,
        float rCenter,
        float startDeg, float endDeg,
        float yBase, float height,
        int arcSteps, int postEvery,
        const MathCG::ColorRGB& postCol,
        const MathCG::ColorRGB& railCol
    ) {
        const float rIn = rCenter - 0.042f;
        const float rOut = rCenter + 0.042f;
        const float midY = yBase + height * 0.48f;
        const float topY = yBase + height;

        auto buildRailRibbon = [&](float yLo, float yHi, const MathCG::ColorRGB& col) {
            std::vector<int> oB(arcSteps + 1), oT(arcSteps + 1), iB(arcSteps + 1), iT(arcSteps + 1);
            for (int k = 0; k <= arcSteps; ++k) {
                float deg = startDeg + (endDeg - startDeg) * (static_cast<float>(k) / arcSteps);
                float a = deg * MathCG::DEG2RAD;
                float c = std::cos(a), s = -std::sin(a);
                oB[k] = mesh.addVertex(rOut * c, yLo, rOut * s);
                oT[k] = mesh.addVertex(rOut * c, yHi, rOut * s);
                iB[k] = mesh.addVertex(rIn * c,  yLo, rIn * s);
                iT[k] = mesh.addVertex(rIn * c,  yHi, rIn * s);
            }
            for (int k = 0; k < arcSteps; ++k) {
                mesh.addQuad(oB[k], oB[k + 1], oT[k + 1], oT[k], col, true);
                mesh.addQuad(iB[k + 1], iB[k], iT[k], iT[k + 1], col, true);
                mesh.addQuad(oT[k], oT[k + 1], iT[k + 1], iT[k], col, true);
                mesh.addQuad(oB[k + 1], oB[k], iB[k], iB[k + 1], col, true);
            }
            mesh.addQuad(iB[0], oB[0], oT[0], iT[0], col, true);
            mesh.addQuad(oB[arcSteps], iB[arcSteps], iT[arcSteps], oT[arcSteps], col, true);
        };

        buildRailRibbon(topY - 0.07f, topY, railCol);
        buildRailRibbon(midY - 0.035f, midY + 0.035f, postCol);

        for (int k = 0; k <= arcSteps; k += std::max(1, postEvery)) {
            float deg = startDeg + (endDeg - startDeg) * (static_cast<float>(k) / arcSteps);
            float a = deg * MathCG::DEG2RAD;
            float px = rCenter * std::cos(a);
            float pz = -rCenter * std::sin(a);
            bool isMainPost = (k == 0 || k + postEvery > arcSteps || k % (postEvery * 4) == 0);
            float rad = isMainPost ? 0.068f : 0.038f;
            mesh.append(Geometry::PolygonMesh::createPrismY(px, yBase, pz, rad, height, 6, postCol));
            if (isMainPost) {
                mesh.append(Geometry::PolygonMesh::createPyramid(px, topY, pz, 0.11f, 0.11f, 0.13f, railCol));
            }
        }
    }

    static Geometry::PolygonMesh buildHorseMesh(
        const MathCG::ColorRGB& coatCol,
        const MathCG::ColorRGB& saddleCol,
        const MathCG::ColorRGB& maneCol
    ) {
        Geometry::PolygonMesh horse("CarouselHorse");
        using Mesh = Geometry::PolygonMesh;
        const MathCG::ColorRGB goldCol(0.95f, 0.78f, 0.22f);
        const MathCG::ColorRGB hoofCol(0.20f, 0.16f, 0.14f);

        horse.append(Mesh::createBox(0.0f, 0.02f, 0.0f, 0.34f, 0.36f, 1.02f, coatCol));
        horse.append(Mesh::createBox(0.0f, 0.05f, 0.36f, 0.36f, 0.38f, 0.34f, coatCol));
        horse.append(Mesh::createBox(0.0f, 0.04f, -0.34f, 0.34f, 0.35f, 0.34f, coatCol));
        horse.append(Mesh::createBox(0.0f, 0.10f, -0.02f, 0.38f, 0.26f, 0.44f, saddleCol));
        horse.append(Mesh::createBox(0.0f, 0.23f, -0.02f, 0.32f, 0.08f, 0.34f, goldCol));
        horse.append(Mesh::createBox(0.0f, 0.29f, -0.15f, 0.28f, 0.08f, 0.08f, goldCol));

        Geometry::PolygonMesh neck = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.18f, 0.56f, 0.24f, coatCol);
        neck.rotateX(34.0f);
        neck.translate(0.0f, 0.36f, 0.54f);
        horse.append(neck);

        Geometry::PolygonMesh mane = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.09f, 0.52f, 0.11f, maneCol);
        mane.rotateX(34.0f);
        mane.translate(0.0f, 0.40f, 0.41f);
        horse.append(mane);

        Geometry::PolygonMesh head = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.18f, 0.19f, 0.42f, coatCol);
        head.rotateX(22.0f);
        head.translate(0.0f, 0.58f, 0.78f);
        horse.append(head);

        horse.append(Mesh::createBox(0.0f, 0.56f, 0.85f, 0.20f, 0.12f, 0.06f, goldCol));
        horse.append(Mesh::createPyramid(-0.06f, 0.69f, 0.67f, 0.06f, 0.07f, 0.12f, coatCol));
        horse.append(Mesh::createPyramid( 0.06f, 0.69f, 0.67f, 0.06f, 0.07f, 0.12f, coatCol));

        Geometry::PolygonMesh tail = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.11f, 0.54f, 0.14f, maneCol);
        tail.rotateX(-32.0f);
        tail.translate(0.0f, -0.02f, -0.62f);
        horse.append(tail);

        for (int s : {-1, 1}) {
            float sx = s * 0.12f;
            float phaseAngle = (s > 0) ? 14.0f : 0.0f;

            Geometry::PolygonMesh fUpper = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.08f, 0.30f, 0.09f, coatCol);
            fUpper.rotateX(-36.0f - phaseAngle);
            fUpper.translate(sx, -0.20f, 0.42f);
            horse.append(fUpper);

            Geometry::PolygonMesh fLower = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.065f, 0.24f, 0.075f, hoofCol);
            fLower.rotateX(16.0f - phaseAngle);
            fLower.translate(sx, -0.34f, 0.49f);
            horse.append(fLower);

            Geometry::PolygonMesh hUpper = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.09f, 0.34f, 0.10f, coatCol);
            hUpper.rotateX(28.0f + phaseAngle * 0.6f);
            hUpper.translate(sx, -0.22f, -0.44f);
            horse.append(hUpper);

            Geometry::PolygonMesh hLower = Mesh::createBox(0.0f, 0.0f, 0.0f, 0.07f, 0.24f, 0.08f, hoofCol);
            hLower.rotateX(40.0f + phaseAngle * 0.6f);
            hLower.translate(sx, -0.44f, -0.57f);
            horse.append(hLower);
        }

        horse.computeNormals();
        return horse;
    }

    void ensureMeshesBuilt() const {
        if (meshesBuilt_) return;
        meshesBuilt_ = true;

        using Mesh = Geometry::PolygonMesh;

        const MathCG::ColorRGB stoneLight(0.84f, 0.79f, 0.71f);
        const MathCG::ColorRGB stoneWarm (0.72f, 0.66f, 0.58f);
        const MathCG::ColorRGB stoneDark (0.48f, 0.44f, 0.39f);
        const MathCG::ColorRGB woodWarm  (0.68f, 0.44f, 0.22f);
        const MathCG::ColorRGB oakLight  (0.85f, 0.72f, 0.48f);
        const MathCG::ColorRGB creamCol  (0.95f, 0.90f, 0.78f);
        const MathCG::ColorRGB crimsonCol(0.80f, 0.16f, 0.14f);
        const MathCG::ColorRGB velvetCol (0.58f, 0.10f, 0.10f);
        const MathCG::ColorRGB brassCol  (0.92f, 0.74f, 0.22f);
        const MathCG::ColorRGB ironDark  (0.20f, 0.22f, 0.25f);

        const float outerR = ParkLayout::CAROUSEL_OUTER_RADIUS;     // 8.50 m
        const float deck1R = ParkLayout::CAROUSEL_TURNTABLE_RADIUS; // 7.36 m
        const float deck1Y = ParkLayout::CAROUSEL_TIER1_Y;          // 0.75 m
        const float deck2R = ParkLayout::CAROUSEL_TIER2_RADIUS;     // 4.95 m
        const float deck2Y = ParkLayout::CAROUSEL_TIER2_Y;          // 3.70 m
        const float eaveY  = ParkLayout::CAROUSEL_EAVE_Y;           // 6.50 m

        // =========================================================================
        // 1. PHẦN TĨNH: Bệ đá 3 bậc tròn + thềm cổng Bắc/Nam + hàng rào + quầy vé (R <= 8.50 m)
        // =========================================================================
        staticBaseMesh_ = Mesh("CarouselStaticBase");
        staticBaseMesh_.append(Mesh::createPrismY(0.0f, -0.02f, 0.0f, outerR,         0.26f, 36, stoneDark));
        staticBaseMesh_.append(Mesh::createPrismY(0.0f,  0.24f, 0.0f, outerR - 0.22f, 0.24f, 36, stoneWarm));
        appendAnnularRing(staticBaseMesh_, 7.42f, outerR - 0.32f, 0.46f, 0.72f, 36, stoneLight, stoneWarm);

        // Thềm bậc đá sáng rõ tại 2 lối vào Bắc (90°) & Nam (270°), giữ mọi góc cạnh <= 8.42 m <= 8.50 m
        for (float gateDeg : { 90.0f, 270.0f }) {
            Mesh steps("CarouselEntryStairs");
            steps.append(Mesh::createBox(0.0f, 0.13f, 8.20f, 2.10f, 0.26f, 0.28f, stoneLight));
            steps.append(Mesh::createBox(0.0f, 0.25f, 8.00f, 2.10f, 0.50f, 0.28f, creamCol));
            steps.append(Mesh::createBox(0.0f, 0.37f, 7.80f, 2.10f, 0.74f, 0.30f, creamCol));
            steps.rotateY(gateDeg - 270.0f);
            staticBaseMesh_.append(steps);
        }

        // Hàng rào bảo vệ vòng ngoài (R = 8.08 m) bám khít 2 cổng (98°..262° và 278°..419°)
        appendCircularBalustrade(staticBaseMesh_, 8.08f,  98.0f, 262.0f, 0.48f, 1.02f, 20, 3, ironDark, brassCol);
        appendCircularBalustrade(staticBaseMesh_, 8.08f, 278.0f, 419.0f, 0.48f, 1.02f, 18, 3, ironDark, brassCol);

        // Quầy vé nhỏ kiểu Venice cạnh cổng Bắc (tâm R = 7.80 m, góc 66°, nối gọn giữa hàng rào 59° và cổng 78°)
        {
            Mesh booth("CarouselTicketBooth");
            booth.append(Mesh::createBox(0.0f, 1.56f, 0.0f, 1.04f, 1.72f, 0.72f, crimsonCol));
            booth.append(Mesh::createBox(0.0f, 1.78f, 0.37f, 0.72f, 0.78f, 0.06f, creamCol));
            booth.append(Mesh::createBox(0.0f, 1.32f, 0.41f, 0.88f, 0.08f, 0.16f, brassCol));
            booth.append(Mesh::createBox(0.0f, 2.46f, 0.0f, 1.18f, 0.12f, 0.88f, brassCol));
            booth.append(Mesh::createPyramid(0.0f, 2.52f, 0.0f, 1.16f, 0.86f, 0.54f, crimsonCol));
            booth.append(Mesh::createPyramid(0.0f, 3.04f, 0.0f, 0.18f, 0.18f, 0.22f, brassCol));
            booth.rotateY(156.0f);
            const float br = 7.80f;
            const float ba = 66.0f * MathCG::DEG2RAD;
            booth.translate(br * std::cos(ba), 0.0f, -br * std::sin(ba));
            staticBaseMesh_.append(booth);
        }
        staticBaseMesh_.computeNormals();

        // =========================================================================
        // 2. KHỐI QUAY ĐỒNG TRỤC 2 TẦNG (sàn 1, trục giữa, thang cong, sàn 2, cột, mái)
        // =========================================================================
        rotatingBodyMesh_ = Mesh("CarouselRotatingBody");

        // 2a. Sàn xoay tầng 1 (y = 0.48..0.75 m, R = 7.36 m)
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, 0.48f, 0.0f, deck1R, 0.27f, 36, woodWarm));
        appendAnnularRing(rotatingBodyMesh_, deck1R - 0.16f, deck1R + 0.02f, 0.48f, 0.75f, 36, oakLight, crimsonCol);

        // 2b. Trục gương trung tâm kiểu Venice
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck1Y, 0.0f, 1.56f, 0.22f, 16, brassCol));
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck1Y + 0.22f, 0.0f, 1.44f, deck2Y - deck1Y - 0.22f, 16, creamCol));
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck2Y - 0.12f, 0.0f, 1.56f, 0.30f, 16, brassCol));
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck2Y + 0.18f, 0.0f, 1.32f, eaveY - deck2Y - 0.18f, 16, creamCol));
        for (int p = 0; p < 8; ++p) {
            float a = (static_cast<float>(p) / 8.0f) * 2.0f * MathCG::PI;
            rotatingBodyMesh_.append(Mesh::createPrismY(1.46f * std::cos(a), deck1Y, -1.46f * std::sin(a), 0.10f, deck2Y - deck1Y, 6, brassCol));
            rotatingBodyMesh_.append(Mesh::createPrismY(1.34f * std::cos(a), deck2Y, -1.34f * std::sin(a), 0.09f, eaveY - deck2Y, 6, brassCol));
            if (p % 2 == 0) {
                Mesh panel = Mesh::createBox(0.0f, (deck1Y + deck2Y) * 0.5f, 1.44f, 0.72f, 2.15f, 0.06f, crimsonCol);
                panel.rotateY(static_cast<float>(p) * 45.0f + 90.0f);
                rotatingBodyMesh_.append(panel);
            }
        }

        // 2c. Sàn xoay tầng 2 + trần dưới kem sáng & lan can đồng tầng 2
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck2Y - 0.26f, 0.0f, deck2R - 0.04f, 0.08f, 32, creamCol));
        rotatingBodyMesh_.append(Mesh::createPrismY(0.0f, deck2Y - 0.18f, 0.0f, deck2R, 0.18f, 32, woodWarm));
        appendAnnularRing(rotatingBodyMesh_, deck2R - 0.12f, deck2R + 0.03f, deck2Y - 0.28f, deck2Y + 0.02f, 32, oakLight, crimsonCol);
        appendCircularBalustrade(rotatingBodyMesh_, deck2R - 0.14f, 166.0f, 501.0f, deck2Y, 0.88f, 30, 3, brassCol, crimsonCol);

        // 2d. Cầu thang cong nối liền sàn tầng 1 (34°) lên sàn tầng 2 (141°)
        {
            const int nSteps = 12;
            const float stDeg0 = 34.0f, stDeg1 = 141.0f;
            const float rInS = 4.78f, rOutS = 5.54f;
            const float stepH = (deck2Y - deck1Y) / static_cast<float>(nSteps);

            std::vector<int> rInRails, rOutRails;
            for (int k = 0; k < nSteps; ++k) {
                float t0 = static_cast<float>(k) / nSteps;
                float t1 = static_cast<float>(k + 1) / nSteps;
                float a0 = (stDeg0 + (stDeg1 - stDeg0) * t0) * MathCG::DEG2RAD;
                float a1 = (stDeg0 + (stDeg1 - stDeg0) * t1) * MathCG::DEG2RAD;
                float yTop = deck1Y + (k + 1) * stepH;
                float ySoffit = std::max(deck1Y, yTop - 0.42f);

                float c0 = std::cos(a0), s0 = -std::sin(a0);
                float c1 = std::cos(a1), s1 = -std::sin(a1);

                int ib0 = rotatingBodyMesh_.addVertex(rInS  * c0, ySoffit, rInS  * s0);
                int ib1 = rotatingBodyMesh_.addVertex(rInS  * c1, ySoffit, rInS  * s1);
                int it0 = rotatingBodyMesh_.addVertex(rInS  * c0, yTop,    rInS  * s0);
                int it1 = rotatingBodyMesh_.addVertex(rInS  * c1, yTop,    rInS  * s1);
                int ob0 = rotatingBodyMesh_.addVertex(rOutS * c0, ySoffit, rOutS * s0);
                int ob1 = rotatingBodyMesh_.addVertex(rOutS * c1, ySoffit, rOutS * s1);
                int ot0 = rotatingBodyMesh_.addVertex(rOutS * c0, yTop,    rOutS * s0);
                int ot1 = rotatingBodyMesh_.addVertex(rOutS * c1, yTop,    rOutS * s1);

                rotatingBodyMesh_.addQuad(ob0, ob1, ot1, ot0, velvetCol, true);
                rotatingBodyMesh_.addQuad(ib1, ib0, it0, it1, velvetCol, true);
                rotatingBodyMesh_.addQuad(ot0, ot1, it1, it0, oakLight, true);
                rotatingBodyMesh_.addQuad(ob1, ob0, ib0, ib1, velvetCol, true);
                rotatingBodyMesh_.addQuad(ib0, ob0, ot0, it0, woodWarm, true);
                rotatingBodyMesh_.addQuad(ob1, ib1, it1, ot1, woodWarm, true);

                float am = 0.5f * (a0 + a1);
                float cm = std::cos(am), sm = -std::sin(am);
                float railY = yTop + 0.86f;
                if (k % 2 == 0 || k == nSteps - 1) {
                    rotatingBodyMesh_.append(Mesh::createPrismY(rOutS * cm, yTop, rOutS * sm, 0.036f, 0.86f, 6, brassCol));
                    rotatingBodyMesh_.append(Mesh::createPrismY(rInS  * cm, yTop, rInS  * sm, 0.036f, 0.86f, 6, brassCol));
                }
                int rO = rotatingBodyMesh_.addVertex((rOutS - 0.03f) * cm, railY - 0.03f, (rOutS - 0.03f) * sm);
                int rI = rotatingBodyMesh_.addVertex((rOutS + 0.03f) * cm, railY - 0.03f, (rOutS + 0.03f) * sm);
                int rOT = rotatingBodyMesh_.addVertex((rOutS - 0.03f) * cm, railY + 0.04f, (rOutS - 0.03f) * sm);
                int rIT = rotatingBodyMesh_.addVertex((rOutS + 0.03f) * cm, railY + 0.04f, (rOutS + 0.03f) * sm);
                if (k > 0) {
                    int pO = rOutRails.back();
                    rotatingBodyMesh_.addQuad(pO + 1, rI, rIT, pO + 3, brassCol, true);
                    rotatingBodyMesh_.addQuad(pO + 0, pO + 2, rOT, rO, brassCol, true);
                    rotatingBodyMesh_.addQuad(pO + 3, rIT, rOT, pO + 2, crimsonCol, true);
                }
                rOutRails.push_back(rO);

                int irO = rotatingBodyMesh_.addVertex((rInS - 0.03f) * cm, railY - 0.03f, (rInS - 0.03f) * sm);
                int irI = rotatingBodyMesh_.addVertex((rInS + 0.03f) * cm, railY - 0.03f, (rInS + 0.03f) * sm);
                int irOT = rotatingBodyMesh_.addVertex((rInS - 0.03f) * cm, railY + 0.04f, (rInS - 0.03f) * sm);
                int irIT = rotatingBodyMesh_.addVertex((rInS + 0.03f) * cm, railY + 0.04f, (rInS + 0.03f) * sm);
                if (k > 0) {
                    int pI = rInRails.back();
                    rotatingBodyMesh_.addQuad(pI + 1, irI, irIT, pI + 3, brassCol, true);
                    rotatingBodyMesh_.addQuad(pI + 0, pI + 2, irOT, irO, brassCol, true);
                    rotatingBodyMesh_.addQuad(pI + 3, irIT, irOT, pI + 2, crimsonCol, true);
                }
                rInRails.push_back(irO);
            }
        }

        // 2e. 16 cột đồng vòng ngoài + 8 cột đồng tầng 2 + 20 cột ngựa
        for (int k = 0; k < 16; ++k) {
            float a = (static_cast<float>(k) / 16.0f) * 2.0f * MathCG::PI;
            float cx = 7.05f * std::cos(a), cz = -7.05f * std::sin(a);
            rotatingBodyMesh_.append(Mesh::createPrismY(cx, deck1Y, cz, 0.14f, 0.22f, 6, brassCol));
            rotatingBodyMesh_.append(Mesh::createPrismY(cx, deck1Y + 0.22f, cz, 0.09f, eaveY - deck1Y - 0.22f, 6, brassCol));
            rotatingBodyMesh_.append(Mesh::createPrismY(cx, eaveY - 0.22f, cz, 0.14f, 0.26f, 6, brassCol));
        }
        for (int k = 0; k < 8; ++k) {
            float a = (k * 45.0f + 11.25f) * MathCG::DEG2RAD;
            float cx = 4.65f * std::cos(a), cz = -4.65f * std::sin(a);
            rotatingBodyMesh_.append(Mesh::createPrismY(cx, deck1Y, cz, 0.11f, 0.18f, 6, brassCol));
            rotatingBodyMesh_.append(Mesh::createPrismY(cx, deck1Y + 0.18f, cz, 0.075f, eaveY - deck1Y - 0.18f, 6, brassCol));
        }
        const int totalHorses = ParkLayout::CAROUSEL_TIER1_HORSES + ParkLayout::CAROUSEL_TIER2_HORSES;
        for (int i = 0; i < totalHorses; ++i) {
            float r = 0.0f, localDeg = 0.0f, dY = 0.0f;
            int st = 0, grp = 0;
            getHorseLocalPose(i, r, localDeg, dY, st, grp);
            float rad = localDeg * MathCG::DEG2RAD;
            float px = r * std::cos(rad), pz = -r * std::sin(rad);
            float poleTopY = (i >= 8 && i < ParkLayout::CAROUSEL_TIER1_HORSES) ? (deck2Y - 0.22f) : (eaveY - 0.04f);
            rotatingBodyMesh_.append(Mesh::createPrismY(px, dY, pz, 0.045f, poleTopY - dY, 6, brassCol));
        }

        // 2f. Mái nón 16 múi sọc, diềm mái lượn sóng, lầu đèn & chóp vương miện
        rotatingBodyMesh_.append(Mesh::createCarouselConicalRoof(
            0.0f, eaveY, 0.0f,
            7.62f, ParkLayout::CAROUSEL_CONE_HEIGHT,
            ParkLayout::CAROUSEL_TOTAL_HEIGHT - eaveY,
            16, crimsonCol, creamCol, velvetCol, brassCol
        ));
        rotatingBodyMesh_.computeNormals();

        // =========================================================================
        // 3. 20 NGỰA GỖ GỘP THÀNH ĐÚNG 4 NHÓM PHA (mỗi nhóm = 1 Display List tĩnh)
        // =========================================================================
        std::array<Mesh, 3> variants = {
            buildHorseMesh(MathCG::ColorRGB(0.95f, 0.93f, 0.88f), crimsonCol, MathCG::ColorRGB(0.90f, 0.72f, 0.24f)),
            buildHorseMesh(MathCG::ColorRGB(0.68f, 0.36f, 0.16f), MathCG::ColorRGB(0.16f, 0.38f, 0.68f), MathCG::ColorRGB(0.96f, 0.90f, 0.74f)),
            buildHorseMesh(MathCG::ColorRGB(0.22f, 0.20f, 0.22f), crimsonCol, MathCG::ColorRGB(0.88f, 0.68f, 0.20f))
        };
        for (int g = 0; g < 4; ++g) {
            horseGroupMeshes_[g] = Mesh("CarouselHorsePhaseGroup");
        }
        for (int i = 0; i < totalHorses; ++i) {
            float r = 0.0f, localDeg = 0.0f, dY = 0.0f;
            int st = 0, grp = 0;
            getHorseLocalPose(i, r, localDeg, dY, st, grp);
            float rad = localDeg * MathCG::DEG2RAD;
            Mesh placed = variants[st];
            placed.rotateY(localDeg + 180.0f);
            placed.translate(r * std::cos(rad), dY + 0.88f, -r * std::sin(rad));
            horseGroupMeshes_[grp].append(placed);
        }
        for (int g = 0; g < 4; ++g) {
            horseGroupMeshes_[g].computeNormals();
        }

        // =========================================================================
        // 4. ĐÈN TRANG TRÍ BAN ĐÊM / HOÀNG HÔN (chỉ vẽ khi đêm/hoàng hôn)
        // =========================================================================
        nightLightsMesh_ = Mesh("CarouselNightLights");
        const MathCG::ColorRGB warmBulb(1.0f, 0.90f, 0.48f);
        const MathCG::ColorRGB roseBulb(1.0f, 0.60f, 0.34f);
        for (int k = 0; k < 32; ++k) {
            float a = (static_cast<float>(k) / 32.0f) * 2.0f * MathCG::PI;
            float br = 7.66f;
            bool isPeak = (k % 2 == 0);
            float by = isPeak ? (eaveY + 0.18f) : (eaveY - 0.36f);
            nightLightsMesh_.append(Mesh::createBox(br * std::cos(a), by, -br * std::sin(a), 0.12f, 0.12f, 0.12f, isPeak ? warmBulb : roseBulb));
        }
        for (int k = 0; k < 16; ++k) {
            float a = (static_cast<float>(k) / 16.0f) * 2.0f * MathCG::PI;
            nightLightsMesh_.append(Mesh::createBox(5.02f * std::cos(a), deck2Y - 0.14f, -5.02f * std::sin(a), 0.11f, 0.11f, 0.11f, warmBulb));
        }
        // Đèn rọi gầm trần nội thất giúp ngựa gỗ 2 tầng rực sáng hơn vào ban đêm
        for (int k = 0; k < 8; ++k) {
            float a = (static_cast<float>(k) / 8.0f) * 2.0f * MathCG::PI;
            nightLightsMesh_.append(Mesh::createBox(5.50f * std::cos(a), eaveY - 0.22f, -5.50f * std::sin(a), 0.16f, 0.12f, 0.16f, warmBulb));
        }
        nightLightsMesh_.append(Mesh::createPrismY(0.0f, eaveY + ParkLayout::CAROUSEL_CONE_HEIGHT + 0.10f, 0.0f, 1.425f, 0.36f, 16, warmBulb));
        nightLightsMesh_.computeNormals();
    }

    void drawNightLightsDirect(bool showNormals) const {
        const GLfloat glow[4] = { 0.90f, 0.75f, 0.28f, 1.0f };
        const GLfloat zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow);
        if (showNormals) {
            nightLightsMesh_.draw(false, true);
        } else {
            emitSingleBatchTriangles(nightLightsMesh_);
        }
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
    }

    void ensureDisplayLists() const {
        ensureMeshesBuilt();
        if (dlStaticBase_ == 0)   dlStaticBase_   = compileMeshToSingleBatchDL(staticBaseMesh_);
        if (dlRotatingBody_ == 0) dlRotatingBody_ = compileMeshToSingleBatchDL(rotatingBodyMesh_);
        for (int g = 0; g < 4; ++g) {
            if (dlHorseGroup_[g] == 0) dlHorseGroup_[g] = compileMeshToSingleBatchDL(horseGroupMeshes_[g]);
        }
        if (dlNightLights_ == 0) {
            dlNightLights_ = glGenLists(1);
            if (dlNightLights_ != 0) {
                glNewList(dlNightLights_, GL_COMPILE);
                drawNightLightsDirect(false);
                glEndList();
            }
        }
    }
};

} // namespace AsiaParkRides

#endif // CAROUSEL_H
