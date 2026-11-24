#ifndef NIGHT_ILLUMINATION_H
#define NIGHT_ILLUMINATION_H

/**
 * @file NightIllumination.h
 * @brief Đợt 9: Hệ thống Chiếu sáng Ban đêm cho Công viên Châu Á (Đà Nẵng).
 *
 * Thành phần:
 *  1. Trụ đèn đôi cao 6.0 m (cột + tay đòn + 2 bóng cầu) sinh THỦ TỤC dọc toàn bộ 7 tuyến
 *     lối đi trong ParkLayout.h, bước 30 m, đảo bên trái/phải; tự động loại bỏ vị trí
 *     nằm trong khối công trình (bảng chặn hình chữ nhật/bán kính) hoặc trên mặt hồ
 *     (kiểm tra ParkLayout::isPointInLake - thuật toán Ray-Casting).
 *  2. Dây đèn festoon nối hai trụ liền kề cùng tuyến: dây võng parabol, 14 bóng nhỏ.
 *  3. Dải LED điểm (bulb) viền diềm mái / thân tháp của: Cổng Ngọ Môn, Tháp Đồng Hồ,
 *     Đu quay ngựa gỗ, Golden Sky Tower, 6 chòi mái ngói, Quảng trường Ngôi sao Angkor,
 *     Dãy quầy mái sọc, 4 đu quay tròn, Sân chơi, Xe va chạm, Thuyền cướp biển,
 *     đường ray 4 tàu lượn nhỏ (MiniCoaster) và 8 công trình Đợt 8c (nếu đang bật [K]).
 *  4. Quầng sáng billboard cộng màu (GL_SRC_ALPHA, GL_ONE) + hiệu ứng chạy đuổi / nhấp nháy
 *     HSV theo thời gian, tắt hoàn toàn vào ban ngày.
 *  5. 4 nguồn sáng điểm bổ sung GL_LIGHT3..6 với suy giảm bậc hai 1/D^2 (Chương 6) để
 *     nền đất và quảng trường không còn tối đen; bật/tắt bằng phím [G].
 *
 * Ràng buộc kỹ thuật (theo yêu cầu đồ án):
 *  - KHÔNG dùng glutSolid*, KHÔNG dùng texture: mọi hình học sinh bằng đa giác / mặt tham số.
 *  - Phần hình học tĩnh (cột, tay đòn, chụp đèn) gói trong 1 OpenGL Display List.
 *  - Phần phát sáng vẽ động mỗi khung hình nhưng chỉ ~8 tam giác/bóng để giữ FPS.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"
#include "curves/BezierTrack.h"
#include "geometry/ParametricShapes.h"
#include "geometry/MeshModel.h"
#include "geometry/MeshBuilders.h"

namespace AsiaParkRides {

class NightIllumination {
public:
    bool  enabled      = true;   // Phím [G] / menu chuột phải
    float animTime     = 0.0f;   // Đồng hồ hiệu ứng chạy đuổi
    float poleHeight   = 6.0f;
    float poleSpacing  = 30.0f;

    /** Một điểm LED phát sáng (bóng đèn / bulb / điểm viền mái) */
    struct LedPoint {
        MathCG::Vec3  pos;
        float         radius;   // bán kính quầng sáng (m)
        float         phase;    // pha hiệu ứng chạy đuổi
        MathCG::ColorRGB tint;  // màu gốc
    };

    /** Một đoạn ống neon thẳng (dải LED dọc thân tháp, cột) */
    struct LightStrip {
        MathCG::Vec3  a;
        MathCG::Vec3  b;
        MathCG::ColorRGB tint;
        float         phase;
    };

    /** Vũng sáng hắt xuống mặt đất dưới chân trụ đèn (cộng màu, không dùng GL_LIGHTi) */
    struct GroundPool {
        MathCG::Vec3  pos;
        float         radius;
        MathCG::ColorRGB tint;
        float         phase;
    };

    NightIllumination() {
        buildFixtures();
    }

    void update(float dt) {
        animTime = std::fmod(animTime + dt, 600.0f);
    }

    /**
     * @brief Bổ sung dải LED dọc đường ray của một tàu lượn nhỏ / monorail (gọi sau khi init track)
     */
    void addTrackLeds(const BezierTrack& track, float stepMeters = 26.0f) {
        if (track.samples.size() < 4 || track.getTotalLength() <= 1.0f) return;

        int step = static_cast<int>(std::lround(track.samples.size() * stepMeters / track.getTotalLength()));
        if (step < 2) step = 2;

        int k = 0;
        for (size_t i = 0; i < track.samples.size(); i += static_cast<size_t>(step)) {
            const MathCG::Vec3& p = track.samples[i].position;
            LedPoint lp;
            lp.pos    = MathCG::Vec3(p.x, p.y + 0.95f, p.z);
            lp.radius = 0.85f;
            lp.phase  = static_cast<float>(k++) * 0.11f;
            MathCG::HSVtoRGB(std::fmod(190.0f + static_cast<float>(k) * 23.0f, 360.0f), 0.85f, 1.0f,
                             lp.tint.r, lp.tint.g, lp.tint.b);
            leds_.push_back(lp);
        }
        fixturesDirty_ = true;
    }

    /**
     * @brief Cấu hình 4 nguồn sáng điểm bổ sung (GL_LIGHT3..6) cho mặt đất / quảng trường.
     *        Chỉ bật khi Hoàng hôn / Ban đêm. Nguồn sáng điểm có suy giảm bậc hai:
     *        I = I_s * k_d * max(0, cos(theta)) / (c0 + c1*D + c2*D^2)
     */
    void applyNightLights(const TimeOfDay& tod, bool eastEnabled) const {
        if (!enabled || tod == TimeOfDay::DAY || !lightingOn_) {
            for (int i = 3; i <= 6; ++i) glDisable(GL_LIGHT0 + i);
            return;
        }

        const float k = (tod == TimeOfDay::NIGHT) ? 1.0f : 0.55f;

        struct PointLightSpec {
            float x, y, z;
            float r, g, b;
            float intensity;
            float quadAtten;
        };
        const PointLightSpec specs[4] = {
            {   0.0f,  9.0f,   92.0f, 1.00f, 0.86f, 0.60f, 0.95f, 0.00055f }, // Quảng trường Trung tâm - Cổng Ngọ Môn
            { -196.0f,  9.0f,   34.0f, 1.00f, 0.82f, 0.52f, 0.90f, 0.00050f }, // Ngã ba trục trò chơi khu Tây
            { -215.0f,  9.0f, -150.0f, 0.72f, 0.86f, 1.00f, 0.80f, 0.00048f }, // Khu Angkor / Paradise Fall ven hồ
            {  118.0f,  9.0f,  -55.0f, 0.78f, 0.90f, 1.00f, eastEnabled ? 0.85f : 0.0f, 0.00050f } // Nửa Đông (Đợt 8c)
        };

        for (int i = 0; i < 4; ++i) {
            const int id = GL_LIGHT3 + i;
            const PointLightSpec& s = specs[i];
            if (s.intensity <= 0.001f) {
                glDisable(static_cast<GLenum>(id));
                continue;
            }
            GLfloat pos[4]  = { s.x, s.y, s.z, 1.0f };
            GLfloat diff[4] = { s.r * s.intensity * k, s.g * s.intensity * k, s.b * s.intensity * k, 1.0f };
            GLfloat spec[4] = { diff[0] * 0.45f, diff[1] * 0.45f, diff[2] * 0.45f, 1.0f };
            GLfloat amb[4]  = { diff[0] * 0.20f, diff[1] * 0.20f, diff[2] * 0.20f, 1.0f };

            glEnable(static_cast<GLenum>(id));
            glLightfv(static_cast<GLenum>(id), GL_POSITION, pos);
            glLightfv(static_cast<GLenum>(id), GL_AMBIENT,  amb);
            glLightfv(static_cast<GLenum>(id), GL_DIFFUSE,  diff);
            glLightfv(static_cast<GLenum>(id), GL_SPECULAR, spec);
            glLightf (static_cast<GLenum>(id), GL_CONSTANT_ATTENUATION,  0.75f);
            glLightf (static_cast<GLenum>(id), GL_LINEAR_ATTENUATION,    0.0035f);
            glLightf (static_cast<GLenum>(id), GL_QUADRATIC_ATTENUATION, s.quadAtten);
        }
    }

    /**
     * @brief Vẽ phần hình học đặc của trụ đèn (cột, tay đòn, chụp) - luôn hiển thị cả ngày lẫn đêm
     */
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

    /**
     * @brief Vẽ toàn bộ điểm sáng + quầng sáng cộng màu (chỉ ban đêm / hoàng hôn).
     *
     * Tối ưu hiệu năng: dựng toàn bộ billboard trong MỘT khối glBegin(GL_TRIANGLES) duy nhất
     * (thay vì 2 khối GL_TRIANGLE_FAN cho mỗi bóng), và loại bỏ bóng ở xa hơn ngưỡng
     * cullDistance_ tính từ vị trí camera (giải mã ngược ma trận ModelView).
     * Phép trộn cộng (GL_SRC_ALPHA, GL_ONE) nên thứ tự vẽ không ảnh hưởng kết quả.
     */
    void drawGlow(bool isNightOrSunset, float cullDistance = 1400.0f, float haloDistance = 170.0f) const {
        if (!enabled || !isNightOrSunset) return;
        if (leds_.empty() && strips_.empty()) return;

        // 1. Trích 2 trục phải/lên của hệ tọa độ quan sát + vị trí camera thế giới
        GLfloat m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        MathCG::Vec3 camRight(m[0], m[4], m[8]);
        MathCG::Vec3 camUp(m[1], m[5], m[9]);
        if (camRight.length() < 1e-6f) camRight = MathCG::Vec3(1.0f, 0.0f, 0.0f);
        if (camUp.length() < 1e-6f)    camUp    = MathCG::Vec3(0.0f, 1.0f, 0.0f);
        camRight = camRight.normalized();
        camUp    = camUp.normalized();

        // Camera world position = -R^T * t với R là 3x3 quay trong ma trận view
        const float camX = -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]);
        const float camY = -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]);
        const float camZ = -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]);
        const float cull2 = cullDistance * cullDistance;
        const float halo2 = haloDistance * haloDistance;

        GLboolean wasLit  = glIsEnabled(GL_LIGHTING);
        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        // 2. Dải ống neon dọc thân tháp (GL_LINES, hiệu ứng sóng chạy)
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

        // 3. Bóng đèn + quầng sáng: 1 khối tam giác duy nhất, 6 tam giác quầng + 6 tam giác lõi
        glBegin(GL_TRIANGLES);
        for (const auto& led : leds_) {
            const float dx = led.pos.x - camX;
            const float dy = led.pos.y - camY;
            const float dz = led.pos.z - camZ;
            const float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 > cull2) continue;

            const float pulse = 0.62f + 0.38f * std::sin(animTime * 2.4f - led.phase * 6.2831853f);
            const float r = led.tint.r, g = led.tint.g, b = led.tint.b;

            // 3.1. Quầng sáng ngoài — CHỈ vẽ khi bóng ở gần (d2 <= halo2).
            //      Ở khoảng cách xa, quầng sáng chỉ vài điểm ảnh nên bỏ qua để tiết kiệm 50% tam giác.
            const float rOut = led.radius;
            if (d2 <= halo2) {
            glColor4f(r * pulse, g * pulse, b * pulse, 0.30f);
            for (int i = 0; i < 6; ++i) {
                const float a0 = (static_cast<float>(i)     / 6.0f) * MathCG::TWO_PI;
                const float a1 = (static_cast<float>(i + 1) / 6.0f) * MathCG::TWO_PI;
                glVertex3f(led.pos.x, led.pos.y, led.pos.z);
                glColor4f(r, g, b, 0.0f);
                glVertex3f(led.pos.x + (camRight.x * std::cos(a0) + camUp.x * std::sin(a0)) * rOut,
                           led.pos.y + (camRight.y * std::cos(a0) + camUp.y * std::sin(a0)) * rOut,
                           led.pos.z + (camRight.z * std::cos(a0) + camUp.z * std::sin(a0)) * rOut);
                glVertex3f(led.pos.x + (camRight.x * std::cos(a1) + camUp.x * std::sin(a1)) * rOut,
                           led.pos.y + (camRight.y * std::cos(a1) + camUp.y * std::sin(a1)) * rOut,
                           led.pos.z + (camRight.z * std::cos(a1) + camUp.z * std::sin(a1)) * rOut);
                glColor4f(r * pulse, g * pulse, b * pulse, 0.30f);
            }
            }

            // 3.2. Lõi sáng đặc (hơi ngả trắng để bóng đèn có cảm giác cháy sáng)
            const float rCore = led.radius * 0.34f;
            glColor4f(std::min(1.0f, r * pulse + 0.35f),
                      std::min(1.0f, g * pulse + 0.35f),
                      std::min(1.0f, b * pulse + 0.30f), 0.92f);
            for (int i = 0; i < 6; ++i) {
                const float a0 = (static_cast<float>(i)     / 6.0f) * MathCG::TWO_PI;
                const float a1 = (static_cast<float>(i + 1) / 6.0f) * MathCG::TWO_PI;
                glVertex3f(led.pos.x, led.pos.y, led.pos.z);
                glVertex3f(led.pos.x + (camRight.x * std::cos(a0) + camUp.x * std::sin(a0)) * rCore,
                           led.pos.y + (camRight.y * std::cos(a0) + camUp.y * std::sin(a0)) * rCore,
                           led.pos.z + (camRight.z * std::cos(a0) + camUp.z * std::sin(a0)) * rCore);
                glVertex3f(led.pos.x + (camRight.x * std::cos(a1) + camUp.x * std::sin(a1)) * rCore,
                           led.pos.y + (camRight.y * std::cos(a1) + camUp.y * std::sin(a1)) * rCore,
                           led.pos.z + (camRight.z * std::cos(a1) + camUp.z * std::sin(a1)) * rCore);
            }
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        if (wasCull) glEnable(GL_CULL_FACE);
        if (wasLit)  glEnable(GL_LIGHTING);
    }

    /**
     * @brief Vũng sáng hắt xuống nền đất / mặt lối đi dưới chân trụ đèn.
     *        Gọi sau khi vẽ nền đất & lối đi, trước lớp kiến trúc.
     */
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

        // Gộp mọi vũng sáng vào MỘT khối tam giác: 12 tam giác quầng + 12 tam giác lõi mỗi vũng
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

            // Lõi sáng đậm sát chân trụ
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

