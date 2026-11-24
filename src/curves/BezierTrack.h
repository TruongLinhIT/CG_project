#ifndef BEZIER_TRACK_H
#define BEZIER_TRACK_H

/**
 * @file BezierTrack.h
 * @brief Chương 5: Bộ tạo Đường ray Bezier Bậc 3 Từng đoạn Liên tục C^1, Khung Tiếp tuyến & Đường ray 3D
 *
 * Cài đặt:
 *  - Các đoạn cong Bezier bậc 3 ghép nối P_0..P_3 được tính thông qua đa thức cơ sở Bernstein B_k^3(u)
 *  - Ràng buộc liên tục khép kín tường minh C^0 (P_3^(i) = P_0^(i+1)) và C^1 (P_1^(i+1) = 2*P_3^(i) - P_2^(i))
 *  - Đạo hàm bậc nhất P'(t) & đạo hàm bậc hai P''(t) để tính góc hướng (Yaw), góc chúc/ngóc (Pitch), và góc nghiêng theo độ cong (Roll / banking)
 *  - Bảng tham số hóa theo độ dài cung (Arc-length parameterization) giúp đặt các toa tàu cách đều nhau chính xác
 *  - Hình học đường ray 3D sinh tự động (hai thanh ray đôi, ống xương sống trung tâm / dầm hộp monorail, thanh tà-vẹt ngang, trụ đỡ)
 *  - Hiển thị trực quan hóa phục vụ kiểm tra (Debug): Đa giác điều khiển Bezier P_0..P_3, các vector tiếp tuyến P'(t) và pháp tuyến hướng lên
 */

#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/MathCG.h"
#include "core/ParkLayout.h"
#include "core/Lighting.h"

struct CubicBezierSegment {
    MathCG::Vec3 p0;
    MathCG::Vec3 p1;
    MathCG::Vec3 p2;
    MathCG::Vec3 p3;
};

struct TrackFrame {
    MathCG::Vec3 position;
    MathCG::Vec3 tangent;  // Vector đơn vị hướng tiến về trước (+Z cục bộ)
    MathCG::Vec3 right;    // Vector đơn vị hướng ngang sang phải (+X cục bộ)
    MathCG::Vec3 up;       // Vector pháp tuyến đơn vị hướng lên (+Y cục bộ)
    float yawDeg   = 0.0f; // Góc quay quanh trục Y thế giới: atan2(Tx, Tz)
    float pitchDeg = 0.0f; // Góc quay quanh trục X cục bộ: -asin(Ty)
    float rollDeg  = 0.0f; // Góc nghiêng (banking) quanh trục Z cục bộ
};

struct TrackConfig {
    float trackGauge        = 1.80f;
    float railRadius        = 0.14f;
    float spineRadius       = 0.36f;
    float spineDropOffset   = 0.72f;
    float tieSpacing        = 2.35f;
    float pillarSpacing     = 14.5f;
    float pillarRadius      = 0.45f;
    int   samplesPerSegment = 28;
    int   railRadialSteps   = 8;
    float maxBankDeg        = 32.0f;
    float bankScale         = 11.0f;
    bool  isMonorailBeam    = false;

    MathCG::ColorRGB railColor   = MathCG::ColorRGB(0.92f, 0.28f, 0.16f);
    MathCG::ColorRGB spineColor  = MathCG::ColorRGB(0.95f, 0.74f, 0.15f);
    MathCG::ColorRGB tieColor    = MathCG::ColorRGB(0.26f, 0.28f, 0.34f);
    MathCG::ColorRGB pillarColor = MathCG::ColorRGB(0.72f, 0.74f, 0.78f);
};

class BezierTrack {
public:
    TrackConfig                     config;
    std::vector<CubicBezierSegment> segments;
    std::vector<TrackFrame>         samples;
    std::vector<float>              cumulativeArcLen; // kích thước == samples.size() + 1 (bao gồm cạnh khép kín vòng)
    float                           totalLength = 0.0f;

    BezierTrack() = default;

