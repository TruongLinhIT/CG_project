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
#include <algorithm>
#include <cctype>

#include "core/MathCG.h"
#include "core/Camera.h"
#include "core/Lighting.h"
#include "curves/BezierTrack.h"
#include "curves/RollerCoaster.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"
#include "rides/SunWheel.h"
#include "rides/SkyTower.h"
#include "rides/Carousel.h"
#include "rides/NgoMonGate.h"
#include "ui/Minimap2D.h"
#include "ui/HUD.h"
#include "actors/Human.h"
#include "rides/Playground.h"
#include "rides/BumperCars.h"
#include "rides/PirateShip.h"

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

    // Sun Wheel Real Specs (Asia Park Da Nang):
    // Diameter = 100m (R = 50m), Center Height = 65m (Total Height = 115m),
    // 32 radial spokes, 64 cabins (delta = 5.625 deg)
    MathCG::Vec3 sunWheelCenter = MathCG::Vec3(0.0f, 65.0f, -38.0f);
    float        sunWheelRadius = 50.0f;
    int          numSpokes      = 32;
    int          numCabins      = 64;

    // Mouse interaction state
    int  lastMouseX     = 0;
    int  lastMouseY     = 0;
    bool leftMouseDown  = false;
    bool midMouseDown   = false;

    // Smooth Drone Flight Input State (updated at 60 FPS in timerCallback)
    bool keyDown[256]      = { false };
    bool specialLeftDown   = false;
    bool specialRightDown  = false;
    bool specialUpDown     = false;
    bool specialDownDown   = false;
    bool specialPgUpDown   = false;
    bool specialPgDnDown   = false;
    int  prevElapsedMs     = 0;
};

static SimulationState      gState;
static Camera               gCamera;
static LightingSystem       gLighting;
static AsiaParkCurvesSystem gCurves;
static AsiaParkRides::SunWheel   gSunWheel;
static AsiaParkRides::SkyTower   gSkyTower;
static AsiaParkRides::Carousel   gCarousel;
static AsiaParkRides::NgoMonGate  gNgoMonGate;
static AsiaParkUI::Minimap2D     gMinimap;
static AsiaParkRides::Playground  gPlayground;
static AsiaParkRides::BumperCars  gBumperCars;
static AsiaParkRides::PirateShip  gPirateShip;
static std::vector<AsiaParkActors::Human> gHumans;

// ============================================================================
// GLUT RIGHT-CLICK MENU IDENTIFIERS
// ============================================================================

enum MenuOption {
    MENU_CAM_ORBIT = 10,
    MENU_CAM_SUNWHEEL,
    MENU_CAM_COASTER,
    MENU_CAM_MONORAIL,
    MENU_CAM_DRONE,
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
    return gSunWheel.getCabinWorldPosition(0);
}

static void clearHeldFlightKeys() {
    std::fill(std::begin(gState.keyDown), std::end(gState.keyDown), false);
    gState.specialLeftDown  = false;
    gState.specialRightDown = false;
    gState.specialUpDown    = false;
    gState.specialDownDown  = false;
    gState.specialPgUpDown  = false;
    gState.specialPgDnDown  = false;
}

static void setDroneInputMode(bool enabled) {
    clearHeldFlightKeys();
    glutIgnoreKeyRepeat(enabled ? 1 : 0);
}

static void activateCameraMode(CameraMode nextMode) {
    if (nextMode == CameraMode::DRONE_FREE) {
        if (gCamera.cameraMode != CameraMode::DRONE_FREE) {
            MathCG::Vec3 activeEye;
            MathCG::Vec3 activeLook;
            TrackFrame queenFrame    = gCurves.getQueenCobraCameraFrame();
            TrackFrame monorailFrame = gCurves.getMonorailCameraFrame();
            gCamera.getActiveEyeAndLookDir(
                getTrackedCabinWorldPos(),
                queenFrame.position, queenFrame.tangent, queenFrame.up,
                monorailFrame.position, monorailFrame.tangent, monorailFrame.up,
                activeEye, activeLook
            );
            gCamera.syncDroneFromPose(activeEye, activeLook);
        }
        gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
        gCamera.cameraMode     = CameraMode::DRONE_FREE;
        setDroneInputMode(true);
    } else {
        gCamera.cameraMode = nextMode;
        if (nextMode != CameraMode::FREE_ORBIT) {
            gCamera.projectionMode = ProjectionMode::PERSPECTIVE;
        }
        setDroneInputMode(false);
    }
}

