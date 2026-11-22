#ifndef MINIMAP_2D_H
#define MINIMAP_2D_H

/**
 * @file Minimap2D.h
 * @brief Chương 1 & 3: Bản đồ nhỏ Radar 2D, Thuật toán xén đoạn thẳng Cohen-Sutherland & Thuật toán Bresenham/Midpoint
 *
 * Triển khai tường minh các thuật toán trong đề cương Đồ họa Máy tính:
 *  - Chương 1: Đường ống biến đổi tọa độ 2D WCS -> NCS -> DCS:
 *      * WCS: Tọa độ mặt đất ngang trong thế giới 3D (x, z) thuộc [-135m, 135m]
 *      * NCS: Tọa độ thiết bị chuẩn hóa x0, y0 thuộc [0, 1]
 *      * DCS: Tọa độ màn hình vùng nhìn thiết bị (xd, yd) tính bằng điểm ảnh (pixel)
 *  - Chương 3: Thuật toán xén đoạn thẳng Cohen-Sutherland 4-bit:
 *      * Mã vùng (Outcodes): INSIDE (0000), LEFT (0001), RIGHT (0010), BOTTOM (0100), TOP (1000)
 *      * Kiểm tra bit: (c1 | c2) == 0 (Chấp nhận tầm thường), (c1 & c2) != 0 (Loại bỏ tầm thường)
 *      * Tính giao điểm giải tích với các cạnh biên cho các đoạn thẳng không tầm thường
 *  - Chương 3: Thuật toán đường tròn Midpoint (Bresenham Circle):
 *      * Tham số quyết định nguyên p_0 = 1 - R
 *      * Vẽ điểm đối xứng 8 phần tám (octant) qua GL_POINTS
 *      * Dùng cho các vòng tròn cự ly đồng tâm của radar (25m, 50m, 75m, 100m)
 *  - Chương 3: Thuật toán đoạn thẳng Bresenham:
 *      * Rời rạc hóa đoạn thẳng bằng số học nguyên sử dụng biến tích lũy sai số d
 *      * Dùng cho tia quét radar xoay tròn và đường chữ thập tâm ngắm radar
 *  - Lớp phủ các hạng mục công viên:
 *      * Ranh giới hồ trung tâm
 *      * Sun Wheel, Sky Tower, Carousel, Cổng Ngọ Môn, Thủy đình ven hồ
 *      * Đường ray tàu lượn Bezier & tàu điện Monorail (được xén bằng thuật toán Cohen-Sutherland)
 *      * Vị trí Camera & hình quạt thể hiện hướng nhìn (FOV)
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include "core/MathCG.h"

namespace AsiaParkUI {

// ============================================================================
// 1. MÃ VÙNG 4-BIT & XÉN ĐOẠN THẲNG COHEN-SUTHERLAND (Chương 3)
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
     * @brief Xén một đoạn thẳng (x0, y0) -> (x1, y1) theo khung hình chữ nhật [xmin, ymin, xmax, ymax]
     * @return true nếu một phần đoạn thẳng nhìn thấy được, false nếu bị loại bỏ hoàn toàn
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
                // Chấp nhận tầm thường: Cả hai đầu mút đều nằm bên trong
                accept = true;
                break;
            } else if (code0 & code1) {
                // Loại bỏ tầm thường: Cả hai đầu mút cùng nằm ở một vùng bên ngoài
                break;
            } else {
                // Tính giao điểm với cạnh biên của khung xén
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
// 2. RỜI RẠC HÓA BRESENHAM / MIDPOINT (Chương 3)
// ============================================================================

namespace RasterAlgorithms {
    /**
     * @brief Thuật toán đường tròn Midpoint (Chương 3) vẽ các điểm đối xứng 8 hướng
     */
    inline void drawMidpointCircle(int cx, int cy, int radius) {
        int x = 0;
        int y = radius;
        int p = 1 - radius; // Tham số quyết định ban đầu p0 = 1 - R

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
     * @brief Thuật toán đoạn thẳng Bresenham (Chương 3) với biến tích lũy sai số nguyên
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
// 3. LỚP MINIMAP 2D (HỆ THỐNG RADAR)
// ============================================================================

class Minimap2D {
public:
    // Vị trí & kích thước vùng nhìn (viewport) tính bằng điểm ảnh màn hình
    float vpSize   = 220.0f;
    float margin   = 16.0f;

    // Giới hạn tọa độ thế giới của công viên (X: Tây <-> Đông, Z: Bắc <-> Nam)
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
     * @brief Chương 1: Ánh xạ tọa độ thế giới 3D (x, z) -> Tọa độ vùng nhìn thiết bị 2D DCS (xd, yd)
     */
    MathCG::Vec2 worldToScreen(float wx, float wz, int screenWidth) const {
        float vpX = screenWidth - vpSize - margin;
        float vpY = margin;

        // WCS -> NCS [0, 1]
        float nx = (wx - wcsMinX) / (wcsMaxX - wcsMinX);
        float ny = (wz - wcsMinZ) / (wcsMaxZ - wcsMinZ); // Đảo ngược để hướng Bắc (-Z) nằm ở phía trên
        ny = 1.0f - ny; // Hướng Bắc (-Z) ánh xạ lên phần trên của DCS

        // NCS -> DCS
        float sx = vpX + nx * vpSize;
        float sy = vpY + ny * vpSize;
        return MathCG::Vec2(sx, sy);
    }

    /**
     * @brief Hiển thị toàn bộ lớp phủ bản đồ nhỏ radar 2D
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

        // 1. Hình tròn nền radar bán trong suốt
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

        // 2. Chương 3: Thuật toán đường tròn Midpoint cho các vòng cự ly (25m, 50m, 75m, 100m)
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

        // Vòng viền neon phía ngoài
        glColor4f(0.35f, 0.85f, 1.0f, 0.85f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= 48; ++i) {
            float th = (static_cast<float>(i) / 48) * MathCG::TWO_PI;
            glVertex2f(centerX + radarRadius * std::cos(th), centerY + radarRadius * std::sin(th));
        }
        glEnd();

        // 3. Chương 3: Đường chữ thập tâm ngắm Bresenham (Bắc-Nam & Đông-Tây)
        glColor4f(0.25f, 0.65f, 0.85f, 0.40f);
        RasterAlgorithms::drawBresenhamLine(
            static_cast<int>(centerX - radarRadius), static_cast<int>(centerY),
            static_cast<int>(centerX + radarRadius), static_cast<int>(centerY)
        );
        RasterAlgorithms::drawBresenhamLine(
            static_cast<int>(centerX), static_cast<int>(centerY - radarRadius),
            static_cast<int>(centerX), static_cast<int>(centerY + radarRadius)
        );

        // 4. Tia quét radar xoay tròn (Thuật toán đoạn thẳng Bresenham)
        float sweepRad = radarSweepAngleDeg * MathCG::DEG2RAD;
        int sweepX = static_cast<int>(centerX + radarRadius * std::cos(sweepRad));
        int sweepY = static_cast<int>(centerY + radarRadius * std::sin(sweepRad));
        glColor4f(0.40f, 1.0f, 0.65f, 0.75f);
        RasterAlgorithms::drawBresenhamLine(static_cast<int>(centerX), static_cast<int>(centerY), sweepX, sweepY);

        // Vệt sáng hình quạt mờ phía sau tia quét
        glColor4f(0.20f, 0.90f, 0.50f, 0.15f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= 10; ++i) {
            float trailTh = (radarSweepAngleDeg - i * 3.5f) * MathCG::DEG2RAD;
            glVertex2f(centerX + radarRadius * std::cos(trailTh), centerY + radarRadius * std::sin(trailTh));
        }
        glEnd();

        // 5. Vẽ Hồ trung tâm trên Minimap
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

        // 6. Vẽ các đường ray được xén bằng thuật toán Cohen-Sutherland (Chương 3)
        drawClippedPolyline(queenCobraSamples, screenWidth, vpX, vpY, vpX + vpSize, vpY + vpSize, 0.95f, 0.35f, 0.15f);
        drawClippedPolyline(monorailSamples,  screenWidth, vpX, vpY, vpX + vpSize, vpY + vpSize, 0.30f, 0.85f, 0.95f);

        // 7. Các điểm đánh dấu (Blip) công trình trong công viên
        // Vòng quay Sun Wheel
        MathCG::Vec2 swPt = worldToScreen(sunWheelCenter.x, sunWheelCenter.z, screenWidth);
        float swRScreen = (sunWheelRadius / (wcsMaxX - wcsMinX)) * vpSize;
        glColor4f(1.0f, 0.85f, 0.15f, 0.85f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(swPt.x - swRScreen, swPt.y);
        glVertex2f(swPt.x + swRScreen, swPt.y);
        glEnd();
        drawBlip(swPt.x, swPt.y, 4.0f, 1.0f, 0.85f, 0.15f);

        // Tháp rơi tự do Sky Tower
        MathCG::Vec2 stPt = worldToScreen(skyTowerPos.x, skyTowerPos.z, screenWidth);
        drawBlip(stPt.x, stPt.y, 3.5f, 0.95f, 0.35f, 0.85f);

        // Vòng quay ngựa gỗ Carousel
        MathCG::Vec2 crPt = worldToScreen(carouselPos.x, carouselPos.z, screenWidth);
        drawBlip(crPt.x, crPt.y, 3.5f, 0.95f, 0.65f, 0.20f);

        // Cổng Ngọ Môn
        MathCG::Vec2 nmPt = worldToScreen(ngoMonGatePos.x, ngoMonGatePos.z, screenWidth);
        drawBlip(nmPt.x, nmPt.y, 4.0f, 0.90f, 0.20f, 0.15f);

        // Điểm đánh dấu vị trí các đoàn tàu
        MathCG::Vec2 qcTrainPt = worldToScreen(queenCobraTrainPos.x, queenCobraTrainPos.z, screenWidth);
        drawBlip(qcTrainPt.x, qcTrainPt.y, 4.5f, 1.0f, 0.20f, 0.10f); // Đoàn tàu màu đỏ
        MathCG::Vec2 mrTrainPt = worldToScreen(monorailTrainPos.x, monorailTrainPos.z, screenWidth);
        drawBlip(mrTrainPt.x, mrTrainPt.y, 4.5f, 0.20f, 1.0f, 0.40f); // Đoàn tàu màu xanh lá

        // 8. Vị trí Camera & Hình quạt thể hiện góc nhìn
        MathCG::Vec2 camPt = worldToScreen(camEye.x, camEye.z, screenWidth);
        drawBlip(camPt.x, camPt.y, 4.5f, 1.0f, 1.0f, 1.0f); // Chấm trắng biểu thị Camera

        // Hình nón góc nhìn Camera (FOV = 60 độ, hướng dọc theo -camThetaDeg)
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

        // 9. Nhãn 4 hướng chính (N, S, E, W)
        glColor3f(0.95f, 0.95f, 1.0f);
        drawText(centerX - 4.0f, vpY + vpSize - 18.0f, "N");
        drawText(centerX - 4.0f, vpY + 6.0f, "S");
        drawText(vpX + vpSize - 14.0f, centerY - 5.0f, "E");
        drawText(vpX + 6.0f, centerY - 5.0f, "W");

        // Nhãn tiêu đề
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