    /**
     * @brief Xây dựng đường ray Bezier bậc 3 từng đoạn khép kín, liên tục C^0 và C^1
     *        từ các điểm mốc điều khiển (waypoints) sử dụng tiếp tuyến Cardinal:
     *          V_i = tension * (W_{i+1} - W_{i-1})
     *          P_0^(i) = W_i, P_3^(i) = W_{i+1}
     *          P_1^(i) = W_i + V_i / 3
     *          P_2^(i) = W_{i+1} - V_{i+1} / 3
     *        Đảm bảo P_3^(i) == P_0^(i+1) và P_1^(i+1) == 2*P_3^(i) - P_2^(i).
     */
    void buildFromClosedWaypoints(
        const std::vector<MathCG::Vec3>& waypoints,
        float tension = 0.45f,
        const TrackConfig& cfg = TrackConfig()
    ) {
        config = cfg;
        segments.clear();
        const int n = static_cast<int>(waypoints.size());
        if (n < 3) return;

        std::vector<MathCG::Vec3> tangents(n);
        for (int i = 0; i < n; ++i) {
            const MathCG::Vec3& prevW = waypoints[(i - 1 + n) % n];
            const MathCG::Vec3& curW  = waypoints[i];
            const MathCG::Vec3& nextW = waypoints[(i + 1) % n];
            MathCG::Vec3 rawT = (nextW - prevW) * tension;

            // Với tuyến Monorail có chênh lệch độ dài các nhịp lớn (28m .. 177m),
            // kẹp độ dài tay đòn tiếp tuyến theo nhịp ngắn hơn ở hai bên đỉnh i để khử hiện tượng
            // vòng xoắn thừa (overshoot) ở khúc cua hẹp, trong khi vẫn giữ cùng 1 vector tiếp tuyến
            // cho cả nhịp vào và nhịp ra => bảo toàn tuyệt đối tính liên tục C^1: P_1^(i) = 2*P_3^(i-1) - P_2^(i-1).
            if (cfg.isMonorailBeam) {
                float dPrev = (curW - prevW).length();
                float dNext = (nextW - curW).length();
                float maxTangentLen = 1.08f * std::min(dPrev, dNext); // tương ứng tay đòn P1-P0 <= 0.36 * min(dPrev, dNext)
                float rawLen = rawT.length();
                if (rawLen > maxTangentLen && rawLen > 1e-6f) {
                    rawT = rawT * (maxTangentLen / rawLen);
                }
            }
            tangents[i] = rawT;
        }

        segments.reserve(n);
        for (int i = 0; i < n; ++i) {
            int nextIdx = (i + 1) % n;
            CubicBezierSegment seg;
            seg.p0 = waypoints[i];
            seg.p3 = waypoints[nextIdx];
            seg.p1 = seg.p0 + tangents[i] * (1.0f / 3.0f);
            seg.p2 = seg.p3 - tangents[nextIdx] * (1.0f / 3.0f);
            segments.push_back(seg);
        }

        rebuildLookupTables();
    }

    /**
     * @brief Xây dựng đường ray từ danh sách các đoạn cong Bezier bậc 3 tường minh.
     */
    void buildFromSegments(
        const std::vector<CubicBezierSegment>& segs,
        const TrackConfig& cfg = TrackConfig()
    ) {
        config = cfg;
        segments = segs;
        rebuildLookupTables();
    }

    /**
     * @brief Kiểm tra sai số cực đại của tính liên tục C^0: ||P_3^(i) - P_0^(i+1)||
     */
    float verifyC0Continuity() const {
        const int n = static_cast<int>(segments.size());
        if (n < 2) return 0.0f;
        float maxErr = 0.0f;
        for (int i = 0; i < n; ++i) {
            int nextIdx = (i + 1) % n;
            maxErr = std::max(maxErr, (segments[i].p3 - segments[nextIdx].p0).length());
        }
        return maxErr;
    }

    /**
     * @brief Kiểm tra sai số cực đại của tính liên tục C^1: ||P_1^(i+1) - (2*P_3^(i) - P_2^(i))||
     */
    float verifyC1Continuity() const {
        const int n = static_cast<int>(segments.size());
        if (n < 2) return 0.0f;
        float maxErr = 0.0f;
        for (int i = 0; i < n; ++i) {
            int nextIdx = (i + 1) % n;
            MathCG::Vec3 expectedP1 = segments[i].p3 * 2.0f - segments[i].p2;
            maxErr = std::max(maxErr, (segments[nextIdx].p1 - expectedP1).length());
        }
        return maxErr;
    }

    float getTotalLength() const { return totalLength; }

    static float wrap01(float t) {
        float w = std::fmod(t, 1.0f);
        if (w < 0.0f) w += 1.0f;
        if (w >= 1.0f) w = 0.0f;
        return w;
    }

    float wrapDistance(float distMeters) const {
        if (totalLength <= 1e-5f) return 0.0f;
        float w = std::fmod(distMeters, totalLength);
        if (w < 0.0f) w += totalLength;
        if (w >= totalLength) w = 0.0f;
        return w;
    }

    /**
     * @brief Tính vị trí điểm P(t) trên đường cong tại tham số toàn cục t thuộc [0, 1)
     *        sử dụng đa thức cơ sở Bernstein B_k^3(u) của Chương 5.
     */
    MathCG::Vec3 evaluatePoint(float t) const {
        if (segments.empty()) return MathCG::Vec3();
        int segIdx = 0;
        float u = 0.0f;
        mapGlobalParam(t, segIdx, u);
        const CubicBezierSegment& s = segments[segIdx];
        return MathCG::evaluateBezierPoint({ s.p0, s.p1, s.p2, s.p3 }, u);
    }

