#ifndef MATH_CG_H
#define MATH_CG_H

/**
 * @file MathCG.h
 * @brief Core Mathematical Library for Computer Graphics Syllabus
 *
 * Implements explicit formulas from the university syllabus:
 *  - Chapter 1: HSV <-> RGB Color Model conversion & WCS -> NCS -> DCS 2D coordinate mapping
 *  - Chapter 3: 4-bit Cohen-Sutherland Outcodes & Bresenham/Midpoint helpers
 *  - Chapter 4: 3D Vectors, 4x4 Homogeneous Matrices, Arbitrary Pivot Rotation, Planar Reflection
 *  - Chapter 5: Combinatorics C(n,k) & Bernstein Polynomials B_k^n(t)
 *  - Chapter 6: Cross-product Face Normals, Vertex Normal Averaging, Back-face test (V . N >= 0)
 */

#include <cmath>
#include <vector>
#include <algorithm>

namespace MathCG {

constexpr float PI     = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

// ============================================================================
// 1. 3D VECTOR & NORMAL COMPUTATIONS (Chapters 4 & 6)
// ============================================================================

struct Vec2 {
    float x, y;
    Vec2(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}
};

struct Vec3 {
    float x, y, z;

    Vec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
        : x(_x), y(_y), z(_z) {}

    Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    Vec3 operator-() const                { return Vec3(-x, -y, -z); }
    Vec3 operator*(float s) const         { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const         { return ( std::fabs(s) > 1e-8f ) ? Vec3(x / s, y / s, z / s) : Vec3(); }

    Vec3& operator+=(const Vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    Vec3& operator*=(float s)         { x *= s; y *= s; z *= s; return *this; }

    float lengthSquared() const { return x * x + y * y + z * z; }
    float length() const        { return std::sqrt(lengthSquared()); }

    Vec3 normalized() const {
        float len = length();
        if (len < 1e-8f) return Vec3(0.0f, 1.0f, 0.0f);
        return Vec3(x / len, y / len, z / len);
    }
};

inline Vec3 operator*(float s, const Vec3& v) {
    return Vec3(v.x * s, v.y * s, v.z * s);
}

inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return a + (b - a) * t;
}

/**
 * @brief Dot product: A . B = Ax*Bx + Ay*By + Az*Bz
 */
inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * @brief Cross product: N = A x B
 */
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

/**
 * @brief Chapter 6: Compute unit normal vector of a polygon face (P0, P1, P2)
 *        using cross product N = A x B where A = P1 - P0, B = P2 - P0.
 */
inline Vec3 computeFaceNormal(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    Vec3 A = p1 - p0;
    Vec3 B = p2 - p0;
    return cross(A, B).normalized();
}

/**
 * @brief Chapter 6: Compute Gouraud vertex normal by averaging adjacent face normals:
 *        n_p = (1 / k) * sum_{i=1..k}(n_i), then normalized.
 */
inline Vec3 averageVertexNormal(const std::vector<Vec3>& adjacentFaceNormals) {
    if (adjacentFaceNormals.empty()) return Vec3(0.0f, 1.0f, 0.0f);
    Vec3 sum(0.0f, 0.0f, 0.0f);
    for (const Vec3& n : adjacentFaceNormals) {
        sum += n;
    }
    sum = sum / static_cast<float>(adjacentFaceNormals.size());
    return sum.normalized();
}

/**
 * @brief Chapter 6: Explicit Back-face visibility check.
 *        Given Viewing vector V (from eye towards surface point) and outward unit normal N,
 *        a face is a back-face (hidden) if V . N >= 0.
 */
inline bool isBackFace(const Vec3& viewVector, const Vec3& outwardNormal) {
    return dot(viewVector, outwardNormal) >= 0.0f;
}

// ============================================================================
// 2. COLOR MODELS (Chapter 1)
// ============================================================================

struct ColorRGB {
    float r, g, b;
    ColorRGB(float _r = 1.0f, float _g = 1.0f, float _b = 1.0f) : r(_r), g(_g), b(_b) {}
};

/**
 * @brief Chapter 1: Convert HSV color model to RGB color model.
 * @param H Hue angle in degrees, H in [0, 360)
 * @param S Saturation in [0, 1]
 * @param V Value (Brightness) in [0, 1]
 * @param R Output Red in [0, 1]
 * @param G Output Green in [0, 1]
 * @param B Output Blue in [0, 1]
 */
inline void HSVtoRGB(float H, float S, float V, float& R, float& G, float& B) {
    S = std::clamp(S, 0.0f, 1.0f);
    V = std::clamp(V, 0.0f, 1.0f);

    if (S <= 1e-6f) {
        // Achromatic (grayscale)
        R = G = B = V;
        return;
    }

    // Wrap H into [0, 360)
    H = std::fmod(H, 360.0f);
    if (H < 0.0f) H += 360.0f;

    float hSector = H / 60.0f;
    int   k       = static_cast<int>(std::floor(hSector)) % 6;
    float f       = hSector - std::floor(hSector); // fractional part in [0, 1)

    float p = V * (1.0f - S);
    float q = V * (1.0f - S * f);
    float t = V * (1.0f - S * (1.0f - f));

    switch (k) {
        case 0:  R = V; G = t; B = p; break;
        case 1:  R = q; G = V; B = p; break;
        case 2:  R = p; G = V; B = t; break;
        case 3:  R = p; G = q; B = V; break;
        case 4:  R = t; G = p; B = V; break;
        default: R = V; G = p; B = q; break; // case 5
    }
}

// ============================================================================
// 3. COORDINATE SYSTEMS: WCS -> NCS -> DCS (Chapters 1 & 3)
// ============================================================================

/**
 * @brief Maps 2D World Coordinates (WCS) -> Normalized Coordinates (NCS in [0,1])
 *        -> Device/Viewport Coordinates (DCS in pixels).
 *
 * Syllabus formula:
 *   x_0 = (x - x_min) / c_x,  y_0 = (y - y_min) / c_y   with x_0, y_0 in [0, 1]
 *   x_d = vpX + x_0 * vpWidth, y_d = vpY + y_0 * vpHeight
 */
struct CoordinateMapper2D {
    float wcsMinX, wcsMaxX;
    float wcsMinY, wcsMaxY;
    float vpX, vpY, vpWidth, vpHeight;

