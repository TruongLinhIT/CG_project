#ifndef PARAMETRIC_SHAPES_H
#define PARAMETRIC_SHAPES_H

/**
 * @file ParametricShapes.h
 * @brief Chương 5 & 6: Các mặt cong tham số sinh tự động (Mặt kẻ & Mặt tròn xoay)
 *
 * Triển khai việc sinh tường minh các mặt cong tham số thông qua lưới tham số (u, v):
 *  - Chương 5:
 *      1. Mặt trụ (Mặt kẻ - Ruled Cylinder): X(u,v) = r*cos(u), Y(u,v) = r*sin(u), Z(u,v) = h*v
 *      2. Mặt nón / Nón cụt (Mặt kẻ - Ruled Cone / Frustum): X(u,v) = R(v)*cos(u), Y(u,v) = R(v)*sin(u), Z(u,v) = h*v
 *      3. Mặt tròn xoay - Mặt cầu (Sphere): X(u,v) = r*cos(u)*cos(v), Y(u,v) = r*sin(u)*cos(v), Z(u,v) = r*sin(v)
 *      4. Mặt tròn xoay - Mặt xuyến (Toroid): X(u,v) = (R + a*cos(v))*cos(u), Y(u,v) = (R + a*cos(v))*sin(u), Z(u,v) = a*sin(v)
 *      5. Nắp chắn hai đầu / Đĩa tròn (GL_TRIANGLE_FAN)
 *      6. Hình trụ & Hình nón nối giữa hai điểm A và B bất kỳ trong không gian 3D
 *      7. Mặt xuyến gắn đèn LED HSV phân đoạn cho các trò chơi (Sun Wheel, Sky Tower, Carousel)
 *  - Chương 6:
 *      - Vector pháp tuyến đơn vị giải tích chính xác N(u, v) = (dP/du x dP/dv) / ||dP/du x dP/dv||
 *      - Thứ tự đỉnh dải tứ giác ngược chiều kim đồng hồ (GL_CCW) để khử mặt sau chính xác (V . N >= 0)
 *      - Các đoạn thẳng hiển thị trực quan vector pháp tuyến phục vụ giảng dạy & bảo vệ đồ án
 *
 * LƯU Ý: Tuyệt đối không dùng các hàm khối có sẵn glutSolid* để đáp ứng đúng yêu cầu đề cương môn học.
 */

#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <utility>
#include "core/MathCG.h"

namespace ParametricShapes {

// ============================================================================
// 1. CẤU TRÚC DỮ LIỆU LƯU TRỮ ĐỈNH VÀ PHÁP TUYẾN BỀ MẶT
// ============================================================================

struct SurfaceVertex {
    MathCG::Vec3 position;
    MathCG::Vec3 normal;
    MathCG::Vec2 uv;

