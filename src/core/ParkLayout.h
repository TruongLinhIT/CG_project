#ifndef PARK_LAYOUT_H
#define PARK_LAYOUT_H

/**
 * @file ParkLayout.h
 * @brief Bảng hằng số chuẩn hóa bố cục, tọa độ GPS -> Engine (1 unit = 1 m),
 *        đa giác hồ nước, ranh giới công viên, đường bao quanh, Sông Hàn
 *        và hệ thống lối đi cho Công viên Châu Á (Asia Park Đà Nẵng - mốc hiện trạng ~2019).
 *
 * Quy ước hệ tọa độ Engine (bàn tay phải, tỷ lệ 1:1):
 *  - +X : Hướng Đông (East)
 *  - -Z : Hướng Bắc (North), +Z : Hướng Nam (South)
 *  - +Y : Hướng lên (Up), mặt đất / mặt nước hồ tại y = 0
 *  - Gốc cục bộ (0, 0) tại tâm chân đế Vòng quay Mặt Trời (Sun Wheel):
 *      LAT0 = 16.040500°N, LON0 = 108.228380°E
 *      X_local = (lon - LON0) * M_LON
 *      Z_local = -(lat - LAT0) * M_LAT
 *      X_engine = X_local
 *      Z_engine = Z_local - 38.0f  (giữ Sun Wheel tại Engine (0, 65, -38))
 */

#include "core/MathCG.h"

