/**
 * @file main.cpp
 * @brief Entry Point, Interactive State Machine, Camera & GLUT Menu for
 *        "3D Simulation of Asia Park (Da Nang)" - C++ / FreeGLUT
 *
 * Step 1 Deliverable:
 *  - Complete GLUT Window & 60 FPS Timer Loop
 *  - Complete Interactive State Machine (Chapter 1 - Chapter 6 toggles)
 *  - Spherical Orbit Camera (theta, Phi, R, D) + Ride Cameras + Perspective/Ortho Projections
 *  - Right-click Hierarchical GLUT Menu + Keyboard/Mouse Callbacks
 *  - Step 1 Interactive Scene Preview (Planar Lake Reflection at y=0, Hierarchical Sun Wheel
 *    preview with HSV->RGB night LEDs, Bezier track preview, and glutBitmapCharacter HUD)
 */

#include <GL/freeglut.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>

#include "core/MathCG.h"
#include "core/Camera.h"
#include "core/Lighting.h"

// ============================================================================
// GLOBAL SIMULATION & GRAPHICS PIPELINE STATE MACHINE
// ============================================================================

struct SimulationState {
    int windowWidth  = 1280;
    int windowHeight = 768;

    // Animation state (60 FPS timer)
    bool  isPaused          = false;
    float animSpeed         = 1.0f;
    float sunWheelAngleDeg  = 0.0f;  // alpha: main wheel rotation angle
    float carouselAngleDeg  = 0.0f;  // carousel rotation angle
    float carouselPhase     = 0.0f;  // horse vertical oscillation phase
    float skyTowerTime      = 0.0f;  // drives drop tower cycle
    float coasterParamT     = 0.0f;  // parameter t in [0, 1] along Bezier track

    // Chapter 4 & 6 Graphics Pipeline Toggles
    bool wireframeMode      = false; // GL_LINE vs GL_FILL
    bool showNormalsAndCtrl = false; // Debug: surface normals N & Bezier control polygon
    bool backFaceCulling    = true;  // GL_CULL_FACE (V . N >= 0)
    bool zBufferEnabled     = true;  // GL_DEPTH_TEST
    bool lakeReflection     = true;  // Planar reflection across y = 0 + GL_BLEND water
    bool showMinimap        = true;  // 2D WCS->NCS->DCS Minimap with Cohen-Sutherland clipping
    bool showHUD            = true;  // On-screen glutBitmapCharacter state overlay

    // Sun Wheel Center I(x0, y0, z0) and Radius
    MathCG::Vec3 sunWheelCenter = MathCG::Vec3(0.0f, 38.0f, -35.0f);
    float        sunWheelRadius = 30.0f;
    int          numCabins      = 12;

    // Preview Bezier Control Points P0..P6 for Queen Cobra Coaster (Step 1 camera & curve test)
    std::vector<MathCG::Vec3> previewBezierPts = {
        MathCG::Vec3(-75.0f,  4.0f,  45.0f),
        MathCG::Vec3(-55.0f, 38.0f,  10.0f),
        MathCG::Vec3(-15.0f,  6.0f, -20.0f),
        MathCG::Vec3( 25.0f, 44.0f,   5.0f),
        MathCG::Vec3( 65.0f,  8.0f,  40.0f),
        MathCG::Vec3( 30.0f, 22.0f,  75.0f),
        MathCG::Vec3(-75.0f,  4.0f,  45.0f)
    };

    // Mouse interaction state
    int  lastMouseX     = 0;
    int  lastMouseY     = 0;
    bool leftMouseDown  = false;
    bool midMouseDown   = false;
};

static SimulationState gState;
static Camera          gCamera;
static LightingSystem  gLighting;

// ============================================================================
// GLUT RIGHT-CLICK MENU IDENTIFIERS
// ============================================================================

enum MenuOption {
    MENU_CAM_ORBIT = 10,
    MENU_CAM_SUNWHEEL,
    MENU_CAM_COASTER,
    MENU_CAM_RESET,

    MENU_PROJ_PERSPECTIVE = 20,
    MENU_PROJ_ORTHO_TOP,
    MENU_PROJ_ORTHO_FRONT,
    MENU_PROJ_ORTHO_SIDE,