    SurfaceVertex() = default;
    SurfaceVertex(const MathCG::Vec3& p, const MathCG::Vec3& n, const MathCG::Vec2& t = MathCG::Vec2())
        : position(p), normal(n), uv(t) {}
};

// ============================================================================
// 2. TÍNH TOÁN BỀ MẶT GIẢI TÍCH (CÔNG THỨC TRONG ĐỀ CƯƠNG CHƯƠNG 5 & 6)
// ============================================================================

/**
 * @brief Chương 5: Tính điểm trên Mặt trụ kẻ (dọc theo trục Z)
 *        X(u,v) = r * cos(u)
 *        Y(u,v) = r * sin(u)
 *        Z(u,v) = h * v
 *        với u thuộc [0, 2*pi], v thuộc [0, 1]
 *
 * Pháp tuyến: dP/du = (-r*sin(u), r*cos(u), 0)
 *             dP/dv = (0, 0, h)
 *             N = (dP/du x dP/dv) / ||...|| = (cos(u), sin(u), 0)
 */
inline SurfaceVertex evaluateCylinder(float u, float v, float r, float h) {
    float cu = std::cos(u);
    float su = std::sin(u);
    MathCG::Vec3 pos(r * cu, r * su, h * v);
    MathCG::Vec3 normal(cu, su, 0.0f);
    MathCG::Vec2 uv(u / MathCG::TWO_PI, v);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chương 5: Tính điểm trên Mặt nón / Nón cụt kẻ (dọc theo trục Z)
 *        R(v) = rBase + (rTop - rBase) * v
 *        X(u,v) = R(v) * cos(u)
 *        Y(u,v) = R(v) * sin(u)
 *        Z(u,v) = h * v
 *
 * Pháp tuyến: dr = rBase - rTop
 *             slant = sqrt(dr^2 + h^2)
 *             N = ( (h/slant)*cos(u), (h/slant)*sin(u), dr/slant )
 */
inline SurfaceVertex evaluateCone(float u, float v, float rBase, float rTop, float h) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float rCurrent = rBase + (rTop - rBase) * v;
    MathCG::Vec3 pos(rCurrent * cu, rCurrent * su, h * v);

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float nz = dr / slant;
    MathCG::Vec3 normal(nr * cu, nr * su, nz);

    MathCG::Vec2 uv(u / MathCG::TWO_PI, v);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chương 5: Mặt tròn xoay - Tính điểm trên Mặt cầu (trục cực theo trục Z)
 *        u thuộc [0, 2*pi] (kinh độ), v thuộc [-pi/2, pi/2] (vĩ độ)
 *        X(u,v) = r * cos(u) * cos(v)
 *        Y(u,v) = r * sin(u) * cos(v)
 *        Z(u,v) = r * sin(v)
 *
 * Pháp tuyến: N(u,v) = (cos(u)*cos(v), sin(u)*cos(v), sin(v))
 */
inline SurfaceVertex evaluateSphere(float u, float v, float r) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float cv = std::cos(v);
    float sv = std::sin(v);

    MathCG::Vec3 normal(cu * cv, su * cv, sv);
    MathCG::Vec3 pos = normal * r;
    MathCG::Vec2 uv(u / MathCG::TWO_PI, (v + MathCG::HALF_PI) / MathCG::PI);
    return SurfaceVertex(pos, normal, uv);
}

/**
 * @brief Chương 5: Mặt tròn xoay - Tính điểm trên Mặt xuyến (xoay quanh trục Z)
 *        u thuộc [0, 2*pi] (góc quay chính), v thuộc [0, 2*pi] (góc tiết diện ống)
 *        R = bán kính lớn, a = bán kính ống
 *        X(u,v) = (R + a * cos(v)) * cos(u)
 *        Y(u,v) = (R + a * cos(v)) * sin(u)
 *        Z(u,v) = a * sin(v)
 *
 * Pháp tuyến: N(u,v) = (cos(v)*cos(u), cos(v)*sin(u), sin(v))
 */
inline SurfaceVertex evaluateToroid(float u, float v, float R, float a) {
    float cu = std::cos(u);
    float su = std::sin(u);
    float cv = std::cos(v);
    float sv = std::sin(v);

    MathCG::Vec3 normal(cv * cu, cv * su, sv);
    MathCG::Vec3 pos((R + a * cv) * cu, (R + a * cv) * su, a * sv);
    MathCG::Vec2 uv(u / MathCG::TWO_PI, v / MathCG::TWO_PI);
    return SurfaceVertex(pos, normal, uv);
}

// ============================================================================
// 3. VẼ VECTOR PHÁP TUYẾN KIỂM TRA (Chương 6)
// ============================================================================

/**
 * @brief Vẽ các đoạn thẳng biểu diễn vector pháp tuyến phục vụ giảng dạy & bảo vệ đồ án
 */
inline void drawNormalWhiskers(
    const std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>>& whiskers,
    float length = 1.0f,
    float r = 1.0f, float g = 1.0f, float b = 0.0f
) {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    if (lightingWasOn) glDisable(GL_LIGHTING);

    glColor3f(r, g, b);
    glBegin(GL_LINES);
    for (const auto& w : whiskers) {
        glVertex3f(w.first.x, w.first.y, w.first.z);
        MathCG::Vec3 tip = w.first + w.second * length;
        glVertex3f(tip.x, tip.y, tip.z);
    }
    glEnd();

    if (lightingWasOn) glEnable(GL_LIGHTING);
}

// ============================================================================
// 4. SINH ĐĨA TRÒN PHẲNG & NẮP CHẮN HAI ĐẦU (GL_TRIANGLE_FAN)
// ============================================================================

/**
 * @brief Vẽ đĩa tròn phẳng (nắp chắn cho hình trụ/hình nón) tại cao độ z = height
 *        sử dụng GL_TRIANGLE_FAN với vector pháp tuyến chính xác (+Z hoặc -Z)
 */
inline void drawDisk(float r, int slices, float z = 0.0f, bool flipNormal = false) {
    if (slices < 3) slices = 3;
    float nz = flipNormal ? -1.0f : 1.0f;

    glNormal3f(0.0f, 0.0f, nz);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        if (flipNormal) {
            // Cùng chiều kim đồng hồ từ trên nhìn xuống -> Ngược chiều kim đồng hồ (CCW) khi nhìn từ hướng -Z
            u = -u;
        }
        glVertex3f(r * std::cos(u), r * std::sin(u), z);
    }
    glEnd();
}