    CoordinateMapper2D(
        float minX = -120.0f, float maxX = 120.0f,
        float minY = -120.0f, float maxY = 120.0f,
        float vx = 0.0f, float vy = 0.0f, float vw = 220.0f, float vh = 220.0f
    ) : wcsMinX(minX), wcsMaxX(maxX), wcsMinY(minY), wcsMaxY(maxY),
        vpX(vx), vpY(vy), vpWidth(vw), vpHeight(vh) {}

    // Step 1: WCS -> NCS (x0 = x/c, y0 = y/c in [0, 1])
    Vec2 WCStoNCS(float x, float y) const {
        float cx = wcsMaxX - wcsMinX;
        float cy = wcsMaxY - wcsMinY;
        float x0 = (std::fabs(cx) > 1e-6f) ? (x - wcsMinX) / cx : 0.0f;
        float y0 = (std::fabs(cy) > 1e-6f) ? (y - wcsMinY) / cy : 0.0f;
        return Vec2(x0, y0);
    }

    // Step 2: NCS -> DCS (pixel coordinates inside minimap viewport)
    Vec2 NCStoDCS(float x0, float y0) const {
        return Vec2(vpX + x0 * vpWidth, vpY + y0 * vpHeight);
    }

    // Combined pipeline: WCS -> NCS -> DCS
    Vec2 WCStoDCS(float x, float y) const {
        Vec2 ncs = WCStoNCS(x, y);
        return NCStoDCS(ncs.x, ncs.y);
    }
};

// ============================================================================
// 4. 4x4 HOMOGENEOUS TRANSFORMATION MATRICES (Chapter 4)
// ============================================================================

/**
 * @brief Column-major 4x4 Homogeneous Matrix compatible with glMultMatrixf(m).
 */
struct Mat4 {
    float m[16];

    Mat4() { setIdentity(); }

    void setIdentity() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    // Access element at (row r, col c) in column-major layout
    float& at(int r, int c)             { return m[c * 4 + r]; }
    const float& at(int r, int c) const { return m[c * 4 + r]; }

    Mat4 operator*(const Mat4& rhs) const {
        Mat4 res;
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                res.at(r, c) =
                    at(r, 0) * rhs.at(0, c) +
                    at(r, 1) * rhs.at(1, c) +
                    at(r, 2) * rhs.at(2, c) +
                    at(r, 3) * rhs.at(3, c);
            }
        }
        return res;
    }