    /**
     * @brief Tính vector đạo hàm bậc nhất chưa chuẩn hóa P'(t_local) tại tham số toàn cục t thuộc [0, 1).
     */
    MathCG::Vec3 evaluateDerivativeRaw(float t) const {
        if (segments.empty()) return MathCG::Vec3(0.0f, 0.0f, 1.0f);
        int segIdx = 0;
        float u = 0.0f;
        mapGlobalParam(t, segIdx, u);
        const CubicBezierSegment& s = segments[segIdx];
        return MathCG::evaluateBezierDerivativeRaw({ s.p0, s.p1, s.p2, s.p3 }, u);
    }

    /**
     * @brief Tính vector tiếp tuyến đơn vị T(t) = P'(t) / ||P'(t)|| tại tham số toàn cục t thuộc [0, 1).
     */
    MathCG::Vec3 evaluateTangent(float t) const {
        return evaluateDerivativeRaw(t).normalized();
    }

    /**
     * @brief Tính khung định hướng 3D đầy đủ (Vị trí, Tiếp tuyến, Phải, Lên, Yaw, Pitch, Roll)
     *        tại tham số toàn cục t thuộc [0, 1).
     */
    TrackFrame evaluateFrameAtParamT(float t) const {
        TrackFrame frame;
        if (segments.empty() || samples.empty()) return frame;

        float wt = wrap01(t);
        frame.position = evaluatePoint(wt);
        frame.tangent  = evaluateTangent(wt);

        // Nội suy góc nghiêng (banking) đã làm mượt từ vòng mẫu khép kín tính sẵn
        const int numSamples = static_cast<int>(samples.size());
        float fIdx = wt * static_cast<float>(numSamples);
        int idx0   = static_cast<int>(std::floor(fIdx)) % numSamples;
        int idx1   = (idx0 + 1) % numSamples;
        float frac = fIdx - std::floor(fIdx);

        frame.rollDeg = samples[idx0].rollDeg + (samples[idx1].rollDeg - samples[idx0].rollDeg) * frac;
        computeFrameAxesFromTangentAndRoll(frame.tangent, frame.rollDeg, frame);
        return frame;
    }

    /**
     * @brief Chuyển đổi khoảng cách độ dài cung (mét) dọc theo vòng khép kín sang tham số t thuộc [0, 1).
     */
    float distanceToParamT(float distMeters) const {
        const int numSamples = static_cast<int>(samples.size());
        if (numSamples == 0 || totalLength <= 1e-5f) return 0.0f;

        float d = wrapDistance(distMeters);
        auto it = std::upper_bound(cumulativeArcLen.begin(), cumulativeArcLen.end(), d);
        int idx1 = static_cast<int>(it - cumulativeArcLen.begin());
        int idx0 = std::max(0, idx1 - 1);
        idx1 = std::min(idx1, numSamples);

        float len0 = cumulativeArcLen[idx0];
        float len1 = cumulativeArcLen[idx1];
        float span = len1 - len0;
        float frac = (span > 1e-6f) ? (d - len0) / span : 0.0f;

        float t0 = static_cast<float>(idx0) / static_cast<float>(numSamples);
        float t1 = static_cast<float>(idx1) / static_cast<float>(numSamples);
        return wrap01(t0 + (t1 - t0) * frac);
    }

    /**
     * @brief Chuyển đổi tham số t thuộc [0, 1) sang khoảng cách độ dài cung (mét).
     */
    float paramTToDistance(float t) const {
        const int numSamples = static_cast<int>(samples.size());
        if (numSamples == 0 || totalLength <= 1e-5f) return 0.0f;
        float wt = wrap01(t);
        float fIdx = wt * static_cast<float>(numSamples);
        int idx0 = static_cast<int>(std::floor(fIdx)) % numSamples;
        int idx1 = idx0 + 1;
        float frac = fIdx - std::floor(fIdx);
        return cumulativeArcLen[idx0] + (cumulativeArcLen[idx1] - cumulativeArcLen[idx0]) * frac;
    }

    /**
     * @brief Tính khung tọa độ TrackFrame đầy đủ tại khoảng cách độ dài cung vật lý dọc theo vòng khép kín.
     */
    TrackFrame evaluateFrameAtDistance(float distMeters) const {
        return evaluateFrameAtParamT(distanceToParamT(distMeters));
    }

    /**
     * @brief Trả về danh sách các điểm lấy mẫu dọc tâm đường ray 3D (hữu ích cho Minimap 2D WCS->NCS->DCS).
     */
    std::vector<MathCG::Vec3> getSampledPoints() const {
        std::vector<MathCG::Vec3> pts;
        pts.reserve(samples.size());
        for (const auto& s : samples) {
            pts.push_back(s.position);
        }
        return pts;
    }