namespace ParkLayout {

// ============================================================================
// 1. HẰNG SỐ QUY ĐỔI TỌA ĐỘ ĐỊA LÝ WGS84 -> ENGINE (1 unit = 1 mét)
// ============================================================================
constexpr float GPS_LAT0            = 16.040500f;   // [Ước lượng từ ảnh vệ tinh] ±8m (Wayback 2019/2022 Z=19)
constexpr float GPS_LON0            = 108.228380f;  // [Ước lượng từ ảnh vệ tinh] ±8m
constexpr float WGS84_M_PER_DEG_LAT = 110659.10f;   // Hệ số mét/độ vĩ tuyến tại 16.0405°N
constexpr float WGS84_M_PER_DEG_LON = 107012.75f;   // Hệ số mét/độ kinh tuyến tại 16.0405°N
constexpr float ENGINE_Z_OFFSET     = -38.0f;       // Độ lệch Z_engine so với Z_local
constexpr int   SNAPSHOT_YEAR       = 2019;         // Mốc hiện trạng mô hình (còn Tháp Đồng Hồ, nửa Đông là nền cỏ)

struct PointXZ {
    float x;
    float z;
};

constexpr PointXZ gpsToEngineXZ(float lat, float lon) {
    return {
        (lon - GPS_LON0) * WGS84_M_PER_DEG_LON,
        -(lat - GPS_LAT0) * WGS84_M_PER_DEG_LAT + ENGINE_Z_OFFSET
    };
}

// ============================================================================
// 2. THÔNG SỐ VỊ TRÍ, HƯỚNG XOAY & TỶ LỆ CÁC CÔNG TRÌNH (ĐỢT 1 -> ĐỢT 3)
// ============================================================================

// 2.1. Vòng quay Mặt Trời (Sun Wheel) - [Đã xác minh] chiều cao 115m, ĐK 100m, 64 cabin, 15 phút/vòng
constexpr MathCG::Vec3 SUN_WHEEL_CENTER(0.0f, 65.0f, -38.0f);
constexpr float        SUN_WHEEL_RADIUS              = 50.0f;
constexpr float        SUN_WHEEL_REAL_SPEED_DEG      = 0.4f;  // [Đã xác minh] 360° / (15 * 60s) = 0.4°/s
constexpr float        SUN_WHEEL_DEMO_FAST_MULT      = 5.0f;  // Hệ số tua nhanh trình diễn x5 (2.0°/s, phím V)

// 2.2. Thủy đình trên mặt hồ phía Bắc Sun Wheel - [Ước lượng từ ảnh vệ tinh] ±8m (16.04113°N, 108.22833°E)
constexpr MathCG::Vec3 LAKESIDE_PAVILION_POS(-5.4f, 0.0f, -107.7f);

// 2.3. Tháp rơi tự do Golden Sky Tower (cao 47m) - [Ước lượng từ ảnh vệ tinh] ±10m (16.04065°N, 108.22595°E)
constexpr MathCG::Vec3 SKY_TOWER_POS(-260.0f, 0.0f, -54.6f);
// Lô A1 - Chi tiết nâng cấp hình dáng trong khung bán kính <= 8.0m [Bản vẽ minh họa AI, tham khảo hình dạng]
constexpr float        SKY_TOWER_SHAFT_HEIGHT        = 47.0f;  // [Đã xác minh] Chiều cao thân tháp thép
constexpr float        SKY_TOWER_LOTUS_CROWN_HEIGHT  =  6.0f;  // [Bản vẽ minh họa AI, tham khảo hình dạng] Chỏm hoa sen đỉnh (~6 m)
constexpr float        SKY_TOWER_BASE_MAX_RADIUS     =  7.75f; // Giữ gọn trong bán kính cho phép <= 8.0m
constexpr float        SKY_TOWER_DECK_RADIUS         =  6.20f; // Sàn đón khách tròn trên cao
constexpr float        SKY_TOWER_BOARDING_DECK_Y     =  4.20f; // [Bản vẽ minh họa AI, tham khảo hình dạng] Sàn đón khách cao +4.2m
constexpr float        SKY_TOWER_GONDOLA_MIN_Y       =  4.35f; // Điểm dừng thấp nhất của vòng ghế ngay trên sàn +4.2m

// 2.4. Cổng Ngọ Môn & Tháp Đồng Hồ (mốc 2019) - [Ước lượng từ ảnh vệ tinh] ±8m
// Cổng Ngọ Môn tại 16.03890°N, 108.22853°E (rộng thực tế ~35m -> scale 0.76 từ mô hình gốc 46m)
constexpr MathCG::Vec3 NGO_MON_GATE_POS(16.1f, 0.0f, 139.1f);
constexpr float        NGO_MON_GATE_SCALE            = 0.76f;
// Tháp Đồng Hồ (2019) tại 16.03960°N, 108.22848°E (cách Cổng Ngọ Môn ~77.5m về phía Bắc, scale 1.0)
constexpr MathCG::Vec3 CLOCK_TOWER_POS(10.7f, 0.0f, 61.6f);
constexpr float        CLOCK_TOWER_SCALE             = 1.00f;
// Lô A1 - Khung bao chân đế theo vệ tinh (<= 25x24m), cao ~29m; hình dạng [Bản vẽ minh họa AI, tham khảo hình dạng]
constexpr float        CLOCK_TOWER_PODIUM_W          = 17.60f; // Khối thân đế đá chính 17.6x17.0m (chứa 3 vòm cuốn mỗi mặt)
constexpr float        CLOCK_TOWER_PODIUM_D          = 17.00f;
constexpr float        CLOCK_TOWER_PODIUM_H          =  4.85f;
constexpr float        CLOCK_TOWER_MAX_FOOTPRINT_W   = 24.00f; // Gồm cả 4 cầu thang trục (<= 25.0m vệ tinh)
constexpr float        CLOCK_TOWER_MAX_FOOTPRINT_D   = 23.40f; // Gồm cả 4 cầu thang trục (<= 24.0m vệ tinh)
constexpr float        CLOCK_TOWER_TOTAL_HEIGHT      = 29.40f; // Cao ~29m theo bóng tháp vệ tinh & bản vẽ #04
constexpr float        CLOCK_TOWER_DIAL_Y            = 17.35f; // Tầng giữa trong thân bát giác 5 tầng mái cong
constexpr float        CLOCK_TOWER_DIAL_OFFSET       =  4.02f; // Độ nhô khung dormer gắn mặt đồng hồ (4 hướng)
constexpr float        CLOCK_TOWER_DIAL_RADIUS       =  1.58f; // Bán kính mặt đồng hồ (bộ kim + vạch giờ)

// 2.5. Đu quay Ngựa gỗ (Festival Carousel) - [Chưa xác minh / dùng tạm ứng viên mái nón kẻ sọc 16.03923°N, 108.22689°E]
constexpr MathCG::Vec3 CAROUSEL_POS_UNVERIFIED(-159.4f, 0.0f, 102.5f);

// 2.6. Tàu lượn Queen Cobra (Phương án B: tịnh tiến + xoay -25°, KHÔNG scale, giữ nguyên hình học & animation)
// Tâm đường ray gốc trong RollerCoaster.h: (-85.35, -2.15), nhà ga gốc tại (-68.0, 6.0, 39.0)
// Tâm đích Phương án B: Engine (-255.0, 0.0, -358.0) [Local (-255.0, -320.0)], góc xoay Y = -25.0°
// (Tọa độ nhà ga thực tế theo RCDB #14194 là 16.043515°N, 108.226111°E -> Engine (-242.8, 0.0, -371.6))
constexpr MathCG::Vec3 QUEEN_COBRA_ORIG_CENTER(-85.35f, 0.0f, -2.15f);
constexpr MathCG::Vec3 QUEEN_COBRA_TARGET_CENTER(-255.0f, 0.0f, -358.0f);
constexpr float        QUEEN_COBRA_ROT_Y_DEG         = -25.0f;

// 2.7. Tàu điện một ray Monorail (~1.87 km, 20 điểm dọc dầm bê tông trên ảnh vệ tinh Wayback 2019)
// [Ước lượng từ ảnh vệ tinh] ±10m
// (Đã áp dụng Phương án A sau Đợt 4: dịch WP0, WP1 và Ga Nam lùi -6.2m về phía Bắc tại Z = +121.8m,
//  WP2 về Z = +114.0m để 100% dầm/trụ và Ga Nam nằm ngoài Quảng trường lát gạch Z <= 115.0m
//  đồng thời cách mặt Bắc Cổng Ngọ Môn > 10.2m và cách cánh tường thành phía Tây > 9.4m)
constexpr MathCG::Vec3 MONORAIL_SOUTH_STATION_POS(-24.6f, 0.0f, 121.8f);   // Ga Nam (Phương án A: Z = +121.8m)
constexpr MathCG::Vec3 MONORAIL_NEPAL_STATION_POS(-217.2f, 0.0f, -220.6f); // Ga Nepal (16.04215°N, 108.22635°E)
constexpr float        MONORAIL_SOUTH_STATION_ROT_Y_DEG = -90.0f;          // Dầm chạy hướng Đông -> Tây tại Ga Nam
constexpr float        MONORAIL_NEPAL_STATION_ROT_Y_DEG =   0.0f;          // Sảnh Ga Nepal nằm bên Đông (+X) của hành lang
constexpr float        MONORAIL_CRUISE_SPEED_MPS        =   9.2f;          // ~1885m / 9.2 m/s = ~205s (~3.42 phút/vòng)

constexpr int NUM_MONORAIL_WAYPOINTS = 20;
constexpr MathCG::Vec3 MONORAIL_WAYPOINTS[NUM_MONORAIL_WAYPOINTS] = {
    {   16.1f, 12.0f,  121.8f }, // 0: Cầu vượt phía Bắc Cổng Ngọ Môn (Phương án A: cách cổng 11.6m, ngoài quảng trường Z<=115)
    {  -24.6f, 12.0f,  121.8f }, // 1: Ga Monorail Nam (Phương án A: Z_station thuộc [119.2, 129.0])
    {  -78.1f, 12.2f,  114.0f }, // 2: Đoạn cong Tây-Nam (Phương án A: chuyển tiếp mượt về trục Tây)
    { -153.0f, 12.5f,   78.2f }, // 3: Phía Bắc vòng xoay Ngựa gỗ (16.03945, 108.22695)
    { -238.6f, 12.5f,   78.2f }, // 4: Cạnh Tây-Nam khu trò chơi (16.03945, 108.22615)
    { -274.0f, 12.5f,   63.8f }, // 5: Góc cua sang hành lang phía Tây (16.03958, 108.22582)
    { -281.4f, 13.0f,  -54.6f }, // 6: Phía Tây tháp Sky Tower (16.04065, 108.22575)
    { -294.3f, 13.5f, -231.7f }, // 7: Hành lang Tây-Bắc (16.04225, 108.22563)
    { -260.0f, 14.0f, -270.4f }, // 8: Vòng cua Bắc Ga Nepal (16.04260, 108.22595)
    { -201.2f, 14.0f, -273.7f }, // 9: Qua quảng trường Angkor (16.04263, 108.22650)
    {  -27.8f, 14.5f, -300.3f }, // 10: Cầu vượt qua mặt hồ trung tâm (16.04287, 108.22812)
    {   32.1f, 14.0f, -268.2f }, // 11: Bờ Đông hồ nước (16.04258, 108.22868)
    {   89.9f, 13.5f, -161.9f }, // 12: Sườn Đông hồ nước (16.04162, 108.22922)
    {  157.3f, 13.5f, -184.1f }, // 13: Nhịp uốn phía Đông-Bắc (16.04182, 108.22985)
    {  200.1f, 13.0f, -121.0f }, // 14: Trảng cỏ phía Đông (16.04125, 108.23025)
    {  317.8f, 13.0f, -121.0f }, // 15: Tiến ra sát Đường Đỗ Pháp Thuận (16.04125, 108.23135)
    {  368.1f, 12.5f,   -4.8f }, // 16: Cạnh Đông giáp Đường Đỗ Pháp Thuận (16.04020, 108.23182)
    {  360.6f, 12.5f,   22.9f }, // 17: Góc cua Đông-Nam (16.03995, 108.23175)
    {  194.8f, 12.2f,   33.9f }, // 18: Hành lang возврат phía Nam trảng cỏ Đông (16.03985, 108.23020)
    {  109.2f, 12.0f,  100.3f }  // 19: Khép vòng về phía Đông Cổng Ngọ Môn (16.03925, 108.22940)
};

// ============================================================================
// 3. RANH GIỚI CÔNG VIÊN (~36.1 HA) & HỒ NƯỚC PHÍA BẮC SUN WHEEL
//    [Ước lượng từ ảnh vệ tinh Wayback 2019/2022 & đường bao OSM]
// ============================================================================

constexpr int NUM_PARK_BOUNDARY_PTS = 16;
constexpr PointXZ PARK_BOUNDARY_POLY[NUM_PARK_BOUNDARY_PTS] = {
    { -302.8f,  188.9f }, // 0: Góc Tây-Nam cạnh Cung Thiếu nhi (16.03845, 108.22555)
    { -292.1f,   33.9f }, // 1: Bậc ranh giới Tây-Nam (16.03985, 108.22565)
    { -348.9f,   30.6f }, // 2: Mở rộng ra đường nội bộ phía Tây (16.03988, 108.22512)
    { -370.3f, -231.7f }, // 3: Cạnh Tây dọc đường nội bộ (16.04225, 108.22492)
    { -381.0f, -414.2f }, // 4: Góc Tây-Bắc (16.04390, 108.22482)
    { -313.5f, -441.9f }, // 5: Mép Bắc khu hậu cần Tây-Bắc (16.04415, 108.22545)
    { -217.2f, -449.7f }, // 6: Mũi Bắc bán đảo Queen Cobra (16.04422, 108.22635)
    { -142.3f, -320.2f }, // 7: Eo bờ Tây hồ nước (16.04305, 108.22705)
    {  -67.4f,  -93.3f }, // 8: Góc Tây-Nam bờ hồ (16.04100, 108.22775)
    {   61.0f,  -87.8f }, // 9: Góc Đông-Nam bờ hồ (16.04095, 108.22895)
    {   42.8f, -287.0f }, // 10: Bờ Đông hồ nước (16.04275, 108.22878)
    {   93.1f, -486.2f }, // 11: Mũi Bắc-Đông-Bắc trảng cỏ Đông (16.04455, 108.22925)
    {  248.3f, -464.0f }, // 12: Góc Đông-Bắc giáp Đường Thuận Yến (16.04435, 108.23070)
    {  403.4f,   -4.8f }, // 13: Cạnh Đông giáp Đường Đỗ Pháp Thuận (16.04020, 108.23215)
    {  398.1f,  111.4f }, // 14: Góc Đông-Nam giáp Phan Đăng Lưu (16.03915, 108.23210)
    {   16.1f,  172.3f }  // 15: Quảng trường đón khách Nam Phan Đăng Lưu (16.03860, 108.22853)
};

// Đường bao 11 đỉnh của hồ nước thực tế phía Bắc Sun Wheel (Z_eng <= -73.4m)
constexpr int NUM_LAKE_OUTLINE_PTS = 11;
constexpr PointXZ LAKE_OUTLINE_POLY[NUM_LAKE_OUTLINE_PTS] = {
    {  -59.9f,  -75.6f }, // 0: Góc Tây-Nam bờ kè hồ (16.04084, 108.22782)
    {   57.8f,  -73.4f }, // 1: Góc Đông-Nam bờ kè hồ (16.04082, 108.22892)
    {   42.8f, -253.8f }, // 2: Bờ Đông đoạn giữa (16.04245, 108.22878)
    {   -3.2f, -414.2f }, // 3: Bờ Đông đoạn Bắc (16.04390, 108.22835)
    {   23.5f, -513.8f }, // 4: Cửa hồ thông ra nhánh sông Đông-Bắc (16.04480, 108.22860)
    { -190.5f, -513.8f }, // 5: Cửa hồ phía Bắc cạnh Đảo Xanh (16.04480, 108.22660)
    { -297.5f, -464.0f }, // 6: Nhánh vịnh Tây-Bắc (16.04435, 108.22560)
    { -227.9f, -443.0f }, // 7: Mép nước Bắc bán đảo Queen Cobra (16.04416, 108.22625)
    { -199.0f, -370.0f }, // 8: Bờ Đông bán đảo Queen Cobra (16.04350, 108.22652)
    { -153.0f, -303.6f }, // 9: Bờ hồ khu quảng trường Angkor (16.04290, 108.22695)
    {  -83.5f, -198.5f }  // 10: Bờ Tây hồ đoạn giữa (16.04195, 108.22760)
};

/**
 * @brief Kiểm tra điểm mặt bằng (x, z) có nằm trong lòng hồ nước phía Bắc Sun Wheel hay không
 *        (Thuật toán Ray-Casting trên đa giác 11 đỉnh LAKE_OUTLINE_POLY).
 */
inline bool isPointInLake(float x, float z) {
    bool inside = false;
    for (int i = 0; i < NUM_LAKE_OUTLINE_PTS; ++i) {
        const PointXZ& p1 = LAKE_OUTLINE_POLY[i];
        const PointXZ& p2 = LAKE_OUTLINE_POLY[(i + 1) % NUM_LAKE_OUTLINE_PTS];
        if (((p1.z > z) != (p2.z > z)) &&
            (x < (p2.x - p1.x) * (z - p1.z) / (p2.z - p1.z + 1e-7f) + p1.x)) {
            inside = !inside;
        }
    }
    return inside;
}

// Chia đa giác hồ lõm 11 đỉnh thành 5 hình tứ giác lồi (thứ tự đỉnh CCW khi nhìn từ +Y xuống:
// trên mặt phẳng ảnh chụp từ trên xuống (X tăng sang phải, Z tăng xuống dưới), thứ tự CCW trong không gian 3D
// (+X, +Y, +Z) là ngược chiều kim đồng hồ trên ảnh!)
struct ConvexQuadXZ {
    PointXZ v[4];
};

constexpr int NUM_LAKE_CONVEX_QUADS = 5;
constexpr ConvexQuadXZ LAKE_CONVEX_QUADS[NUM_LAKE_CONVEX_QUADS] = {
    // Dải 1 (Nam hồ - nơi đặt Thủy đình): (0)->(1)->(2)->(10)
    {{ { -59.9f,  -75.6f }, {  57.8f,  -73.4f }, {  42.8f, -253.8f }, { -83.5f, -198.5f } }},
    // Dải 2 (Trung tâm hồ - nơi cầu Monorail bắc qua): (10)->(2)->(3)->(9)
    {{ { -83.5f, -198.5f }, {  42.8f, -253.8f }, {  -3.2f, -414.2f }, {-153.0f, -303.6f } }},
    // Dải 3 (Đông-Bắc Queen Cobra): (9)->(3)->(3)->(8)
    {{ {-153.0f, -303.6f }, {  -3.2f, -414.2f }, {  -3.2f, -414.2f }, {-199.0f, -370.0f } }},
    // Dải 4 (Bắc Queen Cobra): (8)->(3)->(4)->(7)
    {{ {-199.0f, -370.0f }, {  -3.2f, -414.2f }, {  23.5f, -513.8f }, {-227.9f, -443.0f } }},
    // Dải 5 (Cửa vịnh Tây-Bắc): (7)->(4)->(5)->(6)
    {{ {-227.9f, -443.0f }, {  23.5f, -513.8f }, {-190.5f, -513.8f }, {-297.5f, -464.0f } }}
};

// 10 trụ đèn lồng Á Đông (ParametricShapes) rải dọc bờ kè phía Nam của hồ mới (Z_eng = -70.0m)
constexpr int NUM_LAKESIDE_LANTERNS = 10;
constexpr MathCG::Vec3 LAKESIDE_LANTERN_POS[NUM_LAKESIDE_LANTERNS] = {
    { -56.0f, 0.0f, -70.0f },
    { -44.0f, 0.0f, -70.0f },
    { -32.0f, 0.0f, -70.0f },
    { -20.0f, 0.0f, -70.0f },
    {  -9.5f, 0.0f, -70.0f }, // Bên trái đầu cầu Thủy đình
    {  -1.3f, 0.0f, -70.0f }, // Bên phải đầu cầu Thủy đình
    {  12.0f, 0.0f, -70.0f },
    {  25.0f, 0.0f, -70.0f },
    {  38.0f, 0.0f, -70.0f },
    {  52.0f, 0.0f, -70.0f }
};

// ============================================================================
// 4. ĐƯỜNG BAO QUANH CÔNG VIÊN & DẢI SÔNG HÀN (OSM + ẢNH VỆ TINH 2019)
// ============================================================================

// 4.1. Đường Phan Đăng Lưu (cạnh Nam công viên, OSM way 302765736 / 1280089217, rộng ~20m)
constexpr int NUM_ROAD_PHAN_DANG_LUU_PTS = 4;
constexpr PointXZ ROAD_PHAN_DANG_LUU[NUM_ROAD_PHAN_DANG_LUU_PTS] = {
    { -440.0f,  218.0f },
    { -120.0f,  198.0f },
    {   95.0f,  182.0f },
    {  435.0f,  125.0f }
};
constexpr float ROAD_PHAN_DANG_LUU_WIDTH = 20.0f;

// 4.2. Đường Đỗ Pháp Thuận nối liền Đường Thuận Yến (cạnh Đông công viên ven sông Hàn, OSM way 302765742 & 643263435)
constexpr int NUM_ROAD_EAST_BORDER_PTS = 4;
constexpr PointXZ ROAD_EAST_BORDER[NUM_ROAD_EAST_BORDER_PTS] = {
    {  412.0f,  132.0f },
    {  416.0f,   -5.0f },
    {  345.0f, -210.0f },
    {  262.0f, -478.0f }
};
constexpr float ROAD_EAST_BORDER_WIDTH = 16.0f;

// 4.3. Đường phụ phía Tây công viên / nhánh nối Đường Bạch Đằng (OSM way 1392384128 / 1392384120)
constexpr int NUM_ROAD_WEST_BORDER_PTS = 4;
constexpr PointXZ ROAD_WEST_BORDER[NUM_ROAD_WEST_BORDER_PTS] = {
    { -356.0f,  214.0f },
    { -362.0f,   25.0f },
    { -384.0f, -230.0f },
    { -396.0f, -445.0f }
};
constexpr float ROAD_WEST_BORDER_WIDTH = 14.0f;

// 4.4. Dải mặt nước Sông Hàn phía Đông Đường Thuận Yến / Đỗ Pháp Thuận (OSM relation 19756034)
constexpr ConvexQuadXZ HAN_RIVER_QUAD = {{
    {  438.0f,  220.0f },
    {  620.0f,  220.0f },
    {  490.0f, -520.0f },
    {  282.0f, -520.0f }
}};

// ============================================================================
// 5. HỆ THỐNG LỐI ĐI NỘI BỘ (DẢI PHẲNG QUAD-STRIP NÂNG LÊN VÀI CM TRÁNH Z-FIGHTING)
//    [Ước lượng từ ảnh vệ tinh Wayback 2019 (wb19_south / wb19_mid / wb19_north)]
//    Nối: Cổng chính Ngọ Môn -> Tháp Đồng Hồ -> Sun Wheel -> Thủy đình/bờ hồ
//         -> Ga Monorail Nam -> Ngựa gỗ -> Sky Tower -> Ga Monorail Nepal -> Queen Cobra.
//    Đã kiểm tra hình học: Không có lối đi mặt đất nào cắt qua mặt hồ hay chân công trình.
// ============================================================================

constexpr float PATH_Y_MAIN      = 0.04f; // Nâng 4 cm trên nền cỏ để khử Z-fighting
constexpr float PATH_Y_SECONDARY = 0.035f;
constexpr float PIER_Y_DECK      = 0.25f; // Cầu gỗ dẫn ra Thủy đình cao 25 cm trên mặt nước hồ

// 5.1. Tuyến trục chính Nam - Bắc (rộng 6.0 m):
// Cổng Ngọ Môn -> Quảng trường Nam -> vòng qua mép Đông Tháp Đồng Hồ -> Quảng trường Sun Wheel -> bờ kè Nam hồ nước
constexpr float PATH_MAIN_SPINE_WIDTH = 6.0f;
constexpr int   NUM_PATH_MAIN_SPINE_PTS = 10;
constexpr PointXZ PATH_MAIN_SPINE[NUM_PATH_MAIN_SPINE_PTS] = {
    {   16.1f,  166.0f }, // 0: Lối vào từ Quảng trường Đường Phan Đăng Lưu (16.03866, 108.22853)
    {   16.1f,  139.1f }, // 1: Đi xuyên qua vòm giữa Cổng Ngọ Môn (16.03890, 108.22853)
    {   16.1f,   94.8f }, // 2: Sân trong phía Bắc Cổng Ngọ Môn (16.03930, 108.22853)
    {   30.0f,   74.9f }, // 3: Nhánh vòng qua phía Đông đế Tháp Đồng Hồ (16.03948, 108.22866)
    {   30.0f,   48.3f }, // 4: Phía Bắc-Đông Tháp Đồng Hồ (16.03972, 108.22866)
    {    0.0f,   28.4f }, // 5: Quảng trường trung tâm phía Nam Sun Wheel (16.03990, 108.22838)
    {    0.0f,    0.7f }, // 6: Sân đón khách phía Nam Sun Wheel (16.04015, 108.22838)
    {  -34.2f,   -2.6f }, // 7: Góc Tây-Nam bệ Sun Wheel (16.04018, 108.22806)
    {  -34.2f,  -66.8f }, // 8: Hành lang phía Tây Sun Wheel ra bờ hồ (16.04076, 108.22806)
    {   -5.4f,  -66.8f }  // 9: Đầu cầu Thủy đình trên bờ kè Nam hồ nước (16.04076, 108.22833)
};

// 5.2. Đường dạo bộ dọc bờ kè Nam hồ nước (rộng 5.0 m):
constexpr float PATH_SOUTH_LAKE_PROM_WIDTH = 5.0f;
constexpr int   NUM_PATH_SOUTH_LAKE_PROM_PTS = 3;
constexpr PointXZ PATH_SOUTH_LAKE_PROM[NUM_PATH_SOUTH_LAKE_PROM_PTS] = {
    {  -63.5f,  -66.8f }, // 0: Đầu Tây bờ kè Nam hồ nước (16.04076, 108.22779)
    {   -5.4f,  -66.8f }, // 1: Giữa bờ kè tại đầu cầu Thủy đình (16.04076, 108.22833)
    {   53.5f,  -65.7f }  // 2: Đầu Đông bờ kè Nam hồ nước (16.04075, 108.22888)
};

// 5.3. Cầu gỗ dẫn từ bờ kè Nam ra Thủy đình trên mặt hồ (rộng 3.0 m, cao y = 0.25 m):
// Ghi chú: Đây là cây cầu thực tế trên ảnh vệ tinh wb19_south.png nối bờ hồ ra bệ đá Thủy đình.
constexpr float PATH_PAVILION_PIER_WIDTH = 3.0f;
constexpr int   NUM_PATH_PAVILION_PIER_PTS = 2;
constexpr PointXZ PATH_PAVILION_PIER[NUM_PATH_PAVILION_PIER_PTS] = {
    {   -5.4f,  -66.8f }, // 0: Mố cầu bờ Nam (16.04076, 108.22833)
    {   -5.4f, -102.7f }  // 1: Mép Nam bệ đá Thủy đình (16.04108, 108.22833)
};

// 5.4. Tuyến trục chính khu trò chơi phía Tây (rộng 5.0 m):
// Sân trong Ngọ Môn -> Ga Monorail Nam -> Ngựa gỗ -> hành lang Đông Port of Sky Treasure (-195, -20)
// -> Sky Tower -> Ga Monorail Nepal -> Queen Cobra
constexpr float PATH_WEST_SPINE_WIDTH = 5.0f;
constexpr int   NUM_PATH_WEST_SPINE_PTS = 14;
constexpr PointXZ PATH_WEST_SPINE[NUM_PATH_WEST_SPINE_PTS] = {
    {   16.1f,   94.8f }, // 0: Tách từ sân trong phía Bắc Cổng Ngọ Môn (16.03930, 108.22853)
    {  -24.6f,  112.5f }, // 1: Sân đón phía Bắc Ga Monorail Nam mới (cách mép Bắc ga Z=119.2 là 4.2m)
    {  -70.6f,  100.3f }, // 2: Lối đi hướng sang quảng trường hoa (16.03925, 108.22772)
    { -113.4f,   81.5f }, // 3: Quảng trường hoa khu Tây-Nam (16.03942, 108.22732)
    { -159.4f,   86.5f }, // 4: Phía Bắc Đu quay Ngựa gỗ (-159.4, 102.5) và phía Nam dầm Monorail (Z=78.2)
    { -188.0f,   42.0f }, // 5: Ngã ba trục trò chơi miền Tây (giữa Pirate Ship và Port of Sky Treasure)
    { -195.0f,  -20.0f }, // 6: Hành lang phía Đông Port of Sky Treasure (-224, +22) dẫn lên Sky Tower
    { -244.0f,  -54.6f }, // 7: Quảng trường phía Đông chân tháp Sky Tower (-260, -54.6)
    { -233.3f, -109.9f }, // 8: Hành lang phía Tây đường ray số 8 Paradise Fall (-199, -119.9)
    { -227.9f, -187.4f }, // 9: Lối dẫn lên khu Nepal (16.04185, 108.22625)
    { -231.1f, -220.6f }, // 10: Quảng trường phía Tây Ga Monorail Nepal (16.04215, 108.22622)
    { -214.0f, -275.9f }, // 11: Mép Nam quảng trường ngôi sao Angkor (16.04265, 108.22638)
    { -231.1f, -303.6f }, // 12: Lối lên bán đảo Queen Cobra (16.04290, 108.22622)
    { -246.1f, -334.6f }  // 13: Quảng trường đón khách phía Nam nhà ga Queen Cobra (16.04318, 108.22608)
};

// 5.5. Lối đi phụ ven bờ Tây hồ nước (rộng 3.0 m):
// Nối đầu Tây bờ kè Nam hồ nước -> chạy dọc bờ Tây hồ -> nhập vào quảng trường ngôi sao Angkor
constexpr float PATH_LAKESIDE_WEST_WIDTH = 3.0f;
constexpr int   NUM_PATH_LAKESIDE_WEST_PTS = 6;
constexpr PointXZ PATH_LAKESIDE_WEST[NUM_PATH_LAKESIDE_WEST_PTS] = {
    {  -63.5f,  -66.8f }, // 0: Đầu Tây bờ kè Nam hồ (16.04076, 108.22779)
    {  -75.5f, -132.1f }, // 1: Bờ Tây hồ đoạn Nam (16.04135, 108.22767)
    {  -91.5f, -198.5f }, // 2: Bờ Tây hồ đoạn giữa (16.04195, 108.22752)
    { -124.0f, -252.0f }, // 3: Bờ Tây hồ đoạn Bắc (bám sát bờ kè hồ phía Đông nhà mái xanh Angkor #18)
    { -160.0f, -292.0f }, // 4: Mép Đông-Bắc quảng trường Angkor (16.04278, 108.22672)
    { -214.0f, -275.9f }  // 5: Nhập vào trục chính phía Tây tại Angkor (16.04265, 108.22638)
};

// 5.6. Lối đi phụ nối ngang Quảng trường Sun Wheel sang trục trò chơi phía Tây (rộng 3.5 m):
constexpr float PATH_CENTRAL_CONNECTOR_WIDTH = 3.5f;
constexpr int   NUM_PATH_CENTRAL_CONNECTOR_PTS = 5;
constexpr PointXZ PATH_CENTRAL_CONNECTOR[NUM_PATH_CENTRAL_CONNECTOR_PTS] = {
    {    0.0f,   28.4f }, // 0: Quảng trường phía Nam Sun Wheel (16.03990, 108.22838)
    {  -35.0f,   36.0f }, // 1: Vòng qua phía Nam Dãy gian hàng mái sọc #6 (-50, +12)
    {  -72.0f,   36.0f }, // 2: Giữa Đu quay tròn #10 (-94.2, +17.3) và Nhà mái xanh Bắc #7 (-113.4, +55.0)
    { -145.0f,   33.9f }, // 3: Phía Nam Pirate Ship (-172, -2) và Đu quay tròn #9 (-132.7, +12.9)
    { -188.0f,   42.0f }  // 4: Nhập vào trục chính phía Tây tại ngã ba (-188, +42)
};

// ============================================================================
// 6. CÁC CÔNG TRÌNH KIẾN TRÚC ĐỢT 4: CỔNG NGỌ MÔN 2 CÁNH TƯỜNG THÀNH,
//    HỒ SEN, SÂN TRƯỚC, QUẢNG TRƯỜNG, SÂN KHẤU SUN WHEEL & CÁC CHÒI MÁI NGÓI
//    [Ước lượng từ ảnh vệ tinh Wayback 2019 & ảnh toàn cảnh ban đêm]
// ============================================================================

// 6.1. Hai cánh tường thành có răng cưa kéo dài hai bên Cổng Ngọ Môn (+16.1, 0, +139.1, scale 0.76)
//      Mỗi cánh dài 58.0 m trong hệ tọa độ cục bộ cổng (~44.1 m sau scale -> tổng bề ngang toàn cụm ~123.1 m)
constexpr float NGO_MON_WING_SPAN_LOCAL      = 58.0f;
constexpr float NGO_MON_WING_HEIGHT_LOCAL    =  6.4f;
constexpr float NGO_MON_WING_DEPTH_LOCAL     =  4.2f;
constexpr float NGO_MON_SIDE_ARCH_X_LOCAL    = 42.0f; // Vị trí cổng vòm phụ mỗi cánh tường thành

// 6.2. Hai Hồ sen vuông phía Nam Cổng Ngọ Môn & Sân trước phía Bắc cổng
constexpr MathCG::Vec3 LOTUS_POND_WEST_POS(-18.0f, 0.0f, 155.0f);
constexpr MathCG::Vec3 LOTUS_POND_EAST_POS( 50.0f, 0.0f, 155.0f);
constexpr float        LOTUS_POND_WIDTH      = 26.0f;
constexpr float        LOTUS_POND_DEPTH      = 14.0f;

constexpr MathCG::Vec3 GATE_FORECOURT_POS(16.1f, 0.0f, 124.0f);
constexpr float        GATE_FORECOURT_WIDTH  = 46.0f;
constexpr float        GATE_FORECOURT_DEPTH  = 18.0f;

// 6.3. Quảng trường Trung tâm lát gạch (từ sân trước Ngọ Môn tới chân sân khấu Sun Wheel)
constexpr MathCG::Vec3 CENTRAL_PLAZA_CENTER(16.1f, 0.0f, 56.0f);
constexpr float        CENTRAL_PLAZA_WIDTH   = 86.0f;  // X thuộc [-26.9, +59.1]
constexpr float        CENTRAL_PLAZA_DEPTH   = 118.0f; // Z thuộc [-3.0, +115.0]
constexpr float        CENTRAL_PLAZA_Y       = 0.028f; // Nâng 2.8 cm trên nền cỏ tránh Z-fighting

// 6.4. Hai cụm Chòi mái ngói Á Đông đối xứng hai bên Quảng trường Trung tâm
constexpr MathCG::Vec3 PLAZA_PAVILION_WEST_POS(-24.0f, 0.0f, 65.0f);
constexpr MathCG::Vec3 PLAZA_PAVILION_EAST_POS( 48.0f, 0.0f, 65.0f);
constexpr float        PLAZA_PAVILION_WIDTH  = 16.0f;
constexpr float        PLAZA_PAVILION_DEPTH  =  9.0f;

// 6.5. Cụm Sân khấu & Nhà ga chân Sun Wheel (hộp sân khấu + 2 khối nhà gắn 2 màn hình tròn + 2 chòi bên)
//      Đặt tại (0.0, 0.0, -15.9) với chiều sâu 9.0 m (Z thuộc [-20.4, -11.4]) — đã lùi thêm 1.8 m về phía Nam
//      để cách mép Nam của 2 bệ móng bê tông chân chữ A Sun Wheel (Z thuộc [-28.2, -22.8]) là 2.40 m
//      và cách ống thép chân chữ A (Z = -25.5) là 5.10 m, nhìn cận thoáng và không sát chân bệ.
constexpr MathCG::Vec3 SUN_WHEEL_STAGE_POS(0.0f, 0.0f, -15.9f);
constexpr float        SUN_WHEEL_STAGE_TOTAL_WIDTH = 52.0f; // X thuộc [-26.0, +26.0]
constexpr float        SUN_WHEEL_STAGE_DEPTH       =  9.0f; // Z thuộc [-20.4, -11.4]
constexpr float        SUN_WHEEL_SCREEN_RADIUS     =  3.1f; // Đường kính màn hình tròn = 6.2 m

// Hai chòi mái ngói hai bên cánh chân Sun Wheel (|X| thuộc [34.0, 46.0], cách bệ chân chữ A |X|<=29.7 là 4.3 m)
constexpr MathCG::Vec3 SUN_WHEEL_SIDE_PAVILION_WEST_POS(-40.0f, 0.0f, -22.2f);
constexpr MathCG::Vec3 SUN_WHEEL_SIDE_PAVILION_EAST_POS( 40.0f, 0.0f, -22.2f);
constexpr float        SUN_WHEEL_SIDE_PAVILION_SIZE = 12.0f;

// 6.6. Hai Chòi vọng cảnh nhỏ ven bờ Nam hồ nước (đối xứng hai bên Thủy đình (-5.4, 0, -107.7))
constexpr MathCG::Vec3 LAKE_SHORE_PAVILION_WEST_POS(-49.2f, 0.0f, -88.0f);
constexpr MathCG::Vec3 LAKE_SHORE_PAVILION_EAST_POS( 40.7f, 0.0f, -91.1f);
constexpr float        LAKE_SHORE_PAVILION_WIDTH = 10.0f;
constexpr float        LAKE_SHORE_PAVILION_DEPTH =  8.0f;

// ============================================================================
// 7. DANH MỤC HẰNG SỐ ĐÃ DUYỆT CHO ĐỢT 5A & 5B (KHU TÂY & NAM HỒ; NỬA ĐÔNG TRỐNG)
// ============================================================================

// 7.1. Nhóm 5A - 4 Trò tàu lượn nhỏ (dùng chung lớp MiniCoaster) & 3 Trò chơi dời từ PR #1:
//      Ghi chú nhãn & đối chiếu độ lệch giữa tọa độ GPS gốc (RCDB/Commons) và tâm khung bao đã chọn:
//       - Garuda Valley [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]:
//         GPS RCDB (16.038689, 108.227418) -> Engine (-102.9, +162.4) nằm ở đầu vòng ray phía Đông-Nam;
//         tâm khung bao toàn cụm (nhà ga Tây + cổng Commons 49374160882 + vòng ray) trên ảnh vệ tinh Z19
//         là (-124.1, +155.7) (lệch dX = -21.2m, dZ = -6.7m, |d| = 22.2m).
//       - Port of Sky Treasure [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]:
//         GPS RCDB (16.039907, 108.226363) -> Engine (-215.8, +27.6);
//         tâm khung bao cụm nhà ga chữ T + vòng ray là (-224.0, +22.0) (dX = -8.2m, dZ = -5.6m, |d| = 9.9m).
//       - Tòa nhà đá Paradise Fall [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]:
//         GPS RCDB (16.040998, 108.226822) -> Engine (-166.7, -93.1) nằm tại đầu Đông cổng vào
//         (Commons 49358210737); tâm tòa nhà đá là (-179.8, -91.1) (|d| = 13.2m).
//       - Ray số 8 Paradise Fall [Ước lượng từ ảnh vệ tinh; hình dạng đơn giản hóa]:
//         Nằm nối tiếp ở phía Tây-Bắc của Tòa nhà đá Paradise Fall với tâm tại (-199.0, -119.9)
//         (cách ghim cổng Đông RCDB |d| = 41.9m nên gắn nhãn [Ước lượng từ ảnh vệ tinh]).
//       - Highway Boat [Đã xác minh tên/vị trí; hình dạng đơn giản hóa]:
//         GPS RCDB (16.042034, 108.226667) -> Engine (-183.3, -207.8);
//         tâm khung bao toàn cụm ray là (-176.0, -214.0) (dX = +7.3m, dZ = -6.2m, |d| = 9.6m).
constexpr MathCG::Vec3 GARUDA_VALLEY_CENTER(-124.1f, 0.0f, 155.7f);        // [Đã xác minh tên/vị trí; hình dạng đơn giản hóa] (34x26m)
constexpr MathCG::Vec3 PORT_SKY_TREASURE_CENTER(-224.0f, 0.0f, 22.0f);     // [Đã xác minh tên/vị trí; hình dạng đơn giản hóa] (36x52m)
constexpr MathCG::Vec3 PARADISE_FALL_BLDG_POS(-179.8f, 0.0f, -91.1f);      // [Đã xác minh tên/vị trí; hình dạng đơn giản hóa] (24x14m)
constexpr MathCG::Vec3 PARADISE_FALL_TRACK_CENTER(-199.0f, 0.0f, -119.9f); // [Ước lượng từ ảnh vệ tinh; hình dạng đơn giản hóa] (34x24m)
constexpr MathCG::Vec3 HIGHWAY_BOAT_CENTER(-176.0f, 0.0f, -214.0f);        // [Đã xác minh tên/vị trí; hình dạng đơn giản hóa] (34x26m)

// Bảng 8 điểm điều khiển cục bộ (so với tâm từng tàu lượn nhỏ) cho lớp MiniCoaster dùng chung (Đợt 5A):
constexpr int NUM_MINI_COASTER_WAYPOINTS = 8;

// 1) Garuda Valley (vòng lượn bầu dục có đồi kéo phía Bắc cao 8.2m, nhà ga phía Tây)
constexpr MathCG::Vec3 GARUDA_VALLEY_LOCAL_WPS[NUM_MINI_COASTER_WAYPOINTS] = {
    { -14.0f, 2.6f,   0.0f }, { -14.0f, 2.8f,  -9.0f },
    {  -3.0f, 7.8f, -11.0f }, {  11.0f, 8.2f,  -9.0f },
    {  14.5f, 5.5f,   0.0f }, {  11.0f, 3.6f,   9.5f },
    {   0.0f, 4.4f,  10.5f }, { -11.0f, 3.0f,   8.5f }
};

// 2) Port of Sky Treasure (vòng lượn chữ S kéo dài Bắc-Nam cao 9.2m, nhà ga mái ngói phía Tây-Nam)
constexpr MathCG::Vec3 PORT_SKY_TREASURE_LOCAL_WPS[NUM_MINI_COASTER_WAYPOINTS] = {
    { -13.0f, 2.8f,   0.0f }, { -14.0f, 4.5f, -18.0f },
    {  -3.0f, 9.2f, -22.0f }, {  11.0f, 8.5f, -17.0f },
    {  13.0f, 5.2f,  -2.0f }, {   6.0f, 6.5f,  10.0f },
    {  12.0f, 4.0f,  20.0f }, {  -4.0f, 3.0f,  19.0f }
};

// 3) Paradise Fall (vòng ray số 8 thắt eo ở giữa, cao 8.8m, kết nối cụm Tòa nhà đá PARADISE_FALL_BLDG_POS)
constexpr MathCG::Vec3 PARADISE_FALL_LOCAL_WPS[NUM_MINI_COASTER_WAYPOINTS] = {
    {  14.0f, 3.0f,   4.0f }, {  15.0f, 7.5f,  -7.0f },
    {   3.0f, 8.8f,  -9.5f }, {  -5.0f, 6.2f,  -2.5f },
    { -15.0f, 7.2f,  -8.5f }, { -16.0f, 4.5f,   4.0f },
    {  -5.0f, 3.2f,   8.5f }, {   4.0f, 4.2f,   2.5f }
};

// 4) Highway Boat (vòng lượn tàu thuyền khu Tây-Bắc cạnh Ga Monorail Nepal, cao 8.5m)
constexpr MathCG::Vec3 HIGHWAY_BOAT_LOCAL_WPS[NUM_MINI_COASTER_WAYPOINTS] = {
    { -14.0f, 2.8f,   0.0f }, { -13.0f, 7.8f, -10.0f },
    {   0.0f, 8.5f, -11.5f }, {  13.5f, 6.5f,  -9.0f },
    {  15.0f, 4.2f,   1.0f }, {  11.0f, 5.5f,  10.0f },
    {  -1.0f, 3.5f,  11.0f }, { -12.0f, 2.8f,   8.5f }
};

// 3 Trò chơi có sẵn từ PR #1 của nhóm dời về đúng vị trí khu Tây (Đợt 5A) - [Ước lượng từ ảnh vệ tinh — mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 PLAYGROUND_TARGET_POS(-104.9f, 0.0f, -24.7f);       // Sân chơi trẻ em (22x14m)
constexpr MathCG::Vec3 BUMPER_CARS_TARGET_POS(-206.5f, 0.0f, 108.1f);      // Sân xe va chạm khu Tây-Nam (22x14m)
constexpr MathCG::Vec3 PIRATE_SHIP_TARGET_POS(-172.0f, 0.0f, -2.0f);       // Thuyền cướp biển gần Port of Sky Treasure (18x12m)

// 7.2. Nhóm 5B - Các khối nhà tĩnh, đu quay tròn & Quảng trường Angkor:
//      Nhãn bắt buộc cho toàn bộ nhóm 5B: [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
//      Đã kiểm tra hình học: 0 va chạm với 6 tuyến lối đi, đa giác hồ nước, dầm & trụ Monorail và các trò Đợt 1-5A.
constexpr MathCG::Vec3 STRIPED_STALLS_POS(-50.0f, 0.0f, 12.0f);             // #6:  Dãy 3 hàng gian hàng mái sọc đỏ-trắng (16x32m) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 FLOWER_PLAZA_CENTER(-108.0f, 0.0f, 76.0f);           //      Sân lát hình bông hoa giữa 2 nhà mái xanh #7 & #8 (ĐK 22m) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 BLUE_ROOF_HALL_NORTH_POS(-113.4f, 0.0f, 55.0f);      // #7:  Nhà mái ngói xanh Bắc sân bông hoa (26x12m, xoay +12°) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 BLUE_ROOF_HALL_SOUTH_POS(-102.7f, 0.0f, 118.0f);     // #8:  Nhà mái ngói xanh Nam sân bông hoa (22x11m, xoay -15°, phía Nam dầm Monorail) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 ROUND_SWING_RIDE_1_POS(-132.7f, 0.0f, 12.9f);        // #9:  Đu quay tròn 1 mái cam-trắng (ĐK 17m, 16 ghế) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 ROUND_SPIN_RIDE_2_POS(-94.2f, 0.0f, 17.3f);          // #10: Đu quay tròn 2 đĩa tròn nhỏ (ĐK 13m, 12 ghế) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 RED_ROOF_LAKESIDE_HALL_POS(-94.2f, 0.0f, -71.2f);    // #12: Nhà ngói đỏ lớn ven hồ (22x18m, mái cong 2 tầng) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 CENTRAL_ROTUNDA_RIDE_POS(-141.3f, 0.0f, -137.6f);    // #13: Công trình tròn có hành lang cột khu giữa (ĐK 18m, 16 ghế) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 ROUND_SPIN_RIDE_3_POS(-143.4f, 0.0f, -179.6f);       // #14: Đu quay tròn 3 khu giữa (ĐK 15m, 14 ghế) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 WEST_CANOPY_THEATER_POS(-256.0f, 0.0f, -128.7f);     // #15: Nhà mái xanh-trắng khu Tây (20x20m, mái cong vòm rộng) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 ANGKOR_LAKESIDE_HALL_POS(-138.0f, 0.0f, -242.0f);    // #18: Nhà mái xanh khu Angkor ven hồ (20x14m, xoay -22°) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]
constexpr MathCG::Vec3 ANGKOR_STAR_PLAZA_POS(-185.0f, 0.0f, -254.0f);       // #19: Quảng trường Ngôi sao & 4 tháp chòi nhọn Angkor (24x24m) [Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]

// ============================================================================
// 8. ĐỢT 6A & 6B: TIỂU CẢNH ĐÈN ĐƯỜNG, GHẾ NGHỈ, BỒN HOA, CỘT CỜ, BIỂN CHỈ DẪN
//    VÀ 123 CÂY XANH PHÂN BỐ KHU TÂY / TÂY-NAM / QUẢNG TRƯỜNG / BỜ HỒ (NỬA ĐÔNG TRỐNG)
//    Đã kiểm tra bằng script: 0 cây trong hồ, 0 cây trên lối đi, 0 cây đè công trình,
//    cách dầm và trụ Monorail >= 3.6m.
// ============================================================================

struct TreePlacement {
    float x;
    float z;
    int   style; // 0: Cây tán tròn rộng, 1: Cây thông/tùng tháp nhọn, 2: Cây cọ/dừa cảnh quan
    float scale;
};

struct OrientedPropXZ {
    float x;
    float z;
    float rotYDeg;
    int   variant;
};

// 8.1. 24 Trụ đèn cảnh quan dọc Quảng trường Trung tâm, Sân cổng Ngọ Môn & Trục lối đi chính phía Tây
//      (Chỉ dùng vật liệu phát sáng emissive khi chiều/tối, KHÔNG dùng thêm nguồn sáng GL_LIGHTi)
constexpr int NUM_PARK_PATH_LAMPS = 24;
constexpr PointXZ PARK_PATH_LAMP_POS[NUM_PARK_PATH_LAMPS] = {
    // Dọc hai mép Đông - Tây Quảng trường Trung tâm
    { -25.5f, 106.0f }, {  57.5f, 106.0f },
    { -25.5f,  84.0f }, {  57.5f,  84.0f },
    { -25.5f,  46.0f }, {  57.5f,  46.0f },
    { -25.5f,  22.0f }, {  57.5f,  22.0f },
    { -25.5f,   0.0f }, {  57.5f,   0.0f },
    // Sân đón phía Nam & phía Bắc Cổng Ngọ Môn
    {   4.0f, 156.0f }, {  28.2f, 156.0f },
    {   4.0f, 114.0f }, {  28.2f, 114.0f },
    // Dọc trục lối đi khu Tây & đường nối ngang
    { -68.0f,  95.0f }, { -112.0f,  87.0f },
    { -156.0f,  81.0f }, { -183.0f,  46.0f },
    { -190.0f, -18.0f }, { -238.0f, -50.0f },
    { -228.0f,-108.0f }, { -222.0f,-185.0f },
    {  -68.0f,  40.0f }, { -142.0f,  38.0f }
};

// 8.2. 16 Ghế nghỉ công viên đặt dọc rìa quảng trường và lối đi
constexpr int NUM_PARK_BENCHES = 16;
constexpr OrientedPropXZ PARK_BENCH_POS[NUM_PARK_BENCHES] = {
    { -25.0f,  95.0f,  90.0f, 0 }, {  57.0f,  95.0f, -90.0f, 0 },
    { -25.0f,  34.0f,  90.0f, 0 }, {  57.0f,  34.0f, -90.0f, 0 },
    { -25.0f,  10.0f,  90.0f, 0 }, {  57.0f,  10.0f, -90.0f, 0 },
    {   6.0f, 150.0f,  90.0f, 0 }, {  26.2f, 150.0f, -90.0f, 0 },
    { -45.0f, -63.0f,   0.0f, 0 }, {  22.0f, -63.0f,   0.0f, 0 },
    { -96.0f,  74.0f, -45.0f, 0 }, { -120.0f, 74.0f,  45.0f, 0 },
    { -183.0f, 32.0f,  90.0f, 0 }, { -190.0f,-10.0f,  90.0f, 0 },
    { -238.0f,-62.0f,  45.0f, 0 }, { -208.0f,-270.0f,  0.0f, 0 }
};

// 8.3. 16 Bồn hoa hộp màu trang trí tại Quảng trường Cổng, Quảng trường Trung tâm & khu Tây
constexpr int NUM_PARK_FLOWER_PLANTERS = 16;
constexpr OrientedPropXZ PARK_FLOWER_PLANTER_POS[NUM_PARK_FLOWER_PLANTERS] = {
    {   5.5f, 160.0f,  0.0f, 0 }, {  26.7f, 160.0f,  0.0f, 1 },
    {   5.5f, 112.0f,  0.0f, 2 }, {  26.7f, 112.0f,  0.0f, 0 },
    {  -8.0f,  86.0f,  0.0f, 1 }, {  38.0f,  86.0f,  0.0f, 2 },
    {  -8.0f,  42.0f,  0.0f, 0 }, {  38.0f,  42.0f,  0.0f, 1 },
    { -18.0f,   6.0f,  0.0f, 2 }, {  18.0f,   6.0f,  0.0f, 0 },
    { -18.0f,  -6.0f,  0.0f, 1 }, {  18.0f,  -6.0f,  0.0f, 2 },
    { -98.0f,  66.0f, 12.0f, 0 }, { -118.0f, 66.0f, 12.0f, 1 },
    { -162.0f, 82.0f,  0.0f, 2 }, { -238.0f,-46.0f,  0.0f, 0 }
};

// 8.4. 6 Cột cờ lễ hội tại Sân Cổng Ngọ Môn và Quảng trường Sân khấu Sun Wheel
constexpr int NUM_PARK_FLAGPOLES = 6;
constexpr PointXZ PARK_FLAGPOLE_POS[NUM_PARK_FLAGPOLES] = {
    {  -3.0f, 148.0f }, {  35.2f, 148.0f }, // 2 cột cờ trước Cổng Ngọ Môn
    { -28.0f,  -7.5f }, { -21.0f,  -7.5f }, // 2 cột cờ cánh Tây Sân khấu Sun Wheel
    {  21.0f,  -7.5f }, {  28.0f,  -7.5f }  // 2 cột cờ cánh Đông Sân khấu Sun Wheel
};

// 8.5. 6 Biển chỉ dẫn đa hướng tại các ngã ba lối đi chính
constexpr int NUM_PARK_SIGNBOARDS = 6;
constexpr OrientedPropXZ PARK_SIGNBOARD_POS[NUM_PARK_SIGNBOARDS] = {
    {  10.0f,  96.0f,   0.0f, 0 }, // Ngã ba sân Bắc Ngọ Môn rẽ sang Ga Monorail Nam
    {  -4.0f,  25.0f,  30.0f, 1 }, // Ngã ba Quảng trường Sun Wheel rẽ sang khu Tây
    { -110.0f, 86.0f, -20.0f, 2 }, // Ngã ba Sân bông hoa khu Tây-Nam
    { -183.0f, 44.0f,  45.0f, 0 }, // Ngã ba Port of Sky Treasure / Pirate Ship
    { -239.0f,-52.0f,  15.0f, 1 }, // Quảng trường Đông Sky Tower
    { -210.0f,-272.0f,-30.0f, 2 }  // Ngã ba Quảng trường Angkor dẫn lên Queen Cobra
};

// 8.6. 123 Cây xanh đơn giản (thân trụ + tán cầu/nón/lá cọ, vẽ bằng Display List)
constexpr int NUM_PARK_TREES = 123;
constexpr TreePlacement PARK_TREES[NUM_PARK_TREES] = {
    {  -42.0f,  152.0f, 0, 0.90f }, {  -36.0f,  166.0f, 1, 1.05f },
    {   -2.0f,  169.0f, 2, 1.18f }, {   34.0f,  169.0f, 0, 0.96f },
    {   68.0f,  152.0f, 1, 1.25f }, {   76.0f,  152.0f, 2, 1.10f },
    {  -46.0f,  127.0f, 0, 0.88f }, {   44.0f,  127.0f, 1, 0.90f },
    {   58.0f,  127.0f, 2, 1.05f }, {   72.0f,  127.0f, 0, 1.18f },
    {   -4.0f,  119.0f, 1, 0.96f }, {   52.0f,  121.0f, 2, 1.25f },
    {  -31.0f,  102.0f, 0, 1.10f }, {  -31.0f,   88.0f, 1, 0.88f },
    {  -31.0f,   76.0f, 2, 0.90f }, {  -31.0f,   50.0f, 0, 1.05f },
    {  -31.0f,   40.0f, 1, 1.18f }, {   63.0f,  104.0f, 2, 0.96f },
    {   63.0f,   90.0f, 0, 1.25f }, {   63.0f,   76.0f, 1, 1.10f },
    {   63.0f,   50.0f, 2, 0.88f }, {   63.0f,   36.0f, 0, 0.90f },
    {   63.0f,   20.0f, 1, 1.05f }, {   63.0f,    4.0f, 2, 1.18f },
    {   63.0f,  -12.0f, 0, 0.96f }, {   52.0f,  -20.0f, 1, 1.25f },
    {   52.0f,  -38.0f, 2, 1.10f }, {   52.0f,  -54.0f, 0, 0.88f },
    {  -56.0f,  -58.0f, 1, 0.90f }, {  -42.0f,  -58.0f, 2, 1.05f },
    {  -22.0f,  -61.0f, 0, 1.18f }, {   10.0f,  -61.0f, 1, 0.96f },
    {   26.0f,  -61.0f, 2, 1.25f }, {   42.0f,  -59.0f, 0, 1.10f },
    {  -71.0f,  -82.0f, 1, 0.88f }, {  -73.0f,  -98.0f, 2, 0.90f },
    {  -83.0f, -145.0f, 0, 1.05f }, {  -87.0f, -163.0f, 1, 1.18f },
    {  -91.0f, -179.0f, 2, 0.96f }, { -102.0f, -207.0f, 0, 1.25f },
    { -112.0f, -223.0f, 1, 1.10f }, { -135.0f, -271.0f, 2, 0.88f },
    { -148.0f, -287.0f, 0, 0.90f }, {  -52.0f,  111.0f, 1, 1.05f },
    {  -64.0f,  108.0f, 2, 1.18f }, {  -80.0f,  124.0f, 0, 0.96f },
    {  -64.0f,  130.0f, 1, 1.25f }, {  -82.0f,  142.0f, 2, 1.10f },
    {  -96.0f,  146.0f, 0, 0.88f }, {  -92.0f,  162.0f, 1, 0.90f },
    { -148.0f,  148.0f, 2, 1.05f }, { -158.0f,  136.0f, 0, 1.18f },
    { -174.0f,  132.0f, 1, 0.96f }, { -186.0f,  128.0f, 2, 1.25f },
    { -228.0f,  124.0f, 0, 1.10f }, { -238.0f,  108.0f, 1, 0.88f },
    { -235.0f,   92.0f, 2, 0.90f }, { -180.0f,  106.0f, 0, 1.05f },
    { -180.0f,   92.0f, 1, 1.18f }, { -142.0f,   96.0f, 2, 0.96f },
    { -134.0f,   68.0f, 0, 1.25f }, { -148.0f,   66.0f, 1, 1.10f },
    { -166.0f,   66.0f, 2, 0.88f }, { -180.0f,   66.0f, 0, 0.90f },
    {  -82.0f,   86.0f, 1, 1.05f }, {  -64.0f,   86.0f, 2, 1.18f },
    {  -50.0f,   82.0f, 0, 0.96f }, {  -50.0f,   68.0f, 1, 1.25f },
    {  -68.0f,   68.0f, 2, 1.10f }, {  -84.0f,   66.0f, 0, 0.88f },
    {  -84.0f,   50.0f, 1, 0.90f }, {  -68.0f,   50.0f, 2, 1.05f },
    {  -50.0f,   50.0f, 0, 1.18f }, {  -72.0f,   22.0f, 1, 0.96f },
    {  -72.0f,    6.0f, 2, 1.25f }, {  -72.0f,  -10.0f, 0, 1.10f },
    {  -72.0f,  -26.0f, 1, 0.88f }, {  -72.0f,  -44.0f, 2, 0.90f },
    {  -88.0f,   -4.0f, 0, 1.05f }, {  -88.0f,  -42.0f, 1, 1.18f },
    { -116.0f,  -44.0f, 2, 0.96f }, { -134.0f,  -44.0f, 0, 1.25f },
    { -152.0f,  -44.0f, 1, 1.10f }, { -170.0f,  -44.0f, 2, 0.88f },
    { -156.0f,   18.0f, 0, 0.90f }, { -156.0f,  -22.0f, 1, 1.05f },
    { -188.0f,  -44.0f, 2, 1.18f }, { -206.0f,  -44.0f, 0, 0.96f },
    { -222.0f,  -54.0f, 1, 1.25f }, { -134.0f,  -66.0f, 2, 1.10f },
    { -152.0f,  -66.0f, 0, 0.88f }, { -170.0f,  -66.0f, 1, 0.90f },
    { -206.0f,  -66.0f, 2, 1.05f }, { -224.0f,  -68.0f, 0, 1.18f },
    { -248.0f,  -78.0f, 1, 0.96f }, { -124.0f,  -88.0f, 2, 1.25f },
    { -144.0f,  -88.0f, 0, 1.10f }, { -156.0f, -106.0f, 1, 0.88f },
    { -134.0f, -112.0f, 2, 0.90f }, { -114.0f, -112.0f, 0, 1.05f },
    {  -96.0f, -112.0f, 1, 1.18f }, {  -98.0f, -134.0f, 2, 0.96f },
    { -116.0f, -134.0f, 0, 1.25f }, { -116.0f, -156.0f, 1, 1.10f },
    { -102.0f, -156.0f, 2, 0.88f }, { -106.0f, -176.0f, 0, 0.90f },
    { -122.0f, -176.0f, 1, 1.05f }, { -166.0f, -142.0f, 2, 1.18f },
    { -166.0f, -162.0f, 0, 0.96f }, { -166.0f, -182.0f, 1, 1.25f },
    { -186.0f, -156.0f, 2, 1.10f }, { -206.0f, -156.0f, 0, 0.88f },
    { -216.0f, -174.0f, 1, 0.90f }, { -246.0f, -160.0f, 2, 1.05f },
    { -256.0f, -178.0f, 0, 1.18f }, { -256.0f, -198.0f, 1, 0.96f },
    { -256.0f, -218.0f, 2, 1.25f }, { -256.0f, -242.0f, 0, 1.10f },
    { -124.0f, -198.0f, 1, 0.88f }, { -144.0f, -204.0f, 2, 0.90f },
    { -152.0f, -220.0f, 0, 1.05f }, { -160.0f, -256.0f, 1, 1.18f },
    { -212.0f, -252.0f, 2, 0.96f }
};

} // namespace ParkLayout

#endif // PARK_LAYOUT_H
