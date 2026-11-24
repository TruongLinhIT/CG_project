/**
 * @file main.cpp
 * @brief Điểm khởi đầu chương trình (Entry Point), Máy trạng thái tương tác, Camera & Menu GLUT cho
 *        "Mô phỏng 3D Công viên Châu Á (Đà Nẵng)" - C++ / FreeGLUT
 *
 * Sản phẩm bàn giao Bước 1:
 *  - Cửa sổ GLUT hoàn chỉnh & Vòng lặp bộ định thời (Timer Loop) 60 FPS
 *  - Máy trạng thái tương tác đầy đủ (Bật/tắt các tính năng từ Chương 1 đến Chương 6)
 *  - Camera quỹ đạo cầu (theta, Phi, R, D) + Camera góc nhìn trên trò chơi + Phép chiếu Phối cảnh/Trực giao
 *  - Menu chuột phải phân cấp GLUT + Các hàm gọi lại (Callback) bàn phím và chuột
 *  - Cảnh mô phỏng tương tác Bước 1 (Phản xạ mặt phẳng hồ tại y=0, mô hình phân cấp Sun Wheel
 *    với đèn LED ban đêm HSV->RGB, đường ray Bezier và bảng thông số HUD bằng glutBitmapCharacter)
 */

#include <GL/freeglut.h>
#ifndef _WIN32
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif
#include <cstdio>
#include <cstring>
#include <chrono>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

#include "core/MathCG.h"
#include "core/ParkLayout.h"
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
#include "rides/ParkArchitecture.h"
#include "ui/Minimap2D.h"
#include "ui/HUD.h"
#include "actors/Human.h"
#include "rides/Playground.h"
#include "rides/BumperCars.h"
#include "rides/PirateShip.h"
#include "rides/MiniCoaster.h"
#include "rides/NightIllumination.h"

// ============================================================================
// MÁY TRẠNG THÁI MÔ PHỎNG & ĐƯỜNG ỐNG ĐỒ HỌA TOÀN CỤC
// ============================================================================

struct SimulationState {
    int windowWidth  = 1280;
    int windowHeight = 768;

    // Trạng thái hoạt ảnh (bộ định thời 60 FPS)
    bool  isPaused          = false;
    float animSpeed         = 1.0f;
    float sunWheelAngleDeg  = 0.0f;  // alpha: góc quay chính của vòng quay Sun Wheel
    float carouselAngleDeg  = 0.0f;  // góc quay của vòng quay ngựa gỗ Carousel
    float carouselPhase     = 0.0f;  // pha dao động lên xuống theo phương thẳng đứng của ngựa gỗ
    float skyTowerTime      = 0.0f;  // biến thời gian điều khiển chu kỳ tháp rơi tự do Sky Tower
    float coasterParamT     = 0.0f;  // tham số t thuộc [0, 1] dọc theo đường ray Bezier

    // Các tùy chọn bật/tắt đường ống đồ họa Chương 4 & 6
    bool wireframeMode      = false; // Chế độ khung dây (GL_LINE) so với tô đặc (GL_FILL)
    bool showNormalsAndCtrl = false; // Gỡ lỗi: hiển thị pháp tuyến bề mặt N & đa giác kiểm soát Bezier
    bool backFaceCulling    = true;  // Loại bỏ mặt sau GL_CULL_FACE (V . N >= 0)
    bool zBufferEnabled     = true;  // Kiểm tra độ sâu Z-Buffer (GL_DEPTH_TEST)
    bool lakeReflection     = true;  // Phản xạ qua mặt phẳng y = 0 + mặt nước bán trong suốt GL_BLEND
    bool showMinimap        = true;  // Bản đồ nhỏ 2D WCS->NCS->DCS với thuật toán xén Cohen-Sutherland
    bool showHUD            = true;  // Lớp phủ hiển thị trạng thái trên màn hình bằng glutBitmapCharacter

    // Thông số thực tế của Vòng quay Sun Wheel (Công viên Châu Á - Đà Nẵng):
    // Đường kính = 100m (R = 50m), Độ cao tâm = 65m (Tổng chiều cao = 115m),
    // 32 nan hoa hướng tâm, 64 cabin (góc lệch delta = 5.625 độ), tốc độ thực 0.4 độ/s
    MathCG::Vec3 sunWheelCenter = ParkLayout::SUN_WHEEL_CENTER;
    float        sunWheelRadius = ParkLayout::SUN_WHEEL_RADIUS;
    int          numSpokes      = 32;
    int          numCabins      = 64;

    // Trạng thái tương tác chuột
    int  lastMouseX     = 0;
    int  lastMouseY     = 0;
    bool leftMouseDown  = false;
    bool midMouseDown   = false;

    // Trạng thái phím điều khiển bay Drone mượt mà (cập nhật ở tần số 60 FPS trong timerCallback)
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
static AsiaParkRides::ParkArchitecture gParkArch;
static AsiaParkUI::Minimap2D     gMinimap;
static AsiaParkRides::Playground  gPlayground;
static AsiaParkRides::BumperCars  gBumperCars;
static AsiaParkRides::PirateShip  gPirateShip;
static std::array<AsiaParkRides::MiniCoaster, 4> gMiniCoasters;
static AsiaParkRides::NightIllumination gNightLights;
static std::vector<AsiaParkActors::Human> gHumans;

// ============================================================================
// CÁC MÃ ĐỊNH DANH MENU CHUỘT PHẢI GLUT
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
    MENU_TOGGLE_SUNWHEEL_FAST,
    MENU_TOGGLE_NIGHT_LIGHTS,
    MENU_TOGGLE_HUD,
    MENU_TOGGLE_MINIMAP,
    MENU_EXIT = 99
};

// ============================================================================
// CÁC HÀM BỔ TRỢ TÍNH VỊ TRÍ CAMERA TRÒ CHƠI & VẼ CẢNH MÔ PHỎNG
// ============================================================================

/**
 * @brief Tính tọa độ thế giới của Cabin #0 trên vòng quay Sun Wheel sử dụng
 *        phép quay quanh tâm I(x0, y0, z0) ở Chương 4:
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
 * @brief Vẽ chuỗi văn bản bitmap tại vị trí điểm ảnh 2D (x, y) bằng glutBitmapCharacter (Chương 3)
 */
[[maybe_unused]] static void drawBitmapText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}

/**
 * @brief Chuyển tiếp lời gọi sang thư viện ParametricShapes (Chương 5 & 6)
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
 * @brief Chương 4 & 5: Vẽ vòng quay Sun Wheel thông qua lớp AsiaParkRides::SunWheel
 */
static void drawHierarchicalSunWheelPreview() {
    bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
    gSunWheel.draw(gLighting.ledHueDeg, isNightOrSunset, gState.showNormalsAndCtrl);
}

/**
 * @brief Chương 4 & 6: Thủy đình Á Đông ven hồ được xây dựng bằng lớp PolygonMesh
 */
