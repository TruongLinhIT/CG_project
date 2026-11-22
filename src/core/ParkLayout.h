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

// 2.4. Cổng Ngọ Môn & Tháp Đồng Hồ (mốc 2019) - [Ước lượng từ ảnh vệ tinh] ±8m
// Cổng Ngọ Môn tại 16.03890°N, 108.22853°E (rộng thực tế ~35m -> scale 0.76 từ mô hình gốc 46m)
constexpr MathCG::Vec3 NGO_MON_GATE_POS(16.1f, 0.0f, 139.1f);
constexpr float        NGO_MON_GATE_SCALE            = 0.76f;
// Tháp Đồng Hồ (2019) tại 16.03960°N, 108.22848°E (cách Cổng Ngọ Môn ~77.5m về phía Bắc, scale 1.0)
constexpr MathCG::Vec3 CLOCK_TOWER_POS(10.7f, 0.0f, 61.6f);
constexpr float        CLOCK_TOWER_SCALE             = 1.00f;

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
constexpr MathCG::Vec3 MONORAIL_SOUTH_STATION_POS(-24.6f, 0.0f, 128.0f);   // Ga Nam (16.03900°N, 108.22815°E)
constexpr MathCG::Vec3 MONORAIL_NEPAL_STATION_POS(-217.2f, 0.0f, -220.6f); // Ga Nepal (16.04215°N, 108.22635°E)

constexpr int NUM_MONORAIL_WAYPOINTS = 20;
constexpr MathCG::Vec3 MONORAIL_WAYPOINTS[NUM_MONORAIL_WAYPOINTS] = {
    {   16.1f, 12.0f,  128.0f }, // 0: Cầu vượt trước Cổng Ngọ Môn (16.03900, 108.22853)
    {  -24.6f, 12.0f,  128.0f }, // 1: Ga Monorail Nam (16.03900, 108.22815)
    {  -78.1f, 12.2f,  119.1f }, // 2: Đoạn cong Tây-Nam (16.03908, 108.22765)
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
// Sân trong Ngọ Môn -> Ga Monorail Nam -> Ngựa gỗ -> Sky Tower -> Ga Monorail Nepal -> Queen Cobra
constexpr float PATH_WEST_SPINE_WIDTH = 5.0f;
constexpr int   NUM_PATH_WEST_SPINE_PTS = 14;
constexpr PointXZ PATH_WEST_SPINE[NUM_PATH_WEST_SPINE_PTS] = {
    {   16.1f,   94.8f }, // 0: Tách từ sân trong phía Bắc Cổng Ngọ Môn (16.03930, 108.22853)
    {  -24.6f,  111.4f }, // 1: Sân trước Ga Monorail Nam (16.03915, 108.22815)
    {  -70.6f,  100.3f }, // 2: Lối đi hướng sang quảng trường hoa (16.03925, 108.22772)
    { -113.4f,   81.5f }, // 3: Quảng trường hoa khu Tây-Nam (16.03942, 108.22732)
    { -159.4f,   81.5f }, // 4: Phía Bắc Đu quay Ngựa gỗ (16.03942, 108.22689)
    { -174.4f,   33.9f }, // 5: Ngã ba trục trò chơi miền Tây (16.03985, 108.22675)
    { -227.9f,  -32.5f }, // 6: Lối dẫn về quảng trường Sky Tower (16.04045, 108.22625)
    { -246.1f,  -54.6f }, // 7: Quảng trường phía Đông chân tháp Sky Tower (16.04065, 108.22608)
    { -233.3f, -109.9f }, // 8: Hành lang phía Bắc Sky Tower (16.04115, 108.22620)
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
    { -142.0f, -268.0f }, // 3: Bờ Tây hồ đoạn Bắc (16.04258, 108.22705)
    { -177.6f, -290.3f }, // 4: Mép Đông quảng trường Angkor (16.04278, 108.22672)
    { -214.0f, -275.9f }  // 5: Nhập vào trục chính phía Tây tại Angkor (16.04265, 108.22638)
};

// 5.6. Lối đi phụ nối ngang Quảng trường Sun Wheel sang trục trò chơi phía Tây (rộng 3.5 m):
constexpr float PATH_CENTRAL_CONNECTOR_WIDTH = 3.5f;
constexpr int   NUM_PATH_CENTRAL_CONNECTOR_PTS = 3;
constexpr PointXZ PATH_CENTRAL_CONNECTOR[NUM_PATH_CENTRAL_CONNECTOR_PTS] = {
    {    0.0f,   28.4f }, // 0: Quảng trường phía Nam Sun Wheel (16.03990, 108.22838)
    {  -67.4f,   33.9f }, // 1: Điểm giữa lối nối Đông-Tây (16.03985, 108.22775)
    { -174.4f,   33.9f }  // 2: Nhập vào trục chính phía Tây (16.03985, 108.22675)
};

} // namespace ParkLayout

#endif // PARK_LAYOUT_H
