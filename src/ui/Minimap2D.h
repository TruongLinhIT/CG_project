#ifndef MINIMAP_2D_H
#define MINIMAP_2D_H

/**
 * @file Minimap2D.h
 * @brief Chapter 1 & 3: 2D Radar Minimap, Cohen-Sutherland Line Clipping & Bresenham/Midpoint Algorithms
 *
 * Implements explicit Computer Graphics syllabus algorithms:
 *  - Chapter 1: WCS -> NCS -> DCS 2D Coordinate Transformation Pipeline:
 *      * WCS: World 3D horizontal ground coordinates (x, z) in [-135m, 135m]
 *      * NCS: Normalized Device Coordinates x0, y0 in [0, 1]
 *      * DCS: Device Viewport Screen Coordinates (xd, yd) in pixels
 *  - Chapter 3: 4-bit Cohen-Sutherland Line Clipping Algorithm:
 *      * Outcodes: INSIDE (0000), LEFT (0001), RIGHT (0010), BOTTOM (0100), TOP (1000)
 *      * Bitwise tests: (c1 | c2) == 0 (Trivial Accept), (c1 & c2) != 0 (Trivial Reject)
 *      * Analytical boundary edge intersections for non-trivial segments
 *  - Chapter 3: Midpoint Circle Algorithm (Bresenham Circle):
 *      * Integer decision parameter p_0 = 1 - R
 *      * 8-way octant symmetry point plotting via GL_POINTS
 *      * Used for concentric radar range rings (25m, 50m, 75m, 100m)
 *  - Chapter 3: Bresenham Line Algorithm:
 *      * Integer arithmetic line rasterization using error accumulator d
 *      * Used for rotating radar sweeping beam and radar reticle crosshairs
 *  - Park Features Overlay:
 *      * Central Lake boundary
 *      * Sun Wheel, Sky Tower, Carousel, Ngo Mon Gate, Lakeside Pavilion
 *      * Bezier Roller Coaster & Monorail tracks (clipped with Cohen-Sutherland)
 *      * Camera position & viewing orientation FOV wedge
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include "core/MathCG.h"

namespace AsiaParkUI {

// ============================================================================
// 1. COHEN-SUTHERLAND 4-BIT OUTCODES & CLIPPING (Chapter 3)
// ============================================================================

namespace CohenSutherland {
    constexpr int INSIDE = 0; // 0000
    constexpr int LEFT   = 1; // 0001
    constexpr int RIGHT  = 2; // 0010
    constexpr int BOTTOM = 4; // 0100
    constexpr int TOP    = 8; // 1000

    inline int computeOutCode(float x, float y, float xmin, float ymin, float xmax, float ymax) {
        int code = INSIDE;
        if (x < xmin)      code |= LEFT;
        else if (x > xmax) code |= RIGHT;
        if (y < ymin)      code |= BOTTOM;
        else if (y > ymax) code |= TOP;
        return code;
    }

    /**
     * @brief Clips a line segment (x0, y0) -> (x1, y1) against rectangular box [xmin, ymin, xmax, ymax]
     * @return true if part of line is visible, false if entirely rejected
     */
    inline bool clipLine(
        float& x0, float& y0,
        float& x1, float& y1,
        float xmin, float ymin, float xmax, float ymax
    ) {
        int code0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
        int code1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
        bool accept = false;

        while (true) {
            if ((code0 | code1) == 0) {
                // Trivial Accept: Both endpoints inside
                accept = true;
                break;
            } else if (code0 & code1) {
                // Trivial Reject: Both endpoints share an outside zone
                break;
            } else {
                // Calculate intersection with bounding edge
                float x = 0.0f, y = 0.0f;
                int outcodeOut = code0 ? code0 : code1;

                if (outcodeOut & TOP) {
                    x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                    y = ymax;
                } else if (outcodeOut & BOTTOM) {
                    x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                    y = ymin;
                } else if (outcodeOut & RIGHT) {
                    y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                    x = xmax;
                } else if (outcodeOut & LEFT) {
                    y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                    x = xmin;
                }

                if (outcodeOut == code0) {
                    x0 = x;
                    y0 = y;
                    code0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
                } else {
                    x1 = x;
                    y1 = y;
                    code1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
                }
            }
        }
        return accept;
    }
} // namespace CohenSutherland

// ============================================================================
// 2. BRESENHAM / MIDPOINT RASTERIZATION (Chapter 3)
// ============================================================================

