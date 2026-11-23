#ifndef MESH_BUILDERS_H
#define MESH_BUILDERS_H

/**
 * @file MeshBuilders.h
 * @brief Đợt 8a: Thư viện hàm dựng lưới đa giác kiến trúc DÙNG CHUNG (header-only), gom các helper
 *        từng bị trùng lặp giữa NgoMonGate.h và ParkArchitecture.h và bổ sung các helper mới:
 *          - appendBoxByYRange / appendRotatedTranslatedMesh (gom từ 2 file cũ, giữ nguyên hành vi)
 *          - appendOrientedPolygon  : đa giác phẳng tự đảo chiều đỉnh theo hướng pháp tuyến mong muốn
 *          - appendStaircase        : cầu thang đá bậc liền khối + hai má lan can thấp
 *          - appendOctagonalPrism   : lăng trụ bát giác (các mặt phẳng quay đúng 4 hướng Đông/Tây/Nam/Bắc)
 *          - appendOctagonalRoof    : mái ngói bát giác cong (lõm, đầu đao vểnh) có diềm + mặt dưới kín
 *          - appendOctagonalTieredTower : tháp bát giác nhiều tầng mái thu nhỏ dần + chóp vàng
 *
 *  Nguyên tắc: KHÔNG dùng glutSolid*; mọi mặt tự tính pháp tuyến bằng tích có hướng (computeNormals ở nơi gọi);
 *  mặt đa giác sinh bằng đỉnh riêng (không dùng chung) để giữ cạnh sắc khi tô bóng Gouraud.
 */

#include <vector>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"
#include "geometry/MeshModel.h"