static void drawLakesidePavilionMesh() {
    static Geometry::PolygonMesh pavilionMesh;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        // 1. Bệ móng đá
        auto podium = Geometry::PolygonMesh::createBox(0.0f, 0.4f, 0.0f, 10.0f, 0.8f, 10.0f, MathCG::ColorRGB(0.55f, 0.52f, 0.48f));
        pavilionMesh.append(podium);

        // 2. Các cột trụ ở 4 góc
        float cornerOffsets[4][2] = { {-3.8f, -3.8f}, {3.8f, -3.8f}, {3.8f, 3.8f}, {-3.8f, 3.8f} };
        for (int i = 0; i < 4; ++i) {
            auto pillar = Geometry::PolygonMesh::createPrismY(cornerOffsets[i][0], 0.8f, cornerOffsets[i][1], 0.28f, 4.2f, 12, MathCG::ColorRGB(0.72f, 0.22f, 0.16f));
            pavilionMesh.append(pillar);
        }

        // 3. Xà đỡ diềm mái
        auto beam = Geometry::PolygonMesh::createBox(0.0f, 5.1f, 0.0f, 8.4f, 0.4f, 8.4f, MathCG::ColorRGB(0.85f, 0.65f, 0.22f));
        pavilionMesh.append(beam);

        // 4. Mái cong Á Đông
        auto roof = Geometry::PolygonMesh::createCurvedAsianRoof(0.0f, 5.3f, 0.0f, 9.6f, 9.6f, 2.6f, 2.4f, MathCG::ColorRGB(0.88f, 0.35f, 0.12f));
        pavilionMesh.append(roof);

        // 5. Chóp đỉnh mái
        auto pinnacle = Geometry::PolygonMesh::createPyramid(0.0f, 7.9f, 0.0f, 0.9f, 0.9f, 1.2f, MathCG::ColorRGB(0.95f, 0.82f, 0.25f));
        pavilionMesh.append(pinnacle);

        pavilionMesh.computeNormals();
    }

    glPushMatrix();
    glTranslatef(
        ParkLayout::LAKESIDE_PAVILION_POS.x,
        ParkLayout::LAKESIDE_PAVILION_POS.y,
        ParkLayout::LAKESIDE_PAVILION_POS.z
    ); // Thủy đình thực tế trên mặt hồ phía Bắc Sun Wheel (-5.4, 0.0, -107.7)
    bool smooth = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH);
    pavilionMesh.draw(smooth, gState.showNormalsAndCtrl);
    glPopMatrix();
}

/**
 * @brief Vẽ một tuyến dải phẳng (quad-strip) sát mặt đất cho đường giao thông hoặc lối đi bộ
 *        Nâng cao độ y thêm vài cm để khử hiện tượng Z-fighting với mặt phẳng nền.
 */
static void drawFlatPolylineStrip(
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

        // Vector pháp tuyến ngang (-dz, dx) trong mặt phẳng XZ để mở rộng bề ngang dải
        float nx = (-dz / len) * halfW;
        float nz = ( dx / len) * halfW;

        glBegin(GL_QUADS);
        glVertex3f(pts[i].x     - nx, y, pts[i].z     - nz);
        glVertex3f(pts[i].x     + nx, y, pts[i].z     + nz);
        glVertex3f(pts[i + 1].x + nx, y, pts[i + 1].z + nz);
        glVertex3f(pts[i + 1].x - nx, y, pts[i + 1].z - nz);
        glEnd();
    }

    // Vẽ đĩa tròn nối khớp mượt tại các đỉnh gấp khúc để dải lối đi không bị hở góc
    const int jointSlices = 10;
    for (int i = 0; i < count; ++i) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(pts[i].x, y, pts[i].z);
        for (int s = 0; s <= jointSlices; ++s) {
            float ang = (static_cast<float>(s) / jointSlices) * MathCG::TWO_PI;
            glVertex3f(
                pts[i].x + halfW * std::cos(ang),
                y,
                pts[i].z - halfW * std::sin(ang)
            );
        }
        glEnd();
    }
}

/**
 * @brief Vẽ mặt đất công viên, ranh giới ~36.1 ha, đường bao quanh, Sông Hàn,
 *        Hồ nước phản xạ bán trong suốt phía Bắc Sun Wheel (y = 0) & Hệ thống lối đi nội bộ
 */
static void emitParkHorizonBand(TimeOfDay tod) {
    float inR = 0.18f, inG = 0.35f, inB = 0.20f;
    float midR = 0.32f, midG = 0.52f, midB = 0.48f;
    float skyR = 0.52f, skyG = 0.77f, skyB = 0.96f;
    float mtnR = 0.38f, mtnG = 0.56f, mtnB = 0.76f;

    if (tod == TimeOfDay::SUNSET) {
        inR  = 0.22f; inG  = 0.28f; inB  = 0.18f;
        midR = 0.52f; midG = 0.36f; midB = 0.26f;
        skyR = 0.88f; skyG = 0.48f; skyB = 0.30f;
        mtnR = 0.56f; mtnG = 0.34f; mtnB = 0.32f;
    } else if (tod == TimeOfDay::NIGHT) {
        inR  = 0.06f; inG  = 0.11f; inB  = 0.10f;
        midR = 0.05f; midG = 0.08f; midB = 0.12f;
        skyR = 0.03f; skyG = 0.04f; skyB = 0.09f;
        mtnR = 0.06f; mtnG = 0.08f; mtnB = 0.16f;
    }

    const int   horizonSlices = 48;
    const float rInner = 280.0f;
    const float rMid   = 820.0f;
    const float rOuter = 2150.0f;
    const float yBase  = -0.10f;

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(inR, inG, inB);
    glVertex3f(0.0f, yBase, -120.0f);
    for (int i = 0; i <= horizonSlices; ++i) {
        float a = (static_cast<float>(i) / horizonSlices) * MathCG::TWO_PI;
        glVertex3f(rInner * std::cos(a), yBase, -120.0f - rInner * std::sin(a));
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= horizonSlices; ++i) {
        float a = (static_cast<float>(i) / horizonSlices) * MathCG::TWO_PI;
        float ca = std::cos(a);
        float sa = -std::sin(a);
        glColor3f(inR, inG, inB);
        glVertex3f(rInner * ca, yBase, -120.0f + rInner * sa);
        glColor3f(midR, midG, midB);
        glVertex3f(rMid * ca, yBase, -120.0f + rMid * sa);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= horizonSlices; ++i) {
        float a = (static_cast<float>(i) / horizonSlices) * MathCG::TWO_PI;
        float ca = std::cos(a);
        float sa = -std::sin(a);
        glColor3f(midR, midG, midB);
        glVertex3f(rMid * ca, yBase, -120.0f + rMid * sa);
        glColor3f(skyR, skyG, skyB);
        glVertex3f(rOuter * ca, yBase, -120.0f + rOuter * sa);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= horizonSlices; ++i) {
        float a = (static_cast<float>(i) / horizonSlices) * MathCG::TWO_PI;
        float ca = std::cos(a);
        float sa = std::sin(a);
        float hPeak = 45.0f + 55.0f * (0.5f + 0.5f * std::sin(a * 3.0f + 0.6f)) * (0.5f + 0.5f * std::cos(a * 5.0f));
        glColor3f(skyR, skyG, skyB);
        glVertex3f(1980.0f * ca, yBase, -120.0f + 1980.0f * sa);
        glColor3f(mtnR, mtnG, mtnB);
        glVertex3f(1980.0f * ca, hPeak, -120.0f + 1980.0f * sa);
    }
    glEnd();
}

