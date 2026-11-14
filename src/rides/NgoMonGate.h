#ifndef NGO_MON_GATE_H
#define NGO_MON_GATE_H

/**
 * @file NgoMonGate.h
 * @brief Chapter 4 & 6: Architectural Simulation of Asia Park's Grand Entrance:
 *        Cổng Ngọ Môn (Meridian Imperial Gate) & Tháp Đồng Hồ (Central Clock Tower)
 *
 * Implements:
 *  - Authentic Asia Park Da Nang Entrance Architecture:
 *      * Location: (0.0, 0.0, 86.0) at the South Entrance Plaza facing city arrival
 *      * Footprint: 48m width x 16m depth x 28m peak tower height
 *  - Chapter 4 3D Geometric Representation via Geometry::PolygonMesh:
 *      * 1. Massive Stone Bastion Base (Bệ Thành Lũy) with battered sloping stone courses
 *      * 2. Three Arched Barrel Vault Gateways:
 *           - Central Royal Arch (Lối đi chính Ngọ Môn)
 *           - Left & Right Flanking Gates (Tả Đoan Môn & Hữu Đoan Môn)
 *           generated via Geometry::PolygonMesh::createArchVault
 *      * 3. Lầu Ngũ Phụng (Five-Phoenix Pavilion):
 *           - Red timber columns (createPrismY) & white stone balustrades
 *           - Double-tiered curved Asian hip-and-gable flared roofs (createCurvedAsianRoof)
 *      * 4. Central Grand Clock Tower (Tháp Đồng Hồ):
 *           - 4-sided illuminated clock faces with hour and minute hands
 *           - Imperial tiered crown & golden pinnacle spire
 *  - Chapter 6 Illumination, Shading & Normal Calculations:
 *      * Fully computed face & vertex normals via PolygonMesh::computeNormals()
 *      * Backlit glowing clock dials & suspended red imperial lanterns at night
 */

#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include <vector>
#include "core/MathCG.h"
#include "core/Lighting.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class NgoMonGate {
public:
    MathCG::Vec3 position       = MathCG::Vec3(0.0f, 0.0f, 86.0f);
    float        width          = 46.0f;
    float        depth          = 15.0f;
    float        bastionHeight  = 6.8f;
    float        towerHeight    = 27.5f;

    NgoMonGate() {
        buildGateMesh();
    }

    /**
     * @brief Render the Grand Meridian Gate & Clock Tower
     */
    void draw(bool isNightOrSunset, bool showNormals = false, bool smoothShading = true) const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // 1. Render Pre-built Architectural Polygon Mesh
        gateMesh.draw(smoothShading, showNormals);

        // 2. Render 4-Sided Illuminated Clock Faces on Central Tower
        drawClockFaces(isNightOrSunset);

        // 3. Render Suspended Red Imperial Lanterns beneath Pavilion Eaves
        drawLanterns(isNightOrSunset, showNormals);

        glPopMatrix();
    }

