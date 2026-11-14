#ifndef HUD_H
#define HUD_H

/**
 * @file HUD.h
 * @brief Chapter 1 & 3: Heads-Up Display (HUD) Dashboard & Bitmap Text System
 *
 * Implements:
 *  - Chapter 3 glutBitmapCharacter on-screen bitmap text rendering
 *  - Real-time graphics pipeline state visualization:
 *      * Camera Orbit Coordinates: Azimuth (theta), Elevation (Phi), Distance (R), Viewplane (D)
 *      * Ride POV Tracking: Sun Wheel Cabin #0, Queen Cobra, Monorail
 *      * Projection Modes: Perspective (gluPerspective) vs Parallel Ortho (glOrtho)
 *      * Illumination & Shading: Gouraud (GL_SMOOTH) vs Lambert Flat (GL_FLAT)
 *      * Hidden Surface Removal: Back-face Culling (V . N >= 0) & Z-Buffer Depth Test
 *      * Time of Day & Dynamic HSV LED Hue angle
 *      * Bezier Roller Coaster real-time telemetry (parameter t, speed m/s)
 *      * Keyboard Shortcuts & Interactive Guide
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
     * @brief Draw a bitmap text string at (x, y) in DCS pixel coordinates
     */
    static void drawBitmapText(float x, float y, void* font, const char* text) {
        glRasterPos2f(x, y);
        for (int i = 0; text[i] != '\0'; ++i) {
            glutBitmapCharacter(font, text[i]);
        }
    }

    /**
     * @brief Render the complete on-screen telemetry dashboard
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
        float coasterSpeedMps
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

        // 1. Semi-transparent dark glass dashboard background at top-left
        const float panelX      = 12.0f;
        const float panelTop    = windowHeight - 12.0f;
        const float panelHeight = 258.0f;
        const float panelWidth  = 660.0f;
        const float panelBottom = panelTop - panelHeight;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Background quad
        glColor4f(0.03f, 0.06f, 0.12f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(panelX, panelTop);
        glVertex2f(panelX, panelBottom);
        glVertex2f(panelX + panelWidth, panelBottom);
        glVertex2f(panelX + panelWidth, panelTop);
        glEnd();

        // Neon border frame
        glColor4f(0.25f, 0.65f, 0.95f, 0.60f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(panelX, panelTop);
        glVertex2f(panelX, panelBottom);
        glVertex2f(panelX + panelWidth, panelBottom);
        glVertex2f(panelX + panelWidth, panelTop);
        glEnd();
        glDisable(GL_BLEND);

        // 2. Dashboard Text Output
        float y = panelTop - 22.0f;
        const float dy = 18.5f;
        char buf[256];

        // Header Title
        glColor3f(1.0f, 0.85f, 0.25f);
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_HELVETICA_18, "CONG VIEN CHAU A (DA NANG) - MO PHONG DO HOA 3D");
        y -= (dy + 5.0f);

        // Telemetry lines
        glColor3f(0.90f, 0.95f, 1.0f);

        std::snprintf(buf, sizeof(buf), "[C / 1-4] Camera    : %s", camera.getCameraModeName());
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[P]       Phep chieu: %s", camera.getProjectionModeName());
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[Quy dao] theta=%.1f do | Phi=%.1f do | R=%.1f | D=%.1f",
                      camera.thetaDeg, camera.phiDeg, camera.distanceR, camera.viewPlaneD);
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

        std::snprintf(buf, sizeof(buf), "[Bezier]  Queen Cobra t=%.3f | Van toc=%.1f m/s",
                      coasterParamT, coasterSpeedMps);
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

        glColor3f(0.65f, 0.85f, 1.0f);
        drawBitmapText(panelX + 14.0f, y, GLUT_BITMAP_9_BY_15,
                       "[H] An bang | [+/-] Zoom | [[ / ]] Doi D | [ESC] Thoat");

        // Restore OpenGL matrices & capabilities
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