    MENU_SHADE_GOURAUD = 30,
    MENU_SHADE_LAMBERT,
    MENU_TOGGLE_WIREFRAME,
    MENU_TOGGLE_CULLING,
    MENU_TOGGLE_ZBUFFER,
    MENU_TOGGLE_NORMALS,
    MENU_TOGGLE_REFLECTION,

    MENU_TIME_DAY = 40,
    MENU_TIME_SUNSET,
    MENU_TIME_NIGHT,

    MENU_ANIM_TOGGLE = 50,
    MENU_ANIM_FASTER,
    MENU_ANIM_SLOWER,
    MENU_TOGGLE_HUD,
    MENU_TOGGLE_MINIMAP,
    MENU_EXIT = 99
};

// ============================================================================
// HELPER FUNCTIONS FOR RIDE CAMERA POSITIONS & STEP 1 SCENE PREVIEW
// ============================================================================

/**
 * @brief Compute World Position of Cabin #0 on the Sun Wheel using
 *        Chapter 4 rotation around center I(x0, y0, z0):
 *        P_cabin = T(I) * R_z(alpha) * (R_wheel, 0, 0)
 */
static MathCG::Vec3 getTrackedCabinWorldPos() {
    float alphaRad = gState.sunWheelAngleDeg * MathCG::DEG2RAD;
    float localX   = gState.sunWheelRadius * std::cos(alphaRad);
    float localY   = gState.sunWheelRadius * std::sin(alphaRad);
    return MathCG::Vec3(
        gState.sunWheelCenter.x + localX,
        gState.sunWheelCenter.y + localY - 2.0f,
        gState.sunWheelCenter.z
    );
}

/**
 * @brief Draw a bitmap text string at 2D pixel position (x, y) using glutBitmapCharacter (Chapter 3)
 */
static void drawBitmapText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}

/**
 * @brief Step 1 Parametric Ruled Cylinder helper:
 *        X(u,v) = r*cos(u), Y(u,v) = r*sin(u), Z(u,v) = h*v  (u in [0, 2pi], v in [0, 1])
 */
static void drawParametricCylinder(float r, float h, int slices, int stacks, bool drawNormals) {
    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float z0 = h * v0;
        float z1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u   = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu  = std::cos(u);
            float su  = std::sin(u);
            glNormal3f(cu, su, 0.0f);
            glVertex3f(r * cu, r * su, z1);
            glVertex3f(r * cu, r * su, z0);
        }
        glEnd();
    }

    if (drawNormals) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        for (int i = 0; i < slices; i += 2) {
            float u  = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u), su = std::sin(u);
            float px = r * cu, py = r * su, pz = 0.5f * h;
            glVertex3f(px, py, pz);
            glVertex3f(px + cu * 1.5f, py + su * 1.5f, pz);
        }
        glEnd();
        if (gLighting.lightingEnabled) glEnable(GL_LIGHTING);
    }
}

/**
 * @brief Step 1 Parametric Toroid (Surface of Revolution) helper:
 *        X(u,v) = (R + a*cos(v))*cos(u)
 *        Y(u,v) = (R + a*cos(v))*sin(u)
 *        Z(u,v) = a*sin(v)
 */
static void drawParametricToroid(float R, float a, int uSteps, int vSteps, bool drawNormals) {
    for (int i = 0; i < uSteps; ++i) {
        float u0 = (static_cast<float>(i)     / uSteps) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(i + 1) / uSteps) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            // Surface normal N(u, v) = (cos(v)*cos(u), cos(v)*sin(u), sin(v))
            glNormal3f(cv * cu1, cv * su1, sv);
            glVertex3f((R + a * cv) * cu1, (R + a * cv) * su1, a * sv);

            glNormal3f(cv * cu0, cv * su0, sv);
            glVertex3f((R + a * cv) * cu0, (R + a * cv) * su0, a * sv);
        }
        glEnd();
    }

    if (drawNormals) {
        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        for (int i = 0; i < uSteps; i += 3) {
            float u  = (static_cast<float>(i) / uSteps) * MathCG::TWO_PI;
            float cu = std::cos(u), su = std::sin(u);
            for (int j = 0; j < vSteps; j += 3) {
                float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
                float cv = std::cos(v), sv = std::sin(v);
                float px = (R + a * cv) * cu;
                float py = (R + a * cv) * su;
                float pz = a * sv;
                glVertex3f(px, py, pz);
                glVertex3f(px + cv * cu * 2.0f, py + cv * su * 2.0f, pz + sv * 2.0f);
            }
        }
        glEnd();
        if (gLighting.lightingEnabled) glEnable(GL_LIGHTING);
    }
}