private:
    mutable Geometry::PolygonMesh gateMesh;

    /**
     * @brief Procedurally build the complete Ngo Mon Gate & Clock Tower PolygonMesh
     */
    void buildGateMesh() {
        gateMesh.clear();
        gateMesh.name = "NgoMonImperialGate";

        MathCG::ColorRGB stoneCol(0.62f, 0.60f, 0.58f); // Ancient gray fortress stone
        MathCG::ColorRGB redPillarCol(0.78f, 0.16f, 0.14f); // Imperial lacquered vermilion
        MathCG::ColorRGB goldRoofCol(0.95f, 0.70f, 0.12f); // Imperial yellow glazed tiles (Hoang Luu Ly)
        MathCG::ColorRGB darkTrimCol(0.22f, 0.20f, 0.22f); // Dark timber beam lintels
        MathCG::ColorRGB balustradeCol(0.88f, 0.88f, 0.90f); // White carved marble balustrade

        // ====================================================================
        // 1. BASTION PODIUM BASE & WINGS (Bệ Đài Thành Cổ)
        // ====================================================================
        // Central Gate Bastion (holds the 3 arches)
        float centerW = 24.0f;
        float wingW   = (width - centerW) * 0.5f; // 11m each wing
        float hZ      = depth * 0.5f;

        // Central podium block above arches (y = 5.2 .. 6.8)
        auto centerTop = Geometry::PolygonMesh::createBox(
            0.0f, 6.0f, 0.0f,
            centerW, 1.6f, depth,
            stoneCol
        );
        gateMesh.append(centerTop);

        // Left & Right fortress pier pillars forming the 3 arches
        // Pier 0: Far-Left (x = -10.5 .. -8.0)
        auto pierL0 = Geometry::PolygonMesh::createBox(-9.25f, 2.6f, 0.0f, 2.5f, 5.2f, depth, stoneCol);
        // Pier 1: Mid-Left (x = -4.0 .. -2.0)
        auto pierL1 = Geometry::PolygonMesh::createBox(-3.0f,  2.6f, 0.0f, 2.0f, 5.2f, depth, stoneCol);
        // Pier 2: Mid-Right (x = 2.0 .. 4.0)
        auto pierR1 = Geometry::PolygonMesh::createBox( 3.0f,  2.6f, 0.0f, 2.0f, 5.2f, depth, stoneCol);
        // Pier 3: Far-Right (x = 8.0 .. 10.5)
        auto pierR0 = Geometry::PolygonMesh::createBox( 9.25f, 2.6f, 0.0f, 2.5f, 5.2f, depth, stoneCol);

        gateMesh.append(pierL0);
        gateMesh.append(pierL1);
        gateMesh.append(pierR1);
        gateMesh.append(pierR0);

        // East & West Bastion Wings
        auto wingLeft = Geometry::PolygonMesh::createBox(
            -(centerW * 0.5f + wingW * 0.5f), 3.4f, 0.0f,
            wingW, bastionHeight, depth,
            stoneCol
        );
        auto wingRight = Geometry::PolygonMesh::createBox(
             (centerW * 0.5f + wingW * 0.5f), 3.4f, 0.0f,
            wingW, bastionHeight, depth,
            stoneCol
        );
        gateMesh.append(wingLeft);
        gateMesh.append(wingRight);

        // Stone Base Molding / Plinth
        auto basePlinth = Geometry::PolygonMesh::createBox(0.0f, 0.25f, 0.0f, width + 2.0f, 0.5f, depth + 1.6f, MathCG::ColorRGB(0.48f, 0.46f, 0.44f));
        gateMesh.append(basePlinth);

        // ====================================================================
        // 2. THREE ARCHED BARREL VAULTS (Vòm Cuốn Cổng Ngọ Môn)
        // ====================================================================
        // Center Royal Arch (Radius = 2.0m, Height = 4.8m)
        auto centralVault = Geometry::PolygonMesh::createArchVault(
            0.0f, 2.8f, -hZ, hZ,
            1.95f, 2.55f, 14,
            MathCG::ColorRGB(0.74f, 0.70f, 0.65f)
        );
        gateMesh.append(centralVault);

        // Left Side Arch (Radius = 1.4m, Height = 4.2m)
        auto leftVault = Geometry::PolygonMesh::createArchVault(
            -6.0f, 2.4f, -hZ, hZ,
            1.40f, 1.85f, 12,
            MathCG::ColorRGB(0.70f, 0.66f, 0.62f)
        );
        gateMesh.append(leftVault);

        // Right Side Arch (Radius = 1.4m, Height = 4.2m)
        auto rightVault = Geometry::PolygonMesh::createArchVault(
             6.0f, 2.4f, -hZ, hZ,
            1.40f, 1.85f, 12,
            MathCG::ColorRGB(0.70f, 0.66f, 0.62f)
        );
        gateMesh.append(rightVault);

        // ====================================================================
        // 3. WHITE STONE BALUSTRADES & TERRACE FLOOR (Lan Can Đá Lầu Ngũ Phụng)
        // ====================================================================
        // Terrace floor deck
        auto terraceFloor = Geometry::PolygonMesh::createBox(0.0f, bastionHeight + 0.15f, 0.0f, width + 0.6f, 0.30f, depth + 0.6f, MathCG::ColorRGB(0.75f, 0.72f, 0.70f));
        gateMesh.append(terraceFloor);

        // Perimeter carved balustrade walls (h = 0.9m)
        float bY = bastionHeight + 0.30f + 0.45f;
        // Front & Back balustrade
        auto balFront = Geometry::PolygonMesh::createBox(0.0f, bY,  hZ + 0.2f, width + 0.4f, 0.90f, 0.30f, balustradeCol);
        auto balBack  = Geometry::PolygonMesh::createBox(0.0f, bY, -hZ - 0.2f, width + 0.4f, 0.90f, 0.30f, balustradeCol);
        // Left & Right balustrade
        auto balLeft  = Geometry::PolygonMesh::createBox(-width * 0.5f - 0.2f, bY, 0.0f, 0.30f, 0.90f, depth + 0.4f, balustradeCol);
        auto balRight = Geometry::PolygonMesh::createBox( width * 0.5f + 0.2f, bY, 0.0f, 0.30f, 0.90f, depth + 0.4f, balustradeCol);
        gateMesh.append(balFront);
        gateMesh.append(balBack);
        gateMesh.append(balLeft);
        gateMesh.append(balRight);

        // ====================================================================
        // 4. LẦU NGŨ PHỤNG (FIVE-PHOENIX PAVILION - Double-Tiered Curved Roofs)
        // ====================================================================
        float pavY = bastionHeight + 0.30f;
        float pavW = 34.0f;
        float pavD = 10.5f;

        // Red Lacquer timber columns supporting Lower Tier Roof
        const int colCols = 8;
        for (int c = 0; c < colCols; ++c) {
            float frac = static_cast<float>(c) / (colCols - 1);
            float px = -pavW * 0.45f + frac * pavW * 0.90f;
            for (int s = -1; s <= 1; s += 2) {
                float pz = s * (pavD * 0.42f);
                auto col = Geometry::PolygonMesh::createPrismY(px, pavY, pz, 0.26f, 3.8f, 10, redPillarCol);
                gateMesh.append(col);
            }
        }

        // Lower Pavilion Ceiling & Beam lintel
        auto lowerLintel = Geometry::PolygonMesh::createBox(0.0f, pavY + 3.8f, 0.0f, pavW + 1.2f, 0.35f, pavD + 1.2f, darkTrimCol);
        gateMesh.append(lowerLintel);

        // Lower Tier Curved Asian Roof (Hoang Luu Ly Golden Glazed Tiles)
        auto lowerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, pavY + 4.15f, 0.0f,
            pavW + 3.2f, pavD + 3.2f,
            2.2f, 2.8f,
            goldRoofCol
        );
        gateMesh.append(lowerRoof);

        // Upper Mezzanine Pavilion Walls & Columns
        float upPavY = pavY + 4.15f + 1.8f;
        float upPavW = 24.0f;
        float upPavD = 7.8f;

        auto upChamber = Geometry::PolygonMesh::createBox(0.0f, upPavY + 1.4f, 0.0f, upPavW, 2.8f, upPavD, MathCG::ColorRGB(0.72f, 0.14f, 0.12f));
        gateMesh.append(upChamber);

        // Upper Tier Curved Asian Roof
        auto upperRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, upPavY + 2.8f, 0.0f,
            upPavW + 3.0f, upPavD + 3.0f,
            2.5f, 2.6f,
            goldRoofCol
        );
        gateMesh.append(upperRoof);

        // ====================================================================
        // 5. CENTRAL CLOCK TOWER (THÁP ĐỒNG HỒ TRUNG TÂM)
        // ====================================================================
        float towerBaseY = upPavY + 2.8f + 1.2f;
        float towerSideW = 6.4f;
        float towerBodyH = 7.5f;

        // Clock Tower Body (Soaring above the central gate)
        auto clockBody = Geometry::PolygonMesh::createBox(
            0.0f, towerBaseY + towerBodyH * 0.5f, 0.0f,
            towerSideW, towerBodyH, towerSideW,
            stoneCol
        );
        gateMesh.append(clockBody);

        // Golden corner pilasters & cornice moldings
        auto towerCornice = Geometry::PolygonMesh::createBox(
            0.0f, towerBaseY + towerBodyH + 0.25f, 0.0f,
            towerSideW + 1.2f, 0.50f, towerSideW + 1.2f,
            goldRoofCol
        );
        gateMesh.append(towerCornice);

        // Clock Tower Multi-Tier Pagoda Roof (Chóp Mái Tháp Đồng Hồ)
        auto towerRoof = Geometry::PolygonMesh::createCurvedAsianRoof(
            0.0f, towerBaseY + towerBodyH + 0.50f, 0.0f,
            towerSideW + 2.2f, towerSideW + 2.2f,
            2.6f, 1.8f,
            goldRoofCol
        );
        gateMesh.append(towerRoof);

        // Golden Pinnacle Spire at the very peak (H = 27.5m)
        float peakY = towerBaseY + towerBodyH + 0.50f + 2.6f;
        auto spire = Geometry::PolygonMesh::createPyramid(
            0.0f, peakY, 0.0f,
            1.2f, 1.2f, 2.4f,
            MathCG::ColorRGB(0.98f, 0.85f, 0.20f)
        );
        gateMesh.append(spire);

        // Compute all normals for the entire assembled gate
        gateMesh.computeNormals();
    }

    /**
     * @brief Render the 4-sided illuminated Clock Faces on the central tower
     */
    void drawClockFaces(bool isNightOrSunset) const {
        float clockY = bastionHeight + 0.30f + 4.15f + 1.8f + 2.8f + 1.2f + 3.8f; // ~23m
        float offset = 3.25f; // slightly proud of tower wall (towerSideW / 2 + 0.05)

        for (int side = 0; side < 4; ++side) {
            glPushMatrix();
            glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, clockY, offset);

            // Clock face bezel (Golden Toroid)
            glColor3f(0.95f, 0.78f, 0.15f);
            ParametricShapes::drawToroid(2.10f, 0.15f, 24, 8, false);

            // Dial disc (White with optional warm backlight emission)
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

            // Dark Roman/Hour markers around dial (12 ticks)
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
            // Hour hand (pointing to 2 o'clock)
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(0.85f * std::cos(MathCG::DEG2RAD * 30.0f), 0.85f * std::sin(MathCG::DEG2RAD * 30.0f), 0.03f);
            // Minute hand (pointing to 10 minutes past the hour)
            glVertex3f(0.0f, 0.0f, 0.03f);
            glVertex3f(1.35f * std::cos(MathCG::DEG2RAD * 60.0f), 1.35f * std::sin(MathCG::DEG2RAD * 60.0f), 0.03f);
            glEnd();
            glLineWidth(1.0f);

            // Center golden pin
            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawSphere(0.15f, 10, 8, false);

            glPopMatrix();
        }
    }

    /**
     * @brief Render festive imperial lanterns suspended along the pavilion eaves
     */
    void drawLanterns(bool isNightOrSunset, bool showNormals) const {
        (void)showNormals;
        float eaveY = bastionHeight + 0.30f + 4.15f;
        float lanternZ = depth * 0.5f + 1.2f;

        const float lanternX[] = { -14.0f, -8.0f, -2.5f, 2.5f, 8.0f, 14.0f };
        for (float lx : lanternX) {
            glPushMatrix();
            glTranslatef(lx, eaveY - 0.75f, lanternZ);

            // Suspension cord
            glColor3f(0.85f, 0.70f, 0.20f);
            ParametricShapes::drawCylinderY(0.025f, 0.75f, 6, 2, true, false);

            // Crimson lantern sphere
            if (isNightOrSunset) {
                GLfloat emit[4] = { 1.0f, 0.18f, 0.12f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
                glColor3f(1.0f, 0.25f, 0.15f);
            } else {
                glColor3f(0.88f, 0.16f, 0.12f);
            }
            ParametricShapes::drawSphere(0.42f, 12, 10, false);
            if (isNightOrSunset) {
                GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
            }

            // Bottom golden tassel
            glTranslatef(0.0f, -0.45f, 0.0f);
            glColor3f(0.95f, 0.80f, 0.15f);
            ParametricShapes::drawConeY(0.14f, 0.0f, -0.35f, 8, 2, true, false);

            glPopMatrix();
        }
    }
};

} // namespace AsiaParkRides

#endif // NGO_MON_GATE_H