/**
 * @brief Vẽ đĩa tròn phẳng nằm trên mặt phẳng XZ (dọc theo trục Y)
 */
inline void drawDiskY(float r, int slices, float y = 0.0f, bool flipNormal = false) {
    if (slices < 3) slices = 3;
    float ny = flipNormal ? -1.0f : 1.0f;

    glNormal3f(0.0f, ny, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, y, 0.0f);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        if (flipNormal) {
            u = -u;
        }
        glVertex3f(r * std::cos(u), y, r * std::sin(u));
    }
    glEnd();
}

// ============================================================================
// 5. MẶT TRỤ THAM SỐ (MẶT TRỤ - MẶT KẺ)
// ============================================================================

/**
 * @brief Chương 5: Vẽ hình trụ tham số dọc theo trục Z cục bộ [0, h]
 *        bằng GL_QUAD_STRIP với pháp tuyến tường minh glNormal3f.
 *        Các mặt trước có thứ tự đỉnh ngược chiều kim đồng hồ (CCW) khi nhìn từ bên ngoài.
 */
inline void drawCylinder(
    float r, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float z0 = h * v0;
        float z1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            // Pháp tuyến đơn vị hướng ra ngoài: (cos(u), sin(u), 0)
            glNormal3f(cu, su, 0.0f);
            // Đưa đỉnh trên (z1) trước rồi đến đỉnh dưới (z0) để tạo chiều quấn CCW hướng ra ngoài
            glVertex3f(r * cu, r * su, z1);
            glVertex3f(r * cu, r * su, z0);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * cu, r * su, (z0 + z1) * 0.5f),
                    MathCG::Vec3(cu, su, 0.0f)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        drawDisk(r, slices, 0.0f, true);  // Nắp đáy dưới (pháp tuyến -Z)
        drawDisk(r, slices, h, false);    // Nắp đáy trên (pháp tuyến +Z)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 1.0f, 1.0f, 0.0f);
    }
}

/**
 * @brief Chương 5: Vẽ hình trụ tham số dọc theo trục Y cục bộ [0, h]
 *        Hữu ích cho các cột trụ thẳng đứng trong công viên, thân tháp Sky Tower, cột vòng quay ngựa gỗ.
 */
inline void drawCylinderY(
    float r, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float y0 = h * v0;
        float y1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(cu, 0.0f, su);
            glVertex3f(r * cu, y1, r * su);
            glVertex3f(r * cu, y0, r * su);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * cu, (y0 + y1) * 0.5f, r * su),
                    MathCG::Vec3(cu, 0.0f, su)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        drawDiskY(r, slices, 0.0f, true); // Nắp đáy dưới (pháp tuyến -Y)
        drawDiskY(r, slices, h, false);   // Nắp đáy trên (pháp tuyến +Y)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 1.0f, 1.0f, 0.0f);
    }
}

// ============================================================================
// 6. MẶT NÓN / NÓN CỤT THAM SỐ (MẶT NÓN - MẶT KẺ)
// ============================================================================

/**
 * @brief Chương 5: Vẽ hình nón hoặc nón cụt tham số dọc theo trục Z cục bộ [0, h]
 *        Đáy tại z = 0 với bán kính rBase, đỉnh tại z = h với bán kính rTop.
 *        Nếu rTop == 0.0f thì tạo ra đỉnh nón nhọn.
 */
inline void drawCone(
    float rBase, float rTop, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float nz = dr / slant;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float r0 = rBase + (rTop - rBase) * v0;
        float r1 = rBase + (rTop - rBase) * v1;
        float z0 = h * v0;
        float z1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(nr * cu, nr * su, nz);
            glVertex3f(r1 * cu, r1 * su, z1);
            glVertex3f(r0 * cu, r0 * su, z0);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((r0 + r1) * 0.5f * cu, (r0 + r1) * 0.5f * su, (z0 + z1) * 0.5f),
                    MathCG::Vec3(nr * cu, nr * su, nz)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        if (rBase > 1e-5f) drawDisk(rBase, slices, 0.0f, true); // Nắp đáy dưới (-Z)
        if (rTop  > 1e-5f) drawDisk(rTop,  slices, h, false);   // Nắp đáy trên (+Z)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, rBase * 0.35f, 1.0f, 0.8f, 0.1f);
    }
}