/**
 * @brief Step 1 Hierarchical Sun Wheel Preview:
 *        Demonstrates T(I) * R_z(alpha) * T(-I) wheel rotation around I(x0, y0, z0),
 *        counter-rotation R_z(-alpha) at each cabin pivot so cabins remain strictly upright,
 *        and Chapter 1 HSVtoRGB dynamic LED color cycling.
 */
static void drawHierarchicalSunWheelPreview() {
    const MathCG::Vec3& I = gState.sunWheelCenter;
    const float R         = gState.sunWheelRadius;
    const float alpha     = gState.sunWheelAngleDeg;

    glPushMatrix();

    // Support A-frame legs anchored to ground surrounding I(x0, y0, z0)
    glColor3f(0.85f, 0.85f, 0.90f);
    for (int side = -1; side <= 1; side += 2) {
        for (int leg = -1; leg <= 1; leg += 2) {
            glPushMatrix();
            glTranslatef(I.x + leg * 18.0f, 0.0f, I.z + side * 4.5f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(leg * 24.0f, 0.0f, 1.0f, 0.0f);
            drawParametricCylinder(0.85f, 42.0f, 10, 2, gState.showNormalsAndCtrl);
            glPopMatrix();
        }
    }

    // Apply composite rotation around arbitrary wheel center I(x0, y0, z0):
    // M = T(I) * R_z(alpha) * T(-I)
    MathCG::Mat4 wheelRot = MathCG::Mat4::rotateZAroundPoint(I, alpha);
    glMultMatrixf(wheelRot.m);

    // Move into wheel-centered coordinates at I(x0, y0, z0)
    glTranslatef(I.x, I.y, I.z);

    // Compute dynamic HSV -> RGB color for the Sun Wheel rim & spokes
    float ledR = 1.0f, ledG = 0.5f, ledB = 0.1f;
    MathCG::HSVtoRGB(gLighting.ledHueDeg, 0.90f, 1.0f, ledR, ledG, ledB);
    glColor3f(ledR, ledG, ledB);

    // Dual outer toroidal rims
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, side * 2.2f);
        drawParametricToroid(R, 0.75f, 36, 12, gState.showNormalsAndCtrl);
        glPopMatrix();
    }

    // Radial spokes + Hierarchical Cabins with inverse rotation -alpha
    for (int k = 0; k < gState.numCabins; ++k) {
        float spokeAngleDeg = (360.0f / gState.numCabins) * k;
        float spokeHue      = std::fmod(gLighting.ledHueDeg + k * (360.0f / gState.numCabins), 360.0f);
        float sr, sg, sb;
        MathCG::HSVtoRGB(spokeHue, 0.85f, 1.0f, sr, sg, sb);

        glPushMatrix();
        glRotatef(spokeAngleDeg, 0.0f, 0.0f, 1.0f);

        // Spoke cylinder along +X (rotated +90 deg around Y)
        glColor3f(sr, sg, sb);
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawParametricCylinder(0.22f, R, 8, 1, false);
        glPopMatrix();

        // Translate to cabin pivot on rim at distance R
        glTranslatef(R, 0.0f, 0.0f);

        // MANDATORY HIERARCHICAL COUNTER-ROTATION:
        // Cancel both the spoke angle and the wheel's rotation alpha so cabin stays strictly upright!
        glRotatef(-(spokeAngleDeg + alpha), 0.0f, 0.0f, 1.0f);

        // Render upright cabin (PolygonMesh / Face-Normals preview box)
        glColor3f(0.95f, 0.95f, 0.98f);
        glPushMatrix();
        glTranslatef(0.0f, -1.8f, 0.0f);
        glScalef(2.4f, 2.8f, 2.2f);
        // Explicit 6-face unit cube with cross-product normals N = A x B
        glBegin(GL_QUADS);
        // Front (+Z)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
        // Back (-Z)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
        // Right (+X)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
        // Left (-X)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
        // Top (+Y)
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
        // Bottom (-Y)
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glEnd();
        glPopMatrix();

        glPopMatrix();
    }

    glPopMatrix();
}