namespace Geometry {
namespace MeshBuilders {

/**
 * @brief Thêm hình hộp theo biên độ cao [yBottom, yTop] để các tầng kiến trúc luôn chồng khít liền khối.
 */
inline void appendBoxByYRange(
    PolygonMesh& target,
    float cx, float yBottom, float yTop, float cz,
    float sx, float sz,
    const MathCG::ColorRGB& col
) {
    float sy = yTop - yBottom;
    if (sy <= 1e-4f) return;
    float cy = 0.5f * (yBottom + yTop);
    target.append(PolygonMesh::createBox(cx, cy, cz, sx, sy, sz, col));
}

/**
 * @brief Xoay lưới cục bộ quanh trục Y góc rotYDeg rồi tịnh tiến tới worldPos và gộp vào target.
 */
inline void appendRotatedTranslatedMesh(
    PolygonMesh& target,
    PolygonMesh localMesh,
    const MathCG::Vec3& worldPos,
    float rotYDeg
) {
    float rad = rotYDeg * MathCG::DEG2RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    for (auto& v : localMesh.vertices) {
        float rx =  v.x * c + v.z * s;
        float rz = -v.x * s + v.z * c;
        v.x = worldPos.x + rx;
        v.y = worldPos.y + v.y;
        v.z = worldPos.z + rz;
    }
    target.append(localMesh);
}

/**
 * @brief Thêm 1 đa giác phẳng (>= 3 đỉnh) bằng đỉnh riêng; tự đảo thứ tự đỉnh sao cho pháp tuyến Newell
 *        cùng phía với outwardHint (mặt trước CCW khi nhìn từ phía hint) -> không bao giờ bị back-face culling nhầm.
 */
inline void appendOrientedPolygon(
    PolygonMesh& target,
    const std::vector<MathCG::Vec3>& pts,
    const MathCG::ColorRGB& col,
    const MathCG::Vec3& outwardHint
) {
    const int n = static_cast<int>(pts.size());
    if (n < 3) return;
    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
    for (int i = 0; i < n; ++i) {
        const MathCG::Vec3& a = pts[i];
        const MathCG::Vec3& b = pts[(i + 1) % n];
        nx += (a.y - b.y) * (a.z + b.z);
        ny += (a.z - b.z) * (a.x + b.x);
        nz += (a.x - b.x) * (a.y + b.y);
    }
    const bool flip = (nx * outwardHint.x + ny * outwardHint.y + nz * outwardHint.z) < 0.0f;
    std::vector<int> idx(n);
    for (int i = 0; i < n; ++i) {
        idx[i] = target.addVertex(pts[flip ? (n - 1 - i) : i]);
    }
    if (n == 3) {
        target.addTriangle(idx[0], idx[1], idx[2], col, true);
    } else if (n == 4) {
        target.addQuad(idx[0], idx[1], idx[2], idx[3], col, true);
    } else {
        target.addFace(Face(idx, col));
    }
}

/**
 * @brief Cầu thang đá liền khối: các bậc là hộp từ yBase tới cao độ bậc, kèm 2 má lan can thấp hai bên.
 *        (wallX, wallZ) là điểm giữa chân mặt tường/bệ phía trên (đỉnh cầu thang); (dirX, dirZ) là vector
 *        đơn vị trục (±1,0) hoặc (0,±1) chỉ hướng đi RA XA bệ (bậc thấp dần theo hướng này).
 *        Bậc i (i = 0 sát bệ) cao totalRise * (numSteps - i) / numSteps tính từ yBase.
 */
inline void appendStaircase(
    PolygonMesh& target,
    float wallX, float wallZ, int dirX, int dirZ,
    float yBase, float width, float run, float totalRise, int numSteps,
    const MathCG::ColorRGB& stepCol,
    const MathCG::ColorRGB& cheekCol,
    float cheekExtra = 0.35f
) {
    if (numSteps < 1) return;
    const float tread = run / static_cast<float>(numSteps);
    const bool alongX = (dirX != 0);
    for (int i = 0; i < numSteps; ++i) {
        const float top = yBase + totalRise * static_cast<float>(numSteps - i) / static_cast<float>(numSteps);
        const float dc  = (static_cast<float>(i) + 0.5f) * tread;
        const float cx  = wallX + dirX * dc;
        const float cz  = wallZ + dirZ * dc;
        appendBoxByYRange(target, cx, yBase, top, cz,
                          alongX ? tread : width, alongX ? width : tread, stepCol);
        for (int side : {-1, 1}) {
            const float off = side * (width * 0.5f + 0.2f);
            appendBoxByYRange(target,
                              alongX ? cx : cx + off, yBase, top + cheekExtra,
                              alongX ? cz + off : cz,
                              alongX ? tread : 0.4f, alongX ? 0.4f : tread, cheekCol);
        }
    }
}

namespace detail {
// Bán kính đỉnh của bát giác có khoảng cách tâm-cạnh (apothem) cho trước
inline float octVertexRadius(float apothem) {
    return apothem / std::cos(MathCG::PI / 8.0f);
}
// Góc của đỉnh thứ i: 22.5 + 45 * i độ -> các cạnh phẳng nằm đúng trục X / Z (0, 90, 180, 270 độ)
inline float octVertexAngle(int i) {
    return (22.5f + 45.0f * static_cast<float>(i)) * MathCG::DEG2RAD;
}
inline float octFaceAngle(int i) {
    return (45.0f * static_cast<float>(i + 1)) * MathCG::DEG2RAD; // góc pháp tuyến cạnh giữa đỉnh i và i+1
}
} // namespace detail

/**
 * @brief Lăng trụ bát giác đều theo cao độ [yBottom, yTop]; apothem = khoảng cách từ tâm tới mỗi mặt phẳng.
 */
inline void appendOctagonalPrism(
    PolygonMesh& target,
    float cx, float yBottom, float yTop, float cz,
    float apothem,
    const MathCG::ColorRGB& col
) {
    if (yTop - yBottom <= 1e-4f) return;
    const float R = detail::octVertexRadius(apothem);
    float px[8], pz[8];
    for (int i = 0; i < 8; ++i) {
        float a = detail::octVertexAngle(i);
        px[i] = cx + R * std::cos(a);
        pz[i] = cz + R * std::sin(a);
    }
    for (int i = 0; i < 8; ++i) {
        int j = (i + 1) % 8;
        float fa = detail::octFaceAngle(i);
        appendOrientedPolygon(target, {
            MathCG::Vec3(px[i], yBottom, pz[i]), MathCG::Vec3(px[j], yBottom, pz[j]),
            MathCG::Vec3(px[j], yTop,    pz[j]), MathCG::Vec3(px[i], yTop,    pz[i])
        }, col, MathCG::Vec3(std::cos(fa), 0.0f, std::sin(fa)));
    }
    std::vector<MathCG::Vec3> topRing, botRing;
    for (int i = 0; i < 8; ++i) {
        topRing.emplace_back(px[i], yTop,    pz[i]);
        botRing.emplace_back(px[i], yBottom, pz[i]);
    }
    appendOrientedPolygon(target, topRing, col, MathCG::Vec3(0.0f,  1.0f, 0.0f));
    appendOrientedPolygon(target, botRing, col, MathCG::Vec3(0.0f, -1.0f, 0.0f));
}

/**
 * @brief Mái ngói bát giác cong kiểu Á Đông (profile lõm, đầu đao vểnh nhẹ):
 *        đặt tại cao độ y0 (cao độ mặt dưới diềm trên đầu cột/thân), cao `height`, diềm mái có apothem eaveApothem.
 *        capApothem > 0: mái cụt, đỉnh phẳng (để đặt thân tầng kế tiếp); capApothem = 0: mái nhọn đỉnh chóp.
 */
inline void appendOctagonalRoof(
    PolygonMesh& target,
    float cx, float y0, float cz,
    float eaveApothem, float height, float capApothem,
    const MathCG::ColorRGB& col,
    float eaveLift = 0.30f
) {
    constexpr int   kRings = 5;
    const float     tSamples[kRings] = { 0.0f, 0.14f, 0.40f, 0.70f, 1.0f };
    const bool      hasCap = (capApothem > 1e-3f);

    float ringApo[kRings], ringY[kRings];
    for (int k = 0; k < kRings; ++k) {
        const float t = tSamples[k];
        ringApo[k] = eaveApothem - (eaveApothem - capApothem) * t;
        ringY[k]   = y0 + height * std::pow(t, 1.7f) + eaveLift * std::max(0.0f, 1.0f - t / 0.2f);
    }

    float rx[kRings][8], rz[kRings][8];
    for (int k = 0; k < kRings; ++k) {
        const float R = detail::octVertexRadius(ringApo[k]);
        for (int i = 0; i < 8; ++i) {
            const float a = detail::octVertexAngle(i);
            rx[k][i] = cx + R * std::cos(a);
            rz[k][i] = cz + R * std::sin(a);
        }
    }

    // Các dải mái từ diềm lên đỉnh
    for (int k = 0; k < kRings - 1; ++k) {
        const bool lastToApex = (!hasCap && k == kRings - 2);
        for (int i = 0; i < 8; ++i) {
            const int j = (i + 1) % 8;
            const float fa = detail::octFaceAngle(i);
            const MathCG::Vec3 hint(std::cos(fa), 1.0f, std::sin(fa));
            if (lastToApex) {
                appendOrientedPolygon(target, {
                    MathCG::Vec3(rx[k][i], ringY[k], rz[k][i]),
                    MathCG::Vec3(rx[k][j], ringY[k], rz[k][j]),
                    MathCG::Vec3(cx, ringY[k + 1], cz)
                }, col, hint);
            } else {
                appendOrientedPolygon(target, {
                    MathCG::Vec3(rx[k][i],     ringY[k],     rz[k][i]),
                    MathCG::Vec3(rx[k][j],     ringY[k],     rz[k][j]),
                    MathCG::Vec3(rx[k + 1][j], ringY[k + 1], rz[k + 1][j]),
                    MathCG::Vec3(rx[k + 1][i], ringY[k + 1], rz[k + 1][i])
                }, col, hint);
            }
        }
    }

    // Nắp phẳng trên đỉnh mái cụt
    if (hasCap) {
        std::vector<MathCG::Vec3> capRing;
        for (int i = 0; i < 8; ++i) capRing.emplace_back(rx[kRings - 1][i], ringY[kRings - 1], rz[kRings - 1][i]);
        appendOrientedPolygon(target, capRing, col, MathCG::Vec3(0.0f, 1.0f, 0.0f));
    }

    // Diềm đứng mép mái + mặt dưới kín (nhìn từ dưới lên không thấy lỗ hổng)
    const float yFascia = y0 - 0.15f;
    for (int i = 0; i < 8; ++i) {
        const int j = (i + 1) % 8;
        const float fa = detail::octFaceAngle(i);
        appendOrientedPolygon(target, {
            MathCG::Vec3(rx[0][i], yFascia,  rz[0][i]), MathCG::Vec3(rx[0][j], yFascia,  rz[0][j]),
            MathCG::Vec3(rx[0][j], ringY[0], rz[0][j]), MathCG::Vec3(rx[0][i], ringY[0], rz[0][i])
        }, col, MathCG::Vec3(std::cos(fa), 0.0f, std::sin(fa)));
        appendOrientedPolygon(target, {
            MathCG::Vec3(cx, yFascia, cz),
            MathCG::Vec3(rx[0][i], yFascia, rz[0][i]),
            MathCG::Vec3(rx[0][j], yFascia, rz[0][j])
        }, col, MathCG::Vec3(0.0f, -1.0f, 0.0f));
    }
}

/**
 * @brief Một tầng của tháp bát giác nhiều tầng mái.
 */
struct OctTier {
    float apothem;      // khoảng cách tâm -> mặt phẳng thân tầng
    float bodyHeight;   // chiều cao thân tầng
    float eaveApothem;  // apothem mép mái ngói của tầng
    float roofHeight;   // chiều cao mái ngói (từ diềm tới đỉnh/nắp)
};

/**
 * @brief Tháp bát giác nhiều tầng mái thu nhỏ dần (phong cách tháp Á Đông): mỗi tầng gồm chân phào, thân, gờ cổ diềm
 *        và mái ngói cong bát giác; tầng cuối có mái nhọn + chóp vàng. Trả về cao độ đỉnh chóp.
 *        Ghi lại cao độ đáy/đỉnh thân của từng tầng vào bodyY0 / bodyY1 (nếu khác nullptr) để gắn chi tiết (mặt đồng hồ...).
 */
inline float appendOctagonalTieredTower(
    PolygonMesh& target,
    float cx, float cz, float yBase,
    const OctTier* tiers, int numTiers,
    const MathCG::ColorRGB& wallCol,
    const MathCG::ColorRGB& wallAltCol,
    const MathCG::ColorRGB& trimCol,
    const MathCG::ColorRGB& roofCol,
    const MathCG::ColorRGB& finialCol,
    float* bodyY0 = nullptr,
    float* bodyY1 = nullptr
) {
    float y = yBase;
    for (int t = 0; t < numTiers; ++t) {
        const OctTier& tr = tiers[t];
        const bool isLast = (t == numTiers - 1);
        const MathCG::ColorRGB& wc = (t % 2 == 0) ? wallCol : wallAltCol;
        const float yTop = y + tr.bodyHeight;

        if (bodyY0) bodyY0[t] = y;
        if (bodyY1) bodyY1[t] = yTop;

        appendOctagonalPrism(target, cx, y,          y + 0.35f, cz, tr.apothem + 0.25f, trimCol); // chân phào
        appendOctagonalPrism(target, cx, y + 0.30f,  yTop,      cz, tr.apothem,         wc);      // thân tầng
        appendOctagonalPrism(target, cx, yTop - 0.25f, yTop + 0.15f, cz, tr.apothem + 0.35f, trimCol); // gờ cổ diềm

        const float capApo = isLast ? 0.0f : (tiers[t + 1].apothem + 0.30f);
        const float roofY0 = yTop + 0.15f;
        appendOctagonalRoof(target, cx, roofY0, cz, tr.eaveApothem, tr.roofHeight, capApo, roofCol);
        y = roofY0 + tr.roofHeight - 0.02f; // thân tầng kế tiếp cắm nhẹ vào nắp mái -> liền khối
    }

    // Chóp vàng trên đỉnh tầng cuối: cột bát giác nhỏ + chóp tứ giác nhọn
    const float apexY = y + 0.02f;
    appendOctagonalPrism(target, cx, apexY - 0.15f, apexY + 0.35f, cz, 0.28f, finialCol);
    target.append(PolygonMesh::createPyramid(cx, apexY + 0.35f, cz, 0.70f, 0.70f, 0.55f, finialCol));
    return apexY + 0.90f;
}

} // namespace MeshBuilders
} // namespace Geometry

#endif // MESH_BUILDERS_H
