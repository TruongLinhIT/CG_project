#ifndef NIGHT_ILLUMINATION_H
#define NIGHT_ILLUMINATION_H

/**
 * @file NightIllumination.h
 * @brief Đợt 9 (từ 9b92030, đã điều chỉnh khớp Lô A1–B1 & Lô L1): Hệ thống Chiếu sáng Ban đêm cho Công viên Châu Á (Đà Nẵng).
 *
 * Thành phần:
 *  1. Trụ đèn đôi cao 6.0 m (cột + tay đòn + 2 chụp đèn + 2 bóng) sinh THỦ TỤC dọc 5 tuyến lối đi 2019
 *     trong ParkLayout.h, bước 30 m, đảo bên trái/phải; tự động loại bỏ vị trí nằm trong khối công trình
 *     (bảng chặn khớp Lô A1–B1), quá gần trụ đèn hiện hữu, hoặc trên mặt hồ (ParkLayout::isPointInLake).
 *  2. Dây đèn festoon nối hai trụ liền kề cùng tuyến: dây võng parabol, 14 bóng nhỏ.
 *  3. Dải LED điểm (bulb) & ống neon khớp hình học Lô A1–B1 của: Cổng Ngọ Môn, Tháp Đồng Hồ (5 tầng mái A1),
 *     Golden Sky Tower (khớp cột & sàn +4.2 m A1), Carousel (vũng sáng & trụ hàng rào tĩnh, không trùng dlNightLights_),
 *     6 chòi mái ngói, Thủy đình, dãy quầy mái sọc, 4 đu quay tròn, Sân chơi, Xe va chạm, Thuyền cướp biển,
 *     Quần thể Angkor Wat B1 và đường ray Queen Cobra / Monorail / 4 MiniCoaster. (Không gồm Đợt 8c).
 *  4. Quầng sáng billboard cộng màu (GL_SRC_ALPHA, GL_ONE) + hiệu ứng chạy đuổi / nhấp nháy theo thời gian.
 *  5. 3 nguồn sáng điểm bổ sung GL_LIGHT3..5 với suy giảm bậc hai 1/D^2 (tổng GL_LIGHT0..5 = 6/8 <= 8); bật/tắt [G].
 */

#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "curves/BezierTrack.h"
#include "geometry/MeshModel.h"

namespace AsiaParkRides {

class NightIllumination {
public:
    bool  enabled     = true;  // Phím [G] / menu chuột phải
    float animTime    = 0.0f;  // Đồng hồ hiệu ứng chạy đuổi
    float poleHeight  = 6.0f;
    float poleSpacing = 30.0f;

    struct LedPoint {
        MathCG::Vec3     pos;
        float            radius;
        float            phase;
        MathCG::ColorRGB tint;
    };

    struct LightStrip {
        MathCG::Vec3     a;
        MathCG::Vec3     b;
        MathCG::ColorRGB tint;
        float            phase;
    };

    struct GroundPool {
        MathCG::Vec3     pos;
        float            radius;
        MathCG::ColorRGB tint;
        float            phase;
    };

    NightIllumination() {
        buildFixtures();
    }

    ~NightIllumination() {
        if (fixturesList_ != 0) {
            glDeleteLists(fixturesList_, 1);
            fixturesList_ = 0;
        }
    }

    NightIllumination(const NightIllumination&) = delete;
    NightIllumination& operator=(const NightIllumination&) = delete;

    void update(float dt) {
        animTime = std::fmod(animTime + dt, 600.0f);
    }

    template <typename TransformFn>
    void addTrackLeds(const BezierTrack& track, float stepMeters, float yOffset, TransformFn&& xform) {
        if (track.samples.size() < 4 || track.getTotalLength() <= 1.0f) return;

        int step = static_cast<int>(std::lround(track.samples.size() * stepMeters / track.getTotalLength()));
        if (step < 2) step = 2;

        int k = 0;
        for (size_t i = 0; i < track.samples.size(); i += static_cast<size_t>(step)) {
            const MathCG::Vec3 p = xform(track.samples[i].position);
            LedPoint lp;
            lp.pos    = MathCG::Vec3(p.x, p.y + yOffset, p.z);
            lp.radius = 0.85f;
            lp.phase  = static_cast<float>(k++) * 0.11f;
            MathCG::HSVtoRGB(std::fmod(190.0f + static_cast<float>(k) * 23.0f, 360.0f), 0.85f, 1.0f,
                             lp.tint.r, lp.tint.g, lp.tint.b);
            leds_.push_back(lp);
        }
    }

    void addTrackLeds(const BezierTrack& track, float stepMeters = 26.0f, float yOffset = -0.25f) {
        addTrackLeds(track, stepMeters, yOffset, [](const MathCG::Vec3& p) { return p; });
    }