/**
 * @brief Step 1 Preview of Bernstein Bezier Track & Oriented Coaster Car
 */
static void drawBezierTrackPreview() {
    const auto& ctrl = gState.previewBezierPts;

    // Draw Bezier curve sampled via Bernstein polynomials B_k^n(t)
    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);
    glColor3f(1.0f, 0.30f, 0.15f);
    glBegin(GL_LINE_STRIP);
    const int samples = 100;
    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i) / samples;
        MathCG::Vec3 p = MathCG::evaluateBezierPoint(ctrl, t);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    // Debug toggle: Draw Bezier Control Polygon P0..Pn
    if (gState.showNormalsAndCtrl) {
        glLineWidth(1.5f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (const auto& pt : ctrl) {
            glVertex3f(pt.x, pt.y, pt.z);
        }
        glEnd();

        glPointSize(7.0f);
        glColor3f(0.0f, 1.0f, 0.2f);
        glBegin(GL_POINTS);
        for (const auto& pt : ctrl) {
            glVertex3f(pt.x, pt.y, pt.z);
        }
        glEnd();
    }
    glLineWidth(1.0f);
    if (gLighting.lightingEnabled) glEnable(GL_LIGHTING);

    // Draw Coaster Car oriented along tangent P'(t) (yaw & pitch)
    MathCG::Vec3 carPos = MathCG::evaluateBezierPoint(ctrl, gState.coasterParamT);
    MathCG::Vec3 tang   = MathCG::evaluateBezierTangent(ctrl, gState.coasterParamT);

    float yawDeg   = std::atan2(tang.x, tang.z) * MathCG::RAD2DEG;
    float pitchDeg = -std::asin(std::clamp(tang.y, -1.0f, 1.0f)) * MathCG::RAD2DEG;

    glPushMatrix();
    glTranslatef(carPos.x, carPos.y, carPos.z);
    glRotatef(yawDeg,   0.0f, 1.0f, 0.0f);
    glRotatef(pitchDeg, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.82f, 0.10f);
    drawParametricCylinder(1.2f, 3.5f, 12, 1, gState.showNormalsAndCtrl);
    glPopMatrix();
}

/**
 * @brief Draw Park Ground Grid & Semi-Transparent Reflection Lake at y = 0
 */