/**
 * @brief Chương 5: Vẽ hình nón hoặc nón cụt tham số dọc theo trục Y cục bộ [0, h]
 *        Hữu ích cho mái nón vòng quay ngựa gỗ Carousel, tháp chùa, chóp trang trí.
 */
inline void drawConeY(
    float rBase, float rTop, float h,
    int slices = 24, int stacks = 4,
    bool closed = true,
    bool drawNormals = false
) {
    if (slices < 3) slices = 3;
    if (stacks < 1) stacks = 1;

    float dr = rBase - rTop;
    float slant = std::sqrt(dr * dr + h * h);
    if (slant < 1e-6f) slant = 1.0f;
    float nr = h / slant;
    float ny = dr / slant;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float v0 = static_cast<float>(j)     / stacks;
        float v1 = static_cast<float>(j + 1) / stacks;
        float r0 = rBase + (rTop - rBase) * v0;
        float r1 = rBase + (rTop - rBase) * v1;
        float y0 = h * v0;
        float y1 = h * v1;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            glNormal3f(nr * cu, ny, nr * su);
            glVertex3f(r1 * cu, y1, r1 * su);
            glVertex3f(r0 * cu, y0, r0 * su);

            if (drawNormals && j == stacks / 2 && (i % 2 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((r0 + r1) * 0.5f * cu, (y0 + y1) * 0.5f, (r0 + r1) * 0.5f * su),
                    MathCG::Vec3(nr * cu, ny, nr * su)
                });
            }
        }
        glEnd();
    }

    if (closed) {
        if (rBase > 1e-5f) drawDiskY(rBase, slices, 0.0f, true); // Nắp đáy dưới (-Y)
        if (rTop  > 1e-5f) drawDiskY(rTop,  slices, h, false);   // Nắp đáy trên (+Y)
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, rBase * 0.35f, 1.0f, 0.8f, 0.1f);
    }
}

// ============================================================================
// 7. MẶT CẦU THAM SỐ (MẶT CẦU - MẶT TRÒN XOAY)
// ============================================================================

/**
 * @brief Chương 5: Vẽ mặt cầu tham số có tâm tại gốc tọa độ (0,0,0)
 *        Lưới kinh độ - vĩ độ sử dụng GL_QUAD_STRIP với pháp tuyến giải tích
 */
inline void drawSphere(
    float r,
    int slices = 24, int stacks = 16,
    bool drawNormals = false
) {
    if (slices < 4) slices = 4;
    if (stacks < 2) stacks = 2;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int j = 0; j < stacks; ++j) {
        float phi0 = -MathCG::HALF_PI + (static_cast<float>(j)     / stacks) * MathCG::PI;
        float phi1 = -MathCG::HALF_PI + (static_cast<float>(j + 1) / stacks) * MathCG::PI;
        float cPhi0 = std::cos(phi0), sPhi0 = std::sin(phi0);
        float cPhi1 = std::cos(phi1), sPhi1 = std::sin(phi1);

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float theta = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
            float cTh = std::cos(theta);
            float sTh = std::sin(theta);

            // Vòng vĩ tuyến trên (phi1)
            float nx1 = cPhi1 * cTh;
            float ny1 = sPhi1;
            float nz1 = cPhi1 * sTh;
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(r * nx1, r * ny1, r * nz1);

            // Vòng vĩ tuyến dưới (phi0)
            float nx0 = cPhi0 * cTh;
            float ny0 = sPhi0;
            float nz0 = cPhi0 * sTh;
            glNormal3f(nx0, ny0, nz0);
            glVertex3f(r * nx0, r * ny0, r * nz0);

            if (drawNormals && (j % 3 == 0) && (i % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3(r * nx0, r * ny0, r * nz0),
                    MathCG::Vec3(nx0, ny0, nz0)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, r * 0.35f, 0.2f, 1.0f, 0.2f);
    }
}

// ============================================================================
// 8. MẶT XUYẾN THAM SỐ (MẶT XUYẾN - MẶT TRÒN XOAY)
// ============================================================================