static void emitParkSurfacesAndPathsStatic() {
    // 2. Mặt phẳng cỏ công viên theo ranh giới thực tế (~36.1 ha)
    glColor3f(0.16f, 0.36f, 0.18f);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-220.0f, -0.04f, -180.0f);
    for (int i = 8; i >= 0; --i) {
        glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[i].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[i].z);
    }
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[15].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[15].z);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(200.0f, -0.04f, -100.0f);
    for (int i = 15; i >= 9; --i) {
        glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[i].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[i].z);
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex3f(-220.0f, -0.04f, -180.0f);
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[15].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[15].z);
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[8].x,  -0.04f, ParkLayout::PARK_BOUNDARY_POLY[8].z);

    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[8].x,  -0.04f, ParkLayout::PARK_BOUNDARY_POLY[8].z);
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[15].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[15].z);
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[9].x,  -0.04f, ParkLayout::PARK_BOUNDARY_POLY[9].z);

    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[9].x,  -0.04f, ParkLayout::PARK_BOUNDARY_POLY[9].z);
    glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[15].x, -0.04f, ParkLayout::PARK_BOUNDARY_POLY[15].z);
    glVertex3f( 200.0f, -0.04f, -100.0f);
    glEnd();

    glColor3f(0.28f, 0.52f, 0.30f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < ParkLayout::NUM_PARK_BOUNDARY_PTS; ++i) {
        glVertex3f(ParkLayout::PARK_BOUNDARY_POLY[i].x, 0.015f, ParkLayout::PARK_BOUNDARY_POLY[i].z);
    }
    glEnd();
    glLineWidth(1.0f);

    // 3. Dải mặt nước Sông Hàn
    glColor3f(0.06f, 0.24f, 0.44f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(ParkLayout::HAN_RIVER_QUAD.v[0].x, -0.05f, ParkLayout::HAN_RIVER_QUAD.v[0].z);
    glVertex3f(ParkLayout::HAN_RIVER_QUAD.v[1].x, -0.05f, ParkLayout::HAN_RIVER_QUAD.v[1].z);
    glVertex3f(ParkLayout::HAN_RIVER_QUAD.v[2].x, -0.05f, ParkLayout::HAN_RIVER_QUAD.v[2].z);
    glVertex3f(ParkLayout::HAN_RIVER_QUAD.v[3].x, -0.05f, ParkLayout::HAN_RIVER_QUAD.v[3].z);
    glEnd();

    // 4. Các tuyến đường nhựa bao quanh công viên
    drawFlatPolylineStrip(ParkLayout::ROAD_PHAN_DANG_LUU, ParkLayout::NUM_ROAD_PHAN_DANG_LUU_PTS, ParkLayout::ROAD_PHAN_DANG_LUU_WIDTH, -0.02f, 0.22f, 0.24f, 0.27f);
    drawFlatPolylineStrip(ParkLayout::ROAD_EAST_BORDER, ParkLayout::NUM_ROAD_EAST_BORDER_PTS, ParkLayout::ROAD_EAST_BORDER_WIDTH, -0.02f, 0.22f, 0.24f, 0.27f);
    drawFlatPolylineStrip(ParkLayout::ROAD_WEST_BORDER, ParkLayout::NUM_ROAD_WEST_BORDER_PTS, ParkLayout::ROAD_WEST_BORDER_WIDTH, -0.02f, 0.22f, 0.24f, 0.27f);

    // 5. Hồ nước thực tế phía Bắc Sun Wheel (GL_BLEND) & bờ kè
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.28f, 0.48f, 0.68f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    for (int q = 0; q < ParkLayout::NUM_LAKE_CONVEX_QUADS; ++q) {
        const auto& quad = ParkLayout::LAKE_CONVEX_QUADS[q];
        glVertex3f(quad.v[0].x, 0.0f, quad.v[0].z);
        glVertex3f(quad.v[1].x, 0.0f, quad.v[1].z);
        glVertex3f(quad.v[2].x, 0.0f, quad.v[2].z);
        glVertex3f(quad.v[3].x, 0.0f, quad.v[3].z);
    }
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.55f, 0.56f, 0.58f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < ParkLayout::NUM_LAKE_OUTLINE_PTS; ++i) {
        glVertex3f(ParkLayout::LAKE_OUTLINE_POLY[i].x, 0.03f, ParkLayout::LAKE_OUTLINE_POLY[i].z);
    }
    glEnd();
    glLineWidth(1.0f);

    // 6. Hệ thống lối đi nội bộ & cầu gỗ ra Thủy đình
    drawFlatPolylineStrip(ParkLayout::PATH_MAIN_SPINE, ParkLayout::NUM_PATH_MAIN_SPINE_PTS, ParkLayout::PATH_MAIN_SPINE_WIDTH, ParkLayout::PATH_Y_MAIN, 0.74f, 0.68f, 0.58f);
    drawFlatPolylineStrip(ParkLayout::PATH_SOUTH_LAKE_PROM, ParkLayout::NUM_PATH_SOUTH_LAKE_PROM_PTS, ParkLayout::PATH_SOUTH_LAKE_PROM_WIDTH, ParkLayout::PATH_Y_MAIN, 0.74f, 0.68f, 0.58f);
    drawFlatPolylineStrip(ParkLayout::PATH_WEST_SPINE, ParkLayout::NUM_PATH_WEST_SPINE_PTS, ParkLayout::PATH_WEST_SPINE_WIDTH, ParkLayout::PATH_Y_MAIN, 0.74f, 0.68f, 0.58f);
    drawFlatPolylineStrip(ParkLayout::PATH_LAKESIDE_WEST, ParkLayout::NUM_PATH_LAKESIDE_WEST_PTS, ParkLayout::PATH_LAKESIDE_WEST_WIDTH, ParkLayout::PATH_Y_SECONDARY, 0.66f, 0.62f, 0.54f);
    drawFlatPolylineStrip(ParkLayout::PATH_CENTRAL_CONNECTOR, ParkLayout::NUM_PATH_CENTRAL_CONNECTOR_PTS, ParkLayout::PATH_CENTRAL_CONNECTOR_WIDTH, ParkLayout::PATH_Y_SECONDARY, 0.66f, 0.62f, 0.54f);
    drawFlatPolylineStrip(ParkLayout::PATH_PAVILION_PIER, ParkLayout::NUM_PATH_PAVILION_PIER_PTS, ParkLayout::PATH_PAVILION_PIER_WIDTH, ParkLayout::PIER_Y_DECK, 0.58f, 0.36f, 0.20f);
}

static void emitLakesideLanternsFrame(bool showNormals) {
    for (int i = 0; i < ParkLayout::NUM_LAKESIDE_LANTERNS; ++i) {
        const auto& pos = ParkLayout::LAKESIDE_LANTERN_POS[i];
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);

        glColor3f(0.24f, 0.25f, 0.28f);
        ParametricShapes::drawCylinderY(0.42f, 0.35f, 16, 2, true, showNormals);
        glColor3f(0.38f, 0.40f, 0.44f);
        ParametricShapes::drawCylinderY(0.16f, 3.80f, 16, 4, true, showNormals);

        glPushMatrix();
        glTranslatef(0.0f, 3.80f, 0.0f);
        glColor3f(0.85f, 0.70f, 0.20f);
        ParametricShapes::drawToroidY(0.32f, 0.07f, 16, 8, showNormals);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 4.60f, 0.0f);
        glColor3f(0.72f, 0.18f, 0.14f);
        ParametricShapes::drawConeY(0.72f, 0.0f, 0.55f, 16, 2, true, showNormals);
        glPopMatrix();

        glPopMatrix();
    }
}