/**
 * @brief Draw a bitmap text string at 2D pixel position (x, y) using glutBitmapCharacter (Chapter 3)
 */
[[maybe_unused]] static void drawBitmapText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}

/**
 * @brief Forwarding to Member 2's ParametricShapes library (Chapter 5 & 6)
 */
[[maybe_unused]] static void drawParametricCylinder(float r, float h, int slices, int stacks, bool drawNormals) {
    ParametricShapes::drawCylinder(r, h, slices, stacks, false, drawNormals);
}

[[maybe_unused]] static void drawParametricToroid(float R, float a, int uSteps, int vSteps, bool drawNormals) {
    ParametricShapes::drawToroid(R, a, uSteps, vSteps, drawNormals);
}

[[maybe_unused]] static void drawCylinderBetween(
    const MathCG::Vec3& a,
    const MathCG::Vec3& b,
    float rA,
    float rB,
    int slices
) {
    ParametricShapes::drawCylinderBetween(a, b, rA, rB, slices, false);
}

/**
 * @brief Chapter 4 & 5: Sun Wheel rendering delegated to Member 2's AsiaParkRides::SunWheel
 */
static void drawHierarchicalSunWheelPreview() {
    bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
    gSunWheel.draw(gLighting.ledHueDeg, isNightOrSunset, gState.showNormalsAndCtrl);
}

/**
 * @brief Chapter 4 & 6: Lakeside Asian Viewing Pavilion built via Member 2's PolygonMesh
 */
static void drawLakesidePavilionMesh() {
    static Geometry::PolygonMesh pavilionMesh;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        // 1. Foundation Podium
        auto podium = Geometry::PolygonMesh::createBox(0.0f, 0.4f, 0.0f, 10.0f, 0.8f, 10.0f, MathCG::ColorRGB(0.55f, 0.52f, 0.48f));
        pavilionMesh.append(podium);

        // 2. Pillars at 4 corners
        float cornerOffsets[4][2] = { {-3.8f, -3.8f}, {3.8f, -3.8f}, {3.8f, 3.8f}, {-3.8f, 3.8f} };
        for (int i = 0; i < 4; ++i) {
            auto pillar = Geometry::PolygonMesh::createPrismY(cornerOffsets[i][0], 0.8f, cornerOffsets[i][1], 0.28f, 4.2f, 12, MathCG::ColorRGB(0.72f, 0.22f, 0.16f));
            pavilionMesh.append(pillar);
        }

        // 3. Eaves beam
        auto beam = Geometry::PolygonMesh::createBox(0.0f, 5.1f, 0.0f, 8.4f, 0.4f, 8.4f, MathCG::ColorRGB(0.85f, 0.65f, 0.22f));
        pavilionMesh.append(beam);

        // 4. Asian Curved Roof
        auto roof = Geometry::PolygonMesh::createCurvedAsianRoof(0.0f, 5.3f, 0.0f, 9.6f, 9.6f, 2.6f, 2.4f, MathCG::ColorRGB(0.88f, 0.35f, 0.12f));
        pavilionMesh.append(roof);

        // 5. Roof pinnacle
        auto pinnacle = Geometry::PolygonMesh::createPyramid(0.0f, 7.9f, 0.0f, 0.9f, 0.9f, 1.2f, MathCG::ColorRGB(0.95f, 0.82f, 0.25f));
        pavilionMesh.append(pinnacle);

        pavilionMesh.computeNormals();
    }

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 28.0f); // South viewing deck facing the lake
    bool smooth = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH);
    pavilionMesh.draw(smooth, gState.showNormalsAndCtrl);
    glPopMatrix();
}

/**
 * @brief Draw Park Ground Grid & Semi-Transparent Reflection Lake at y = 0
 */