/**
 * @brief Chương 5: Vẽ mặt xuyến tham số (Toroid) trong mặt phẳng XY quay quanh trục Z
 *        X(u,v) = (R + a*cos(v))*cos(u)
 *        Y(u,v) = (R + a*cos(v))*sin(u)
 *        Z(u,v) = a*sin(v)
 *        Hữu ích cho vành bánh xe Sun Wheel, vòng trục tâm, các vòng xuyến trang trí.
 */
inline void drawToroid(
    float R, float a,
    int uSteps = 32, int vSteps = 16,
    bool drawNormals = false
) {
    if (uSteps < 3) uSteps = 3;
    if (vSteps < 3) vSteps = 3;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int i = 0; i < uSteps; ++i) {
        float u0 = (static_cast<float>(i)     / uSteps) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(i + 1) / uSteps) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            // Pháp tuyến đơn vị: (cv * cos(u), cv * sin(u), sv)
            glNormal3f(cv * cu0, cv * su0, sv);
            glVertex3f((R + a * cv) * cu0, (R + a * cv) * su0, a * sv);

            glNormal3f(cv * cu1, cv * su1, sv);
            glVertex3f((R + a * cv) * cu1, (R + a * cv) * su1, a * sv);

            if (drawNormals && (i % 3 == 0) && (j % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((R + a * cv) * cu0, (R + a * cv) * su0, a * sv),
                    MathCG::Vec3(cv * cu0, cv * su0, sv)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, a * 1.5f, 0.0f, 1.0f, 1.0f);
    }
}

/**
 * @brief Chương 5: Vẽ mặt xuyến tham số (Toroid) trong mặt phẳng XZ quay quanh trục Y
 *        Hữu ích cho vòng ghế khách trên tháp Sky Tower, các đai tròn nằm ngang.
 */
inline void drawToroidY(
    float R, float a,
    int uSteps = 32, int vSteps = 16,
    bool drawNormals = false
) {
    if (uSteps < 3) uSteps = 3;
    if (vSteps < 3) vSteps = 3;

    std::vector<std::pair<MathCG::Vec3, MathCG::Vec3>> normalWhiskers;

    for (int i = 0; i < uSteps; ++i) {
        float u0 = (static_cast<float>(i)     / uSteps) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(i + 1) / uSteps) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            glNormal3f(cv * cu0, sv, cv * su0);
            glVertex3f((R + a * cv) * cu0, a * sv, (R + a * cv) * su0);

            glNormal3f(cv * cu1, sv, cv * su1);
            glVertex3f((R + a * cv) * cu1, a * sv, (R + a * cv) * su1);

            if (drawNormals && (i % 3 == 0) && (j % 3 == 0)) {
                normalWhiskers.push_back({
                    MathCG::Vec3((R + a * cv) * cu0, a * sv, (R + a * cv) * su0),
                    MathCG::Vec3(cv * cu0, sv, cv * su0)
                });
            }
        }
        glEnd();
    }

    if (drawNormals && !normalWhiskers.empty()) {
        drawNormalWhiskers(normalWhiskers, a * 1.5f, 0.0f, 1.0f, 1.0f);
    }
}

// ============================================================================
// 9. KHỐI TRỤ QUÉT NỐI HAI ĐIỂM 3D BẤT KỲ (A -> B)
// ============================================================================

/**
 * @brief Vẽ hình trụ / nón cụt nối giữa hai điểm A và B bất kỳ trong không gian 3D
 *        với bán kính rA tại A và rB tại B.
 *        Tính hệ trục tọa độ trực chuẩn {fwd, right, up} dọc theo vector A->B.
 */