static void emitLakesideLanternGlobes(bool showNormals) {
    glColor3f(1.0f, 0.90f, 0.50f);
    for (int i = 0; i < ParkLayout::NUM_LAKESIDE_LANTERNS; ++i) {
        const auto& pos = ParkLayout::LAKESIDE_LANTERN_POS[i];
        glPushMatrix();
        glTranslatef(pos.x, pos.y + 4.25f, pos.z);
        ParametricShapes::drawSphere(0.40f, 16, 12, showNormals);
        glPopMatrix();
    }
}

static void drawParkGroundAndLake() {
    static GLuint sHorizonList[3]    = { 0, 0, 0 };
    static GLuint sGroundStaticList  = 0;
    static GLuint sLanternFrameList  = 0;
    static GLuint sLanternGlobeList  = 0;

    int todIdx = 0;
    if (gLighting.timeOfDay == TimeOfDay::SUNSET) todIdx = 1;
    else if (gLighting.timeOfDay == TimeOfDay::NIGHT) todIdx = 2;

    if (sHorizonList[todIdx] == 0) {
        sHorizonList[todIdx] = glGenLists(1);
        if (sHorizonList[todIdx] != 0) {
            glNewList(sHorizonList[todIdx], GL_COMPILE);
            emitParkHorizonBand(gLighting.timeOfDay);
            glEndList();
        }
    }
    if (sGroundStaticList == 0) {
        sGroundStaticList = glGenLists(1);
        if (sGroundStaticList != 0) {
            glNewList(sGroundStaticList, GL_COMPILE);
            emitParkSurfacesAndPathsStatic();
            glEndList();
        }
    }

    // 1. Mặt đất đường chân trời (tắt tạm LIGHTING & CULL_FACE bên ngoài Display List)
    GLboolean wasLit  = glIsEnabled(GL_LIGHTING);
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    if (sHorizonList[todIdx] != 0) {
        glCallList(sHorizonList[todIdx]);
    } else {
        emitParkHorizonBand(gLighting.timeOfDay);
    }
    if (wasCull) glEnable(GL_CULL_FACE);
    if (wasLit)  glEnable(GL_LIGHTING);

    // 2..6. Nền cỏ, Sông Hàn, 3 đường nhựa, hồ nước, bờ kè & lối đi tĩnh
    if (sGroundStaticList != 0) {
        glCallList(sGroundStaticList);
    } else {
        emitParkSurfacesAndPathsStatic();
    }

    // 7. 10 trụ đèn lồng bờ Nam hồ (Display List cho phần thân & chao bóng khi tắt debug normals)
    if (!gState.showNormalsAndCtrl) {
        if (sLanternFrameList == 0) {
            sLanternFrameList = glGenLists(1);
            if (sLanternFrameList != 0) {
                glNewList(sLanternFrameList, GL_COMPILE);
                emitLakesideLanternsFrame(false);
                glEndList();
            }
        }
        if (sLanternGlobeList == 0) {
            sLanternGlobeList = glGenLists(1);
            if (sLanternGlobeList != 0) {
                glNewList(sLanternGlobeList, GL_COMPILE);
                emitLakesideLanternGlobes(false);
                glEndList();
            }
        }
        if (sLanternFrameList != 0) glCallList(sLanternFrameList);
        else emitLakesideLanternsFrame(false);

        if (gLighting.lightingEnabled) {
            GLfloat emit[4] = { 1.0f, 0.85f, 0.35f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
        }
        if (sLanternGlobeList != 0) glCallList(sLanternGlobeList);
        else emitLakesideLanternGlobes(false);
        if (gLighting.lightingEnabled) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
    } else {
        emitLakesideLanternsFrame(true);
        if (gLighting.lightingEnabled) {
            GLfloat emit[4] = { 1.0f, 0.85f, 0.35f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, emit);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
        }
        emitLakesideLanternGlobes(true);
        if (gLighting.lightingEnabled) {
            GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
        }
    }
}

static void rebuildNightLightLeds() {
    static bool added = false;
    if (added) return;
    added = true;

    gNightLights.addTrackLeds(
        gCurves.queenCobraTrack, 30.0f, -0.25f,
        [](const MathCG::Vec3& p) { return AsiaParkCurvesSystem::transformQueenCobraPoint(p); }
    );
    gNightLights.addTrackLeds(gCurves.monorailTrack,   34.0f, -0.75f);
    for (const auto& mc : gMiniCoasters) gNightLights.addTrackLeds(mc.track, 26.0f, -0.25f);
}

static void displayCallback() {
    gLighting.applyClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Áp dụng trạng thái khử mặt khuất & chế độ vẽ đa giác (Chương 6)
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

    // 2. Áp dụng phép biến đổi chiếu & quan sát (Chương 4)
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

    // 3. Áp dụng chiếu sáng & tô bóng (Chương 6)
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gLighting.applyLights(gState.sunWheelCenter);
    gNightLights.applyNightLights(gLighting.timeOfDay);
    gLighting.applyCoasterHeadlight(
        queenFrame.position + queenFrame.up * 0.8f + queenFrame.tangent * 1.5f,
        queenFrame.tangent
    );

    // 4. Lượt vẽ phản xạ qua mặt phẳng hồ trung tâm y = 0 (Chương 4)
    if (gState.lakeReflection) {
        glPushMatrix();
        MathCG::Mat4 reflectMat = MathCG::Mat4::reflectionAcrossYPlane(0.0f);
        glMultMatrixf(reflectMat.m);

        // Phép phản xạ làm đảo chiều định hướng đỉnh đa giác (CCW <-> CW)
        if (gState.backFaceCulling) glFrontFace(GL_CW);
        drawHierarchicalSunWheelPreview();
        if (gState.backFaceCulling) glFrontFace(GL_CCW);

        glPopMatrix();
    }

    // 5. Vẽ cảnh 3D chính
    bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
    bool smooth = (gLighting.shadingModel == ShadingModel::GOURAUD_SMOOTH);

    drawParkGroundAndLake();
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gParkArch.draw(isNightOrSunset, gState.showNormalsAndCtrl, smooth);
    gNightLights.drawGroundPools(isNightOrSunset);
    drawLakesidePavilionMesh();
    drawHierarchicalSunWheelPreview();
    gCurves.draw(gState.showNormalsAndCtrl, gLighting.lightingEnabled, gLighting.timeOfDay);

    // Các trò chơi & công trình kiến trúc biểu tượng
    gSkyTower.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gCarousel.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gNgoMonGate.draw(isNightOrSunset, gState.showNormalsAndCtrl, smooth);
    gPlayground.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gBumperCars.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    gPirateShip.draw(isNightOrSunset, gState.showNormalsAndCtrl);
    for (const auto& mc : gMiniCoasters) {
        mc.draw(isNightOrSunset, gState.showNormalsAndCtrl, smooth);
    }
    // Đợt 9: Trụ đèn công viên, dây đèn festoon & LED viền công trình (bật/tắt [G])
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gNightLights.drawFixtures(gState.showNormalsAndCtrl);
    gNightLights.drawGlow(isNightOrSunset);
    for (const auto& h : gHumans) h.draw(isNightOrSunset);

    // 6. Vẽ các lớp phủ giao diện 2D (Luôn ở chế độ tô đặc GL_FILL)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 6.1 Bản đồ nhỏ Radar 2D (Chương 1 & 3: WCS->NCS->DCS, Cohen-Sutherland, Đường tròn Midpoint, Đoạn thẳng Bresenham)
    if (gState.showMinimap) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, gState.windowWidth, 0.0, gState.windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        std::vector<MathCG::Vec3> qcPoints;
        for (const auto& s : gCurves.queenCobraTrack.samples) {
            qcPoints.push_back(AsiaParkCurvesSystem::transformQueenCobraPoint(s.position));
        }
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
            std::clamp(activeEye.x, gMinimap.wcsMinX + 12.0f, gMinimap.wcsMaxX - 12.0f),
            activeEye.y,
            std::clamp(activeEye.z, gMinimap.wcsMinZ + 12.0f, gMinimap.wcsMaxZ - 12.0f)
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

    // 6.2 Bảng thông số điều khiển HUD trên màn hình (Chương 3: glutBitmapCharacter)
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
            gCurves.queenCobraTrain.lastSpeedMps,
            gSunWheel.getEffectiveSpeedDeg(),
            gSunWheel.fastDemoMode,
            gNightLights.enabled
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

    // Chế độ bay Drone tự do vẫn hoạt động ngay cả khi tạm dừng các trò chơi để quan sát chi tiết
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
        // Quay vòng quay Sun Wheel theo góc alpha và biến thiên góc màu HSV Hue H trong [0, 360)
        gSunWheel.update(dt);
        gState.sunWheelAngleDeg = gSunWheel.currentAngleDeg;
        gLighting.ledHueDeg     = std::fmod(gLighting.ledHueDeg     + 45.0f * dt, 360.0f);

        // Cập nhật chuyển động của Carousel, Sky Tower & tàu lượn Bezier / Monorail dọc theo đường ray khép kín C^1
        gCarousel.update(dt);
        gState.carouselAngleDeg = gCarousel.currentAngleDeg;
        gState.carouselPhase    = gCarousel.oscillationPhase;

        gSkyTower.update(dt);
        gMinimap.update(dt);
        gState.skyTowerTime     = gSkyTower.cycleTime;
        gCurves.update(dt);
        gParkArch.update(dt);
        gPlayground.update(dt);
        gBumperCars.update(dt);
        gPirateShip.update(dt);
        for (auto& mc : gMiniCoasters) mc.update(dt);
        gNightLights.update(dt);
        for (auto& h : gHumans) h.update(dt);
        gState.coasterParamT    = gCurves.queenCobraTrain.paramT;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0); // Khoảng ~60 FPS
}