    /**
     * @brief Cấu hình 3 nguồn sáng điểm bổ sung (GL_LIGHT3..5) cho nền quảng trường & khu Tây.
     *        Tổng nguồn sáng phần cứng tối đa là GL_LIGHT0..5 = 6/8.
     */
    void applyNightLights(const TimeOfDay& tod) const {
        if (!enabled || tod == TimeOfDay::DAY || !lightingOn_) {
            for (int i = 3; i <= 7; ++i) glDisable(static_cast<GLenum>(GL_LIGHT0 + i));
            return;
        }

        const float k = (tod == TimeOfDay::NIGHT) ? 1.0f : 0.55f;

        struct PointLightSpec {
            float x, y, z;
            float r, g, b;
            float intensity;
            float quadAtten;
        };
        const PointLightSpec specs[3] = {
            {    0.0f, 9.0f,   92.0f, 1.00f, 0.86f, 0.60f, 0.95f, 0.00055f }, // Quảng trường Trung tâm - Cổng Ngọ Môn
            { -196.0f, 9.0f,   34.0f, 1.00f, 0.82f, 0.52f, 0.90f, 0.00050f }, // Ngã ba trục trò chơi khu Tây
            { -215.0f, 9.0f, -150.0f, 0.72f, 0.86f, 1.00f, 0.80f, 0.00048f }  // Khu Angkor / Paradise Fall ven hồ
        };

        glDisable(GL_LIGHT6);
        glDisable(GL_LIGHT7);
        for (int i = 0; i < 3; ++i) {
            const GLenum id = static_cast<GLenum>(GL_LIGHT3 + i);
            const PointLightSpec& s = specs[i];
            GLfloat pos[4]  = { s.x, s.y, s.z, 1.0f };
            GLfloat diff[4] = { s.r * s.intensity * k, s.g * s.intensity * k, s.b * s.intensity * k, 1.0f };
            GLfloat spec[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Tắt specular trên 3 đèn phụ để giữ >= 65 FPS trên Intel iGPU
            GLfloat amb[4]  = { diff[0] * 0.20f, diff[1] * 0.20f, diff[2] * 0.20f, 1.0f };

            glEnable(id);
            glLightfv(id, GL_POSITION, pos);
            glLightfv(id, GL_AMBIENT,  amb);
            glLightfv(id, GL_DIFFUSE,  diff);
            glLightfv(id, GL_SPECULAR, spec);
            glLightf (id, GL_CONSTANT_ATTENUATION,  0.75f);
            glLightf (id, GL_LINEAR_ATTENUATION,    0.0035f);
            glLightf (id, GL_QUADRATIC_ATTENUATION, s.quadAtten);
        }
    }

    void drawFixtures(bool showNormals = false) const {
        if (poleMesh_.vertices.empty()) return;

        if (!showNormals) {
            if (fixturesList_ == 0) {
                fixturesList_ = glGenLists(1);
                if (fixturesList_ != 0) {
                    glNewList(fixturesList_, GL_COMPILE);
                    poleMesh_.draw(true, false);
                    glEndList();
                }
            }
            if (fixturesList_ != 0) {
                glCallList(fixturesList_);
                return;
            }
        }
        poleMesh_.draw(!showNormals, showNormals);
    }

    void drawGlow(bool isNightOrSunset, float cullDistance = 850.0f, float haloDistance = 120.0f) const {
        if (!enabled || !isNightOrSunset) return;
        if (leds_.empty() && strips_.empty()) return;

        GLfloat m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        MathCG::Vec3 camRight(m[0], m[4], m[8]);
        MathCG::Vec3 camUp(m[1], m[5], m[9]);
        if (camRight.length() < 1e-6f) camRight = MathCG::Vec3(1.0f, 0.0f, 0.0f);
        if (camUp.length() < 1e-6f)    camUp    = MathCG::Vec3(0.0f, 1.0f, 0.0f);
        camRight = camRight.normalized();
        camUp    = camUp.normalized();

        const float camX = -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]);
        const float camY = -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]);
        const float camZ = -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]);
        const float cull2 = cullDistance * cullDistance;
        const float halo2 = haloDistance * haloDistance;
        constexpr float nearCore2 = 45.0f * 45.0f;

        // Tính trước 6 hướng đơn vị trên mặt phẳng camera để tránh gọi cos/sin hàng chục nghìn lần mỗi khung
        static constexpr float kCos6[6] = { 1.0f, 0.5f, -0.5f, -1.0f, -0.5f, 0.5f };
        static constexpr float kSin6[6] = { 0.0f, 0.8660254f, 0.8660254f, 0.0f, -0.8660254f, -0.8660254f };
        float hexX[6], hexY[6], hexZ[6];
        for (int i = 0; i < 6; ++i) {
            hexX[i] = camRight.x * kCos6[i] + camUp.x * kSin6[i];
            hexY[i] = camRight.y * kCos6[i] + camUp.y * kSin6[i];
            hexZ[i] = camRight.z * kCos6[i] + camUp.z * kSin6[i];
        }
        const float q0x =  camRight.x + camUp.x, q0y =  camRight.y + camUp.y, q0z =  camRight.z + camUp.z;
        const float q1x = -camRight.x + camUp.x, q1y = -camRight.y + camUp.y, q1z = -camRight.z + camUp.z;

        GLboolean wasLit  = glIsEnabled(GL_LIGHTING);
        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        if (!strips_.empty()) {
            glLineWidth(2.4f);
            glBegin(GL_LINES);
            for (const auto& st : strips_) {
                float wave = 0.55f + 0.45f * std::sin(animTime * 1.8f - st.phase * 6.2831853f);
                glColor3f(st.tint.r * (0.35f + 0.65f * wave),
                          st.tint.g * (0.35f + 0.65f * wave),
                          st.tint.b * (0.35f + 0.65f * wave));
                glVertex3f(st.a.x, st.a.y, st.a.z);
                glColor3f(st.tint.r, st.tint.g, st.tint.b);
                glVertex3f(st.b.x, st.b.y, st.b.z);
            }
            glEnd();
            glLineWidth(1.0f);
        }

        const float pulseBase = animTime * 2.4f;
        glBegin(GL_TRIANGLES);
        for (const auto& led : leds_) {
            const float dx = led.pos.x - camX;
            const float dy = led.pos.y - camY;
            const float dz = led.pos.z - camZ;
            const float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 > cull2) continue;

            const float pulse = 0.62f + 0.38f * std::sin(pulseBase - led.phase * 6.2831853f);
            const float r = led.tint.r, g = led.tint.g, b = led.tint.b;
            const float px = led.pos.x, py = led.pos.y, pz = led.pos.z;

            if (d2 <= halo2) {
                const float rOut = led.radius;
                for (int i = 0; i < 6; ++i) {
                    const int i1 = (i + 1) % 6;
                    glColor4f(r * pulse, g * pulse, b * pulse, 0.30f);
                    glVertex3f(px, py, pz);
                    glColor4f(r, g, b, 0.0f);
                    glVertex3f(px + hexX[i]  * rOut, py + hexY[i]  * rOut, pz + hexZ[i]  * rOut);
                    glVertex3f(px + hexX[i1] * rOut, py + hexY[i1] * rOut, pz + hexZ[i1] * rOut);
                }
            }

            const float rCore = led.radius * 0.34f;
            glColor4f(std::min(1.0f, r * pulse + 0.35f),
                      std::min(1.0f, g * pulse + 0.35f),
                      std::min(1.0f, b * pulse + 0.30f), 0.92f);
            if (d2 <= nearCore2) {
                const float v0x = px + hexX[0] * rCore, v0y = py + hexY[0] * rCore, v0z = pz + hexZ[0] * rCore;
                for (int i = 1; i <= 4; ++i) {
                    glVertex3f(v0x, v0y, v0z);
                    glVertex3f(px + hexX[i]     * rCore, py + hexY[i]     * rCore, pz + hexZ[i]     * rCore);
                    glVertex3f(px + hexX[i + 1] * rCore, py + hexY[i + 1] * rCore, pz + hexZ[i + 1] * rCore);
                }
            } else {
                const float s0x = q0x * rCore, s0y = q0y * rCore, s0z = q0z * rCore;
                const float s1x = q1x * rCore, s1y = q1y * rCore, s1z = q1z * rCore;
                glVertex3f(px - s0x, py - s0y, pz - s0z);
                glVertex3f(px + s1x, py + s1y, pz + s1z);
                glVertex3f(px + s0x, py + s0y, pz + s0z);
                glVertex3f(px - s0x, py - s0y, pz - s0z);
                glVertex3f(px + s0x, py + s0y, pz + s0z);
                glVertex3f(px - s1x, py - s1y, pz - s1z);
            }
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        if (wasCull) glEnable(GL_CULL_FACE);
        if (wasLit)  glEnable(GL_LIGHTING);
    }

    void drawGroundPools(bool isNightOrSunset, float cullDistance = 340.0f) const {
        if (!enabled || !isNightOrSunset || pools_.empty()) return;

        GLfloat m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        const float camX = -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]);
        const float camY = -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]);
        const float camZ = -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]);
        const float cull2 = cullDistance * cullDistance;

        GLboolean wasLit  = glIsEnabled(GL_LIGHTING);
        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -4.0f);

        glBegin(GL_TRIANGLES);
        for (const auto& gp : pools_) {
            const float dx = gp.pos.x - camX;
            const float dy = gp.pos.y - camY;
            const float dz = gp.pos.z - camZ;
            if (dx * dx + dy * dy + dz * dz > cull2) continue;

            const float br = 0.80f + 0.20f * std::sin(animTime * 1.1f - gp.phase * 6.2831853f);
            const float y  = gp.pos.y;
            const float r  = gp.tint.r, g = gp.tint.g, b = gp.tint.b;

            glColor4f(r * br, g * br, b * br, 0.34f);
            for (int i = 0; i < 12; ++i) {
                const float a0 = (static_cast<float>(i)     / 12.0f) * MathCG::TWO_PI;
                const float a1 = (static_cast<float>(i + 1) / 12.0f) * MathCG::TWO_PI;
                glVertex3f(gp.pos.x, y, gp.pos.z);
                glColor4f(r, g, b, 0.0f);
                glVertex3f(gp.pos.x + gp.radius * std::cos(a0), y, gp.pos.z + gp.radius * std::sin(a0));
                glVertex3f(gp.pos.x + gp.radius * std::cos(a1), y, gp.pos.z + gp.radius * std::sin(a1));
                glColor4f(r * br, g * br, b * br, 0.34f);
            }

            glColor4f(r * br, g * br, b * br, 0.72f);
            const float rCore = gp.radius * 0.34f;
            for (int i = 0; i < 12; ++i) {
                const float a0 = (static_cast<float>(i)     / 12.0f) * MathCG::TWO_PI;
                const float a1 = (static_cast<float>(i + 1) / 12.0f) * MathCG::TWO_PI;
                glVertex3f(gp.pos.x, y, gp.pos.z);
                glColor4f(r, g, b, 0.0f);
                glVertex3f(gp.pos.x + rCore * std::cos(a0), y, gp.pos.z + rCore * std::sin(a0));
                glVertex3f(gp.pos.x + rCore * std::cos(a1), y, gp.pos.z + rCore * std::sin(a1));
                glColor4f(r * br, g * br, b * br, 0.72f);
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        if (wasCull) glEnable(GL_CULL_FACE);
        if (wasLit)  glEnable(GL_LIGHTING);
    }

    int ledCount()   const { return static_cast<int>(leds_.size()); }
    int stripCount() const { return static_cast<int>(strips_.size()); }
    int poolCount()  const { return static_cast<int>(pools_.size()); }

    void setLightingEnabled(bool on) { lightingOn_ = on; }

private:
    mutable Geometry::PolygonMesh poleMesh_;
    mutable GLuint fixturesList_ = 0;
    std::vector<LedPoint>   leds_;
    std::vector<LightStrip> strips_;
    std::vector<GroundPool> pools_;
    bool lightingOn_ = true;

    struct RectBlock { float minX, maxX, minZ, maxZ; };
    struct CircleBlock { float x, z, r; };

    static bool blockedByStructure(float x, float z) {
        static const RectBlock rects[] = {
            {  -27.0f,   59.0f,  130.0f, 150.0f }, // Cổng Ngọ Môn + 2 cánh tường thành
            {  -32.0f,   -4.0f,  147.0f, 163.0f }, // Hồ sen Tây
            {   36.0f,   64.0f,  147.0f, 163.0f }, // Hồ sen Đông
            {   -1.8f,   23.2f,   49.6f,  73.6f }, // Tháp Đồng Hồ Lô A1 (24x23.4 m)
            {  -33.0f,  -15.0f,   60.0f,  70.0f }, // Chòi mái ngói Tây quảng trường
            {   39.0f,   57.0f,   60.0f,  70.0f }, // Chòi mái ngói Đông quảng trường
            {  -27.5f,   27.5f,  -27.0f, -10.0f }, // Sân khấu Sun Wheel
            {  -22.0f,   22.0f,  -47.0f, -26.0f }, // Bệ móng chân chữ A Sun Wheel
            {  -47.0f,  -33.0f,  -29.0f, -15.0f }, // Chòi bên Tây Sun Wheel
            {   33.0f,   47.0f,  -29.0f, -15.0f }, // Chòi bên Đông Sun Wheel
            {  -55.0f,  -43.0f,  -93.0f, -83.0f }, // Chòi vọng cảnh Tây ven hồ
            {   35.0f,   47.0f,  -96.0f, -86.0f }, // Chòi vọng cảnh Đông ven hồ
            {  -60.0f,  -40.0f,   -8.0f,  32.0f }, // Dãy 3 hàng quầy mái sọc
            { -128.0f,  -99.0f,   48.0f,  62.0f }, // Nhà mái ngói xanh Bắc
            { -115.0f,  -90.0f,  112.0f, 124.0f }, // Nhà mái ngói xanh Nam
            { -106.0f,  -82.0f,  -81.0f, -61.0f }, // Nhà ngói đỏ lớn ven hồ
            { -267.0f, -245.0f, -140.0f, -118.0f }, // Nhà mái xanh khu Tây
            { -149.0f, -127.0f, -250.0f, -234.0f }, // Nhà mái xanh Angkor ven hồ
            { -199.5f, -170.5f, -268.5f, -239.5f }, // Quảng trường Ngôi sao & Đền Angkor Wat Lô B1
            {  -62.0f,  -18.0f,  -34.0f,  -16.0f }  // Sân chơi trẻ em
        };
        for (const auto& r : rects) {
            if (x >= r.minX && x <= r.maxX && z >= r.minZ && z <= r.maxZ) return true;
        }

        static const CircleBlock circles[] = {
            { -159.4f,  102.5f, 12.0f }, // Carousel Lô A3 (ĐK 17 m)
            { -260.0f,  -54.6f, 10.5f }, // Golden Sky Tower Lô A1 (R <= 8 m)
            { -132.7f,   12.9f, 10.5f }, // Đu quay tròn #9
            {  -94.2f,   17.3f,  8.5f }, // Đu quay tròn #10
            { -141.3f, -137.6f, 11.0f }, // Công trình tròn #13
            { -143.4f, -179.6f,  9.5f }, // Đu quay tròn #14
            { -206.5f,  108.1f, 12.0f }, // Xe va chạm
            { -172.0f,   -2.0f, 10.0f }, // Thuyền cướp biển
            { -124.1f,  155.7f, 20.0f }, // Garuda Valley
            { -224.0f,   22.0f, 22.0f }, // Port of Sky Treasure
            { -179.8f,  -91.1f, 17.5f }, // Tòa nhà đá Paradise Fall + lầu ga Lô A2
            { -199.0f, -119.9f, 19.0f }, // Vòng ray số 8 Paradise Fall
            { -176.0f, -214.0f, 19.0f }  // Highway Boat
        };
        for (const auto& c : circles) {
            float dx = x - c.x, dz = z - c.z;
            if (dx * dx + dz * dz <= c.r * c.r) return true;
        }
        return false;
    }

    static bool nearExistingParkLamp(float x, float z) {
        constexpr float kMinDist2 = 5.0f * 5.0f;
        for (int i = 0; i < ParkLayout::NUM_PARK_PATH_LAMPS; ++i) {
            float dx = x - ParkLayout::PARK_PATH_LAMP_POS[i].x;
            float dz = z - ParkLayout::PARK_PATH_LAMP_POS[i].z;
            if (dx * dx + dz * dz < kMinDist2) return true;
        }
        for (int i = 0; i < ParkLayout::NUM_LAKESIDE_LANTERNS; ++i) {
            float dx = x - ParkLayout::LAKESIDE_LANTERN_POS[i].x;
            float dz = z - ParkLayout::LAKESIDE_LANTERN_POS[i].z;
            if (dx * dx + dz * dz < kMinDist2) return true;
        }
        return false;
    }

    void buildFixtures() {
        // 1. Trụ đèn đôi dọc 5 tuyến lối đi 2019 (không gồm Đợt 8c)
        addPolesAlongPath(ParkLayout::PATH_MAIN_SPINE,        ParkLayout::NUM_PATH_MAIN_SPINE_PTS,        ParkLayout::PATH_MAIN_SPINE_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_SOUTH_LAKE_PROM,   ParkLayout::NUM_PATH_SOUTH_LAKE_PROM_PTS,   ParkLayout::PATH_SOUTH_LAKE_PROM_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_WEST_SPINE,        ParkLayout::NUM_PATH_WEST_SPINE_PTS,        ParkLayout::PATH_WEST_SPINE_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_LAKESIDE_WEST,     ParkLayout::NUM_PATH_LAKESIDE_WEST_PTS,     ParkLayout::PATH_LAKESIDE_WEST_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_CENTRAL_CONNECTOR, ParkLayout::NUM_PATH_CENTRAL_CONNECTOR_PTS, ParkLayout::PATH_CENTRAL_CONNECTOR_WIDTH);

        // 2. LED viền công trình khớp Lô A1–B1 & Lô L1
        buildGateLeds();
        buildClockTowerLeds();
        buildCarouselLeds();
        buildSkyTowerLeds();
        buildPavilionLeds();
        buildWestSideLeds();

        poleMesh_.computeNormals();
    }

    void addPole(float x, float z, float phase) {
        const MathCG::ColorRGB ironCol(0.26f, 0.27f, 0.31f);
        const MathCG::ColorRGB goldCol(0.86f, 0.70f, 0.26f);
        const float H = poleHeight;

        Geometry::PolygonMesh::appendBoxByYRange(poleMesh_, x, 0.0f, 0.42f, z, 0.62f, 0.62f, ironCol);
        poleMesh_.append(Geometry::PolygonMesh::createPrismY(x, 0.36f, z, 0.145f, H - 0.30f, 8, ironCol));
        poleMesh_.append(Geometry::PolygonMesh::createPrismY(x, H - 0.10f, z, 0.26f, 0.18f, 8, goldCol));

        Geometry::PolygonMesh::appendBoxByYRange(poleMesh_, x, H - 0.28f, H - 0.14f, z, 2.20f, 0.14f, goldCol);
        for (int s : {-1, 1}) {
            poleMesh_.append(Geometry::PolygonMesh::createPyramid(
                x + s * 1.00f, H - 0.02f, z, 0.92f, 0.92f, 0.34f, ironCol));
        }
        poleMesh_.append(Geometry::PolygonMesh::createPyramid(x, H + 0.06f, z, 0.46f, 0.46f, 0.42f, goldCol));

        const float ph[2] = { phase, phase + 0.35f };
        for (int s : {-1, 1}) {
            LedPoint lp;
            lp.pos    = MathCG::Vec3(x + s * 1.00f, H - 0.12f, z);
            lp.radius = 1.05f;
            lp.phase  = ph[s > 0 ? 1 : 0];
            lp.tint   = MathCG::ColorRGB(1.00f, 0.88f, 0.58f);
            leds_.push_back(lp);
        }

        GroundPool gp;
        gp.pos    = MathCG::Vec3(x, 0.055f, z);
        gp.radius = 5.20f;
        gp.tint   = MathCG::ColorRGB(1.00f, 0.82f, 0.48f);
        gp.phase  = phase;
        pools_.push_back(gp);
    }

    void addPolesAlongPath(const ParkLayout::PointXZ* pts, int count, float pathWidth) {
        if (!pts || count < 2) return;

        const float sideOffset = pathWidth * 0.5f + 1.45f;
        float phaseCounter = 0.0f;
        int   sideToggle   = 1;

        std::vector<MathCG::Vec3> chain;
        chain.reserve(static_cast<size_t>(count) * 4);

        for (int i = 0; i < count - 1; ++i) {
            const float x0 = pts[i].x,     z0 = pts[i].z;
            const float x1 = pts[i + 1].x, z1 = pts[i + 1].z;
            const float dx = x1 - x0, dz = z1 - z0;
            const float len = std::sqrt(dx * dx + dz * dz);
            if (len < 1e-3f) continue;

            const float ux = dx / len, uz = dz / len;
            const float nx = -uz,     nz =  ux;

            for (float t = 0.0f; t < len; t += poleSpacing) {
                float px = x0 + ux * t + nx * sideOffset * static_cast<float>(sideToggle);
                float pz = z0 + uz * t + nz * sideOffset * static_cast<float>(sideToggle);

                if (!ParkLayout::isPointInLake(px, pz) &&
                    !blockedByStructure(px, pz) &&
                    !nearExistingParkLamp(px, pz)) {
                    addPole(px, pz, phaseCounter);
                    chain.push_back(MathCG::Vec3(px, poleHeight - 0.12f, pz));
                    phaseCounter += 0.21f;
                }
                sideToggle = -sideToggle;
            }
        }

        for (size_t i = 0; i + 1 < chain.size(); ++i) {
            const MathCG::Vec3& a = chain[i];
            const MathCG::Vec3& b = chain[i + 1];
            float d = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.z - a.z) * (b.z - a.z));
            if (d < 6.0f || d > 42.0f) continue;

            const float sag = 0.30f + 0.055f * d;
            const int   numBulbs = 14;
            for (int k = 1; k < numBulbs; ++k) {
                float u = static_cast<float>(k) / static_cast<float>(numBulbs);
                LedPoint lp;
                lp.pos = MathCG::Vec3(
                    a.x + (b.x - a.x) * u,
                    a.y + (b.y - a.y) * u - sag * std::sin(MathCG::PI * u),
                    a.z + (b.z - a.z) * u
                );
                lp.radius = 0.52f;
                lp.phase  = static_cast<float>(i) * 0.17f + u * 0.9f;
                MathCG::HSVtoRGB(std::fmod(42.0f + u * 300.0f + static_cast<float>(i) * 26.0f, 360.0f),
                                 0.72f, 1.0f, lp.tint.r, lp.tint.g, lp.tint.b);
                leds_.push_back(lp);
            }
        }

        addPathEdgeStrips(pts, count, pathWidth);
    }

    void addPathEdgeStrips(const ParkLayout::PointXZ* pts, int count, float pathWidth) {
        const float edgeOffset = pathWidth * 0.5f - 0.35f;
        const float spacing    = 6.5f;
        const MathCG::ColorRGB edgeTint(1.00f, 0.86f, 0.62f);
        float phase = 0.0f;

        for (int i = 0; i < count - 1; ++i) {
            const float x0 = pts[i].x,     z0 = pts[i].z;
            const float x1 = pts[i + 1].x, z1 = pts[i + 1].z;
            const float dx = x1 - x0, dz = z1 - z0;
            const float len = std::sqrt(dx * dx + dz * dz);
            if (len < 1e-3f) continue;

            const float ux = dx / len, uz = dz / len;
            const float nx = -uz,     nz =  ux;

            int stepIdx = 0;
            for (float t = 0.0f; t <= len; t += spacing, ++stepIdx) {
                for (int side : {-1, 1}) {
                    float px = x0 + ux * t + nx * edgeOffset * static_cast<float>(side);
                    float pz = z0 + uz * t + nz * edgeOffset * static_cast<float>(side);
                    if (ParkLayout::isPointInLake(px, pz)) continue;

                    LedPoint lp;
                    lp.pos    = MathCG::Vec3(px, 0.30f, pz);
                    lp.radius = 1.15f;
                    lp.phase  = phase;
                    lp.tint   = edgeTint;
                    leds_.push_back(lp);

                    if ((stepIdx & 1) == 0) {
                        GroundPool gp;
                        gp.pos    = MathCG::Vec3(px, 0.052f, pz);
                        gp.radius = 2.80f;
                        gp.tint   = MathCG::ColorRGB(1.00f, 0.80f, 0.50f);
                        gp.phase  = phase;
                        pools_.push_back(gp);
                    }
                    phase += 0.045f;
                }
            }
        }
    }

    void addRectOutline(float cx, float cz, float halfW, float halfD,
                        float y, float rotDeg, float spacing,
                        const MathCG::ColorRGB& tint, float radius, float phaseBase) {
        const float rad = rotDeg * MathCG::DEG2RAD;
        const float cs = std::cos(rad), sn = std::sin(rad);

        auto emit = [&](float lx, float lz, float ph) {
            LedPoint lp;
            lp.pos    = MathCG::Vec3(cx + lx * cs - lz * sn, y, cz + lx * sn + lz * cs);
            lp.radius = radius;
            lp.phase  = ph;
            lp.tint   = tint;
            leds_.push_back(lp);
        };

        const float perW = 2.0f * halfW, perD = 2.0f * halfD;
        int nW = std::max(2, static_cast<int>(perW / spacing));
        int nD = std::max(2, static_cast<int>(perD / spacing));
        for (int i = 0; i <= nW; ++i) {
            float lx = -halfW + perW * static_cast<float>(i) / nW;
            emit(lx, -halfD, phaseBase + 0.02f * i);
            emit(lx,  halfD, phaseBase + 0.02f * i + 0.5f);
        }
        for (int i = 1; i < nD; ++i) {
            float lz = -halfD + perD * static_cast<float>(i) / nD;
            emit(-halfW, lz, phaseBase + 0.02f * i + 0.25f);
            emit( halfW, lz, phaseBase + 0.02f * i + 0.75f);
        }
    }

    void addRing(float cx, float cz, float r, float y, int n,
                 const MathCG::ColorRGB& tint, float radius, float phaseBase) {
        if (n < 3) n = 3;
        for (int i = 0; i < n; ++i) {
            float a = (static_cast<float>(i) / n) * MathCG::TWO_PI;
            LedPoint lp;
            lp.pos    = MathCG::Vec3(cx + r * std::cos(a), y, cz + r * std::sin(a));
            lp.radius = radius;
            lp.phase  = phaseBase + static_cast<float>(i) / static_cast<float>(n);
            lp.tint   = tint;
            leds_.push_back(lp);
        }
    }

    void addVerticalStrip(float x, float z, float y0, float y1,
                          const MathCG::ColorRGB& tint, float phase) {
        LightStrip st;
        st.a     = MathCG::Vec3(x, y0, z);
        st.b     = MathCG::Vec3(x, y1, z);
        st.tint  = tint;
        st.phase = phase;
        strips_.push_back(st);
    }

    void addPool(float x, float z, float radius, const MathCG::ColorRGB& tint, float phase) {
        GroundPool gp;
        gp.pos    = MathCG::Vec3(x, 0.055f, z);
        gp.radius = radius;
        gp.tint   = tint;
        gp.phase  = phase;
        pools_.push_back(gp);
    }

    // 2.1. Cổng Ngọ Môn (scale 0.76)
    void buildGateLeds() {
        const float s  = ParkLayout::NGO_MON_GATE_SCALE;
        const float gx = ParkLayout::NGO_MON_GATE_POS.x;
        const float gz = ParkLayout::NGO_MON_GATE_POS.z;
        const MathCG::ColorRGB warmGold(1.00f, 0.76f, 0.28f);
        const MathCG::ColorRGB warmRed (1.00f, 0.30f, 0.18f);

        addRectOutline(gx, gz, 23.0f * s, 7.6f * s, 6.80f * s, 0.0f, 1.9f, warmGold, 0.80f, 0.00f);
        addRectOutline(gx, gz, 12.5f * s, 4.8f * s, 16.50f * s, 0.0f, 1.7f, warmRed,  0.78f, 0.30f);
        addRectOutline(gx, gz, 19.8f * s, 5.8f * s, 12.60f * s, 0.0f, 1.8f, warmGold, 0.72f, 0.15f);

        for (int side : {-1, 1}) {
            float wx = gx + side * (23.0f + ParkLayout::NGO_MON_WING_SPAN_LOCAL * 0.5f) * s;
            addRectOutline(wx, gz, ParkLayout::NGO_MON_WING_SPAN_LOCAL * 0.5f * s,
                           ParkLayout::NGO_MON_WING_DEPTH_LOCAL * 0.5f * s,
                           ParkLayout::NGO_MON_WING_HEIGHT_LOCAL * s, 0.0f, 2.1f, warmGold, 0.62f, 0.4f);
            addPool(wx - 12.0f, gz - 9.0f, 10.0f, MathCG::ColorRGB(1.00f, 0.72f, 0.34f), 0.4f);
            addPool(wx + 12.0f, gz - 9.0f, 10.0f, MathCG::ColorRGB(1.00f, 0.72f, 0.34f), 0.6f);
        }

        addPool(gx, gz - 12.0f, 26.0f, MathCG::ColorRGB(1.00f, 0.74f, 0.36f), 0.0f);
        addPool(gx, ParkLayout::GATE_FORECOURT_POS.z, 24.0f, MathCG::ColorRGB(1.00f, 0.70f, 0.34f), 0.3f);
    }

    // 2.2. Tháp Đồng Hồ Lô A1: khớp 5 tầng mái cong bát giác (29.4m) + 4 ống neon góc bệ + 4 mặt đồng hồ
    void buildClockTowerLeds() {
        const float cx = ParkLayout::CLOCK_TOWER_POS.x;
        const float cz = ParkLayout::CLOCK_TOWER_POS.z;
        const MathCG::ColorRGB warmGold(1.00f, 0.80f, 0.34f);

        // Theo bảng octTiers[5] trong NgoMonGate.h: 8 mũi góc mái cong tại góc 22.5 + k*45 độ
        const float roofApo[5]  = { 6.55f, 5.65f, 5.35f, 4.55f, 3.85f };
        const float eaveTipY[5] = { 11.89f, 15.11f, 20.00f, 23.30f, 26.22f };
        const float invCos22_5  = 1.0f / std::cos(MathCG::PI / 8.0f);

        for (int t = 0; t < 5; ++t) {
            float tipR = roofApo[t] * invCos22_5 + 0.08f;
            for (int i = 0; i < 8; ++i) {
                float a = (22.5f + 45.0f * static_cast<float>(i)) * MathCG::DEG2RAD;
                LedPoint lp;
                lp.pos    = MathCG::Vec3(cx + tipR * std::cos(a), eaveTipY[t], cz - tipR * std::sin(a));
                lp.radius = 0.78f;
                lp.phase  = 0.06f * static_cast<float>(t) + 0.07f * static_cast<float>(i);
                lp.tint   = warmGold;
                leds_.push_back(lp);
            }
        }

        // 4 ống neon dọc 4 cột góc chân bệ đá Lô A1 (17.6 x 17.0 m, y = 0.45 .. 5.20 m)
        const float pxOff = ParkLayout::CLOCK_TOWER_PODIUM_W * 0.5f - 0.85f;
        const float pzOff = ParkLayout::CLOCK_TOWER_PODIUM_D * 0.5f - 0.85f;
        int sIdx = 0;
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                addVerticalStrip(cx + sx * pxOff, cz + sz * pzOff, 0.45f, 5.15f,
                                 MathCG::ColorRGB(1.00f, 0.68f, 0.24f), 0.25f * static_cast<float>(sIdx++));
            }
        }

        // Quầng dịu phía trước 4 mặt đồng hồ tầng 3 (CLOCK_TOWER_DIAL_Y = 17.35 m, OFFSET = 4.02 m)
        for (int d = 0; d < 4; ++d) {
            float a = static_cast<float>(d) * (MathCG::PI * 0.5f);
            LedPoint lp;
            lp.pos    = MathCG::Vec3(cx + (ParkLayout::CLOCK_TOWER_DIAL_OFFSET + 0.20f) * std::sin(a),
                                     ParkLayout::CLOCK_TOWER_DIAL_Y,
                                     cz + (ParkLayout::CLOCK_TOWER_DIAL_OFFSET + 0.20f) * std::cos(a));
            lp.radius = 0.75f;
            lp.phase  = 0.25f * static_cast<float>(d);
            lp.tint   = MathCG::ColorRGB(0.94f, 0.96f, 1.00f);
            leds_.push_back(lp);
        }
        addPool(cx, cz, 18.0f, MathCG::ColorRGB(1.00f, 0.74f, 0.38f), 0.1f);
    }

    // 2.3. Đu quay Ngựa gỗ Lô A3 + L1: chỉ đặt vũng sáng nền & đèn rào tĩnh ngoài R = 8.12 m
    //      (dải bóng diềm mái & tầng 2 đang quay do Carousel::dlNightLights_ đảm nhận, tránh trùng/trôi)
    void buildCarouselLeds() {
        const MathCG::Vec3& p = ParkLayout::CAROUSEL_POS_UNVERIFIED;
        const MathCG::ColorRGB warmFence(1.00f, 0.86f, 0.44f);
        for (int i = 0; i < 16; ++i) {
            float deg = static_cast<float>(i) * 22.5f;
            if ((deg > 58.0f && deg < 104.0f) || (deg > 256.0f && deg < 284.0f)) continue; // chừa 2 cổng & quầy vé
            float rad = deg * MathCG::DEG2RAD;
            LedPoint lp;
            lp.pos    = MathCG::Vec3(p.x + 8.12f * std::cos(rad), 1.54f, p.z - 8.12f * std::sin(rad));
            lp.radius = 0.55f;
            lp.phase  = static_cast<float>(i) / 16.0f;
            lp.tint   = warmFence;
            leds_.push_back(lp);
        }
        addPool(p.x, p.z, 15.5f, MathCG::ColorRGB(1.00f, 0.84f, 0.50f), 0.0f);
    }

    // 2.4. Golden Sky Tower Lô A1: 4 ống neon dọc cột trên sàn +4.2 m, vành sàn tròn R=6.18 m, cổ búp sen & vũng sáng
    //      (KHÔNG đặt vòng tĩnh ở y = 42m vì Gondola đã có LED gầm di chuyển theo Y)
    void buildSkyTowerLeds() {
        const MathCG::Vec3& p = ParkLayout::SKY_TOWER_POS;
        const MathCG::ColorRGB coolLED(0.42f, 0.80f, 1.00f);
        const MathCG::ColorRGB goldLED(1.00f, 0.78f, 0.30f);

        for (int i = 0; i < 4; ++i) {
            float a = (static_cast<float>(i) / 4.0f) * MathCG::TWO_PI + MathCG::PI / 4.0f;
            addVerticalStrip(p.x + 1.72f * std::cos(a), p.z + 1.72f * std::sin(a),
                             4.35f, ParkLayout::SKY_TOWER_SHAFT_HEIGHT - 0.15f,
                             coolLED, 0.3f * static_cast<float>(i));
        }

        // Bám mép gờ vàng sàn tròn +4.20 m (R = 6.18 m, chừa lối cầu thang, chòi Đông & buồng ĐK Nam)
        for (int i = 0; i < 24; ++i) {
            float deg = static_cast<float>(i) * 15.0f;
            if (deg >= 325.0f || deg <= 40.0f) continue; // cầu thang & chòi Đông
            if (deg >= 82.0f  && deg <= 128.0f) continue; // buồng điều khiển Nam (-1.25, +5.20)
            float a = deg * MathCG::DEG2RAD;
            LedPoint lp;
            lp.pos    = MathCG::Vec3(p.x + 6.18f * std::cos(a), ParkLayout::SKY_TOWER_BOARDING_DECK_Y - 0.16f, p.z + 6.18f * std::sin(a));
            lp.radius = 0.65f;
            lp.phase  = 0.1f + deg / 360.0f;
            lp.tint   = goldLED;
            leds_.push_back(lp);
        }

        addRing(p.x, p.z, 2.05f, ParkLayout::SKY_TOWER_SHAFT_HEIGHT + 0.10f, 10, goldLED, 0.62f, 0.25f);
        addPool(p.x, p.z, 14.0f, MathCG::ColorRGB(0.60f, 0.82f, 1.00f), 0.1f);
    }

    // 2.5. 6 chòi mái ngói + Thủy đình
    void buildPavilionLeds() {
        const MathCG::ColorRGB warmGold(1.00f, 0.82f, 0.42f);
        struct Pav { float x, z, hw, hd, y; };
        const Pav pavs[6] = {
            { ParkLayout::PLAZA_PAVILION_WEST_POS.x,          ParkLayout::PLAZA_PAVILION_WEST_POS.z,          8.4f, 5.1f, 3.90f },
            { ParkLayout::PLAZA_PAVILION_EAST_POS.x,          ParkLayout::PLAZA_PAVILION_EAST_POS.z,          8.4f, 5.1f, 3.90f },
            { ParkLayout::SUN_WHEEL_SIDE_PAVILION_WEST_POS.x, ParkLayout::SUN_WHEEL_SIDE_PAVILION_WEST_POS.z, 6.6f, 6.6f, 4.10f },
            { ParkLayout::SUN_WHEEL_SIDE_PAVILION_EAST_POS.x, ParkLayout::SUN_WHEEL_SIDE_PAVILION_EAST_POS.z, 6.6f, 6.6f, 4.10f },
            { ParkLayout::LAKE_SHORE_PAVILION_WEST_POS.x,     ParkLayout::LAKE_SHORE_PAVILION_WEST_POS.z,     5.6f, 4.6f, 3.70f },
            { ParkLayout::LAKE_SHORE_PAVILION_EAST_POS.x,     ParkLayout::LAKE_SHORE_PAVILION_EAST_POS.z,     5.6f, 4.6f, 3.70f }
        };
        for (int i = 0; i < 6; ++i) {
            addRectOutline(pavs[i].x, pavs[i].z, pavs[i].hw, pavs[i].hd,
                           pavs[i].y, 0.0f, 2.0f, warmGold, 0.68f, 0.09f * static_cast<float>(i));
        }

        addRectOutline(-5.4f, -107.7f, 6.2f, 6.2f, 4.55f, 0.0f, 2.0f,
                       MathCG::ColorRGB(1.00f, 0.74f, 0.34f), 0.72f, 0.5f);
    }

    // 2.6. Khu Tây: quầy mái sọc, 4 đu quay tròn, sân chơi, xe va chạm, thuyền cướp biển, Đền Angkor Wat Lô B1
    void buildWestSideLeds() {
        const MathCG::ColorRGB stallCol(1.00f, 0.52f, 0.34f);
        for (int r = 0; r < 3; ++r) {
            float rx = ParkLayout::STRIPED_STALLS_POS.x - 5.2f + r * 5.2f;
            for (int s = 0; s < 6; ++s) {
                float sz = ParkLayout::STRIPED_STALLS_POS.z - 12.2f + s * 4.88f;
                LedPoint lp;
                lp.pos    = MathCG::Vec3(rx, 3.15f, sz);
                lp.radius = 0.70f;
                lp.phase  = 0.05f * static_cast<float>(r * 6 + s);
                lp.tint   = (s % 2 == 0) ? stallCol : MathCG::ColorRGB(1.00f, 0.92f, 0.80f);
                leds_.push_back(lp);
            }
        }

        struct Spin { MathCG::Vec3 pos; float r; float y; MathCG::ColorRGB col; };
        const Spin spins[4] = {
            { ParkLayout::ROUND_SWING_RIDE_1_POS,   9.30f, 5.50f, MathCG::ColorRGB(1.00f, 0.62f, 0.24f) },
            { ParkLayout::ROUND_SPIN_RIDE_2_POS,    7.20f, 4.30f, MathCG::ColorRGB(0.36f, 0.72f, 1.00f) },
            { ParkLayout::CENTRAL_ROTUNDA_RIDE_POS, 9.80f, 5.90f, MathCG::ColorRGB(1.00f, 0.42f, 0.30f) },
            { ParkLayout::ROUND_SPIN_RIDE_3_POS,    8.20f, 5.00f, MathCG::ColorRGB(0.34f, 0.88f, 0.92f) }
        };
        for (int i = 0; i < 4; ++i) {
            addRing(spins[i].pos.x, spins[i].pos.z, spins[i].r, spins[i].y, 20,
                    spins[i].col, 0.66f, 0.07f * static_cast<float>(i));
            addPool(spins[i].pos.x, spins[i].pos.z, spins[i].r * 1.55f, spins[i].col,
                    0.07f * static_cast<float>(i));
        }

        const MathCG::Vec3& pg = ParkLayout::PLAYGROUND_TARGET_POS;
        addPool(pg.x, pg.z, 13.0f, MathCG::ColorRGB(1.00f, 0.72f, 0.42f), 0.0f);
        for (int i = 0; i <= 8; ++i) {
            float u = static_cast<float>(i) / 8.0f;
            LedPoint lp;
            lp.pos    = MathCG::Vec3(pg.x - 3.5f + 7.0f * u, 5.05f + 0.55f * std::sin(MathCG::PI * u), pg.z);
            lp.radius = 0.60f;
            lp.phase  = u * 0.8f;
            MathCG::HSVtoRGB(std::fmod(u * 320.0f, 360.0f), 0.80f, 1.0f, lp.tint.r, lp.tint.g, lp.tint.b);
            leds_.push_back(lp);
        }
        addRing(pg.x + 10.0f, pg.z, 1.15f, 4.20f, 8, MathCG::ColorRGB(1.00f, 0.72f, 0.24f), 0.58f, 0.3f);

        const MathCG::Vec3& bc = ParkLayout::BUMPER_CARS_TARGET_POS;
        addRing(bc.x, bc.z, 11.0f, 3.30f, 24, MathCG::ColorRGB(0.50f, 0.86f, 1.00f), 0.68f, 0.2f);
        addPool(bc.x, bc.z, 16.0f, MathCG::ColorRGB(0.46f, 0.78f, 1.00f), 0.2f);

        const MathCG::Vec3& ps = ParkLayout::PIRATE_SHIP_TARGET_POS;
        for (int s : {-1, 1}) {
            addRing(ps.x + s * 7.0f, ps.z, 1.05f, 7.20f, 8,
                    MathCG::ColorRGB(1.00f, 0.66f, 0.28f), 0.60f, 0.4f + 0.2f * static_cast<float>(s));
        }

        // Quần thể Đền Angkor Wat Lô B1: viền nền sân sao + viền mái hành lang + đỉnh 5 tháp Prang thật (±5.75m -> 11.78m, tâm -> 15.08m)
        const MathCG::Vec3& ak = ParkLayout::ANGKOR_STAR_PLAZA_POS;
        const MathCG::ColorRGB akGold(1.00f, 0.74f, 0.30f);
        addRectOutline(ak.x, ak.z, ParkLayout::ANGKOR_STAR_PLAZA_HALF - 0.4f, ParkLayout::ANGKOR_STAR_PLAZA_HALF - 0.4f,
                       0.18f, 0.0f, 3.2f, akGold, 0.58f, 0.0f);
        addRectOutline(ak.x, ak.z, 7.00f, 7.00f, ParkLayout::ANGKOR_GALLERY_EAVE_Y + 0.05f,
                       0.0f, 2.2f, akGold, 0.62f, 0.2f);
        addPool(ak.x, ak.z, 16.0f, MathCG::ColorRGB(1.00f, 0.66f, 0.28f), 0.0f);
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                LedPoint lp;
                lp.pos    = MathCG::Vec3(ak.x + sx * 5.75f,
                                         ParkLayout::ANGKOR_CORNER_PRANG_BASE_Y + ParkLayout::ANGKOR_CORNER_PRANG_HEIGHT + 0.08f,
                                         ak.z + sz * 5.75f);
                lp.radius = 0.88f;
                lp.phase  = 0.1f * static_cast<float>(sx + sz + 2);
                lp.tint   = MathCG::ColorRGB(1.00f, 0.82f, 0.40f);
                leds_.push_back(lp);
            }
        }
        LedPoint apex;
        apex.pos    = MathCG::Vec3(ak.x,
                                   ParkLayout::ANGKOR_CENTER_PRANG_BASE_Y + ParkLayout::ANGKOR_CENTER_PRANG_HEIGHT + 0.08f,
                                   ak.z);
        apex.radius = 1.18f;
        apex.phase  = 0.55f;
        apex.tint   = MathCG::ColorRGB(1.00f, 0.92f, 0.58f);
        leds_.push_back(apex);
    }
};

} // namespace AsiaParkRides

#endif // NIGHT_ILLUMINATION_H