    Vec3 transformPoint(const Vec3& p) const {
        float xp = at(0, 0) * p.x + at(0, 1) * p.y + at(0, 2) * p.z + at(0, 3);
        float yp = at(1, 0) * p.x + at(1, 1) * p.y + at(1, 2) * p.z + at(1, 3);
        float zp = at(2, 0) * p.x + at(2, 1) * p.y + at(2, 2) * p.z + at(2, 3);
        float wp = at(3, 0) * p.x + at(3, 1) * p.y + at(3, 2) * p.z + at(3, 3);
        if (std::fabs(wp) > 1e-6f && std::fabs(wp - 1.0f) > 1e-6f) {
            return Vec3(xp / wp, yp / wp, zp / wp);
        }
        return Vec3(xp, yp, zp);
    }

    Vec3 transformVector(const Vec3& v) const {
        return Vec3(
            at(0, 0) * v.x + at(0, 1) * v.y + at(0, 2) * v.z,
            at(1, 0) * v.x + at(1, 1) * v.y + at(1, 2) * v.z,
            at(2, 0) * v.x + at(2, 1) * v.y + at(2, 2) * v.z
        );
    }

    static Mat4 fromBasis(const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& pos = Vec3()) {
        Mat4 mat;
        mat.at(0, 0) = right.x;   mat.at(0, 1) = up.x;   mat.at(0, 2) = forward.x; mat.at(0, 3) = pos.x;
        mat.at(1, 0) = right.y;   mat.at(1, 1) = up.y;   mat.at(1, 2) = forward.y; mat.at(1, 3) = pos.y;
        mat.at(2, 0) = right.z;   mat.at(2, 1) = up.z;   mat.at(2, 2) = forward.z; mat.at(2, 3) = pos.z;
        return mat;
    }

    static Mat4 translate(float dx, float dy, float dz) {
        Mat4 mat;
        mat.at(0, 3) = dx;
        mat.at(1, 3) = dy;
        mat.at(2, 3) = dz;
        return mat;
    }

    static Mat4 scale(float sx, float sy, float sz) {
        Mat4 mat;
        mat.at(0, 0) = sx;
        mat.at(1, 1) = sy;
        mat.at(2, 2) = sz;
        return mat;
    }

    static Mat4 rotateX(float angleDeg) {
        float rad = angleDeg * DEG2RAD;
        float c = std::cos(rad), s = std::sin(rad);
        Mat4 mat;
        mat.at(1, 1) = c;  mat.at(1, 2) = -s;
        mat.at(2, 1) = s;  mat.at(2, 2) =  c;
        return mat;
    }

    static Mat4 rotateY(float angleDeg) {
        float rad = angleDeg * DEG2RAD;
        float c = std::cos(rad), s = std::sin(rad);
        Mat4 mat;
        mat.at(0, 0) =  c; mat.at(0, 2) = s;
        mat.at(2, 0) = -s; mat.at(2, 2) = c;
        return mat;
    }

    static Mat4 rotateZ(float angleDeg) {
        float rad = angleDeg * DEG2RAD;
        float c = std::cos(rad), s = std::sin(rad);
        Mat4 mat;
        mat.at(0, 0) = c;  mat.at(0, 1) = -s;
        mat.at(1, 0) = s;  mat.at(1, 1) =  c;
        return mat;
    }

    /**
     * @brief Chapter 4: Composite rotation around an arbitrary center I(x0, y0, z0)
     *        parallel to the Z-axis: M = T(I) * Rz(alpha) * T(-I) in column-vector notation
     *        (equivalent to row-vector T(-I) . Rz(alpha) . T(I)).
     */
    static Mat4 rotateZAroundPoint(const Vec3& centerI, float angleDeg) {
        return translate(centerI.x, centerI.y, centerI.z)
             * rotateZ(angleDeg)
             * translate(-centerI.x, -centerI.y, -centerI.z);
    }

    /**
     * @brief Chapter 4: 3D Planar Reflection Matrix across horizontal water plane y = planeY.
     *        For central lake at y = 0, reflects y -> -y.
     */
    static Mat4 reflectionAcrossYPlane(float planeY = 0.0f) {
        return translate(0.0f, planeY, 0.0f)
             * scale(1.0f, -1.0f, 1.0f)
             * translate(0.0f, -planeY, 0.0f);
    }
};