static void drawParkGroundAndLake() {
    // Outer park lawns (leaving central lake region [-55..55, -60..10] open for planar reflection)
    glColor3f(0.16f, 0.36f, 0.18f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    // North strip
    glVertex3f(-140.0f, -0.05f, -140.0f);
    glVertex3f(-140.0f, -0.05f,  -60.0f);
    glVertex3f( 140.0f, -0.05f,  -60.0f);
    glVertex3f( 140.0f, -0.05f, -140.0f);
    // South strip
    glVertex3f(-140.0f, -0.05f,   10.0f);
    glVertex3f(-140.0f, -0.05f,  140.0f);
    glVertex3f( 140.0f, -0.05f,  140.0f);
    glVertex3f( 140.0f, -0.05f,   10.0f);
    // West strip
    glVertex3f(-140.0f, -0.05f,  -60.0f);
    glVertex3f(-140.0f, -0.05f,   10.0f);
    glVertex3f( -55.0f, -0.05f,   10.0f);
    glVertex3f( -55.0f, -0.05f,  -60.0f);
    // East strip
    glVertex3f(  55.0f, -0.05f,  -60.0f);
    glVertex3f(  55.0f, -0.05f,   10.0f);
    glVertex3f( 140.0f, -0.05f,   10.0f);
    glVertex3f( 140.0f, -0.05f,  -60.0f);
    glEnd();

    // Semi-transparent central lake at y = 0 (GL_BLEND)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.28f, 0.48f, 0.62f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-55.0f, 0.0f, -60.0f);
    glVertex3f(-55.0f, 0.0f,  10.0f);
    glVertex3f( 55.0f, 0.0f,  10.0f);
    glVertex3f( 55.0f, 0.0f, -60.0f);
    glEnd();
    glDisable(GL_BLEND);
}

/**
 * @brief Render on-screen HUD using glutBitmapCharacter (Chapter 3)
 */
static void drawOnScreenHUD() {
    if (!gState.showHUD) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, gState.windowWidth, 0.0, gState.windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Semi-transparent HUD backdrop panel in top-left corner
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.03f, 0.05f, 0.10f, 0.76f);
    glBegin(GL_QUADS);
    glVertex2f(12.0f, gState.windowHeight - 12.0f);
    glVertex2f(12.0f, gState.windowHeight - 235.0f);
    glVertex2f(485.0f, gState.windowHeight - 235.0f);
    glVertex2f(485.0f, gState.windowHeight - 12.0f);
    glEnd();
    glDisable(GL_BLEND);

    float y = gState.windowHeight - 32.0f;
    const float dy = 19.0f;

    glColor3f(1.0f, 0.85f, 0.25f);
    drawBitmapText(24.0f, y, GLUT_BITMAP_HELVETICA_18, "ASIA PARK DA NANG - 3D CG SIMULATION");
    y -= (dy + 4.0f);

    char buf[256];
    glColor3f(0.90f, 0.95f, 1.0f);

    std::snprintf(buf, sizeof(buf), "[C / 1,2,3] Camera Mode : %s", gCamera.getCameraModeName());
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[P]         Projection  : %s", gCamera.getProjectionModeName());
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[Orbit]     theta=%.1f deg | Phi=%.1f deg | R=%.1f | D=%.1f",
                  gCamera.thetaDeg, gCamera.phiDeg, gCamera.distanceR, gCamera.viewPlaneD);
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[S]         Shading     : %s", gLighting.getShadingModelName());
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[W]         Polygon Mode: %s | [N] Normals/Ctrl: %s",
                  gState.wireframeMode ? "WIREFRAME (GL_LINE)" : "SOLID (GL_FILL)",
                  gState.showNormalsAndCtrl ? "ON" : "OFF");
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[B]         Back-Face   : %s | [Z] Z-Buffer: %s",
                  gState.backFaceCulling ? "ON (GL_CULL_FACE)" : "OFF",
                  gState.zBufferEnabled  ? "ON (GL_DEPTH_TEST)" : "OFF");
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[L]         Time of Day : %s", gLighting.getTimeOfDayName());
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf); y -= dy;

    std::snprintf(buf, sizeof(buf), "[R] Lake Reflection: %s | [SPACE] Anim: %s | Right-Click: Menu",
                  gState.lakeReflection ? "ON (y=0 Matrix)" : "OFF",
                  gState.isPaused ? "PAUSED" : "RUNNING");
    drawBitmapText(24.0f, y, GLUT_BITMAP_9_BY_15, buf);

    if (gState.zBufferEnabled) glEnable(GL_DEPTH_TEST);
    if (gLighting.lightingEnabled) glEnable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================================
// GLUT DISPLAY, TIMER & INPUT CALLBACKS
// ============================================================================