static void drawParkGroundAndLake() {
    // Outer park lawns (leaving central lake region [-58..58, -62..16] open for 115m Sun Wheel reflection)
    glColor3f(0.16f, 0.36f, 0.18f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    // North strip
    glVertex3f(-140.0f, -0.05f, -140.0f);
    glVertex3f(-140.0f, -0.05f,  -62.0f);
    glVertex3f( 140.0f, -0.05f,  -62.0f);
    glVertex3f( 140.0f, -0.05f, -140.0f);
    // South strip
    glVertex3f(-140.0f, -0.05f,   16.0f);
    glVertex3f(-140.0f, -0.05f,  140.0f);
    glVertex3f( 140.0f, -0.05f,  140.0f);
    glVertex3f( 140.0f, -0.05f,   16.0f);
    // West strip
    glVertex3f(-140.0f, -0.05f,  -62.0f);
    glVertex3f(-140.0f, -0.05f,   16.0f);
    glVertex3f( -58.0f, -0.05f,   16.0f);
    glVertex3f( -58.0f, -0.05f,  -62.0f);
    // East strip
    glVertex3f(  58.0f, -0.05f,  -62.0f);
    glVertex3f(  58.0f, -0.05f,   16.0f);
    glVertex3f( 140.0f, -0.05f,   16.0f);
    glVertex3f( 140.0f, -0.05f,  -62.0f);
    glEnd();

    // Semi-transparent central lake at y = 0 (GL_BLEND)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.28f, 0.48f, 0.62f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-58.0f, 0.0f, -62.0f);
    glVertex3f(-58.0f, 0.0f,  16.0f);
    glVertex3f( 58.0f, 0.0f,  16.0f);
    glVertex3f( 58.0f, 0.0f, -62.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Decorative Asian-style promenade lantern posts around the lake (showcasing all 4 ParametricShapes)
    static const MathCG::Vec3 lampLocations[] = {
        { -56.0f, 0.0f,  14.0f },
        { -28.0f, 0.0f,  14.0f },
        {   0.0f, 0.0f,  14.0f },
        {  28.0f, 0.0f,  14.0f },
        {  56.0f, 0.0f,  14.0f },
        { -56.0f, 0.0f, -60.0f },
        { -28.0f, 0.0f, -60.0f },
        {   0.0f, 0.0f, -60.0f },
        {  28.0f, 0.0f, -60.0f },
        {  56.0f, 0.0f, -60.0f }
    };

    for (const auto& pos : lampLocations) {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);

        // 1. Parametric Ruled Cylinder (Base & Pillar)
        glColor3f(0.24f, 0.25f, 0.28f);
        ParametricShapes::drawCylinderY(0.42f, 0.35f, 16, 2, true, gState.showNormalsAndCtrl);
        glColor3f(0.38f, 0.40f, 0.44f);
        ParametricShapes::drawCylinderY(0.16f, 3.80f, 16, 4, true, gState.showNormalsAndCtrl);

        // 2. Parametric Toroid (Decorative collar ring)
        glPushMatrix();
        glTranslatef(0.0f, 3.80f, 0.0f);
        glColor3f(0.85f, 0.70f, 0.20f);
        ParametricShapes::drawToroidY(0.32f, 0.07f, 16, 8, gState.showNormalsAndCtrl);
        glPopMatrix();

        // 3. Parametric Sphere of Revolution (Glowing Lantern Bulb)
        glPushMatrix();
        glTranslatef(0.0f, 4.25f, 0.0f);
        if (gLighting.lightingEnabled) {
            GLfloat emit[4] = { 1.0f, 0.85f, 0.35f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
        }
        glColor3f(1.0f, 0.90f, 0.50f);
        ParametricShapes::drawSphere(0.40f, 16, 12, gState.showNormalsAndCtrl);
        if (gLighting.lightingEnabled) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
        glPopMatrix();

        // 4. Parametric Ruled Cone (Pagoda Roof Cap)
        glPushMatrix();
        glTranslatef(0.0f, 4.60f, 0.0f);
        glColor3f(0.72f, 0.18f, 0.14f);
        ParametricShapes::drawConeY(0.72f, 0.0f, 0.55f, 16, 2, true, gState.showNormalsAndCtrl);
        glPopMatrix();

        glPopMatrix();
    }
}

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

    MathCG::Vec3 cabinPos    = getTrackedCabinWorldPos();
    TrackFrame queenFrame    = gCurves.getQueenCobraCameraFrame();
    TrackFrame monorailFrame = gCurves.getMonorailCameraFrame();

    gCamera.applyView(
        cabinPos, gCamera.thetaDeg,
        queenFrame.position, queenFrame.tangent, queenFrame.up,
        monorailFrame.position, monorailFrame.tangent, monorailFrame.up
    );

    // 3. Apply Chapter 6 Illumination & Shading
    gLighting.applyLights(gState.sunWheelCenter);
    gLighting.applyCoasterHeadlight(
        queenFrame.position + queenFrame.up * 0.8f + queenFrame.tangent * 1.5f,
        queenFrame.tangent
    );

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

    // 5. Render Main 3D Scene
    drawParkGroundAndLake();
    drawLakesidePavilionMesh();
    drawHierarchicalSunWheelPreview();
    gCurves.draw(gState.showNormalsAndCtrl, gLighting.lightingEnabled, gLighting.timeOfDay);

    // Member 2 Rides & Monumental Architecture
    bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
    bool smooth = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH);
    gSkyTower.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gCarousel.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gNgoMonGate.draw(isNightOrSunset, gState.showNormalsAndCtrl, smooth);
    gPlayground.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gBumperCars.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gPirateShip.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    for (const auto& h : gHumans) h.draw(isNightOrSunset);

    // 6. Render 2D Overlays (Always in GL_FILL)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 6.1 Minimap 2D Radar (Chapter 1 & 3: WCS->NCS->DCS, Cohen-Sutherland, Midpoint Circle, Bresenham Line)
    if (gState.showMinimap) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, gState.windowWidth, 0.0, gState.windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        std::vector<MathCG::Vec3> qcPoints;
        for (const auto& s : gCurves.queenCobraTrack.samples) qcPoints.push_back(s.position);
        std::vector<MathCG::Vec3> mrPoints;
        for (const auto& s : gCurves.monorailTrack.samples) mrPoints.push_back(s.position);

        TrackFrame qcFrame = gCurves.getQueenCobraCameraFrame();
        TrackFrame mrFrame = gCurves.getMonorailCameraFrame();

        MathCG::Vec3 activeEye;
        MathCG::Vec3 activeLook;
        gCamera.getActiveEyeAndLookDir(
            cabinPos,
            qcFrame.position, qcFrame.tangent, qcFrame.up,
            mrFrame.position, mrFrame.tangent, mrFrame.up,
            activeEye, activeLook
        );
        MathCG::Vec3 radarEye(
            std::clamp(activeEye.x, -132.0f, 132.0f),
            activeEye.y,
            std::clamp(activeEye.z, -132.0f, 132.0f)
        );

        gMinimap.draw(
            gState.windowWidth, gState.windowHeight,
            radarEye,
            gCamera.getMinimapYawDeg(activeLook),
            gSunWheel.center,
            gSunWheel.radius,
            qcPoints,
            mrPoints,
            qcFrame.position,
            mrFrame.position,
            gSkyTower.position,
            gCarousel.position,
            gNgoMonGate.position
        );

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    // 6.2 On-Screen Telemetry HUD (Chapter 3: glutBitmapCharacter)
    if (gState.showHUD) {
        AsiaParkUI::HUD::draw(
            gState.windowWidth, gState.windowHeight,
            gCamera,
            gLighting,
            gState.wireframeMode,
            gState.showNormalsAndCtrl,
            gState.backFaceCulling,
            gState.zBufferEnabled,
            gState.lakeReflection,
            gState.showMinimap,
            gState.isPaused,
            gCurves.queenCobraTrain.paramT,
            gCurves.queenCobraTrain.lastSpeedMps
        );
    }

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
    int nowMs = glutGet(GLUT_ELAPSED_TIME);
    float realDt = (gState.prevElapsedMs > 0)
        ? std::clamp((nowMs - gState.prevElapsedMs) * 0.001f, 0.001f, 0.05f)
        : (1.0f / 60.0f);
    gState.prevElapsedMs = nowMs;

    // Free Drone Flight stays active even when park rides are paused for close inspection
    if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
        float moveFwd   = (gState.keyDown['w'] ? 1.0f : 0.0f) - (gState.keyDown['s'] ? 1.0f : 0.0f);
        float moveRight = (gState.keyDown['d'] ? 1.0f : 0.0f) - (gState.keyDown['a'] ? 1.0f : 0.0f);
        float moveUp    = ((gState.keyDown['e'] || gState.specialPgUpDown) ? 1.0f : 0.0f)
                        - ((gState.keyDown['q'] || gState.specialPgDnDown) ? 1.0f : 0.0f);
        float yawRate   = ((gState.specialRightDown ? 1.0f : 0.0f) - (gState.specialLeftDown ? 1.0f : 0.0f)) * 95.0f;
        float pitchRate = ((gState.specialUpDown    ? 1.0f : 0.0f) - (gState.specialDownDown ? 1.0f : 0.0f)) * 75.0f;
        gCamera.updateDrone(realDt, moveFwd, moveRight, moveUp, yawRate, pitchRate);
    }

    const float dt = realDt * gState.animSpeed;

    if (!gState.isPaused) {
        // Rotate Sun Wheel by angle alpha and cycle HSV Hue H in [0, 360)
        gSunWheel.update(dt);
        gState.sunWheelAngleDeg = gSunWheel.currentAngleDeg;
        gLighting.ledHueDeg     = std::fmod(gLighting.ledHueDeg     + 45.0f * dt, 360.0f);

        // Advance Carousel, Sky Tower & Bezier Coaster / Monorail along closed C^1 tracks
        gCarousel.update(dt);
        gState.carouselAngleDeg = gCarousel.currentAngleDeg;
        gState.carouselPhase    = gCarousel.oscillationPhase;

        gSkyTower.update(dt);
        gMinimap.update(dt);
        gState.skyTowerTime     = gSkyTower.cycleTime;
        gCurves.update(dt);
        gPlayground.update(dt);
        gBumperCars.update(dt);
        gPirateShip.update(dt);
        for (auto& h : gHumans) h.update(dt);
        gState.coasterParamT    = gCurves.queenCobraTrain.paramT;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0); // ~60 FPS
}