    /**
     * @brief Vẽ toàn bộ đường ray 3D: thanh ray, ống xương sống / dầm monorail, tà-vẹt ngang, trụ đỡ,
     *        và tùy chọn hiển thị kiểm tra đa giác điều khiển Bezier & vector tiếp tuyến.
     */
    void draw(bool showDebugCtrlAndTangents, bool lightingEnabled) const {
        if (samples.size() < 3) return;

        // 1. Hệ thống cột trụ đỡ nối xuống mặt đất y = 0 (và dàn cầu vượt nếu bắc qua mặt hồ)
        drawSupportPillars();
        if (config.isMonorailBeam) {
            drawLakeOverpassTruss();
        }

        // 2. Các thanh tà-vẹt ngang hoặc đai đỡ dầm
        drawCrossTies();

        // 3. Ống thép xương sống (Queen Cobra) hoặc dầm hộp bê tông (Monorail)
        if (config.isMonorailBeam) {
            drawMonorailBoxGirder();
            // Hai thanh ray dẫn hướng / tiếp điện bên hông dầm bê tông
            LightingSystem::setMaterial(0.65f, 48.0f);
            glColor3f(config.railColor.r, config.railColor.g, config.railColor.b);
            drawClosedTubeRail(-config.trackGauge * 0.5f, -0.22f, config.railRadius);
            drawClosedTubeRail( config.trackGauge * 0.5f, -0.22f, config.railRadius);
            LightingSystem::resetMaterial();
        } else {
            // Ống thép xương sống chịu lực ở chính giữa
            LightingSystem::setMaterial(0.45f, 36.0f);
            glColor3f(config.spineColor.r, config.spineColor.g, config.spineColor.b);
            drawClosedTubeRail(0.0f, -config.spineDropOffset, config.spineRadius);

            // Hai thanh ray chạy tàu hai bên với độ bóng kim loại cao hơn
            LightingSystem::setMaterial(0.75f, 64.0f);
            glColor3f(config.railColor.r, config.railColor.g, config.railColor.b);
            drawClosedTubeRail(-config.trackGauge * 0.5f, 0.0f, config.railRadius);
            drawClosedTubeRail( config.trackGauge * 0.5f, 0.0f, config.railRadius);
            LightingSystem::resetMaterial();
        }

        // 4. Hiển thị trực quan hóa kiểm tra (Debug): Đa giác điều khiển Bezier P_0..P_3 và vector tiếp tuyến P'(t)
        if (showDebugCtrlAndTangents) {
            drawDebugOverlay(lightingEnabled);
        }
    }

private:
    void mapGlobalParam(float t, int& segIdx, float& localU) const {
        const int numSegs = static_cast<int>(segments.size());
        float wt = wrap01(t);
        float scaled = wt * static_cast<float>(numSegs);
        segIdx = static_cast<int>(std::floor(scaled));
        if (segIdx >= numSegs) segIdx = numSegs - 1;
        localU = std::clamp(scaled - static_cast<float>(segIdx), 0.0f, 1.0f);
    }

    static void computeFrameAxesFromTangentAndRoll(
        const MathCG::Vec3& unitTangent,
        float rollDeg,
        TrackFrame& outFrame
    ) {
        outFrame.tangent  = unitTangent.normalized();
        outFrame.yawDeg   = std::atan2(outFrame.tangent.x, outFrame.tangent.z) * MathCG::RAD2DEG;
        outFrame.pitchDeg = -std::asin(std::clamp(outFrame.tangent.y, -1.0f, 1.0f)) * MathCG::RAD2DEG;
        outFrame.rollDeg  = rollDeg;

        // Xây dựng hệ cơ sở trực chuẩn tương đương với glRotatef(yaw,0,1,0) * glRotatef(pitch,1,0,0) * glRotatef(roll,0,0,1)
        MathCG::Mat4 rot = MathCG::Mat4::rotateY(outFrame.yawDeg)
                         * MathCG::Mat4::rotateX(outFrame.pitchDeg)
                         * MathCG::Mat4::rotateZ(outFrame.rollDeg);

        outFrame.right   = rot.transformVector(MathCG::Vec3(1.0f, 0.0f, 0.0f)).normalized();
        outFrame.up      = rot.transformVector(MathCG::Vec3(0.0f, 1.0f, 0.0f)).normalized();
        outFrame.tangent = rot.transformVector(MathCG::Vec3(0.0f, 0.0f, 1.0f)).normalized();
    }