static void applyMenuSelection(int option) {
    switch (option) {
        // Các chế độ Camera
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

        // Các chế độ phép chiếu
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

        // Tô bóng & Trạng thái đường ống đồ họa
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

        // Thời gian trong ngày
        case MENU_TIME_DAY:
            gLighting.timeOfDay = TimeOfDay::DAY;
            break;
        case MENU_TIME_SUNSET:
            gLighting.timeOfDay = TimeOfDay::SUNSET;
            break;
        case MENU_TIME_NIGHT:
            gLighting.timeOfDay = TimeOfDay::NIGHT;
            break;

        // Hoạt ảnh & Giao diện người dùng
        case MENU_ANIM_TOGGLE:
            gState.isPaused = !gState.isPaused;
            break;
        case MENU_ANIM_FASTER:
            gState.animSpeed = std::min(4.0f, gState.animSpeed + 0.25f);
            break;
        case MENU_ANIM_SLOWER:
            gState.animSpeed = std::max(0.25f, gState.animSpeed - 0.25f);
            break;
        case MENU_TOGGLE_SUNWHEEL_FAST:
            gSunWheel.fastDemoMode = !gSunWheel.fastDemoMode;
            break;
        case MENU_TOGGLE_NIGHT_LIGHTS:
            gNightLights.enabled = !gNightLights.enabled;
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

    // Ở chế độ Drone tự do, dành riêng các phím W/A/S/D/E/Q để bay mượt và F/+/- để chỉnh tốc độ
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
        case 'v': case 'V':
            applyMenuSelection(MENU_TOGGLE_SUNWHEEL_FAST);
            break;
        case 'g': case 'G':
            applyMenuSelection(MENU_TOGGLE_NIGHT_LIGHTS);
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
        case 27: // Phím ESC
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
    } else if (button == 3 && state == GLUT_DOWN) { // Cuộn con lăn chuột lên
        gCamera.zoom(-6.0f);
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // Cuộn con lăn chuột xuống
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
    glutAddMenuEntry("Bat/Tat Tua nhanh Sun Wheel x5 (2.0 do/s) [V]", MENU_TOGGLE_SUNWHEEL_FAST);
    glutAddMenuEntry("Bat/Tat He thong den LED ban dem [G]",       MENU_TOGGLE_NIGHT_LIGHTS);
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
// CHỤP ẢNH SCENE NHÌN TỪ TRÊN XUỐNG (ORTHO TOP-DOWN) KHÔNG CẦN MÀN HÌNH X11
// ============================================================================

#ifndef _WIN32
static bool saveFramebufferPPM(const char* outPpmPath, int w, int h) {
    std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    FILE* fp = std::fopen(outPpmPath, "wb");
    if (!fp) return false;
    std::fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int y = h - 1; y >= 0; --y) {
        std::fwrite(&pixels[static_cast<size_t>(y) * w * 3], 1, static_cast<size_t>(w) * 3, fp);
    }
    std::fclose(fp);
    return true;
}

static bool renderHeadlessTopDownPPM(const char* outPpmPath) {
    const int capW = 2258;
    const int capH = 1869;

    // 1. Khởi tạo EGL Surfaceless Mesa Pbuffer (hoạt động cả khi không có X11 DISPLAY)
    constexpr EGLenum kPlatformSurfacelessMesa = 0x31DD;
    EGLDisplay dpy = eglGetPlatformDisplay(kPlatformSurfacelessMesa, nullptr, nullptr);
    if (dpy == EGL_NO_DISPLAY) {
        dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
    if (dpy == EGL_NO_DISPLAY) return false;

    EGLint major = 0, minor = 0;
    if (!eglInitialize(dpy, &major, &minor)) return false;
    if (!eglBindAPI(EGL_OPENGL_API)) return false;

    const EGLint cfgAttribs[] = {
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig cfg = nullptr;
    EGLint numCfgs = 0;
    if (!eglChooseConfig(dpy, cfgAttribs, &cfg, 1, &numCfgs) || numCfgs < 1) return false;

    const EGLint pbufAttribs[] = {
        EGL_WIDTH,  capW,
        EGL_HEIGHT, capH,
        EGL_NONE
    };
    EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbufAttribs);
    if (surf == EGL_NO_SURFACE) return false;

    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, nullptr);
    if (ctx == EGL_NO_CONTEXT) return false;
    if (!eglMakeCurrent(dpy, surf, surf, ctx)) return false;

    // 2. Khởi tạo dữ liệu đường ray & thiết lập trạng thái OpenGL
    gCurves.init();
    AsiaParkRides::initWave5aMiniCoasters(gMiniCoasters);
    gHumans = AsiaParkActors::createDefaultCrowd();
    rebuildNightLightLeds();
    glViewport(0, 0, capW, capH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    gLighting.timeOfDay = TimeOfDay::DAY;
    gLighting.applyClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. Thiết lập phép chiếu trực giao từ trên xuống khớp chính xác khung bao của layout_overlay.png:
    //    Local X in [-490, +470], Local Z in [-485, +305] => Engine X in [-490, +470], Engine Z in [-523, +267]
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-490.0, 470.0, -267.0, 523.0, 1.0, 1500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Đặt mắt tại (0, 600, 0) nhìn thẳng xuống gốc (0, 0, 0) với vector Up hướng Bắc (0, 0, -1)
    // Khi đó: X_eye = X_world, Y_eye = -Z_world, Z_eye = Y_world - 600
    // => Z_world = -523 (cạnh Bắc) ánh xạ tới Y_eye = +523 (mép trên ảnh)
    // => Z_world = +267 (cạnh Nam) ánh xạ tới Y_eye = -267 (mép dưới ảnh)
    gluLookAt(0.0, 600.0, 0.0,
              0.0,   0.0, 0.0,
              0.0,   0.0, -1.0);

    TrackFrame queenFrame = gCurves.getQueenCobraCameraFrame();
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gLighting.applyLights(gState.sunWheelCenter);
    gNightLights.applyNightLights(gLighting.timeOfDay);
    gLighting.applyCoasterHeadlight(
        queenFrame.position + queenFrame.up * 0.8f + queenFrame.tangent * 1.5f,
        queenFrame.tangent
    );

    // 4. Vẽ toàn bộ cảnh 3D
    drawParkGroundAndLake();
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gParkArch.draw(false, false, true);
    gNightLights.drawGroundPools(false);
    drawLakesidePavilionMesh();
    drawHierarchicalSunWheelPreview();
    gCurves.draw(false, gLighting.lightingEnabled, gLighting.timeOfDay);
    gSkyTower.draw(false, false);
    gCarousel.draw(false, false);
    gNgoMonGate.draw(false, false, true);
    gPlayground.draw(false, false);
    gBumperCars.draw(false, false);
    gPirateShip.draw(false, false);
    for (const auto& mc : gMiniCoasters) mc.draw(false, false, true);
    gNightLights.setLightingEnabled(gLighting.lightingEnabled);
    gNightLights.drawFixtures(false);
    gNightLights.drawGlow(false);
    for (const auto& h : gHumans) h.draw(false);

    // 5. Vẽ thêm lưới tọa độ mét (bước 50m / 100m theo Local X, Z) để dễ đối chiếu trực tiếp với layout_overlay.png
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_LINES);
    for (int gx = -450; gx <= 450; gx += 50) {
        bool major = (gx % 100 == 0);
        glColor4f(1.0f, 1.0f, 0.85f, major ? 0.38f : 0.16f);
        glVertex3f(static_cast<float>(gx), 0.15f, -523.0f);
        glVertex3f(static_cast<float>(gx), 0.15f,  267.0f);
    }
    for (int gzLoc = -450; gzLoc <= 300; gzLoc += 50) {
        bool major = (gzLoc % 100 == 0);
        float gzEng = static_cast<float>(gzLoc) + ParkLayout::ENGINE_Z_OFFSET;
        glColor4f(1.0f, 1.0f, 0.85f, major ? 0.38f : 0.16f);
        glVertex3f(-490.0f, 0.15f, gzEng);
        glVertex3f( 470.0f, 0.15f, gzEng);
    }
    glEnd();
    glDisable(GL_BLEND);

    glFlush();
    glFinish();

    bool saved = saveFramebufferPPM(outPpmPath, capW, capH);
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surf);
    eglTerminate(dpy);
    return saved;
}