static void applyMenuSelection(int option) {
    switch (option) {
        // Camera Modes
        case MENU_CAM_ORBIT:
            activateCameraMode(CameraMode::FREE_ORBIT);
            break;
        case MENU_CAM_SUNWHEEL:
            activateCameraMode(CameraMode::SUN_WHEEL_CABIN);
            break;
        case MENU_CAM_COASTER:
            activateCameraMode(CameraMode::COASTER_FIRST_PERSON);
            break;
        case MENU_CAM_MONORAIL:
            activateCameraMode(CameraMode::MONORAIL_FIRST_PERSON);
            break;
        case MENU_CAM_DRONE:
            activateCameraMode(CameraMode::DRONE_FREE);
            break;
        case MENU_CAM_RESET:
            gCamera = Camera();
            setDroneInputMode(false);
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
    unsigned char lowerKey = static_cast<unsigned char>(std::tolower(key));
    gState.keyDown[lowerKey] = true;

    // In Free Drone mode, reserve W/A/S/D/E/Q for smooth flight and F/+/- for speed
    if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
        if (lowerKey == 'w' || lowerKey == 's' || lowerKey == 'a' ||
            lowerKey == 'd' || lowerKey == 'e' || lowerKey == 'q') {
            return;
        }
        if (lowerKey == 'f') {
            gCamera.cycleDroneSpeed();
            glutPostRedisplay();
            return;
        }
    }

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
        case '4':
            applyMenuSelection(MENU_CAM_MONORAIL);
            break;
        case '5':
            applyMenuSelection(MENU_CAM_DRONE);
            break;
        case 'c': case 'C': {
            int nextCam = (static_cast<int>(gCamera.cameraMode) + 1) % 5;
            applyMenuSelection(MENU_CAM_ORBIT + nextCam);
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
        case 'm': case 'M':
            gState.showMinimap = !gState.showMinimap;
            break;
        case 'r': case 'R':
            gState.lakeReflection = !gState.lakeReflection;
            break;
        case 'l': case 'L': {
            int nextTime = (static_cast<int>(gLighting.timeOfDay) + 1) % 3;
            gLighting.timeOfDay = static_cast<TimeOfDay>(nextTime);
            break;
        }
        case 'h': case 'H':
            gState.showHUD = !gState.showHUD;
            break;
        case '+': case '=':
            if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
                gCamera.adjustDroneSpeed(5.0f);
            } else {
                gCamera.zoom(-6.0f);
            }
            break;
        case '-': case '_':
            if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
                gCamera.adjustDroneSpeed(-5.0f);
            } else {
                gCamera.zoom(6.0f);
            }
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