    void rebuildLookupTables() {
        samples.clear();
        cumulativeArcLen.clear();
        totalLength = 0.0f;

        const int numSegs = static_cast<int>(segments.size());
        if (numSegs == 0) return;

        const int totalSamples = numSegs * std::max(8, config.samplesPerSegment);
        samples.resize(totalSamples);
        std::vector<float> rawRoll(totalSamples, 0.0f);

        for (int i = 0; i < totalSamples; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(totalSamples);
            int segIdx = 0;
            float u = 0.0f;
            mapGlobalParam(t, segIdx, u);
            const CubicBezierSegment& s = segments[segIdx];
            std::vector<MathCG::Vec3> ctrl = { s.p0, s.p1, s.p2, s.p3 };

            MathCG::Vec3 pos  = MathCG::evaluateBezierPoint(ctrl, u);
            MathCG::Vec3 d1   = MathCG::evaluateBezierDerivativeRaw(ctrl, u);
            MathCG::Vec3 d2   = MathCG::evaluateBezierSecondDerivative(ctrl, u);
            MathCG::Vec3 tang = d1.normalized();

            samples[i].position = pos;
            samples[i].tangent  = tang;

            // Gia tốc hướng tâm ngang có dấu -> góc nghiêng tự nhiên của tàu lượn khi vào cua
            MathCG::Vec3 flatRight(tang.z, 0.0f, -tang.x);
            if (flatRight.lengthSquared() < 1e-6f) {
                flatRight = MathCG::Vec3(1.0f, 0.0f, 0.0f);
            } else {
                flatRight = flatRight.normalized();
            }

            float speedSq = std::max(1.0f, d1.lengthSquared());
            float latKappa = MathCG::dot(d2, flatRight) / speedSq;
            // Dấu âm làm nghiêng trục +Y cục bộ hướng vào phía tâm khúc cua
            float bank = -std::atan(latKappa * config.bankScale) * MathCG::RAD2DEG;
            rawRoll[i] = std::clamp(bank, -config.maxBankDeg, config.maxBankDeg);
        }

        // Làm mượt tuần hoàn trên vòng mẫu để góc nghiêng Roll chuyển tiếp êm ái qua các điểm nối C^1
        std::vector<float> smoothRoll = rawRoll;
        std::vector<float> nextRoll(totalSamples, 0.0f);
        for (int pass = 0; pass < 6; ++pass) {
            for (int i = 0; i < totalSamples; ++i) {
                int prev = (i - 1 + totalSamples) % totalSamples;
                int next = (i + 1) % totalSamples;
                nextRoll[i] = 0.25f * smoothRoll[prev] + 0.50f * smoothRoll[i] + 0.25f * smoothRoll[next];
            }
            smoothRoll.swap(nextRoll);
        }

        for (int i = 0; i < totalSamples; ++i) {
            computeFrameAxesFromTangentAndRoll(samples[i].tangent, smoothRoll[i], samples[i]);
        }

        // Xây dựng bảng cộng dồn độ dài cung (cumulative arc-length) cho vòng khép kín
        cumulativeArcLen.resize(totalSamples + 1, 0.0f);
        for (int i = 0; i < totalSamples; ++i) {
            int next = (i + 1) % totalSamples;
            float segLen = (samples[next].position - samples[i].position).length();
            cumulativeArcLen[i + 1] = cumulativeArcLen[i] + segLen;
        }
        totalLength = cumulativeArcLen[totalSamples];
    }

    // ------------------------------------------------------------------------
    // Các hàm Bổ trợ Hình học Sinh tự động (Chỉ dùng Đỉnh & Pháp tuyến Tự tính)
    // ------------------------------------------------------------------------

    static void drawOrientedQuadFace(
        const MathCG::Vec3& v0,
        const MathCG::Vec3& v1,
        const MathCG::Vec3& v2,
        const MathCG::Vec3& v3
    ) {
        MathCG::Vec3 n = MathCG::computeFaceNormal(v0, v1, v2);
        glNormal3f(n.x, n.y, n.z);
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }

    static void emitOrientedBoxFaces(
        const MathCG::Vec3& center,
        const MathCG::Vec3& right,
        const MathCG::Vec3& up,
        const MathCG::Vec3& fwd,
        float hx,
        float hy,
        float hz
    ) {
        MathCG::Vec3 rx = right * hx;
        MathCG::Vec3 uy = up    * hy;
        MathCG::Vec3 fz = fwd   * hz;

        MathCG::Vec3 p000 = center - rx - uy - fz;
        MathCG::Vec3 p100 = center + rx - uy - fz;
        MathCG::Vec3 p110 = center + rx + uy - fz;
        MathCG::Vec3 p010 = center - rx + uy - fz;

        MathCG::Vec3 p001 = center - rx - uy + fz;
        MathCG::Vec3 p101 = center + rx - uy + fz;
        MathCG::Vec3 p111 = center + rx + uy + fz;
        MathCG::Vec3 p011 = center - rx + uy + fz;

        drawOrientedQuadFace(p001, p101, p111, p011); // +Z (Mặt trước)
        drawOrientedQuadFace(p100, p000, p010, p110); // -Z (Mặt sau)
        drawOrientedQuadFace(p101, p100, p110, p111); // +X (Mặt phải)
        drawOrientedQuadFace(p000, p001, p011, p010); // -X (Mặt trái)
        drawOrientedQuadFace(p011, p111, p110, p010); // +Y (Mặt trên)
        drawOrientedQuadFace(p000, p100, p101, p001); // -Y (Mặt dưới)
    }

    static void drawOrientedBox(
        const MathCG::Vec3& center,
        const MathCG::Vec3& right,
        const MathCG::Vec3& up,
        const MathCG::Vec3& fwd,
        float hx,
        float hy,
        float hz
    ) {
        glBegin(GL_QUADS);
        emitOrientedBoxFaces(center, right, up, fwd, hx, hy, hz);
        glEnd();
    }