namespace RasterAlgorithms {
    /**
     * @brief Chapter 3 Midpoint Circle Algorithm plotting 8-way symmetric points
     */
    inline void drawMidpointCircle(int cx, int cy, int radius) {
        int x = 0;
        int y = radius;
        int p = 1 - radius; // Initial decision parameter p0 = 1 - R

        auto plot8 = [&](int px, int py) {
            glVertex2i(cx + px, cy + py);
            glVertex2i(cx - px, cy + py);
            glVertex2i(cx + px, cy - py);
            glVertex2i(cx - px, cy - py);
            glVertex2i(cx + py, cy + px);
            glVertex2i(cx - py, cy + px);
            glVertex2i(cx + py, cy - px);
            glVertex2i(cx - py, cy - px);
        };

        glBegin(GL_POINTS);
        plot8(x, y);

        while (x < y) {
            x++;
            if (p < 0) {
                p += 2 * x + 1;
            } else {
                y--;
                p += 2 * (x - y) + 1;
            }
            plot8(x, y);
        }
        glEnd();
    }

    /**
     * @brief Chapter 3 Bresenham Line Algorithm with integer error accumulator
     */
    inline void drawBresenhamLine(int x0, int y0, int x1, int y1) {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        glBegin(GL_POINTS);
        while (true) {
            glVertex2i(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
        glEnd();
    }
} // namespace RasterAlgorithms

// ============================================================================
// 3. MINIMAP 2D CLASS (RADAR SYSTEM)
// ============================================================================

class Minimap2D {
public:
    // Viewport position & size in screen pixels
    float vpSize   = 220.0f;
    float margin   = 16.0f;

    // Park World Coordinate Bounds (X: West <-> East, Z: North <-> South)
    float wcsMinX  = -135.0f;
    float wcsMaxX  =  135.0f;
    float wcsMinZ  = -135.0f;
    float wcsMaxZ  =  135.0f;

    float radarSweepAngleDeg = 0.0f;

    Minimap2D() = default;

    void update(float dt) {
        radarSweepAngleDeg = std::fmod(radarSweepAngleDeg + 90.0f * dt, 360.0f);
    }

    /**
     * @brief Chapter 1: Map 3D World (x, z) -> 2D Viewport DCS (xd, yd)
     */
    MathCG::Vec2 worldToScreen(float wx, float wz, int screenWidth) const {
        float vpX = screenWidth - vpSize - margin;
        float vpY = margin;

        // WCS -> NCS [0, 1]
        float nx = (wx - wcsMinX) / (wcsMaxX - wcsMinX);
        float ny = (wz - wcsMinZ) / (wcsMaxZ - wcsMinZ); // North (-Z) at top or bottom? Invert for North up!
        ny = 1.0f - ny; // North (-Z) maps to upper DCS

        // NCS -> DCS
        float sx = vpX + nx * vpSize;
        float sy = vpY + ny * vpSize;
        return MathCG::Vec2(sx, sy);
    }

    /**
     * @brief Render the complete 2D Minimap radar overlay
     */
    void draw(
        int screenWidth, int screenHeight,
        const MathCG::Vec3& camEye,
        float camThetaDeg,
        const MathCG::Vec3& sunWheelCenter,
        float sunWheelRadius,
        const std::vector<MathCG::Vec3>& queenCobraSamples,
        const std::vector<MathCG::Vec3>& monorailSamples,
        const MathCG::Vec3& queenCobraTrainPos,
        const MathCG::Vec3& monorailTrainPos,
        const MathCG::Vec3& skyTowerPos,
        const MathCG::Vec3& carouselPos,
        const MathCG::Vec3& ngoMonGatePos
    ) {
        (void)screenHeight;

        float vpX = screenWidth - vpSize - margin;
        float vpY = margin;
        float centerX = vpX + vpSize * 0.5f;
        float centerY = vpY + vpSize * 0.5f;
        float radarRadius = vpSize * 0.48f;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // 1. Semi-transparent radar background circle
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.02f, 0.06f, 0.12f, 0.82f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= 40; ++i) {
            float th = (static_cast<float>(i) / 40) * MathCG::TWO_PI;
            glVertex2f(centerX + radarRadius * std::cos(th), centerY + radarRadius * std::sin(th));
        }
        glEnd();

        // 2. Chapter 3: Midpoint Circle Algorithm for Range Rings (25m, 50m, 75m, 100m)
        glColor4f(0.18f, 0.55f, 0.75f, 0.45f);
        glPointSize(1.2f);
        int ring25  = static_cast<int>(radarRadius * 0.25f);
        int ring50  = static_cast<int>(radarRadius * 0.50f);
        int ring75  = static_cast<int>(radarRadius * 0.75f);
        int ring100 = static_cast<int>(radarRadius);
        RasterAlgorithms::drawMidpointCircle(static_cast<int>(centerX), static_cast<int>(centerY), ring25);
        RasterAlgorithms::drawMidpointCircle(static_cast<int>(centerX), static_cast<int>(centerY), ring50);
        RasterAlgorithms::drawMidpointCircle(static_cast<int>(centerX), static_cast<int>(centerY), ring75);
        RasterAlgorithms::drawMidpointCircle(static_cast<int>(centerX), static_cast<int>(centerY), ring100);

        // Outer neon border ring
        glColor4f(0.35f, 0.85f, 1.0f, 0.85f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= 48; ++i) {
            float th = (static_cast<float>(i) / 48) * MathCG::TWO_PI;
            glVertex2f(centerX + radarRadius * std::cos(th), centerY + radarRadius * std::sin(th));
        }
        glEnd();

        // 3. Chapter 3: Bresenham Crosshairs (North-South & East-West)
        glColor4f(0.25f, 0.65f, 0.85f, 0.40f);
        RasterAlgorithms::drawBresenhamLine(
            static_cast<int>(centerX - radarRadius), static_cast<int>(centerY),
            static_cast<int>(centerX + radarRadius), static_cast<int>(centerY)
        );
        RasterAlgorithms::drawBresenhamLine(
            static_cast<int>(centerX), static_cast<int>(centerY - radarRadius),
            static_cast<int>(centerX), static_cast<int>(centerY + radarRadius)
        );

        // 4. Rotating Radar Sweeping Beam (Bresenham Line)
        float sweepRad = radarSweepAngleDeg * MathCG::DEG2RAD;
        int sweepX = static_cast<int>(centerX + radarRadius * std::cos(sweepRad));
        int sweepY = static_cast<int>(centerY + radarRadius * std::sin(sweepRad));
        glColor4f(0.40f, 1.0f, 0.65f, 0.75f);
        RasterAlgorithms::drawBresenhamLine(static_cast<int>(centerX), static_cast<int>(centerY), sweepX, sweepY);

        // Faint sweep fan trail
        glColor4f(0.20f, 0.90f, 0.50f, 0.15f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= 10; ++i) {
            float trailTh = (radarSweepAngleDeg - i * 3.5f) * MathCG::DEG2RAD;
            glVertex2f(centerX + radarRadius * std::cos(trailTh), centerY + radarRadius * std::sin(trailTh));
        }
        glEnd();

        // 5. Draw Central Lake on Minimap
        MathCG::Vec2 lakeNW = worldToScreen(-58.0f, -62.0f, screenWidth);
        MathCG::Vec2 lakeSW = worldToScreen(-58.0f,  16.0f, screenWidth);
        MathCG::Vec2 lakeSE = worldToScreen( 58.0f,  16.0f, screenWidth);
        MathCG::Vec2 lakeNE = worldToScreen( 58.0f, -62.0f, screenWidth);
        glColor4f(0.08f, 0.35f, 0.65f, 0.50f);
        glBegin(GL_QUADS);
        glVertex2f(lakeNW.x, lakeNW.y);
        glVertex2f(lakeSW.x, lakeSW.y);
        glVertex2f(lakeSE.x, lakeSE.y);
        glVertex2f(lakeNE.x, lakeNE.y);
        glEnd();
        glColor4f(0.20f, 0.65f, 0.95f, 0.70f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(lakeNW.x, lakeNW.y);
        glVertex2f(lakeSW.x, lakeSW.y);
        glVertex2f(lakeSE.x, lakeSE.y);
        glVertex2f(lakeNE.x, lakeNE.y);
        glEnd();

        // 6. Draw Tracks clipped with Chapter 3 Cohen-Sutherland Clipping
        drawClippedPolyline(queenCobraSamples, screenWidth, vpX, vpY, vpX + vpSize, vpY + vpSize, 0.95f, 0.35f, 0.15f);
        drawClippedPolyline(monorailSamples,  screenWidth, vpX, vpY, vpX + vpSize, vpY + vpSize, 0.30f, 0.85f, 0.95f);

        // 7. Park Architecture Blips
        // Sun Wheel
        MathCG::Vec2 swPt = worldToScreen(sunWheelCenter.x, sunWheelCenter.z, screenWidth);
        float swRScreen = (sunWheelRadius / (wcsMaxX - wcsMinX)) * vpSize;
        glColor4f(1.0f, 0.85f, 0.15f, 0.85f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(swPt.x - swRScreen, swPt.y);
        glVertex2f(swPt.x + swRScreen, swPt.y);
        glEnd();
        drawBlip(swPt.x, swPt.y, 4.0f, 1.0f, 0.85f, 0.15f);

        // Sky Tower
        MathCG::Vec2 stPt = worldToScreen(skyTowerPos.x, skyTowerPos.z, screenWidth);
        drawBlip(stPt.x, stPt.y, 3.5f, 0.95f, 0.35f, 0.85f);

        // Carousel
        MathCG::Vec2 crPt = worldToScreen(carouselPos.x, carouselPos.z, screenWidth);
        drawBlip(crPt.x, crPt.y, 3.5f, 0.95f, 0.65f, 0.20f);

        // Ngo Mon Gate
        MathCG::Vec2 nmPt = worldToScreen(ngoMonGatePos.x, ngoMonGatePos.z, screenWidth);
        drawBlip(nmPt.x, nmPt.y, 4.0f, 0.90f, 0.20f, 0.15f);

        // Train Positions Blips
        MathCG::Vec2 qcTrainPt = worldToScreen(queenCobraTrainPos.x, queenCobraTrainPos.z, screenWidth);
        drawBlip(qcTrainPt.x, qcTrainPt.y, 4.5f, 1.0f, 0.20f, 0.10f); // Red train
        MathCG::Vec2 mrTrainPt = worldToScreen(monorailTrainPos.x, monorailTrainPos.z, screenWidth);
        drawBlip(mrTrainPt.x, mrTrainPt.y, 4.5f, 0.20f, 1.0f, 0.40f); // Green train

        // 8. Camera Position & Viewing Cone Wedge
        MathCG::Vec2 camPt = worldToScreen(camEye.x, camEye.z, screenWidth);
        drawBlip(camPt.x, camPt.y, 4.5f, 1.0f, 1.0f, 1.0f); // White camera dot

        // Camera viewing cone (FOV = 60 deg, looking along -camThetaDeg)
        float camLookAngleRad = (-camThetaDeg + 90.0f) * MathCG::DEG2RAD;
        float coneLen = 22.0f;
        float fovHalfRad = 28.0f * MathCG::DEG2RAD;

        glColor4f(1.0f, 1.0f, 0.4f, 0.25f);
        glBegin(GL_TRIANGLES);
        glVertex2f(camPt.x, camPt.y);
        glVertex2f(camPt.x + coneLen * std::cos(camLookAngleRad - fovHalfRad), camPt.y + coneLen * std::sin(camLookAngleRad - fovHalfRad));
        glVertex2f(camPt.x + coneLen * std::cos(camLookAngleRad + fovHalfRad), camPt.y + coneLen * std::sin(camLookAngleRad + fovHalfRad));
        glEnd();

        glColor4f(1.0f, 1.0f, 0.6f, 0.85f);
        glBegin(GL_LINES);
        glVertex2f(camPt.x, camPt.y);
        glVertex2f(camPt.x + (coneLen + 4.0f) * std::cos(camLookAngleRad), camPt.y + (coneLen + 4.0f) * std::sin(camLookAngleRad));
        glEnd();

        // 9. Cardinal Direction Labels (N, S, E, W)
        glColor3f(0.95f, 0.95f, 1.0f);
        drawText(centerX - 4.0f, vpY + vpSize - 18.0f, "N");
        drawText(centerX - 4.0f, vpY + 6.0f, "S");
        drawText(vpX + vpSize - 14.0f, centerY - 5.0f, "E");
        drawText(vpX + 6.0f, centerY - 5.0f, "W");

        // Title tag
        glColor3f(0.35f, 0.85f, 1.0f);
        drawText(vpX + 12.0f, vpY + vpSize + 6.0f, "RADAR MINIMAP (2D)");

        glDisable(GL_BLEND);
        glLineWidth(1.0f);
        glPointSize(1.0f);
    }

private:
    void drawBlip(float x, float y, float size, float r, float g, float b) const {
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x - size, y - size);
        glVertex2f(x + size, y - size);
        glVertex2f(x + size, y + size);
        glVertex2f(x - size, y + size);
        glEnd();
    }

    void drawText(float x, float y, const char* str) const {
        glRasterPos2f(x, y);
        for (int i = 0; str[i] != '\0'; ++i) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[i]);
        }
    }

    void drawClippedPolyline(
        const std::vector<MathCG::Vec3>& waypoints,
        int screenWidth,
        float xmin, float ymin, float xmax, float ymax,
        float r, float g, float b
    ) const {
        if (waypoints.size() < 2) return;

        glColor3f(r, g, b);
        glLineWidth(1.6f);
        glBegin(GL_LINES);
        for (size_t i = 0; i < waypoints.size(); ++i) {
            size_t nextI = (i + 1) % waypoints.size();
            MathCG::Vec2 p0 = worldToScreen(waypoints[i].x, waypoints[i].z, screenWidth);
            MathCG::Vec2 p1 = worldToScreen(waypoints[nextI].x, waypoints[nextI].z, screenWidth);

            float cx0 = p0.x, cy0 = p0.y;
            float cx1 = p1.x, cy1 = p1.y;

            if (CohenSutherland::clipLine(cx0, cy0, cx1, cy1, xmin, ymin, xmax, ymax)) {
                glVertex2f(cx0, cy0);
                glVertex2f(cx1, cy1);
            }
        }
        glEnd();
        glLineWidth(1.0f);
    }
};

} // namespace AsiaParkUI

#endif // MINIMAP_2D_H