static void keyboardUpCallback(unsigned char key, int x, int y) {
    (void)x; (void)y;
    unsigned char lowerKey = static_cast<unsigned char>(std::tolower(key));
    gState.keyDown[lowerKey] = false;
    gState.keyDown[key]      = false;
}

static void specialKeysCallback(int key, int x, int y) {
    (void)x; (void)y;
    if (key == GLUT_KEY_F2) {
        gState.wireframeMode = !gState.wireframeMode;
        glutPostRedisplay();
        return;
    } else if (key == GLUT_KEY_F3) {
        gLighting.shadingModel = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH)
            ? ShadingModel::LAMBERT_FLAT
            : ShadingModel::GOURAUD_SMOOTH;
        glutPostRedisplay();
        return;
    }

    if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
        switch (key) {
            case GLUT_KEY_LEFT:      gState.specialLeftDown  = true; break;
            case GLUT_KEY_RIGHT:     gState.specialRightDown = true; break;
            case GLUT_KEY_UP:        gState.specialUpDown    = true; break;
            case GLUT_KEY_DOWN:      gState.specialDownDown  = true; break;
            case GLUT_KEY_PAGE_UP:   gState.specialPgUpDown  = true; break;
            case GLUT_KEY_PAGE_DOWN: gState.specialPgDnDown  = true; break;
            default: break;
        }
        return;
    }

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