inline void drawCylinderBetween(
    const MathCG::Vec3& a,
    const MathCG::Vec3& b,
    float rA,
    float rB,
    int slices = 12,
    bool drawCaps = true
) {
    MathCG::Vec3 diff = b - a;
    float len = diff.length();
    if (len < 1e-4f) return;

    MathCG::Vec3 fwd = diff / len;
    MathCG::Vec3 helper = (std::fabs(fwd.y) < 0.90f)
        ? MathCG::Vec3(0.0f, 1.0f, 0.0f)
        : MathCG::Vec3(1.0f, 0.0f, 0.0f);
    MathCG::Vec3 right = MathCG::cross(helper, fwd).normalized();
    MathCG::Vec3 up    = MathCG::cross(fwd, right).normalized();

    float dr = rA - rB;
    float slant = std::sqrt(dr * dr + len * len);
    if (slant < 1e-6f) slant = 1.0f;
    float nr  = len / slant; // Trọng số pháp tuyến hướng kính
    float nL  = dr  / slant; // Trọng số pháp tuyến dọc trục

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
        float cu = std::cos(u);
        float su = std::sin(u);

        MathCG::Vec3 radialDir = right * cu + up * su;
        MathCG::Vec3 normal    = (radialDir * nr + fwd * nL).normalized();

        MathCG::Vec3 pA = a + radialDir * rA;
        MathCG::Vec3 pB = b + radialDir * rB;

        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(pB.x, pB.y, pB.z);
        glVertex3f(pA.x, pA.y, pA.z);
    }
    glEnd();

    if (drawCaps) {
        if (rA > 1e-4f) {
            glNormal3f(-fwd.x, -fwd.y, -fwd.z);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(a.x, a.y, a.z);
            for (int i = 0; i <= slices; ++i) {
                float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
                MathCG::Vec3 radialDir = right * std::cos(-u) + up * std::sin(-u);
                MathCG::Vec3 p = a + radialDir * rA;
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
        if (rB > 1e-4f) {
            glNormal3f(fwd.x, fwd.y, fwd.z);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(b.x, b.y, b.z);
            for (int i = 0; i <= slices; ++i) {
                float u = (static_cast<float>(i) / slices) * MathCG::TWO_PI;
                MathCG::Vec3 radialDir = right * std::cos(u) + up * std::sin(u);
                MathCG::Vec3 p = b + radialDir * rB;
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
    }
}

// ============================================================================
// 10. MẶT XUYẾN ĐÈN LED HSV PHÂN ĐOẠN (HIỆU ỨNG BAN ĐÊM CHO SUN WHEEL / CAROUSEL / SKY TOWER)
// ============================================================================

/**
 * @brief Chương 1 & 5: Mặt xuyến phân đoạn cho hiệu ứng hoạt ảnh đèn LED đa sắc.
 *        Vẽ độc lập N cung tròn (sector), tính toán màu sắc động HSV -> RGB
 *        kèm theo độ phát sáng (emission) tùy chọn tạo hiệu ứng ban đêm rực rỡ.
 */
inline void drawSegmentedLedToroid(
    float R, float a,
    int numSectors,
    int vSteps,
    float baseHue,
    float hueStepPerSector,
    float saturation = 0.92f,
    float value = 1.0f,
    float emissiveIntensity = 0.85f
) {
    if (numSectors < 3) numSectors = 3;
    if (vSteps < 3) vSteps = 3;

    for (int s = 0; s < numSectors; ++s) {
        float hue = std::fmod(baseHue + s * hueStepPerSector, 360.0f);
        if (hue < 0.0f) hue += 360.0f;
        float lr = 1.0f, lg = 1.0f, lb = 1.0f;
        MathCG::HSVtoRGB(hue, saturation, value, lr, lg, lb);

        float u0 = (static_cast<float>(s)     / numSectors) * MathCG::TWO_PI;
        float u1 = (static_cast<float>(s + 1) / numSectors) * MathCG::TWO_PI;
        float cu0 = std::cos(u0), su0 = std::sin(u0);
        float cu1 = std::cos(u1), su1 = std::sin(u1);

        GLfloat matDiff[4] = { lr, lg, lb, 1.0f };
        GLfloat matEmit[4] = {
            lr * emissiveIntensity,
            lg * emissiveIntensity,
            lb * emissiveIntensity,
            1.0f
        };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  matDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmit);
        glColor3f(lr, lg, lb);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= vSteps; ++j) {
            float v  = (static_cast<float>(j) / vSteps) * MathCG::TWO_PI;
            float cv = std::cos(v), sv = std::sin(v);

            glNormal3f(cv * cu0, cv * su0, sv);
            glVertex3f((R + a * cv) * cu0, (R + a * cv) * su0, a * sv);

            glNormal3f(cv * cu1, cv * su1, sv);
            glVertex3f((R + a * cv) * cu1, (R + a * cv) * su1, a * sv);
        }
        glEnd();
    }

    // Đặt lại độ tự phát sáng (emission) về 0
    GLfloat zeroEmit[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zeroEmit);
}

} // namespace ParametricShapes

#endif // PARAMETRIC_SHAPES_H