// ============================================================================
// 5. PARAMETRIC CURVE BASIS: BERNSTEIN POLYNOMIALS (Chapter 5)
// ============================================================================

/**
 * @brief Binomial coefficient C(n, k) = n! / (k! * (n - k)!)
 */
inline int binomialCoeff(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;
    long long res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - k + i) / i;
    }
    return static_cast<int>(res);
}

/**
 * @brief Chapter 5: Bernstein basis polynomial B_k^n(t) = C(n, k) * (1 - t)^(n - k) * t^k
 */
inline float bernsteinBasis(int n, int k, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float cnk = static_cast<float>(binomialCoeff(n, k));
    return cnk * std::pow(1.0f - t, static_cast<float>(n - k)) * std::pow(t, static_cast<float>(k));
}

/**
 * @brief Evaluate degree-n Bezier curve point P(t) = sum_{k=0..n} P_k * B_k^n(t)
 */
inline Vec3 evaluateBezierPoint(const std::vector<Vec3>& controlPoints, float t) {
    if (controlPoints.empty()) return Vec3();
    int n = static_cast<int>(controlPoints.size()) - 1;
    Vec3 point(0.0f, 0.0f, 0.0f);
    for (int k = 0; k <= n; ++k) {
        point += controlPoints[k] * bernsteinBasis(n, k, t);
    }
    return point;
}

/**
 * @brief Chapter 5: Evaluate degree-n Bezier point via De Casteljau's recursive linear interpolation
 */
inline Vec3 evaluateBezierDeCasteljau(const std::vector<Vec3>& controlPoints, float t) {
    if (controlPoints.empty()) return Vec3();
    t = std::clamp(t, 0.0f, 1.0f);
    std::vector<Vec3> temp = controlPoints;
    const int n = static_cast<int>(temp.size()) - 1;
    for (int r = 1; r <= n; ++r) {
        for (int i = 0; i <= n - r; ++i) {
            temp[i] = lerp(temp[i], temp[i + 1], t);
        }
    }
    return temp[0];
}

/**
 * @brief Evaluate unnormalized Bezier first derivative vector P'(t) = n * sum_{k=0..n-1} (P_{k+1} - P_k) * B_k^{n-1}(t)
 */
inline Vec3 evaluateBezierDerivativeRaw(const std::vector<Vec3>& controlPoints, float t) {
    int n = static_cast<int>(controlPoints.size()) - 1;
    if (n < 1) return Vec3(0.0f, 0.0f, 1.0f);
    Vec3 deriv(0.0f, 0.0f, 0.0f);
    for (int k = 0; k <= n - 1; ++k) {
        Vec3 diff = controlPoints[k + 1] - controlPoints[k];
        deriv += diff * bernsteinBasis(n - 1, k, t);
    }
    return deriv * static_cast<float>(n);
}

/**
 * @brief Evaluate normalized Bezier unit tangent vector T(t) = P'(t) / ||P'(t)||
 */
inline Vec3 evaluateBezierTangent(const std::vector<Vec3>& controlPoints, float t) {
    int n = static_cast<int>(controlPoints.size()) - 1;
    if (n < 1) return Vec3(0.0f, 0.0f, 1.0f);
    return evaluateBezierDerivativeRaw(controlPoints, t).normalized();
}

/**
 * @brief Evaluate Bezier second derivative vector P''(t) = n*(n-1) * sum_{k=0..n-2} (P_{k+2} - 2P_{k+1} + P_k) * B_k^{n-2}(t)
 */
inline Vec3 evaluateBezierSecondDerivative(const std::vector<Vec3>& controlPoints, float t) {
    int n = static_cast<int>(controlPoints.size()) - 1;
    if (n < 2) return Vec3(0.0f, 0.0f, 0.0f);
    Vec3 secondDeriv(0.0f, 0.0f, 0.0f);
    for (int k = 0; k <= n - 2; ++k) {
        Vec3 diff2 = controlPoints[k + 2] - controlPoints[k + 1] * 2.0f + controlPoints[k];
        secondDeriv += diff2 * bernsteinBasis(n - 2, k, t);
    }
    return secondDeriv * static_cast<float>(n * (n - 1));
}

} // namespace MathCG

#endif // MATH_CG_H
