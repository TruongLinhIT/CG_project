#ifndef HUD_H
#define HUD_H

/**
 * @file HUD.h
 * @brief Chương 1 & 3: Hệ thống bảng điều khiển hiển thị trên màn hình (HUD) & Văn bản Bitmap
 *
 * Triển khai:
 *  - Chương 3: Vẽ văn bản bitmap trực tiếp lên màn hình bằng glutBitmapCharacter
 *  - Hiển thị trạng thái đường ống đồ họa (graphics pipeline) theo thời gian thực:
 *      * Tọa độ quỹ đạo Camera: Góc phương vị (theta), Góc nâng (Phi), Khoảng cách (R), Mặt phẳng chiếu (D)
 *      * Góc nhìn thứ nhất (POV) trên trò chơi: Cabin #0 của Sun Wheel, tàu lượn Queen Cobra, tàu điện Monorail
 *      * Chế độ phép chiếu: Phối cảnh (gluPerspective) so với Chiếu song song trực giao (glOrtho)
 *      * Chiếu sáng & Tô bóng: Nội suy Gouraud (GL_SMOOTH) so với Tô bóng phẳng Lambert (GL_FLAT)
 *      * Khử mặt khuất: Loại bỏ mặt sau Back-face Culling (V . N >= 0) & Kiểm tra độ sâu Z-Buffer
 *      * Thời gian trong ngày & Góc màu Hue của đèn LED HSV động
 *      * Thông số đo đạc thời gian thực của tàu lượn Bezier (tham số t, vận tốc m/s)
 *      * Phím tắt bàn phím & Hướng dẫn tương tác
 */

#include <GL/freeglut.h>
#include <cstdio>
#include <string>
#include "core/MathCG.h"
#include "core/Camera.h"
#include "core/Lighting.h"

namespace AsiaParkUI {

class HUD {
public:
    HUD() = default;

    /**
     * @brief Vẽ một chuỗi văn bản bitmap tại tọa độ điểm ảnh (x, y) trong hệ tọa độ thiết bị DCS
     */
    static void drawBitmapText(float x, float y, void* font, const char* text) {
        glRasterPos2f(x, y);
        for (int i = 0; text[i] != '\0'; ++i) {
            glutBitmapCharacter(font, text[i]);
        }
    }

    /**
     * @brief Hiển thị toàn bộ bảng điều khiển thông số (dashboard) trên màn hình
     */
    static void draw(
        int windowWidth, int windowHeight,
        const Camera& camera,
        const LightingSystem& lighting,
        bool wireframeMode,
        bool showNormals,
        bool backFaceCulling,
        bool zBufferEnabled,
        bool lakeReflection,
        bool showMinimap,
        bool isPaused,
        float coasterParamT,
        float coasterSpeedMps,
        float sunWheelSpeedDeg = 0.4f,
        bool sunWheelFastDemo = false,
        bool nightLightsOn = true
    ) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // 1. Nền bảng điều khiển kiểu kính tối bán trong suốt ở góc trên bên trái
        const float panelX      = 12.0f;
        const float panelTop    = windowHeight - 12.0f;
        const float panelHeight = 278.0f;
        const float panelWidth  = 660.0f;
        const float panelBottom = panelTop - panelHeight;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Hình tứ giác làm nền
        glColor4f(0.03f, 0.06f, 0.12f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(panelX, panelTop);
        glVertex2f(panelX, panelBottom);
        glVertex2f(panelX + panelWidth, panelBottom);
        glVertex2f(panelX + panelWidth, panelTop);
        glEnd();

        // Khung viền màu neon
        glColor4f(0.25f, 0.65f, 0.95f, 0.60f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(panelX, panelTop);
        glVertex2f(panelX, panelBottom);
        glVertex2f(panelX + panelWidth, panelBottom);
        glVertex2f(panelX + panelWidth, panelTop);
        glEnd();
        glDisable(GL_BLEND);

        // 2. Xuất nội dung văn bản trên bảng điều khiển
        float y = panelTop - 22.0f;
        const float dy = 18.5f;
        char buf[256];

        // Tiêu đề chính
        glColor3f(1.0f, 0.85f, 0.25f);
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_HELVETICA_18, "CONG VIEN CHAU A (DA NANG) - MO PHONG DO HOA 3D");
        y -= (dy + 5.0f);

        // Các dòng thông số trạng thái
        glColor3f(0.90f, 0.95f, 1.0f);

        std::snprintf(buf, sizeof(buf), "[C / 1-5] Camera    : %s", camera.getCameraModeName());
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[P]       Phep chieu: %s", camera.getProjectionModeName());
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        if (camera.cameraMode == CameraMode::DRONE_FREE) {
            std::snprintf(buf, sizeof(buf), "[Drone]   Vi tri (%.1f, %.1f, %.1f) | Huong %.0f/%.0f | %.0f m/s",
                          camera.dronePos.x, camera.dronePos.y, camera.dronePos.z,
                          camera.droneYawDeg, camera.dronePitchDeg, camera.droneSpeed);
        } else {
            std::snprintf(buf, sizeof(buf), "[Quy dao] theta=%.1f do | Phi=%.1f do | R=%.1f | D=%.1f",
                          camera.thetaDeg, camera.phiDeg, camera.distanceR, camera.viewPlaneD);
        }
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[Bezier]  Queen Cobra t=%.3f (%.1f m/s) | [V] SunWheel: %.1f do/s (%s)",
                      coasterParamT, coasterSpeedMps,
                      sunWheelSpeedDeg,
                      sunWheelFastDemo ? "TUA x5" : "THUC 1x");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[S]       To bong   : %s", lighting.getShadingModelName());
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[W]       Kieu ve   : %s | [N] Phap tuyen/Ctrl: %s",
                      wireframeMode ? "KHUNG DAY (LINE)" : "TO DAC (FILL)",
                      showNormals ? "BAT" : "TAT");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[B]       Khu mat sau: %s | [Z] Z-Buffer: %s",
                      backFaceCulling ? "BAT (CULL)" : "TAT",
                      zBufferEnabled  ? "BAT (DEPTH)" : "TAT");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[L]       Thoi gian : %s | [M] Minimap 2D: %s",
                      lighting.getTimeOfDayName(),
                      showMinimap ? "BAT" : "TAT");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[R] Phan xa ho: %s | [SPACE] %s | Chuot phai: Menu",
                      lakeReflection ? "BAT (y=0)" : "TAT",
                      isPaused ? "TAM DUNG" : "DANG CHAY");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        glColor3f(1.0f, 0.86f, 0.36f);
        std::snprintf(buf, sizeof(buf), "[G] Den LED ban dem: %s (Chuong 6: GL_LIGHT3..5 + Billboard)",
                      nightLightsOn ? "BAT" : "TAT");
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        glColor3f(0.65f, 0.85f, 1.0f);
        if (camera.cameraMode == CameraMode::DRONE_FREE) {
            drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15,
                           "[WASD] Bay | [E/Q] Cao | [F/+/-] Toc do | [Chuot] Xoay | [F2/F3]");
        } else {
            drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15,
                           "[H] An bang | [+/-] Zoom | [[ / ]] Doi D | [5] Drone | [ESC] Thoat");
        }

        // Khôi phục các ma trận và trạng thái OpenGL
        if (zBufferEnabled) glEnable(GL_DEPTH_TEST);
        if (lighting.lightingEnabled) glEnable(GL_LIGHTING);

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
};

} // namespace AsiaParkUI

#endif // HUD_H