static bool renderHeadlessPerspectiveShots() {
    const int capW = 1600;
    const int capH = 1000;

    constexpr EGLenum kPlatformSurfacelessMesa = 0x31DD;
    EGLDisplay dpy = eglGetPlatformDisplay(kPlatformSurfacelessMesa, nullptr, nullptr);
    if (dpy == EGL_NO_DISPLAY) dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) return false;

    EGLint major = 0, minor = 0;
    if (!eglInitialize(dpy, &major, &minor)) return false;
    if (!eglBindAPI(EGL_OPENGL_API)) return false;

    const EGLint cfgAttribs[] = {
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig cfg = nullptr;
    EGLint numCfgs = 0;
    if (!eglChooseConfig(dpy, cfgAttribs, &cfg, 1, &numCfgs) || numCfgs < 1) return false;

    const EGLint pbufAttribs[] = { EGL_WIDTH, capW, EGL_HEIGHT, capH, EGL_NONE };
    EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbufAttribs);
    if (surf == EGL_NO_SURFACE) return false;

    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, nullptr);
    if (ctx == EGL_NO_CONTEXT) return false;
    if (!eglMakeCurrent(dpy, surf, surf, ctx)) return false;

    gCurves.init();
    AsiaParkRides::initWave5aMiniCoasters(gMiniCoasters);
    gHumans = AsiaParkActors::createDefaultCrowd();
    rebuildNightLightLeds();

    struct ShotSpec {
        const char* outPath;
        MathCG::Vec3 eye;
        MathCG::Vec3 center;
        float fovY;
        TimeOfDay tod;
    };

    const ShotSpec shots[3] = {
        // 1. Toàn cảnh góc cao (nhìn từ phía Nam-Tây-Nam bao quát Cổng Ngọ Môn, Tháp Đồng Hồ, Sun Wheel, Hồ Bắc, Monorail & khu Tây)
        {
            "/tmp/shot_overview.ppm",
            MathCG::Vec3(-260.0f, 240.0f, 310.0f),
            MathCG::Vec3( -35.0f,  15.0f, -95.0f),
            52.0f,
            TimeOfDay::DAY
        },
        // 2. Cận cảnh Sun Wheel cùng hồ nước phía Bắc và Thủy đình (nhìn từ bờ Tây-Bắc hồ hướng về Thủy đình & Sun Wheel kèm phản xạ mặt nước)
        {
            "/tmp/shot_sunwheel_lake.ppm",
            MathCG::Vec3( -88.0f,  36.0f, -185.0f),
            MathCG::Vec3(  -2.0f,  32.0f,  -55.0f),
            50.0f,
            TimeOfDay::SUNSET
        },
        // 3. Cận cảnh cụm tàu lượn Queen Cobra tại bán đảo Tây-Bắc (-255, 0, -358, xoay -25 độ, cạnh hồ nước)
        {
            "/tmp/shot_queencobra.ppm",
            MathCG::Vec3(-175.0f,  46.0f, -275.0f),
            MathCG::Vec3(-255.0f,  18.0f, -358.0f),
            50.0f,
            TimeOfDay::DAY
        }
    };

    for (const auto& sh : shots) {
        glViewport(0, 0, capW, capH);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gLighting.timeOfDay = sh.tod;
        gLighting.applyClearColor();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(sh.fovY, static_cast<double>(capW) / capH, 1.0, 2400.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            sh.eye.x,    sh.eye.y,    sh.eye.z,
            sh.center.x, sh.center.y, sh.center.z,
            0.0,         1.0,         0.0
        );

        TrackFrame queenFrame = gCurves.getQueenCobraCameraFrame();
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gLighting.applyLights(gState.sunWheelCenter);
        gNightLights.applyNightLights(gLighting.timeOfDay);
        gLighting.applyCoasterHeadlight(
            queenFrame.position + queenFrame.up * 0.8f + queenFrame.tangent * 1.5f,
            queenFrame.tangent
        );

        // Lượt vẽ phản xạ mặt hồ y = 0
        if (gState.lakeReflection) {
            glPushMatrix();
            MathCG::Mat4 reflectMat = MathCG::Mat4::reflectionAcrossYPlane(0.0f);
            glMultMatrixf(reflectMat.m);
            glFrontFace(GL_CW);
            drawHierarchicalSunWheelPreview();
            drawLakesidePavilionMesh();
            glFrontFace(GL_CCW);
            glPopMatrix();
        }

        drawParkGroundAndLake();
        bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gParkArch.draw(isNightOrSunset, false, true);
        gNightLights.drawGroundPools(isNightOrSunset);
        drawLakesidePavilionMesh();
        drawHierarchicalSunWheelPreview();
        gCurves.draw(false, gLighting.lightingEnabled, gLighting.timeOfDay);
        gSkyTower.draw(isNightOrSunset, false);
        gCarousel.draw(isNightOrSunset, false);
        gNgoMonGate.draw(isNightOrSunset, false, true);
        gPlayground.draw(isNightOrSunset, false);
        gBumperCars.draw(isNightOrSunset, false);
        gPirateShip.draw(isNightOrSunset, false);
        for (const auto& mc : gMiniCoasters) mc.draw(isNightOrSunset, false, true);
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gNightLights.drawFixtures(false);
        gNightLights.drawGlow(isNightOrSunset);
        for (const auto& h : gHumans) h.draw(isNightOrSunset);

        glFlush();
        glFinish();
        if (!saveFramebufferPPM(sh.outPath, capW, capH)) return false;
    }

    // Chụp bộ ảnh kiểm tra Đợt 4, Đợt 5A, Đợt 5B & Đợt 6
    const ShotSpec wave4And5Shots[15] = {
        // 1. Toàn cảnh góc cao Đợt 4
        {
            "/tmp/w4_overview.ppm",
            MathCG::Vec3( 115.0f, 165.0f, 295.0f),
            MathCG::Vec3(   5.0f,  22.0f,  15.0f),
            50.0f,
            TimeOfDay::SUNSET
        },
        // 2. Cận Cổng Ngọ Môn - Góc 1 (Chính diện phía Nam)
        {
            "/tmp/w4_gate_front.ppm",
            MathCG::Vec3(  16.1f,  18.0f, 192.0f),
            MathCG::Vec3(  16.1f,   8.5f, 139.1f),
            48.0f,
            TimeOfDay::SUNSET
        },
        // 3. Cận Cổng Ngọ Môn - Góc 2 (Góc xiên Tây-Nam 45 độ)
        {
            "/tmp/w4_gate_angle.ppm",
            MathCG::Vec3( -26.0f,  19.0f, 174.0f),
            MathCG::Vec3(  16.1f,   9.0f, 139.1f),
            46.0f,
            TimeOfDay::DAY
        },
        // 4. Cận Cổng Ngọ Môn - Góc 3 (Nhìn từ phía Bắc xuống cổng: thấy rõ khoảng cách 11.6m giữa dầm Monorail Z=121.8m và mặt Bắc cổng Z=133.4m, cổng không bị che)
        {
            "/tmp/w4_gate_back.ppm",
            MathCG::Vec3(  22.0f,  44.0f,  82.0f),
            MathCG::Vec3(   8.0f,   7.5f, 139.1f),
            50.0f,
            TimeOfDay::DAY
        },
        // 5. Quảng trường lát gạch nhìn về phía Bắc (Sân khấu chân Sun Wheel đã lùi về Z = -15.9m)
        {
            "/tmp/w4_plaza_to_sw.ppm",
            MathCG::Vec3( -14.0f,  23.0f,  88.0f),
            MathCG::Vec3(   0.0f,  30.0f, -35.0f),
            54.0f,
            TimeOfDay::SUNSET
        },
        // 6. ĐỢT 5A - Ảnh 1: Toàn cảnh khu Tây từ trên cao
        {
            "/tmp/w5a_west_overview.ppm",
            MathCG::Vec3(-335.0f, 175.0f,  195.0f),
            MathCG::Vec3(-175.0f,  10.0f,  -45.0f),
            54.0f,
            TimeOfDay::DAY
        },
        // 7. ĐỢT 5A - Ảnh 2a: Cận cảnh tàu lượn Port of Sky Treasure & Pirate Ship
        {
            "/tmp/w5a_coaster_port_sky.ppm",
            MathCG::Vec3(-170.0f,  28.0f,   72.0f),
            MathCG::Vec3(-216.0f,   5.5f,   12.0f),
            50.0f,
            TimeOfDay::DAY
        },
        // 8. ĐỢT 5A - Ảnh 2b: Cận cảnh tàu lượn Paradise Fall (Tòa nhà núi đá nhiều tầng mới + vòng ray số 8) & Highway Boat
        {
            "/tmp/w5a_coaster_paradise_fall.ppm",
            MathCG::Vec3(-142.0f,  32.0f,  -62.0f),
            MathCG::Vec3(-194.0f,   5.5f, -124.0f),
            50.0f,
            TimeOfDay::DAY
        },
        // 9. ĐỢT 5B - Ảnh 1: Toàn cảnh khu Tây từ trên cao
        {
            "/tmp/w5b_west_overview.ppm",
            MathCG::Vec3(-325.0f, 170.0f,  190.0f),
            MathCG::Vec3(-160.0f,  10.0f,  -55.0f),
            54.0f,
            TimeOfDay::DAY
        },
        // 10. ĐỢT 5B - Ảnh 2: Cận cảnh cụm Nhà mái ngói xanh (#7 & #8) quanh sân bông hoa & Dãy 3 hàng gian hàng mái sọc đỏ trắng (#6)
        {
            "/tmp/w5b_blue_roof_halls.ppm",
            MathCG::Vec3( -52.0f,  38.0f,  135.0f),
            MathCG::Vec3(-108.0f,   5.0f,   68.0f),
            52.0f,
            TimeOfDay::DAY
        },
        // 11. ĐỢT 5B - Ảnh 3: Cận cảnh khu Đu quay tròn (#9, #10, #13, #14), Nhà ngói đỏ ven hồ (#12) & Núi đá Paradise Fall
        {
            "/tmp/w5b_round_rides.ppm",
            MathCG::Vec3( -62.0f,  46.0f,   45.0f),
            MathCG::Vec3(-138.0f,   6.0f,  -75.0f),
            52.0f,
            TimeOfDay::DAY
        },
        // 12. ĐỢT 6 - Ảnh 1: Toàn cảnh góc cao toàn công viên (bao quát Ngọ Môn, Quảng trường, Sun Wheel, Khu Tây nhiều cây xanh, Hồ Bắc & nửa Đông trống)
        {
            "/tmp/w6_overview_high.ppm",
            MathCG::Vec3(-145.0f, 185.0f,  295.0f),
            MathCG::Vec3( -45.0f,  18.0f,  -35.0f),
            54.0f,
            TimeOfDay::SUNSET
        },
        // 13. ĐỢT 6 - Ảnh 2: Cận Quảng trường Cổng Ngọ Môn - Sun Wheel (thấy rõ cột cờ, bồn hoa, đèn đường phát sáng, đèn lồng, cây xanh & người đi dạo)
        {
            "/tmp/w6_gate_plaza_sw.ppm",
            MathCG::Vec3(  24.0f,  34.0f,  178.0f),
            MathCG::Vec3(   4.0f,  22.0f,   -5.0f),
            52.0f,
            TimeOfDay::SUNSET
        },
        // 14. ĐỢT 6 - Ảnh 3: Khu phía Tây có nhiều cây xanh, mái che + hàng cột BumperCars, bệ đá Playground/PirateShip, nhà mái xanh & tàu lượn
        {
            "/tmp/w6_west_trees.ppm",
            MathCG::Vec3(-265.0f,  88.0f,  165.0f),
            MathCG::Vec3(-145.0f,   6.0f,  -45.0f),
            52.0f,
            TimeOfDay::DAY
        },
        // 15. ĐỢT 6 - Ảnh 4: Góc nhìn ngang tầm mắt du khách (eye-level y = 3.2m trên Quảng trường Trung tâm nhìn về Tháp Đồng Hồ, Sân khấu 2 màn hình tròn, Sun Wheel, bồn hoa, đèn đường & người đi bộ)
        {
            "/tmp/w6_eye_level.ppm",
            MathCG::Vec3(  -4.0f,   3.5f,   98.0f),
            MathCG::Vec3(   2.0f,  26.0f,  -25.0f),
            55.0f,
            TimeOfDay::SUNSET
        }
    };

    for (const auto& sh : wave4And5Shots) {
        glViewport(0, 0, capW, capH);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gLighting.timeOfDay = sh.tod;
        gLighting.applyClearColor();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(sh.fovY, static_cast<double>(capW) / capH, 1.0, 2400.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            sh.eye.x,    sh.eye.y,    sh.eye.z,
            sh.center.x, sh.center.y, sh.center.z,
            0.0,         1.0,         0.0
        );

        TrackFrame queenFrame = gCurves.getQueenCobraCameraFrame();
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gLighting.applyLights(gState.sunWheelCenter);
        gNightLights.applyNightLights(gLighting.timeOfDay);
        gLighting.applyCoasterHeadlight(
            queenFrame.position + queenFrame.up * 0.8f + queenFrame.tangent * 1.5f,
            queenFrame.tangent
        );

        bool isNightOrSunset = (gLighting.timeOfDay != TimeOfDay::DAY);
        drawParkGroundAndLake();
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gParkArch.draw(isNightOrSunset, false, true);
        gNightLights.drawGroundPools(isNightOrSunset);
        drawLakesidePavilionMesh();
        drawHierarchicalSunWheelPreview();
        gCurves.draw(false, gLighting.lightingEnabled, gLighting.timeOfDay);
        gSkyTower.draw(isNightOrSunset, false);
        gCarousel.draw(isNightOrSunset, false);
        gNgoMonGate.draw(isNightOrSunset, false, true);
        gPlayground.draw(isNightOrSunset, false);
        gBumperCars.draw(isNightOrSunset, false);
        gPirateShip.draw(isNightOrSunset, false);
        for (const auto& mc : gMiniCoasters) mc.draw(isNightOrSunset, false, true);
        gNightLights.setLightingEnabled(gLighting.lightingEnabled);
        gNightLights.drawFixtures(false);
        gNightLights.drawGlow(isNightOrSunset);
        for (const auto& h : gHumans) h.draw(isNightOrSunset);

        glFlush();
        glFinish();
        if (!saveFramebufferPPM(sh.outPath, capW, capH)) return false;
    }

    // Đo benchmark FPS thuần trong vòng lặp OpenGL 1280x768 (200 khung hình liên tiếp)
    const int benchW = 1280, benchH = 768, benchFrames = 200;
    glViewport(0, 0, benchW, benchH);
    gLighting.timeOfDay = TimeOfDay::SUNSET;
    auto tStart = std::chrono::high_resolution_clock::now();
    for (int f = 0; f < benchFrames; ++f) {
        gSunWheel.update(1.0f / 60.0f);
        gCurves.update(1.0f / 60.0f);
        gParkArch.update(1.0f / 60.0f);
        gNightLights.update(1.0f / 60.0f);
        for (auto& mc : gMiniCoasters) mc.update(1.0f / 60.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(50.0, static_cast<double>(benchW) / benchH, 1.0, 2200.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(110.0, 150.0, 260.0, 0.0, 25.0, -20.0, 0.0, 1.0, 0.0);

        gNightLights.setLightingEnabled(true);
        gLighting.applyLights(gState.sunWheelCenter);
        gNightLights.applyNightLights(gLighting.timeOfDay);

        drawParkGroundAndLake();
        gNightLights.setLightingEnabled(true);
        gParkArch.draw(true, false, true);
        gNightLights.drawGroundPools(true);
        drawLakesidePavilionMesh();
        drawHierarchicalSunWheelPreview();
        gCurves.draw(false, true, gLighting.timeOfDay);
        gSkyTower.draw(true, false);
        gCarousel.draw(true, false);
        gNgoMonGate.draw(true, false, true);
        gPlayground.draw(true, false);
        gBumperCars.draw(true, false);
        gPirateShip.draw(true, false);
        for (const auto& mc : gMiniCoasters) mc.draw(true, false, true);
        gNightLights.setLightingEnabled(true);
        gNightLights.drawFixtures(false);
        gNightLights.drawGlow(true);
        for (const auto& h : gHumans) h.draw(true);
    }
    glFinish();
    auto tEnd = std::chrono::high_resolution_clock::now();
    double elapsedSec = std::chrono::duration<double>(tEnd - tStart).count();
    std::printf("BENCHMARK_FPS_1280x768: %.1f FPS (%d frames in %.3f s)\n",
                benchFrames / std::max(1e-6, elapsedSec), benchFrames, elapsedSec);

    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surf);
    eglTerminate(dpy);
    return true;
}
#endif