private:
    mutable Geometry::PolygonMesh poleMesh_;
    mutable GLuint fixturesList_ = 0;
    std::vector<LedPoint>   leds_;
    std::vector<LightStrip> strips_;
    std::vector<GroundPool> pools_;
    bool fixturesDirty_ = true;
    bool lightingOn_    = true;

public:
    void setLightingEnabled(bool on) { lightingOn_ = on; }

private:
    // ========================================================================
    // BẢNG CHẶN VỊ TRÍ TRỤ ĐÈN (không đặt trụ xuyên vào công trình)
    // ========================================================================
    struct RectBlock { float minX, maxX, minZ, maxZ; };
    struct CircleBlock { float x, z, r; };

    static bool blockedByStructure(float x, float z) {
        static const RectBlock rects[] = {
            {  -27.0f,   59.0f,  130.0f, 150.0f }, // Cổng Ngọ Môn + 2 cánh tường thành
            {  -32.0f,   -4.0f,  147.0f, 163.0f }, // Hồ sen Tây
            {   36.0f,   64.0f,  147.0f, 163.0f }, // Hồ sen Đông
            {    2.0f,   30.0f,   53.5f,  69.5f }, // Tháp Đồng Hồ
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
            { -171.0f,  -91.0f, -269.0f, -239.0f }, // Quảng trường Ngôi sao Angkor
            {  -62.0f,  -18.0f,  -34.0f,  -16.0f }  // Sân chơi trẻ em
        };
        for (const auto& r : rects) {
            if (x >= r.minX && x <= r.maxX && z >= r.minZ && z <= r.maxZ) return true;
        }

        static const CircleBlock circles[] = {
            { -159.4f,  102.5f, 12.0f }, // Carousel
            { -260.0f,  -54.6f, 10.5f }, // Sky Tower
            { -132.7f,   12.9f, 10.5f }, // Đu quay tròn #9
            {  -94.2f,   17.3f,  8.5f }, // Đu quay tròn #10
            { -141.3f, -137.6f, 11.0f }, // Công trình tròn #13
            { -143.4f, -179.6f,  9.5f }, // Đu quay tròn #14
            { -206.5f,  108.1f, 12.0f }, // Xe va chạm
            { -172.0f,   -2.0f, 10.0f }, // Thuyền cướp biển
            { -124.1f,  155.7f, 20.0f }, // Garuda Valley
            { -224.0f,   22.0f, 22.0f }, // Port of Sky Treasure
            { -179.8f,  -91.1f, 14.0f }, // Tòa nhà đá Paradise Fall
            { -199.0f, -119.9f, 19.0f }, // Vòng ray số 8 Paradise Fall
            { -176.0f, -214.0f, 19.0f }  // Highway Boat
        };
        for (const auto& c : circles) {
            float dx = x - c.x, dz = z - c.z;
            if (dx * dx + dz * dz <= c.r * c.r) return true;
        }
        return false;
    }

    static bool blockedByEastMasterPlan(float x, float z) {
        static const CircleBlock east[] = {
            {  18.0f, -152.0f, 16.0f }, { 102.0f,  -45.0f, 17.0f },
            { 135.0f,   12.0f, 16.0f }, { 148.0f,  -86.0f, 17.0f },
            { 226.0f,  -35.0f, 16.0f }, { 268.0f,  -72.0f, 17.0f },
            {  84.0f, -238.0f, 24.0f }, {  66.0f, -118.0f, 14.0f }
        };
        for (const auto& c : east) {
            float dx = x - c.x, dz = z - c.z;
            if (dx * dx + dz * dz <= c.r * c.r) return true;
        }
        return false;
    }

    // ========================================================================
    // SINH HÌNH HỌC
    // ========================================================================
    void buildFixtures() {
        const MathCG::ColorRGB ironCol(0.26f, 0.27f, 0.31f);
        const MathCG::ColorRGB goldCol(0.86f, 0.70f, 0.26f);

        // ------------------------------------------------------------------
        // 1. Trụ đèn đôi dọc 7 tuyến lối đi
        // ------------------------------------------------------------------
        addPolesAlongPath(ParkLayout::PATH_MAIN_SPINE,          ParkLayout::NUM_PATH_MAIN_SPINE_PTS,          ParkLayout::PATH_MAIN_SPINE_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_SOUTH_LAKE_PROM,     ParkLayout::NUM_PATH_SOUTH_LAKE_PROM_PTS,     ParkLayout::PATH_SOUTH_LAKE_PROM_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_WEST_SPINE,          ParkLayout::NUM_PATH_WEST_SPINE_PTS,          ParkLayout::PATH_WEST_SPINE_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_LAKESIDE_WEST,       ParkLayout::NUM_PATH_LAKESIDE_WEST_PTS,       ParkLayout::PATH_LAKESIDE_WEST_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_CENTRAL_CONNECTOR,   ParkLayout::NUM_PATH_CENTRAL_CONNECTOR_PTS,   ParkLayout::PATH_CENTRAL_CONNECTOR_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_EAST_MASTERPLAN_MAIN, ParkLayout::NUM_PATH_EAST_MAIN_PTS,          ParkLayout::PATH_EAST_MASTERPLAN_WIDTH);
        addPolesAlongPath(ParkLayout::PATH_EAST_MASTERPLAN_NORTH, ParkLayout::NUM_PATH_EAST_NORTH_SPUR_PTS,   ParkLayout::PATH_EAST_MASTERPLAN_WIDTH);

        // ------------------------------------------------------------------
        // 2. LED viền diềm mái các công trình biểu tượng
        // ------------------------------------------------------------------
        buildGateLeds();
        buildClockTowerLeds();
        buildCarouselLeds();
        buildSkyTowerLeds();
        buildPavilionLeds();
        buildWestSideLeds();
        buildEastMasterPlanLeds();

        // ------------------------------------------------------------------
        // 3. Đóng gói lưới đa giác trụ đèn
        // ------------------------------------------------------------------
        poleMesh_.computeNormals();
        fixturesDirty_ = false;

        (void)ironCol;
        (void)goldCol;
    }

    void addPole(float x, float z, float phase) {
        namespace MB = Geometry::MeshBuilders;
        const MathCG::ColorRGB ironCol(0.26f, 0.27f, 0.31f);
        const MathCG::ColorRGB goldCol(0.86f, 0.70f, 0.26f);
        const float H = poleHeight;

        // 1. Bệ đế + thân cột + cổ cột
        MB::appendBoxByYRange(poleMesh_, x, 0.0f, 0.42f, z, 0.62f, 0.62f, ironCol);
        poleMesh_.append(Geometry::PolygonMesh::createPrismY(x, 0.36f, z, 0.145f, H - 0.30f, 8, ironCol));
        poleMesh_.append(Geometry::PolygonMesh::createPrismY(x, H - 0.10f, z, 0.26f, 0.18f, 8, goldCol));

        // 2. Tay đòn ngang treo 2 bóng cầu
        MB::appendBoxByYRange(poleMesh_, x, H - 0.28f, H - 0.14f, z, 2.20f, 0.14f, goldCol);
        // 3. Chụp đèn hình nón + chóp trang trí trên đỉnh cột
        for (int s : {-1, 1}) {
            poleMesh_.append(Geometry::PolygonMesh::createPyramid(
                x + s * 1.00f, H - 0.02f, z, 0.92f, 0.92f, 0.34f, ironCol));
        }
        poleMesh_.append(Geometry::PolygonMesh::createPyramid(x, H + 0.06f, z, 0.46f, 0.46f, 0.42f, goldCol));

        // 4. Hai bóng đèn phát sáng (điểm LED)
        const float ph[2] = { phase, phase + 0.35f };
        for (int s : {-1, 1}) {
            LedPoint lp;
            lp.pos    = MathCG::Vec3(x + s * 1.00f, H - 0.12f, z);
            lp.radius = 1.05f;
            lp.phase  = ph[s > 0 ? 1 : 0];
            lp.tint   = MathCG::ColorRGB(1.00f, 0.88f, 0.58f);
            leds_.push_back(lp);
        }

        // 5. Vũng sáng hắt xuống mặt lối đi (y = 0.055 m, trên mặt đường 0.03 m)
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

        // Danh sách trụ đã đặt trên tuyến này để nối dây đèn festoon
        std::vector<MathCG::Vec3> chain;
        chain.reserve(static_cast<size_t>(count) * 4);

        for (int i = 0; i < count - 1; ++i) {
            const float x0 = pts[i].x,     z0 = pts[i].z;
            const float x1 = pts[i + 1].x, z1 = pts[i + 1].z;
            const float dx = x1 - x0, dz = z1 - z0;
            const float len = std::sqrt(dx * dx + dz * dz);
            if (len < 1e-3f) continue;

            const float ux = dx / len, uz = dz / len;
            const float nx = -uz,     nz =  ux;  // pháp tuyến ngang của tuyến

            for (float t = 0.0f; t < len; t += poleSpacing) {
                float px = x0 + ux * t + nx * sideOffset * static_cast<float>(sideToggle);
                float pz = z0 + uz * t + nz * sideOffset * static_cast<float>(sideToggle);

                if (!ParkLayout::isPointInLake(px, pz) && !blockedByStructure(px, pz) &&
                    !blockedByEastMasterPlan(px, pz)) {
                    addPole(px, pz, phaseCounter);
                    chain.push_back(MathCG::Vec3(px, poleHeight - 0.12f, pz));
                    phaseCounter += 0.21f;
                }
                sideToggle = -sideToggle;
            }
        }

        // Dây đèn festoon nối hai trụ liền kề (bỏ qua khoảng cách quá xa)
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

        // Dải LED âm sàn chạy liên tục dọc 2 mép lối đi (làm sáng nền đường ban đêm)
        addPathEdgeStrips(pts, count, pathWidth);
    }

    /**
     * @brief Đặt dải LED âm sàn + vũng sáng liên tục dọc hai mép lối đi.
     *        Mật độ 4.5 m/bóng, bán kính quầng 1.5 m, kèm vũng sáng 3.4 m để mặt đường không tối.
     */
    void addPathEdgeStrips(const ParkLayout::PointXZ* pts, int count, float pathWidth) {
        const float edgeOffset = pathWidth * 0.5f - 0.35f;   // nép sát mép trong của lối đi
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

                    // Vũng sáng chỉ đặt cách quãng (giảm overdraw) với bán kính nhỏ hơn
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

    // --- Bộ sinh LED viền mái hình chữ nhật (có thể xoay quanh Y) ---
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

        // 4 cạnh của hình chữ nhật
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

    // --- Bộ sinh LED viền vòng tròn ---
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

    // --- Bộ sinh ống neon dọc thân tháp ---
    void addVerticalStrip(float x, float z, float y0, float y1,
                          const MathCG::ColorRGB& tint, float phase) {
        LightStrip st;
        st.a     = MathCG::Vec3(x, y0, z);
        st.b     = MathCG::Vec3(x, y1, z);
        st.tint  = tint;
        st.phase = phase;
        strips_.push_back(st);
    }

    // --- Bộ sinh vũng sáng hắt xuống nền ---
    void addPool(float x, float z, float radius, const MathCG::ColorRGB& tint, float phase) {
        GroundPool gp;
        gp.pos    = MathCG::Vec3(x, 0.055f, z);
        gp.radius = radius;
        gp.tint   = tint;
        gp.phase  = phase;
        pools_.push_back(gp);
    }

    // ------------------------------------------------------------------------
    // 2.1. Cổng Ngọ Môn (scale 0.76): viền diềm mái tầng 2 + đài thành + 2 cánh tường
    // ------------------------------------------------------------------------
    void buildGateLeds() {
        const float s  = ParkLayout::NGO_MON_GATE_SCALE;      // 0.76
        const float gx = ParkLayout::NGO_MON_GATE_POS.x;      // 16.1
        const float gz = ParkLayout::NGO_MON_GATE_POS.z;      // 139.1
        const MathCG::ColorRGB warmGold(1.00f, 0.76f, 0.28f);
        const MathCG::ColorRGB warmRed (1.00f, 0.30f, 0.18f);

        // Đài thành tầng nền (cao 6.8 m cục bộ) và Lầu Ngũ Phụng (diềm mái 16.50 m cục bộ)
        addRectOutline(gx, gz, 23.0f * s, 7.6f * s, 6.80f * s, 0.0f, 1.9f, warmGold, 0.80f, 0.00f);
        addRectOutline(gx, gz, 12.5f * s, 4.8f * s, 16.50f * s, 0.0f, 1.7f, warmRed,  0.78f, 0.30f);
        addRectOutline(gx, gz, 19.8f * s, 5.8f * s, 12.60f * s, 0.0f, 1.8f, warmGold, 0.72f, 0.15f);

        // Hai cánh tường thành có răng cưa: viền đỉnh tường (cao 6.4 m cục bộ)
        for (int side : {-1, 1}) {
            float wx = gx + side * (23.0f + ParkLayout::NGO_MON_WING_SPAN_LOCAL * 0.5f) * s;
            addRectOutline(wx, gz, ParkLayout::NGO_MON_WING_SPAN_LOCAL * 0.5f * s,
                           ParkLayout::NGO_MON_WING_DEPTH_LOCAL * 0.5f * s,
                           ParkLayout::NGO_MON_WING_HEIGHT_LOCAL * s, 0.0f, 2.1f, warmGold, 0.62f, 0.4f);
            addPool(wx - 12.0f, gz - 9.0f, 10.0f, MathCG::ColorRGB(1.00f, 0.72f, 0.34f), 0.4f);
            addPool(wx + 12.0f, gz - 9.0f, 10.0f, MathCG::ColorRGB(1.00f, 0.72f, 0.34f), 0.6f);
        }

        // Vũng sáng lớn trước cổng & sân trước Ngọ Môn
        addPool(gx, gz - 12.0f, 26.0f, MathCG::ColorRGB(1.00f, 0.74f, 0.36f), 0.0f);
        addPool(gx, ParkLayout::GATE_FORECOURT_POS.z, 24.0f, MathCG::ColorRGB(1.00f, 0.70f, 0.34f), 0.3f);
    }

    // ------------------------------------------------------------------------
    // 2.2. Tháp Đồng Hồ: 5 vòng bát giác theo 5 tầng + 4 ống neon dọc thân
    // ------------------------------------------------------------------------
    void buildClockTowerLeds() {
        const float cx = ParkLayout::CLOCK_TOWER_POS.x;   // 10.7
        const float cz = ParkLayout::CLOCK_TOWER_POS.z;   // 61.6
        const MathCG::ColorRGB warmGold(1.00f, 0.80f, 0.34f);

        // apothem mặt thân của 5 tầng (theo bảng OctTier trong NgoMonGate.h) + cao độ đỉnh thân
        const float apo[5]   = { 5.90f, 5.20f, 4.70f, 3.70f, 2.20f };
        const float ringY[5] = { 3.30f, 6.90f, 12.30f, 15.10f, 16.60f };
        for (int t = 0; t < 5; ++t) {
            const int n = 8;
            for (int i = 0; i < n; ++i) {
                float a = (static_cast<float>(i) / n) * MathCG::TWO_PI + MathCG::PI / 8.0f;
                LedPoint lp;
                lp.pos    = MathCG::Vec3(cx + apo[t] * std::cos(a), ringY[t], cz + apo[t] * std::sin(a));
                lp.radius = 0.85f;
                lp.phase  = 0.05f * static_cast<float>(t) + static_cast<float>(i) * 0.06f;
                lp.tint   = warmGold;
                leds_.push_back(lp);
            }
        }
        // 4 ống neon dọc 4 mặt tháp (từ chân tới tầng mặt đồng hồ)
        for (int i = 0; i < 4; ++i) {
            float a = (static_cast<float>(i) / 4.0f) * MathCG::TWO_PI + MathCG::PI / 4.0f;
            addVerticalStrip(cx + 4.75f * std::cos(a), cz + 4.75f * std::sin(a),
                             0.9f, 15.0f, MathCG::ColorRGB(1.00f, 0.66f, 0.22f),
                             0.25f * static_cast<float>(i));
        }
        // 4 mặt đồng hồ phát sáng
        addRing(cx, cz, 4.55f, 16.00f, 12, MathCG::ColorRGB(0.92f, 0.96f, 1.00f), 0.70f, 0.5f);
        addPool(cx, cz, 17.0f, MathCG::ColorRGB(1.00f, 0.74f, 0.38f), 0.1f);
    }

    // ------------------------------------------------------------------------
    // 2.3. Đu quay Ngựa gỗ 2 tầng: 2 vòng diềm mái
    // ------------------------------------------------------------------------
    void buildCarouselLeds() {
        const MathCG::Vec3& p = ParkLayout::CAROUSEL_POS_UNVERIFIED;
        const MathCG::ColorRGB warmBulb(1.00f, 0.90f, 0.52f);
        addRing(p.x, p.z, 8.72f, 3.86f, 40, warmBulb, 0.62f, 0.0f);
        addRing(p.x, p.z, 6.62f, 7.22f, 30, warmBulb, 0.60f, 0.35f);
        addPool(p.x, p.z, 15.5f, MathCG::ColorRGB(1.00f, 0.84f, 0.50f), 0.0f);
    }

    // ------------------------------------------------------------------------
    // 2.4. Golden Sky Tower: 4 ống neon dọc cột tháp + vòng sàn đón + vòng Gondola
    // ------------------------------------------------------------------------
    void buildSkyTowerLeds() {
        const MathCG::Vec3& p = ParkLayout::SKY_TOWER_POS;
        const MathCG::ColorRGB coolLED(0.42f, 0.80f, 1.00f);
        const MathCG::ColorRGB goldLED(1.00f, 0.78f, 0.30f);

        for (int i = 0; i < 4; ++i) {
            float a = (static_cast<float>(i) / 4.0f) * MathCG::TWO_PI + MathCG::PI / 4.0f;
            addVerticalStrip(p.x + 1.68f * std::cos(a), p.z + 1.68f * std::sin(a),
                             0.8f, 42.0f, coolLED, 0.3f * static_cast<float>(i));
        }
        addRing(p.x, p.z, 7.05f, 4.24f, 24, goldLED, 0.72f, 0.1f);
        addRing(p.x, p.z, 3.55f, 42.00f, 16, coolLED, 0.68f, 0.6f);
        addPool(p.x, p.z, 14.0f, MathCG::ColorRGB(0.60f, 0.82f, 1.00f), 0.1f);
    }

    // ------------------------------------------------------------------------
    // 2.5. 6 chòi mái ngói (viền diềm mái) + Thủy đình ven hồ
    // ------------------------------------------------------------------------
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

        // Thủy đình giữa hồ (bệ đá + mái cong) - viền mái trên mặt nước
        addRectOutline(-5.4f, -107.7f, 6.2f, 6.2f, 4.55f, 0.0f, 2.0f,
                       MathCG::ColorRGB(1.00f, 0.74f, 0.34f), 0.72f, 0.5f);
    }

    // ------------------------------------------------------------------------
    // 2.6. Khu Tây: quầy mái sọc, 4 đu quay tròn, sân chơi, xe va chạm, thuyền cướp biển, Angkor
    // ------------------------------------------------------------------------
    void buildWestSideLeds() {
        // Dãy 3 hàng quầy mái sọc đỏ-trắng (-50, 12): 3 hàng x 6 quầy
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

        // 4 đu quay tròn: viền mép tán mái nón
        struct Spin { MathCG::Vec3 pos; float r; float y; MathCG::ColorRGB col; };
        const Spin spins[4] = {
            { ParkLayout::ROUND_SWING_RIDE_1_POS,    9.30f, 5.50f, MathCG::ColorRGB(1.00f, 0.62f, 0.24f) },
            { ParkLayout::ROUND_SPIN_RIDE_2_POS,     7.20f, 4.30f, MathCG::ColorRGB(0.36f, 0.72f, 1.00f) },
            { ParkLayout::CENTRAL_ROTUNDA_RIDE_POS,  9.80f, 5.90f, MathCG::ColorRGB(1.00f, 0.42f, 0.30f) },
            { ParkLayout::ROUND_SPIN_RIDE_3_POS,     8.20f, 5.00f, MathCG::ColorRGB(0.34f, 0.88f, 0.92f) }
        };
        for (int i = 0; i < 4; ++i) {
            addRing(spins[i].pos.x, spins[i].pos.z, spins[i].r, spins[i].y, 20,
                    spins[i].col, 0.66f, 0.07f * static_cast<float>(i));
            addPool(spins[i].pos.x, spins[i].pos.z, spins[i].r * 1.55f, spins[i].col,
                    0.07f * static_cast<float>(i));
        }

        // Sân chơi trẻ em: vòm đèn nhiều màu trên khung xích đu + cột cầu trượt
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

        // Sân xe va chạm: vòng đèn quanh hàng rào
        const MathCG::Vec3& bc = ParkLayout::BUMPER_CARS_TARGET_POS;
        addRing(bc.x, bc.z, 11.0f, 3.30f, 24, MathCG::ColorRGB(0.50f, 0.86f, 1.00f), 0.68f, 0.2f);
        addPool(bc.x, bc.z, 16.0f, MathCG::ColorRGB(0.46f, 0.78f, 1.00f), 0.2f);

        // Thuyền cướp biển: 2 vòng đèn trên 2 chân trụ A
        const MathCG::Vec3& ps = ParkLayout::PIRATE_SHIP_TARGET_POS;
        for (int s : {-1, 1}) {
            addRing(ps.x + s * 7.0f, ps.z, 1.05f, 7.20f, 8,
                    MathCG::ColorRGB(1.00f, 0.66f, 0.28f), 0.60f, 0.4f + 0.2f * static_cast<float>(s));
        }

        // Quảng trường Ngôi sao Angkor: viền nền đền + 5 đỉnh tháp Prang phát sáng
        const MathCG::Vec3& ak = ParkLayout::ANGKOR_STAR_PLAZA_POS;
        const MathCG::ColorRGB akGold(1.00f, 0.74f, 0.30f);
        addRectOutline(ak.x, ak.z, 9.6f, 9.6f, 1.52f, 0.0f, 2.0f, akGold, 0.66f, 0.0f);
        addPool(ak.x, ak.z, 15.0f, MathCG::ColorRGB(1.00f, 0.66f, 0.28f), 0.0f);
        for (int sx : {-1, 1}) {
            for (int sz : {-1, 1}) {
                LedPoint lp;
                lp.pos    = MathCG::Vec3(ak.x + sx * 5.6f, 10.85f, ak.z + sz * 5.6f);
                lp.radius = 0.90f;
                lp.phase  = 0.1f * static_cast<float>(sx + sz + 2);
                lp.tint   = MathCG::ColorRGB(1.00f, 0.82f, 0.40f);
                leds_.push_back(lp);
            }
        }
        LedPoint apex;
        apex.pos    = MathCG::Vec3(ak.x, 15.05f, ak.z);
        apex.radius = 1.20f;
        apex.phase  = 0.55f;
        apex.tint   = MathCG::ColorRGB(1.00f, 0.92f, 0.58f);
        leds_.push_back(apex);
    }

    // ------------------------------------------------------------------------
    // 2.7. 8 công trình Quy hoạch Tổng thể Đợt 8c (nửa Đông)
    // ------------------------------------------------------------------------
    void buildEastMasterPlanLeds() {
        const MathCG::ColorRGB warm (1.00f, 0.82f, 0.42f);
        const MathCG::ColorRGB cool (0.46f, 0.84f, 1.00f);
        const MathCG::ColorRGB aqua (0.36f, 1.00f, 0.88f);
        const MathCG::ColorRGB rose (1.00f, 0.48f, 0.56f);

        // 1. Nhà hàng Panda (26 x 18 x 14 m)
        addRectOutline(ParkLayout::W8C_PANDA_RESTAURANT_POS.x, ParkLayout::W8C_PANDA_RESTAURANT_POS.z,
                       13.6f, 9.6f, 9.20f, 0.0f, 2.2f, warm, 0.72f, 0.0f);
        // 2. Hồ phun nước Merlion: vòng quanh bệ hồ + tượng
        addRing(ParkLayout::W8C_MERLION_POOL_POS.x, ParkLayout::W8C_MERLION_POOL_POS.z,
                ParkLayout::W8C_MERLION_POOL_RADIUS + 0.6f, 0.75f, 28, cool, 0.70f, 0.1f);
        // 3. Thuyền Rồng Hoàng Gia (22 x 7 x 11.5 m)
        addRectOutline(ParkLayout::W8C_DRAGON_BOAT_POS.x, ParkLayout::W8C_DRAGON_BOAT_POS.z,
                       11.6f, 4.2f, 4.10f, ParkLayout::W8C_DRAGON_BOAT_ROT_Y_DEG, 1.8f, aqua, 0.62f, 0.2f);
        // 4. The Flying Kirins (18 x 12 x 15 m)
        addRectOutline(ParkLayout::W8C_FLYING_KIRINS_POS.x, ParkLayout::W8C_FLYING_KIRINS_POS.z,
                       9.6f, 6.6f, 3.60f, ParkLayout::W8C_FLYING_KIRINS_ROT_Y_DEG, 1.8f, rose, 0.66f, 0.3f);
        // 5. Sân khấu nổi Symphony of River (42 x 20 x 12 m)
        addRectOutline(ParkLayout::W8C_SYMPHONY_STAGE_POS.x, ParkLayout::W8C_SYMPHONY_STAGE_POS.z,
                       21.6f, 10.6f, 3.40f, ParkLayout::W8C_SYMPHONY_STAGE_ROT_Y_DEG, 2.2f, cool, 0.72f, 0.4f);
        // 6. Nhà hát Âu Cơ (24 x 18 x 12 m)
        addRectOutline(ParkLayout::W8C_AU_CO_THEATER_POS.x, ParkLayout::W8C_AU_CO_THEATER_POS.z,
                       12.6f, 9.6f, 8.40f, ParkLayout::W8C_AU_CO_THEATER_ROT_Y_DEG, 2.2f, warm, 0.70f, 0.15f);
        // 7. Đảo Khủng Long: vòng cảnh quan + 3 đỉnh núi lửa
        addRing(ParkLayout::W8C_DINO_ISLAND_POS.x, ParkLayout::W8C_DINO_ISLAND_POS.z, 13.5f, 0.85f, 24,
                MathCG::ColorRGB(1.00f, 0.44f, 0.22f), 0.68f, 0.6f);
        // 8. Vườn Trúc Nhật Bản: viền cổng Torii + lối đi
        addRectOutline(ParkLayout::W8C_BAMBOO_GARDEN_POS.x, ParkLayout::W8C_BAMBOO_GARDEN_POS.z,
                       12.6f, 10.6f, 3.10f, 0.0f, 2.4f, MathCG::ColorRGB(1.00f, 0.60f, 0.36f), 0.64f, 0.5f);
    }
};

} // namespace AsiaParkRides

#endif // NIGHT_ILLUMINATION_H