static void displayCallback() {
    gLighting.applyClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Apply Chapter 6 Hidden Surface Removal & Polygon State
    if (gState.zBufferEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (gState.backFaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    } else {
        glDisable(GL_CULL_FACE);
    }

    glPolygonMode(GL_FRONT_AND_BACK, gState.wireframeMode ? GL_LINE : GL_FILL);

    // 2. Apply Chapter 4 Projection & Viewing Transformations
    glViewport(0, 0, gState.windowWidth, gState.windowHeight);
    gCamera.applyProjection(gState.windowWidth, gState.windowHeight);

    MathCG::Vec3 cabinPos       = getTrackedCabinWorldPos();
    MathCG::Vec3 coasterPos     = MathCG::evaluateBezierPoint(gState.previewBezierPts, gState.coasterParamT);
    MathCG::Vec3 coasterTangent = MathCG::evaluateBezierTangent(gState.previewBezierPts, gState.coasterParamT);

    gCamera.applyView(cabinPos, gCamera.thetaDeg, coasterPos, coasterTangent);

    // 3. Apply Chapter 6 Illumination & Shading
    gLighting.applyLights(gState.sunWheelCenter);

    // 4. Chapter 4 Planar Reflection Pass across central lake plane y = 0
    if (gState.lakeReflection) {
        glPushMatrix();
        MathCG::Mat4 reflectMat = MathCG::Mat4::reflectionAcrossYPlane(0.0f);
        glMultMatrixf(reflectMat.m);

        // Reflection flips handedness (CCW <-> CW)
        if (gState.backFaceCulling) glFrontFace(GL_CW);
        drawHierarchicalSunWheelPreview();
        if (gState.backFaceCulling) glFrontFace(GL_CCW);

        glPopMatrix();
    }

    // 5. Render Main 3D Scene (Step 1 Preview; Step 2 Ride Modules plug in here)
    drawParkGroundAndLake();
    drawHierarchicalSunWheelPreview();
    drawBezierTrackPreview();

    // 6. Render 2D Overlays (Always in GL_FILL)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawOnScreenHUD();

    glutSwapBuffers();
}

static void reshapeCallback(int width, int height) {
    gState.windowWidth  = (width > 1)  ? width  : 1;
    gState.windowHeight = (height > 1) ? height : 1;
    glViewport(0, 0, gState.windowWidth, gState.windowHeight);
    glutPostRedisplay();
}

static void timerCallback(int value) {
    (void)value;
    const float dt = (1.0f / 60.0f) * gState.animSpeed;

    if (!gState.isPaused) {
        // Rotate Sun Wheel by angle alpha and cycle HSV Hue H in [0, 360)
        gState.sunWheelAngleDeg = std::fmod(gState.sunWheelAngleDeg + 12.0f * dt, 360.0f);
        gLighting.ledHueDeg     = std::fmod(gLighting.ledHueDeg     + 45.0f * dt, 360.0f);

        // Advance Carousel, Sky Tower & Bezier Coaster parameter t in [0, 1]
        gState.carouselAngleDeg = std::fmod(gState.carouselAngleDeg + 35.0f * dt, 360.0f);
        gState.carouselPhase    = std::fmod(gState.carouselPhase    + 3.0f  * dt, MathCG::TWO_PI);
        gState.skyTowerTime    += dt;
        gState.coasterParamT    = std::fmod(gState.coasterParamT    + 0.085f * dt, 1.0f);
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0); // ~60 FPS
}

static void applyMenuSelection(int option) {
    switch (option) {
        // Camera Modes
        case MENU_CAM_ORBIT:
            gCamera.cameraMode = CameraMode::FREE_ORBIT;
            break;
        case MENU_CAM_SUNWHEEL:
            gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
            gCamera.cameraMode     = CameraMode::SUN_WHEEL_CABIN;
            break;
        case MENU_CAM_COASTER:
            gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
            gCamera.cameraMode     = CameraMode::COASTER_FIRST_PERSON;
            break;
        case MENU_CAM_RESET:
            gCamera = Camera();
            break;

        // Projections
        case MENU_PROJ_PERSPECTIVE:
            gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
            break;
        case MENU_PROJ_ORTHO_TOP:
            gCamera.projectionMode = ProjectionMode::ORTHO_TOP;
            break;
        case MENU_PROJ_ORTHO_FRONT:
            gCamera.projectionMode = ProjectionMode::ORTHO_FRONT;
            break;
        case MENU_PROJ_ORTHO_SIDE:
            gCamera.projectionMode = ProjectionMode::ORTHO_SIDE;
            break;

        // Shading & Pipeline States
        case MENU_SHADE_GOURAUD:
            gLighting.shadingModel = ShadingModel::GOURAUD_SMOOTH;
            break;
        case MENU_SHADE_LAMBERT:
            gLighting.shadingModel = ShadingModel::LAMBERT_FLAT;
            break;
        case MENU_TOGGLE_WIREFRAME:
            gState.wireframeMode = !gState.wireframeMode;
            break;
        case MENU_TOGGLE_CULLING:
            gState.backFaceCulling = !gState.backFaceCulling;
            break;
        case MENU_TOGGLE_ZBUFFER:
            gState.zBufferEnabled = !gState.zBufferEnabled;
            break;
        case MENU_TOGGLE_NORMALS:
            gState.showNormalsAndCtrl = !gState.showNormalsAndCtrl;
            break;
        case MENU_TOGGLE_REFLECTION:
            gState.lakeReflection = !gState.lakeReflection;
            break;

        // Time of Day
        case MENU_TIME_DAY:
            gLighting.timeOfDay = TimeOfDay::DAY;
            break;
        case MENU_TIME_SUNSET:
            gLighting.timeOfDay = TimeOfDay::SUNSET;
            break;
        case MENU_TIME_NIGHT:
            gLighting.timeOfDay = TimeOfDay::NIGHT;
            break;

        // Animation & UI
        case MENU_ANIM_TOGGLE:
            gState.isPaused = !gState.isPaused;
            break;
        case MENU_ANIM_FASTER:
            gState.animSpeed = std::min(4.0f, gState.animSpeed + 0.25f);
            break;
        case MENU_ANIM_SLOWER:
            gState.animSpeed = std::max(0.25f, gState.animSpeed - 0.25f);
            break;
        case MENU_TOGGLE_HUD:
            gState.showHUD = !gState.showHUD;
            break;
        case MENU_TOGGLE_MINIMAP:
            gState.showMinimap = !gState.showMinimap;
            break;
        case MENU_EXIT:
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

static void keyboardCallback(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case '1':
            applyMenuSelection(MENU_CAM_ORBIT);
            break;
        case '2':
            applyMenuSelection(MENU_CAM_SUNWHEEL);
            break;
        case '3':
            applyMenuSelection(MENU_CAM_COASTER);
            break;
        case 'c': case 'C': {
            int nextCam = (static_cast<int>(gCamera.cameraMode) + 1) % 3;
            gCamera.cameraMode = static_cast<CameraMode>(nextCam);
            if (gCamera.cameraMode != CameraMode::FREE_ORBIT) {
                gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
            }
            break;
        }
        case 'p': case 'P': {
            int nextProj = (static_cast<int>(gCamera.projectionMode) + 1) % 4;
            gCamera.projectionMode = static_cast<ProjectionMode>(nextProj);
            break;
        }
        case 'w': case 'W':
            gState.wireframeMode = !gState.wireframeMode;
            break;
        case 's': case 'S':
            gLighting.shadingModel = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH)
                ? ShadingModel::LAMBERT_FLAT
                : ShadingModel::GOURAUD_SMOOTH;
            break;
        case 'b': case 'B':
            gState.backFaceCulling = !gState.backFaceCulling;
            break;
        case 'z': case 'Z':
            gState.zBufferEnabled = !gState.zBufferEnabled;
            break;
        case 'n': case 'N':
            gState.showNormalsAndCtrl = !gState.showNormalsAndCtrl;
            break;
        case 'r': case 'R':
            gState.lakeReflection = !gState.lakeReflection;
            break;
        case 'l': case 'L': {
            int nextTime = (static_cast<int>(gLighting.timeOfDay) + 1) % 3;
            gLighting.timeOfDay = static_cast<TimeOfDay>(nextTime);
            break;
        }
        case 'm': case 'M':
            gState.showMinimap = !gState.showMinimap;
            break;
        case 'h': case 'H':
            gState.showHUD = !gState.showHUD;
            break;
        case '+': case '=':
            gCamera.zoom(-6.0f);
            break;
        case '-': case '_':
            gCamera.zoom(6.0f);
            break;
        case '[':
            gCamera.viewPlaneD = std::max(15.0f, gCamera.viewPlaneD - 3.0f);
            break;
        case ']':
            gCamera.viewPlaneD = std::min(120.0f, gCamera.viewPlaneD + 3.0f);
            break;
        case ' ':
            gState.isPaused = !gState.isPaused;
            break;
        case 27: // ESC
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

static void specialKeysCallback(int key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case GLUT_KEY_LEFT:
            gCamera.orbit(-4.0f, 0.0f);
            break;
        case GLUT_KEY_RIGHT:
            gCamera.orbit(4.0f, 0.0f);
            break;
        case GLUT_KEY_UP:
            gCamera.orbit(0.0f, 3.0f);
            break;
        case GLUT_KEY_DOWN:
            gCamera.orbit(0.0f, -3.0f);
            break;
        case GLUT_KEY_PAGE_UP:
            gCamera.zoom(-8.0f);
            break;
        case GLUT_KEY_PAGE_DOWN:
            gCamera.zoom(8.0f);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

static void mouseButtonCallback(int button, int state, int x, int y) {
    gState.lastMouseX = x;
    gState.lastMouseY = y;

    if (button == GLUT_LEFT_BUTTON) {
        gState.leftMouseDown = (state == GLUT_DOWN);
    } else if (button == GLUT_MIDDLE_BUTTON) {
        gState.midMouseDown = (state == GLUT_DOWN);
    } else if (button == 3 && state == GLUT_DOWN) { // Scroll wheel up
        gCamera.zoom(-6.0f);
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // Scroll wheel down
        gCamera.zoom(6.0f);
        glutPostRedisplay();
    }
}

static void mouseMotionCallback(int x, int y) {
    int dx = x - gState.lastMouseX;
    int dy = y - gState.lastMouseY;
    gState.lastMouseX = x;
    gState.lastMouseY = y;

    if (gState.leftMouseDown) {
        gCamera.orbit(-dx * 0.35f, dy * 0.35f);
        glutPostRedisplay();
    } else if (gState.midMouseDown) {
        gCamera.zoom(dy * 0.6f);
        glutPostRedisplay();
    }
}

static void createGLUTMenus() {
    int camSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Free Spherical Orbit Camera [1]",       MENU_CAM_ORBIT);
    glutAddMenuEntry("1st-Person: Sun Wheel Cabin [2]",       MENU_CAM_SUNWHEEL);
    glutAddMenuEntry("1st-Person: Queen Cobra Coaster [3]",   MENU_CAM_COASTER);
    glutAddMenuEntry("Reset Camera Parameters",               MENU_CAM_RESET);

    int projSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Perspective (gluPerspective) [P]",      MENU_PROJ_PERSPECTIVE);
    glutAddMenuEntry("Parallel Ortho: TOP View (XZ)",         MENU_PROJ_ORTHO_TOP);
    glutAddMenuEntry("Parallel Ortho: FRONT View (XY)",       MENU_PROJ_ORTHO_FRONT);
    glutAddMenuEntry("Parallel Ortho: SIDE View (ZY)",        MENU_PROJ_ORTHO_SIDE);

    int shadeSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Gouraud Interpolated (GL_SMOOTH) [S]",  MENU_SHADE_GOURAUD);
    glutAddMenuEntry("Lambert Constant (GL_FLAT) [S]",        MENU_SHADE_LAMBERT);
    glutAddMenuEntry("Toggle Wireframe / Solid [W]",          MENU_TOGGLE_WIREFRAME);
    glutAddMenuEntry("Toggle Back-Face Culling [B]",          MENU_TOGGLE_CULLING);
    glutAddMenuEntry("Toggle Z-Buffer Depth Test [Z]",        MENU_TOGGLE_ZBUFFER);
    glutAddMenuEntry("Toggle Surface Normals & Bezier Ctrl [N]", MENU_TOGGLE_NORMALS);
    glutAddMenuEntry("Toggle Planar Lake Reflection [R]",     MENU_TOGGLE_REFLECTION);

    int timeSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Daylight (High Ambient I_a) [L]",       MENU_TIME_DAY);
    glutAddMenuEntry("Da Nang Sunset (Warm Golden) [L]",      MENU_TIME_SUNSET);
    glutAddMenuEntry("Night LED Show (HSV + 1/D^2) [L]",      MENU_TIME_NIGHT);

    int animSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Pause / Resume Animation [SPACE]",      MENU_ANIM_TOGGLE);
    glutAddMenuEntry("Speed Up Animation",                    MENU_ANIM_FASTER);
    glutAddMenuEntry("Slow Down Animation",                   MENU_ANIM_SLOWER);
    glutAddMenuEntry("Toggle 2D Minimap [M]",                 MENU_TOGGLE_MINIMAP);
    glutAddMenuEntry("Toggle On-Screen HUD [H]",              MENU_TOGGLE_HUD);

    glutCreateMenu(applyMenuSelection);
    glutAddSubMenu("Camera System (Chapter 4)",               camSubMenu);
    glutAddSubMenu("Projection Mode (Chapter 4)",             projSubMenu);
    glutAddSubMenu("Shading & Hidden Surface (Chapter 6)",    shadeSubMenu);
    glutAddSubMenu("Time of Day & Illumination (Chapter 6)",  timeSubMenu);
    glutAddSubMenu("Animation & 2D UI (Chapter 3)",           animSubMenu);
    glutAddMenuEntry("Exit Simulation [ESC]",                 MENU_EXIT);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(gState.windowWidth, gState.windowHeight);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Asia Park (Da Nang) - 3D Computer Graphics Simulation");

    // Register callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutSpecialFunc(specialKeysCallback);
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);
    glutTimerFunc(16, timerCallback, 0);

    // Create right-click hierarchical menu
    createGLUTMenus();

    // Default OpenGL states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glutMainLoop();
    return 0;
}