static void specialKeysUpCallback(int key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case GLUT_KEY_LEFT:      gState.specialLeftDown  = false; break;
        case GLUT_KEY_RIGHT:     gState.specialRightDown = false; break;
        case GLUT_KEY_UP:        gState.specialUpDown    = false; break;
        case GLUT_KEY_DOWN:      gState.specialDownDown  = false; break;
        case GLUT_KEY_PAGE_UP:   gState.specialPgUpDown  = false; break;
        case GLUT_KEY_PAGE_DOWN: gState.specialPgDnDown  = false; break;
        default: break;
    }
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
        if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
            gCamera.droneLook(dx * 0.28f, -dy * 0.25f);
        } else {
            gCamera.orbit(-dx * 0.35f, dy * 0.35f);
        }
        glutPostRedisplay();
    } else if (gState.midMouseDown) {
        if (gCamera.cameraMode == CameraMode::DRONE_FREE) {
            gCamera.droneNudgeForward(-dy * 0.45f);
        } else {
            gCamera.zoom(dy * 0.6f);
        }
        glutPostRedisplay();
    }
}

static void menuStatusCallback(int status, int x, int y) {
    (void)x; (void)y;
    if (status == GLUT_MENU_IN_USE) {
        clearHeldFlightKeys();
    }
}