// ============================================================================
// HÀM MAIN KHỞI CHẠY CHƯƠNG TRÌNH
// ============================================================================

int main(int argc, char** argv) {
#ifndef _WIN32
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--capture-top") == 0 && i + 1 < argc) {
            bool ok = renderHeadlessTopDownPPM(argv[i + 1]);
            if (ok) {
                std::printf("Captured top-down scene to %s\n", argv[i + 1]);
                return 0;
            } else {
                std::fprintf(stderr, "Failed to capture headless top-down scene\n");
                return 1;
            }
        }
        if (std::strcmp(argv[i], "--capture-shots") == 0) {
            bool ok = renderHeadlessPerspectiveShots();
            if (ok) {
                std::printf("Captured 3 perspective shots to /tmp/shot_*.ppm\n");
                return 0;
            } else {
                std::fprintf(stderr, "Failed to capture perspective shots\n");
                return 1;
            }
        }
    }
#endif
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(gState.windowWidth, gState.windowHeight);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Cong vien Chau A (Da Nang) - Mo phong Do hoa May tinh 3D");

    // Đăng ký các hàm gọi lại (Callback) xử lý sự kiện của GLUT
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

    // Khởi tạo hệ thống đường cong & đoàn tàu (Queen Cobra + Monorail)
    gCurves.init();
    AsiaParkRides::initWave5aMiniCoasters(gMiniCoasters);
    rebuildNightLightLeds();
    gHumans = AsiaParkActors::createDefaultCrowd();

    // Tạo menu phân cấp khi nhấp chuột phải
    createGLUTMenus();

    // Thiết lập trạng thái OpenGL mặc định
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glutMainLoop();
    return 0;
}