    void drawClosedTubeRail(float offsetRight, float offsetUp, float radius) const {
        const int n      = static_cast<int>(samples.size());
        const int radial = std::max(6, config.railRadialSteps);
        if (n < 2) return;

        std::vector<float> cosTbl(radial + 1);
        std::vector<float> sinTbl(radial + 1);
        for (int j = 0; j <= radial; ++j) {
            float theta = (static_cast<float>(j % radial) / static_cast<float>(radial)) * MathCG::TWO_PI;
            cosTbl[j] = std::cos(theta);
            sinTbl[j] = std::sin(theta);
        }

        // Giữ nguyên GL_QUAD_STRIP từng đoạn ray để bảo toàn chính xác từng tam giác & 0% sai khác pixel (với bảng cos/sin dựng sẵn)
        for (int i = 0; i < n; ++i) {
            const TrackFrame& f0 = samples[i];
            const TrackFrame& f1 = samples[(i + 1) % n];

            MathCG::Vec3 c0 = f0.position + f0.right * offsetRight + f0.up * offsetUp;
            MathCG::Vec3 c1 = f1.position + f1.right * offsetRight + f1.up * offsetUp;

            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= radial; ++j) {
                float ct = cosTbl[j];
                float st = sinTbl[j];

                MathCG::Vec3 n1 = (f1.right * ct + f1.up * st).normalized();
                MathCG::Vec3 p1 = c1 + n1 * radius;
                glNormal3f(n1.x, n1.y, n1.z);
                glVertex3f(p1.x, p1.y, p1.z);

                MathCG::Vec3 n0 = (f0.right * ct + f0.up * st).normalized();
                MathCG::Vec3 p0 = c0 + n0 * radius;
                glNormal3f(n0.x, n0.y, n0.z);
                glVertex3f(p0.x, p0.y, p0.z);
            }
            glEnd();
        }
    }

    void drawMonorailBoxGirder() const {
        const int n = static_cast<int>(samples.size());
        const float halfWTop = 0.56f;
        const float halfWBot = 0.44f;
        const float yTop     =  0.04f;
        const float yBot     = -1.08f;

        glColor3f(config.spineColor.r, config.spineColor.g, config.spineColor.b);

        glBegin(GL_QUADS);
        for (int i = 0; i < n; ++i) {
            const TrackFrame& f0 = samples[i];
            const TrackFrame& f1 = samples[(i + 1) % n];

            // Các góc tiết diện ngang ngược chiều kim đồng hồ (CCW) trong mặt phẳng (right, up): TR -> TL -> BL -> BR
            MathCG::Vec3 cs0[4] = {
                f0.position + f0.right *  halfWTop + f0.up * yTop,
                f0.position + f0.right * -halfWTop + f0.up * yTop,
                f0.position + f0.right * -halfWBot + f0.up * yBot,
                f0.position + f0.right *  halfWBot + f0.up * yBot
            };
            MathCG::Vec3 cs1[4] = {
                f1.position + f1.right *  halfWTop + f1.up * yTop,
                f1.position + f1.right * -halfWTop + f1.up * yTop,
                f1.position + f1.right * -halfWBot + f1.up * yBot,
                f1.position + f1.right *  halfWBot + f1.up * yBot
            };

            for (int side = 0; side < 4; ++side) {
                int nextSide = (side + 1) % 4;
                // Tích có hướng (cs0[next] - cs0[side]) x (cs1[side] - cs0[side]) hướng ra phía ngoài!
                drawOrientedQuadFace(cs0[side], cs0[nextSide], cs1[nextSide], cs1[side]);
            }
        }
        glEnd();
    }

    void drawCrossTies() const {
        if (totalLength <= 1e-4f || config.tieSpacing <= 0.1f) return;

        int numTies = std::max(8, static_cast<int>(std::round(totalLength / config.tieSpacing)));
        float step  = totalLength / static_cast<float>(numTies);

        glColor3f(config.tieColor.r, config.tieColor.g, config.tieColor.b);

        glBegin(GL_QUADS);
        for (int i = 0; i < numTies; ++i) {
            TrackFrame f = evaluateFrameAtDistance(i * step);

            if (config.isMonorailBeam) {
                // Đai vòng gá đỡ gọn gàng bao quanh dầm bê tông
                emitOrientedBoxFaces(
                    f.position - f.up * 0.52f,
                    f.right, f.up, f.tangent,
                    0.62f, 0.56f, 0.14f
                );
            } else {
                // Thanh tà-vẹt ngang liên kết thanh ray trái và phải
                emitOrientedBoxFaces(
                    f.position - f.up * 0.12f,
                    f.right, f.up, f.tangent,
                    config.trackGauge * 0.54f, 0.08f, 0.12f
                );
                // Thanh chống cổ đứng nối từ thanh ngang xuống ống thép xương sống trung tâm
                emitOrientedBoxFaces(
                    f.position - f.up * (config.spineDropOffset * 0.52f),
                    f.right, f.up, f.tangent,
                    0.12f, config.spineDropOffset * 0.45f, 0.11f
                );
            }
        }
        glEnd();
    }

    static void drawParametricVerticalCylinder(
        float cx,
        float cz,
        float yBottom,
        float yTop,
        float rBottom,
        float rTop,
        int slices
    ) {
        if (yTop <= yBottom + 0.05f) return;

        float dy = yTop - yBottom;
        float dr = rBottom - rTop;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float u  = (static_cast<float>(i % slices) / static_cast<float>(slices)) * MathCG::TWO_PI;
            float cu = std::cos(u);
            float su = std::sin(u);

            MathCG::Vec3 n(cu * dy, dr, su * dy);
            n = n.normalized();
            glNormal3f(n.x, n.y, n.z);

            // Đỉnh dưới trước, đỉnh trên sau -> tạo pháp tuyến CCW hướng ra ngoài quanh trục +Y
            glVertex3f(cx + rBottom * cu, yBottom, cz + rBottom * su);
            glVertex3f(cx + rTop    * cu, yTop,    cz + rTop    * su);
        }
        glEnd();
    }

    void drawSupportPillars() const {
        if (totalLength <= 1e-4f || config.pillarSpacing <= 1.0f) return;

        int numPillars = std::max(4, static_cast<int>(std::round(totalLength / config.pillarSpacing)));
        float step     = totalLength / static_cast<float>(numPillars);

        const MathCG::Vec3 worldX(1.0f, 0.0f, 0.0f);
        const MathCG::Vec3 worldY(0.0f, 1.0f, 0.0f);
        const MathCG::Vec3 worldZ(0.0f, 0.0f, 1.0f);

        std::vector<TrackFrame> pillarFrames;
        pillarFrames.reserve(numPillars);
        for (int i = 0; i < numPillars; ++i) {
            TrackFrame f = evaluateFrameAtDistance(i * step);
            float attachOffset = config.isMonorailBeam ? 1.05f : config.spineDropOffset;
            MathCG::Vec3 attachPt = f.position - f.up * attachOffset;
            if (attachPt.y < 1.2f) continue;
            if (config.isMonorailBeam && ParkLayout::isPointInLake(attachPt.x, attachPt.z)) continue;
            pillarFrames.push_back(f);
        }
        if (pillarFrames.empty()) return;

        // Gom toàn bộ bệ móng bê tông & gối đỡ đỉnh cột vào 1 batch GL_QUADS (giữ nguyên đỉnh/màu)
        glColor3f(0.45f, 0.47f, 0.50f);
        glBegin(GL_QUADS);
        for (const auto& f : pillarFrames) {
            float attachOffset = config.isMonorailBeam ? 1.05f : config.spineDropOffset;
            MathCG::Vec3 attachPt = f.position - f.up * attachOffset;
            emitOrientedBoxFaces(
                MathCG::Vec3(attachPt.x, 0.35f, attachPt.z),
                worldX, worldY, worldZ,
                config.pillarRadius * 2.1f, 0.35f, config.pillarRadius * 2.1f
            );
        }
        glEnd();

        glColor3f(config.tieColor.r, config.tieColor.g, config.tieColor.b);
        glBegin(GL_QUADS);
        for (const auto& f : pillarFrames) {
            float attachOffset = config.isMonorailBeam ? 1.05f : config.spineDropOffset;
            MathCG::Vec3 attachPt = f.position - f.up * attachOffset;
            emitOrientedBoxFaces(
                attachPt,
                f.right, f.up, f.tangent,
                config.pillarRadius * 1.25f, 0.25f, config.pillarRadius * 1.25f
            );
        }
        glEnd();

        glColor3f(config.pillarColor.r, config.pillarColor.g, config.pillarColor.b);
        for (const auto& f : pillarFrames) {
            float attachOffset = config.isMonorailBeam ? 1.05f : config.spineDropOffset;
            MathCG::Vec3 attachPt = f.position - f.up * attachOffset;
            drawParametricVerticalCylinder(
                attachPt.x, attachPt.z,
                0.65f, attachPt.y,
                config.pillarRadius * 1.18f, config.pillarRadius * 0.90f,
                10
            );
        }
    }

    /**
     * @brief Vẽ kết cấu dàn cầu thép chịu lực phía dưới đoạn dầm Monorail vượt qua mặt hồ
     *        và 2 trụ mố cầu đặt ngay trên bờ Tây & bờ Đông của hồ (không có chân trụ nào cắm xuống lòng hồ).
     */
    void drawLakeOverpassTruss() const {
        const int n = static_cast<int>(samples.size());
        if (n < 3) return;

        const MathCG::Vec3 worldX(1.0f, 0.0f, 0.0f);
        const MathCG::Vec3 worldY(0.0f, 1.0f, 0.0f);
        const MathCG::Vec3 worldZ(0.0f, 0.0f, 1.0f);

        auto drawBankAbutmentPillar = [&](const TrackFrame& f) {
            MathCG::Vec3 attachPt = f.position - f.up * 1.05f;
            if (attachPt.y < 1.2f) return;

            glColor3f(0.45f, 0.47f, 0.50f);
            drawOrientedBox(
                MathCG::Vec3(attachPt.x, 0.45f, attachPt.z),
                worldX, worldY, worldZ,
                config.pillarRadius * 2.6f, 0.45f, config.pillarRadius * 2.6f
            );
            glColor3f(config.pillarColor.r, config.pillarColor.g, config.pillarColor.b);
            drawParametricVerticalCylinder(
                attachPt.x, attachPt.z,
                0.85f, attachPt.y,
                config.pillarRadius * 1.45f, config.pillarRadius * 1.15f,
                12
            );
            glColor3f(config.tieColor.r, config.tieColor.g, config.tieColor.b);
            drawOrientedBox(
                attachPt,
                f.right, f.up, f.tangent,
                config.pillarRadius * 1.6f, 0.35f, config.pillarRadius * 1.6f
            );
        };

        glColor3f(0.50f, 0.54f, 0.60f);
        glBegin(GL_QUADS);
        for (int i = 0; i < n; ++i) {
            const TrackFrame& f0 = samples[i];
            const TrackFrame& f1 = samples[(i + 1) % n];
            bool in0 = ParkLayout::isPointInLake(f0.position.x, f0.position.z);
            bool in1 = ParkLayout::isPointInLake(f1.position.x, f1.position.z);

            if (in0 || in1) {
                MathCG::Vec3 mid = (f0.position + f1.position) * 0.5f - f0.up * 1.85f;
                float segLen = (f1.position - f0.position).length();
                emitOrientedBoxFaces(
                    mid,
                    f0.right, f0.up, f0.tangent,
                    1.25f, 0.55f, segLen * 0.52f
                );
            }
        }
        glEnd();

        for (int i = 0; i < n; ++i) {
            const TrackFrame& f0 = samples[i];
            const TrackFrame& f1 = samples[(i + 1) % n];
            bool in0 = ParkLayout::isPointInLake(f0.position.x, f0.position.z);
            bool in1 = ParkLayout::isPointInLake(f1.position.x, f1.position.z);

            if (!in0 && in1) {
                drawBankAbutmentPillar(f0);
            } else if (in0 && !in1) {
                drawBankAbutmentPillar(f1);
            }
        }
    }

    void drawDebugOverlay(bool lightingEnabled) const {
        glDisable(GL_LIGHTING);

        // 1. Đa giác điều khiển Bezier P_0 -> P_1 -> P_2 -> P_3 của từng đoạn
        glLineWidth(1.5f);
        glColor3f(1.0f, 0.95f, 0.20f);
        for (const auto& seg : segments) {
            glBegin(GL_LINE_STRIP);
            glVertex3f(seg.p0.x, seg.p0.y, seg.p0.z);
            glVertex3f(seg.p1.x, seg.p1.y, seg.p1.z);
            glVertex3f(seg.p2.x, seg.p2.y, seg.p2.z);
            glVertex3f(seg.p3.x, seg.p3.y, seg.p3.z);
            glEnd();
        }

        // 2. Các điểm mốc neo P_0 (xanh lá) & các điểm điều khiển tiếp tuyến C^1 P_1, P_2 (vàng)
        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (const auto& seg : segments) {
            glColor3f(0.15f, 1.0f, 0.35f);
            glVertex3f(seg.p0.x, seg.p0.y, seg.p0.z);

            glColor3f(1.0f, 0.85f, 0.15f);
            glVertex3f(seg.p1.x, seg.p1.y, seg.p1.z);
            glVertex3f(seg.p2.x, seg.p2.y, seg.p2.z);
        }
        glEnd();

        // 3. Các vector tiếp tuyến lấy mẫu P'(t) (xanh lơ) và vector hướng lên Up (hồng tím)
        glLineWidth(1.8f);
        glBegin(GL_LINES);
        const int stride = std::max(1, static_cast<int>(samples.size()) / 40);
        for (size_t i = 0; i < samples.size(); i += static_cast<size_t>(stride)) {
            const TrackFrame& f = samples[i];
            MathCG::Vec3 tEnd = f.position + f.tangent * 3.8f;
            MathCG::Vec3 uEnd = f.position + f.up      * 2.6f;

            glColor3f(0.15f, 0.95f, 1.0f);
            glVertex3f(f.position.x, f.position.y, f.position.z);
            glVertex3f(tEnd.x, tEnd.y, tEnd.z);

            glColor3f(1.0f, 0.25f, 0.85f);
            glVertex3f(f.position.x, f.position.y, f.position.z);
            glVertex3f(uEnd.x, uEnd.y, uEnd.z);
        }
        glEnd();

        glLineWidth(1.0f);
        glPointSize(1.0f);
        if (lightingEnabled) glEnable(GL_LIGHTING);
    }
};

#endif // BEZIER_TRACK_H