static void createGLUTMenus() {
    int camSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Camera Quy dao cau tu do [1]",          MENU_CAM_ORBIT);
    glutAddMenuEntry("Goc nhin thu 1: Cabin Sun Wheel [2]",     MENU_CAM_SUNWHEEL);
    glutAddMenuEntry("Goc nhin thu 1: Tau luon Queen Cobra [3]",MENU_CAM_COASTER);
    glutAddMenuEntry("Goc nhin thu 1: Tau dien Monorail [4]",   MENU_CAM_MONORAIL);
    glutAddMenuEntry("Camera Drone Tu Do (Bay quan sat) [5]",   MENU_CAM_DRONE);
    glutAddMenuEntry("Dat lai thong so Camera",                 MENU_CAM_RESET);

    int projSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Phoi canh (gluPerspective) [P]",          MENU_PROJ_PERSPECTIVE);
    glutAddMenuEntry("Truc giao: Chieu bang TOP (XZ)",          MENU_PROJ_ORTHO_TOP);
    glutAddMenuEntry("Truc giao: Chieu dung FRONT (XY)",        MENU_PROJ_ORTHO_FRONT);
    glutAddMenuEntry("Truc giao: Chieu canh SIDE (ZY)",         MENU_PROJ_ORTHO_SIDE);

    int shadeSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("To bong noi suy Gouraud (GL_SMOOTH) [S/F3]", MENU_SHADE_GOURAUD);
    glutAddMenuEntry("To bong hang Lambert (GL_FLAT) [S/F3]",      MENU_SHADE_LAMBERT);
    glutAddMenuEntry("Bat/Tat Khung day / To dac [W/F2]",          MENU_TOGGLE_WIREFRAME);
    glutAddMenuEntry("Bat/Tat Khu mat sau (Back-Face) [B]",     MENU_TOGGLE_CULLING);
    glutAddMenuEntry("Bat/Tat Z-Buffer (Depth Test) [Z]",       MENU_TOGGLE_ZBUFFER);
    glutAddMenuEntry("Bat/Tat Phap tuyen & Da giac Bezier [N]", MENU_TOGGLE_NORMALS);
    glutAddMenuEntry("Bat/Tat Phan xa mat ho (y=0) [R]",        MENU_TOGGLE_REFLECTION);

    int timeSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Ban ngay (Moi truong I_a cao) [L]",       MENU_TIME_DAY);
    glutAddMenuEntry("Hoang hon Da Nang (Nang vang) [L]",       MENU_TIME_SUNSET);
    glutAddMenuEntry("Ban dem LED (HSV + Suy giam 1/D^2) [L]",  MENU_TIME_NIGHT);

    int animSubMenu = glutCreateMenu(applyMenuSelection);
    glutAddMenuEntry("Tam dung / Tiep tuc Hoat anh [SPACE]",    MENU_ANIM_TOGGLE);
    glutAddMenuEntry("Tang toc hoat anh",                       MENU_ANIM_FASTER);
    glutAddMenuEntry("Giam toc hoat anh",                       MENU_ANIM_SLOWER);
    glutAddMenuEntry("Bat/Tat Ban do nho 2D [M]",               MENU_TOGGLE_MINIMAP);
    glutAddMenuEntry("Bat/Tat Bang dieu khien HUD [H]",         MENU_TOGGLE_HUD);

    glutCreateMenu(applyMenuSelection);
    glutAddSubMenu("He thong Camera (Chuong 4)",                camSubMenu);
    glutAddSubMenu("Che do Phep chieu (Chuong 4)",              projSubMenu);
    glutAddSubMenu("To bong & Mat khuat (Chuong 6)",            shadeSubMenu);
    glutAddSubMenu("Thoi gian & Chieu sang (Chuong 6)",         timeSubMenu);
    glutAddSubMenu("Hoat anh & Giao dien (Chuong 3)",           animSubMenu);
    glutAddMenuEntry("Thoat mo phong [ESC]",                    MENU_EXIT);

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
    glutCreateWindow("Cong vien Chau A (Da Nang) - Mo phong Do hoa May tinh 3D");

    // Register callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialKeysCallback);
    glutSpecialUpFunc(specialKeysUpCallback);
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);
    glutMenuStatusFunc(menuStatusCallback);
    glutTimerFunc(16, timerCallback, 0);

    // Initialize Member 1 Curves & Rides (Queen Cobra + Monorail)
    gCurves.init();
    gHumans = AsiaParkActors::createDefaultCrowd();

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
