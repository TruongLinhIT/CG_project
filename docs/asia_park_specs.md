# Tài liệu đối chiếu thông số & bố cục — Asia Park (Đà Nẵng)

> Tài liệu phục vụ chuẩn hóa bố cục và tỷ lệ dự án **Mô phỏng 3D Công viên Châu Á (Asia Park / Da Nang Downtown)**.
> **Mốc hiện trạng mô hình:** Khoảng **năm 2019** (`SNAPSHOT_YEAR = 2019` — thời điểm công viên còn nguyên Tháp Đồng Hồ tại quảng trường trung tâm, đầy đủ các trò chơi phía Tây và khu vực nửa phía Đông ranh giới là nền cỏ đơn giản nơi tuyến Monorail chạy qua).
> Cập nhật: 2026-11-24 (hoàn tất kiểm toán 18 ảnh trong `docs/img/ref/`, kiểm chứng độ tin cậy Wayback 2019 & **triển khai đầy đủ 100% các công trình trong bộ bản vẽ qua Đợt 8a + Đợt 8b + Đợt 8c có phím bật/tắt `[K]`**).

## 0. Quy tắc dữ liệu & thang tin cậy

| Nhãn | Ý nghĩa |
|:---|:---|
| **[Đã xác minh]** | Số liệu/tọa độ đọc trực tiếp từ trang nguồn hoặc API đã mở được nội dung gốc (Sun World, VnExpress, Wikipedia, RCDB, OpenStreetMap/Nominatim, ảnh mặt đất Wikimedia Commons có EXIF GPS). |
| **[Ước lượng từ ảnh vệ tinh]** | Đo trực tiếp trên ảnh vệ tinh gần phương thẳng đứng (nadir) Esri Wayback 2019 (`release 4756`) và Wayback 2022 (`release 45134`) ở độ phân giải Zoom 18–19 (`~0.29–0.57 m/px`), kèm sai số rõ ràng. |
| **[Không tìm thấy]** | Không có nguồn xác nhận đủ tin cậy. **Không tự đoán**; liệt kê ở mục 9 "Cần mình bổ sung". |

---

## 1. Quy ước hệ trục & đơn vị của engine (đọc từ mã nguồn)

| Hạng mục | Giá trị | Bằng chứng trong code |
|:---|:---|:---|
| Đơn vị | **1 unit = 1 mét** (tỷ lệ **1:1**) | Sun Wheel `radius = 50`, tâm `y = 65` → tổng cao 115 m ([SunWheel.h](../src/rides/SunWheel.h)) |
| Trục `+X` | **Hướng Đông** | Drone `yaw = +90°` → nhìn về `+X` ([Camera.h](../src/core/Camera.h), `getDroneForward`) |
| Trục `−Z` | **Hướng Bắc** (`+Z` = Nam) | Drone `yaw = 0°` → nhìn về `−Z`; Minimap ghi `"North (−Z)"` ([Minimap2D.h](../src/ui/Minimap2D.h)) |
| Trục `+Y` | Hướng lên (độ cao), mặt đất/mặt hồ ở `y = 0` | `drawParkGroundAndLake()` trong [main.cpp](../src/main.cpp) |
| Hệ tọa độ | Bàn tay phải, OpenGL | `gluLookAt`, `glFrontFace(GL_CCW)` |

---

## 2. Công thức quy đổi GPS (WGS84) → Descartes cục bộ → tọa độ Engine

Chọn **tâm chân tháp chữ A của Vòng quay Mặt Trời (Sun Wheel)** đo trên ảnh vệ tinh nadir Wayback 2022/2019 (Zoom 19) làm **gốc cục bộ `(0, 0)`**:
`φ0 = 16.040500°N`, `λ0 = 108.228380°E`.

*(Ghi chú hiệu chỉnh so với bản nháp đầu: bản ảnh 2026 chụp xiên góc từ Đông sang Tây nên đỉnh/tâm vòng quay cao 65 m bị dịch ảo về phía Tây ~25 m tới `16.040627°N, 108.228132°E`. Ảnh Wayback 2022 & 2019 Zoom 19 chụp gần thẳng đứng cho thấy rõ 4 chân đế chữ A ngay dưới bánh xe tại `16.040500°N, 108.228380°E`, giảm sai số gốc từ ±40 m xuống ±8 m).*

Hệ số mét/độ theo ellipsoid WGS84 tại `φ0`:

```text
m_lat = 111132.92 − 559.82·cos(2·φ0) + 1.175·cos(4·φ0) = 110 659.10 m/°
m_lon = 111412.84·cos(φ0) − 93.5·cos(3·φ0)             = 107 012.75 m/°
```

Công thức quy đổi sang hệ tọa độ cục bộ `(X_local, Z_local)` tính bằng mét (Đông = `+X`, Nam = `+Z`, Bắc = `−Z`):

```text
X_local = (λ − λ0) · m_lon
Z_local = −(φ − φ0) · m_lat
```

Quy đổi sang hệ tọa độ **Engine tỷ lệ 1:1** (giữ nguyên Sun Wheel đang ở `(0, 65, −38)` để hạn chế sửa đổi dây chuyền ở bước đầu):

```text
X_engine = X_local
Z_engine = Z_local − 38.0
```

---

## 3. Ảnh đối chiếu tổng thể (`layout_overlay.png`)

Bản đồ chồng lớp giữa **bố cục hiện tại trong code (`Xanh ngoc [HT]`)** và **bố cục thực tế / đề xuất (`Cam/Hong [DX]`)** nhìn từ trên xuống được lưu tại:

- [`img/layout_overlay.png`](img/layout_overlay.png) — nền ảnh vệ tinh Wayback 2019 (chưa bị cây che đường ray và còn nguyên Tháp Đồng Hồ), lưới mét `50 m / 100 m`, thước đo `0–200 m`, mũi tên hướng Bắc `N (-Z)`, vị trí các công trình, đường xung quanh, bờ hồ thật và ranh giới công viên đề xuất.
- Ảnh vệ tinh đối chứng khác: [`img/sat_grid.png`](img/sat_grid.png) (Esri 2026-03), [`img/water_mask.png`](img/water_mask.png) (tách vùng mặt nước).

---

## 4. Điểm tham chiếu xung quanh đã xác minh từ OpenStreetMap

| Đối tượng | OSM | GPS (lat, lon) | Local (X, Z) m | Engine (X, Z) m | Tin cậy |
|:---|:---|:---|:---|:---|:---|
| Đài Tưởng niệm Liệt sỹ TP Đà Nẵng (`historic=monument`) | [way 1331565206](https://www.openstreetmap.org/way/1331565206) | 16.040969, 108.224323 | (−434.2, −51.9) | (−434.2, −89.9) | [Đã xác minh] |
| Đường 2 Tháng 9 (đoạn Tây Đài tưởng niệm, `primary`) | [way 1392384112](https://www.openstreetmap.org/way/1392384112) | 16.041000, 108.223300 | (−543.6, −55.3) | (−543.6, −93.3) | [Đã xác minh] |
| Đường phụ phía Tây công viên (`secondary`, đoạn Bắc là Đường Bạch Đằng) | [way 1392384128](https://www.openstreetmap.org/way/1392384128) / [way 1392384120](https://www.openstreetmap.org/way/1392384120) | 16.040500–16.044800, ~108.22485 | (~−378, 0 … −476) | (~−378, −38 … −514) | [Đã xác minh] |
| Cung Thiếu nhi Đà Nẵng (góc Tây-Nam) | [way 667422707](https://www.openstreetmap.org/way/667422707) | 16.038400, 108.225200 | (−340.3, +232.4) | (−340.3, +194.4) | [Đã xác minh] |
| Trạm xe buýt "Công viên Châu Á" | [node 11882946634](https://www.openstreetmap.org/node/11882946634) | 16.038153, 108.225404 | (−318.5, +259.7) | (−318.5, +221.7) | [Đã xác minh] |
| Đường Phan Đăng Lưu (cạnh Nam công viên, `tertiary`) | [way 302765736](https://www.openstreetmap.org/way/302765736) / [way 1280089217](https://www.openstreetmap.org/way/1280089217) | ~16.0381–16.0388, 108.2238–108.2300 | trục Nam tại `Z_loc ≈ +210…+260` | `Z_eng ≈ +172…+222` | [Đã xác minh] |
| Đường Đỗ Pháp Thuận (dọc cạnh Đông Monorail) | [way 302765742](https://www.openstreetmap.org/way/302765742) | 16.040800, 108.231400 | (+323.2, −33.2) | (+323.2, −71.2) | [Đã xác minh] |
| Đường Thuận Yến (ven bờ Tây Sông Hàn) | [way 643263435](https://www.openstreetmap.org/way/643263435) | 16.042000, 108.232000 | (+387.4, −166.0) | (+387.4, −204.0) | [Đã xác minh] |
| Sông Hàn (`waterway=river`) | [relation 19756034](https://www.openstreetmap.org/relation/19756034) | bờ Tây tại `lon ~108.2325–108.2335` | `X_loc ≈ +440…+550` | `X_eng ≈ +440…+550` | [Đã xác minh] |
| Khu "Da Nang Downtown" (`landuse=construction`) | [way 667422708](https://www.openstreetmap.org/way/667422708) | 65 đỉnh (Phụ lục A), diện tích 18.4 ha | — | — | [Đã xác minh] |

---

## 5. Đối chiếu từng công trình/trò chơi (Hiện tại vs Thực tế)

### 5.1. Vòng quay Mặt Trời (Sun Wheel)

| Hạng mục | Scene hiện tại | Thực tế | Scale | Tin cậy | Nguồn & ghi chú |
|:---|:---|:---|:---:|:---|:---|
| Vị trí | `(0, 65, −38)` | **16.04050°N, 108.22838°E** → Gốc `(0, 65, −38)` | 1.00 | [Ước lượng từ ảnh vệ tinh] ±8 m | Chân đế chữ A thấy rõ trên Wayback 2022/2019 Z=19 (`sải Đông–Tây ~101 m`) |
| Chiều cao | **115 m** (`65 + 50`) | **115 m** | **1.00** | [Đã xác minh] | [sunworld.vn](https://sunworld.vn/vi/check-in/vong-quay-sun-wheel-tai-asia-park-niem-tu-hao-cua-da-nang-6125), [VnExpress](https://vnexpress.net/ngam-vong-quay-khong-lo-tai-da-nang-3017385.html), [Wikipedia](https://en.wikipedia.org/wiki/Sun_Wheel_(Da_Nang)) |
| Đường kính | **100 m** (`radius = 50`) | **~101 m** đo ảnh vệ tinh *(chưa mở được tài liệu hãng ghi số 100 m)* | 1.00 | [Ước lượng từ ảnh vệ tinh] ±5 m | Wayback 2022/2019 |
| Số cabin / sức chứa | 64 cabin | **64 cabin, 6 người/cabin, 384 khách/lượt** | 1.00 | [Đã xác minh] | sunworld.vn, VnExpress |
| Tốc độ quay | `0.4°/s` (thực tế) + chế độ tua nhanh trình diễn `×5` (`2.0°/s`, phím `[V]`) | **15 phút/vòng = `0.4°/s`** | tốc độ **1:1** (có phím tua `×5`) | [Đã xác minh] | sunworld.vn, VnExpress, Wikipedia |
| Số nan hoa / LED | 32 nan hoa | — | — | **[Không tìm thấy]** nguồn xác nhận | — |

### 5.2. Tháp rơi tự do Golden Sky Tower

| Hạng mục | Scene hiện tại | Thực tế | Scale | Tin cậy | Nguồn & ghi chú |
|:---|:---|:---|:---:|:---|:---|
| Vị trí | `(76, 0, −22)` *(lệch sang Đông)* | **16.04065°N, 108.22595°E** → Local `(−260.0, −16.6)` → Engine **`(−260.0, 0, −54.6)`** | — | [Ước lượng từ ảnh vệ tinh] ±10 m | Khớp chéo: ảnh mặt đất [Commons 49358003906](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49358003906).jpg) (`16.040402, 108.226638`, góc la bàn `264°W` thấy tháp vàng-cam ở WNW) + biển chỉ dẫn [Commons 49374882021](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49374882021).jpg) + bóng tháp trên Wayback 2022/2019 |
| Chiều cao | **47 m** (`totalHeight`) | **47 m** | **1.00** | [Đã xác minh] | [sunworld.vn](https://sunworld.vn/vi/check-in/vong-quay-sun-wheel-tai-asia-park-niem-tu-hao-cua-da-nang-6125) |
| Bề rộng thân / vòng ghế | 1.65 m / 3.35 m | — | — | **[Không tìm thấy]** | — |

### 5.3. Tàu lượn Queen Cobra

| Hạng mục | Scene hiện tại | Thực tế | Scale | Tin cậy | Nguồn & ghi chú |
|:---|:---|:---|:---:|:---|:---|
| Vị trí nhà ga | `(−68, 6, 39)` | **16.043515°N, 108.226111°E** → Local `(−242.8, −333.6)` → Engine **`(−242.8, 0, −371.6)`** | — | **[Đã xác minh]** | [RCDB #14194](https://rcdb.com/14194.htm) *(vẫn còn khung thép trên ảnh 2026)* |
| Tâm & phủ mặt bằng | `X ∈ [−103, −67.7]`, `Z ∈ [−76, 71.7]` (**35.3×147.7 m**, tâm `(−85.4, −2.2)`) | Tâm cụm đường ray **16.04365°N, 108.22625°E** → Local `(−227.9, −348.6)` → Engine **`(−227.9, 0, −386.6)`** (**~50×104 m**) | **KHÔNG SCALE** *(theo yêu cầu)* | [Ước lượng từ ảnh vệ tinh] ±5 m | Wayback 2019/2022/2026 + [RCDB #14194](https://rcdb.com/14194.htm) *(xem phân tích va chạm khi dời không scale ở Mục 8)* |
| Chiều dài đường ray | 359.2 m | **689 m** (Vekoma SLC 689m Standard, 5 vòng xoắn, 96 s) | giữ nguyên | [Đã xác minh] | [RCDB #14194](https://rcdb.com/14194.htm), [thanhnienmoi.com](https://thanhnienmoi.com/blogs/tau-luon-queen-cobra-tro-choi-mao-hiem-hang-dau-o-da-nang-574.html), [vienkyluc.vn](https://vienkyluc.vn/tin-tuc/tin-tuc/vietmaster-de-xuat-ky-luc-p-72-queen-cobra-cong-vien-asia-park-da-nang-tau-luon-dang-treo-lon-nhat-viet-nam) |
| Chiều cao tối đa | 47.6 m | **34 m** | giữ nguyên | [Đã xác minh] | sunworld.vn, thanhnienmoi.com, vienkyluc.vn |
| Vận tốc tối đa | 38 m/s (137 km/h) | **80 km/h (22.2 m/s)** | giữ nguyên / tốc độ chạy | [Đã xác minh] | thanhnienmoi.com, vienkyluc.vn |

### 5.4. Tàu điện Monorail

| Hạng mục | Scene hiện tại | Thực tế | Scale | Tin cậy | Nguồn & ghi chú |
|:---|:---|:---|:---:|:---|:---|
| Toàn tuyến khép kín | Vòng chữ nhật `X ∈ [±117.6]`, `Z ∈ [−102.4, 108.1]` (**767.7 m**) | Đọc từ 20 điểm dọc dầm trên ảnh vệ tinh: **`X_loc ∈ [−294, +368]`**, **`Z_loc ∈ [−262, +166]`**, dài đo được **`~1 867 m (~1.87 km)`** | Theo điểm số hóa 1:1 (~×2.4) | [Ước lượng từ ảnh vệ tinh] ±10 m | Dầm bê tông liên tục thấy rõ trên Wayback 2019 (`wb2019_z18_park`, `wb2019_east`) + ảnh tàu chạy trên ray [Commons 49374702231](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49374702231).jpg) (`16.042377, 108.226294`). Điểm nổi bật: tuyến bắc cầu vượt **qua mặt hồ** ở `lat ~16.0428` rồi uốn sang phía Đông tới sát **Đường Đỗ Pháp Thuận**. |
| Ga Nam (Cổng chính) | Ga Đông tại `(116, 14, 0)` | **16.03900°N, 108.22815°E** → Local `(−24.6, +166.0)` → Engine **`(−24.6, 0, +128.0)`** | — | [Ước lượng từ ảnh vệ tinh] ±12 m | Khớp biển chỉ dẫn [Commons 49370578306](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49370578306).jpg) (`16.038844, 108.228491`: `"NHA GA MONORAIL <- 20m"`) |
| Ga Nepal (Tây-Bắc) | Chưa có | **16.04215°N, 108.22635°E** → Local `(−217.2, −182.6)` → Engine **`(−217.2, 0, −220.6)`** | — | [Ước lượng từ ảnh vệ tinh] ±10 m | Ảnh trong nhà ga [Commons 49373368582](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49373368582).jpg) (`16.042144, 108.226294`) & biển chỉ dẫn `NHA GA NEPAL` |

### 5.5. Cổng Ngọ Môn & Tháp Đồng Hồ (đang gộp làm 1 trong `NgoMonGate.h`)

| Hạng mục | Scene hiện tại | Thực tế | Scale | Tin cậy | Nguồn & ghi chú |
|:---|:---|:---|:---:|:---|:---|
| Cổng Thành (Lối vào chính Ngọ Môn) | gộp chung tại `(0, 0, 86)` (rộng 46 m, sâu 15 m) | **16.03890°N, 108.22853°E** → Local `(+16.1, +177.1)` → Engine **`(+16.1, 0, +139.1)`** | ~0.76 (rộng thật ~35 m) | [Ước lượng từ ảnh vệ tinh] ±8 m | Ảnh chính diện [Commons 49356303688](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49356303688).jpg) (`16.038336, 108.228500`, hướng `22° NNE` nhìn thẳng vào cổng với Sun Wheel ngay phía sau) |
| Tháp Đồng Hồ (9 mặt, ~29 m, ~550 m²) | gộp chung trên nóc cổng ở `(0, 0, 86)` (cao 27.5 m) | **16.03960°N, 108.22848°E** → Local `(+10.7, +99.6)` → Engine **`(+10.7, 0, +61.6)`** *(cách Cổng ~77 m về phía Bắc)* | ~1.05 | [Ước lượng từ ảnh vệ tinh] ±8 m *(dữ liệu 2019)* | Thấy nguyên vẹn khối đế ~25×24 m & bóng tháp ~22 m trên ảnh Wayback 2019 (`wb19_south.png`); đã bị tháo dỡ trước 12/2022 |

### 5.6. Đu quay ngựa gỗ (Festival Carousel)

| Hạng mục | Scene hiện tại | Thực tế / Tọa độ dùng tạm | Tin cậy | Ghi chú |
|:---|:---|:---|:---|:---|
| Vị trí | `(45, 0, 48)` (Đông Nam) | **Dùng tạm `Engine (−159.4, 0, +102.5)`** (`16.03923°N, 108.22689°E`, `Local (−159.4, +140.5)`) — **Ghi chú: "chưa xác minh"** | **[Không tìm thấy / Chưa xác minh]** | Trên ảnh vệ tinh 2019/2022 ở khu Tây-Nam có 1 mái bạt hình nón kẻ sọc đen-trắng ĐK ~16.5 m tại `16.03923°N, 108.22689°E`, nhưng chưa có ảnh mặt đất biển tên đối chứng. Theo quyết định duyệt: dùng tạm `(-159.4, +102.5)` ở Đợt 3 và ghi chú rõ `"chưa xác minh"` trong `ParkLayout.h` và tài liệu. |
| Kích thước | bán kính 8.5 m (ĐK 17 m), cao 8.8 m | — | **[Không tìm thấy]** | — |

### 5.7. Thủy đình, Trụ đèn & các tàu lượn khác ghi nhận thêm trong thực tế

| Công trình | Thực tế (từ ảnh vệ tinh / RCDB) | Ghi chú với scene hiện tại | Tin cậy |
|:---|:---|:---|:---|
| Thủy đình trên hồ thật | **16.04113°N, 108.22833°E** → Local `(−5.4, −69.7)` → Engine **`(−5.4, 0, −107.7)`** | **Có thật!** Cầu gỗ dẫn ra chòi bát giác trên mặt hồ nằm ngay **phía Bắc** Sun Wheel (scene hiện đặt nhầm ở `(0, 0, 28)` phía Nam Sun Wheel). | [Ước lượng từ ảnh vệ tinh] ±8 m |
| Các tàu lượn khác tại công viên (chưa dựng trong code) | `Garuda Valley` (`16.038689, 108.227418`), `Port of Sky Treasure` (`16.039907, 108.226363`), `Paradise Fall` (`16.040998, 108.226822`), `Highway Boat` (`16.042034, 108.226667`) | Đều xác minh tọa độ qua RCDB & ảnh mặt đất; liệt kê để tham khảo không gian. | [Đã xác minh] (RCDB) |
| 10 trụ đèn lồng quanh hồ trong scene | Không thấy bố cục 10 trụ đều quanh mép chữ nhật như code; dọc lối đi thực tế có cột đèn công viên rải rác | Thuộc chi tiết trang trí trong scene (minh họa `ParametricShapes`). | [Không tìm thấy] bố cục giống code |

---

## 6. Camera & mặt nền hiện tại

| Thành phần | Giá trị hiện tại | File | Nhận xét so với thực tế |
|:---|:---|:---|:---|
| Thảm cỏ nền | `x, z ∈ [−140, 140]` (280×280 m ≈ 7.8 ha) | [main.cpp](../src/main.cpp) | Quá nhỏ; chỉ chứa được phần lõi quanh Sun Wheel. |
| Hồ nước | Chữ nhật `x ∈ [−58, 58], z ∈ [−62, 16]` | [main.cpp](../src/main.cpp) | Đè trực tiếp lên chân Sun Wheel (`z = −38`), sai hướng (lệch về Nam thay vì trải dài về Bắc). |
| Camera Quỹ đạo | `R ∈ [15, 380]`, `target ∈ [±120]`, `far = 1000` | [Camera.h](../src/core/Camera.h) | Khi mở rộng ra 800×750 m cần tăng `far` (~2200), nới `R` tối đa (~850) và `panTarget`. |
| Camera Ortho | Mắt cách `260 m`, `orthoSize = 115`, `zFar = ±500` | [Camera.h](../src/core/Camera.h) | Sẽ cắt mất các công trình xa ở rìa Bắc/Đông/Tây nếu không nới. |
| Drone | Biên bay `x, z ∈ [±220]`, tốc độ `12/35/75 m/s` | [Camera.h](../src/core/Camera.h) | Bị kẹp không bay tới được Queen Cobra (`z ≈ −387`), Sky Tower (`x ≈ −260`) hay bờ Đông (`x ≈ +387`). |
| Minimap 2D | WCS `x, z ∈ [−135, 135]`, hồ vẽ tĩnh hình chữ nhật | [Minimap2D.h](../src/ui/Minimap2D.h) | Mọi công trình xa rìa sẽ bay ra ngoài radar trừ khi mở rộng vùng WCS và thuật toán cắt xén/vẽ hồ mới. |

---

## 7. Đề xuất ranh giới công viên, đường xung quanh, bờ sông & hồ nước (Ước lượng)

> **Lưu ý:** Toàn bộ đa giác ranh giới công viên và bờ hồ dưới đây là **[Ước lượng từ ảnh vệ tinh]** dựa trên mặt đường OSM và mép công trình thấy trên ảnh Wayback 2019/2022 (không phải trích lục địa chính chính thức).

### 7.1. Ranh giới toàn khu công viên đề xuất (`~36.1 ha` bao vùng đất + dải hồ trung tâm theo đường bao Monorail)
Vùng bao khép kín trên [`layout_overlay.png`](img/layout_overlay.png) (nét đứt xanh lá, diện tích tính theo công thức Shoelace ≈ **36.1 ha**, trải rộng `X_engine ∈ [−380, +405]`, `Z_engine ∈ [−445, +175]`):
1. **Cạnh Nam** (giáp **Đường Phan Đăng Lưu**, OSM `way 302765736` / `1280089217`): từ góc Cung Thiếu nhi `(−302, +190)` qua quảng trường Cổng Chính `(+16, +172)` tới góc Đông-Nam gần ngã tư Đường Đỗ Pháp Thuận `(+398, +112)`.
2. **Cạnh Đông** (dọc **Đường Đỗ Pháp Thuận** `way 302765742` & **Đường Thuận Yến** `way 643263435` sát bờ Tây **Sông Hàn**): từ `(+398, +112)` chạy lên Bắc-Tây-Bắc tới mũi Đông-Bắc `(+248, −466)` / `(+91, −488)`.
3. **Cạnh Bắc** (giáp eo vịnh/hồ thông ra sông và khu **Đảo Xanh**): chạy ngang từ mũi Đông-Bắc vòng qua mép Bắc hồ và mũi bán đảo Queen Cobra tại `(−217, −449)` → `(−381, −414)`.
4. **Cạnh Tây** (giáp **tuyến đường phụ dọc phía Đông Đài Tưởng niệm 2/9** `way 1392384128` nối lên **Đường Bạch Đằng** `way 1392384120`): từ `(−381, −414)` chạy thẳng xuống Nam qua mép bãi đỗ xe và Cung Thiếu nhi về `(−302, +190)`.

*(Ghi chú phân biệt: Polygon `way 667422708` "Da Nang Downtown" của OSM rộng 18.4 ha chỉ phủ phân khu lõi Tây-Nam tới quảng trường Sun Wheel, chưa tính trảng cỏ phía Đông nơi tuyến Monorail thật đi qua tới Đường Đỗ Pháp Thuận).*

### 7.2. Hồ nước thực tế (bố cục chữ V/dải kéo dài về phía Bắc)
- Bắt đầu từ bờ kè ngay phía **Bắc** Sun Wheel (`Z_local ≈ −36 m` tức **`Z_engine ≈ −74 m`**), chiều rộng chỗ eo Nam `~116 m` (`X ∈ [−60, +58]`), mở rộng dần lên phía Bắc (`X ∈ [−205, +45]` tại vĩ độ Queen Cobra) và thông lên nhánh sông cạnh Đảo Xanh (`Z_local < −450 m`).
- Mặt phản xạ hồ `y = 0` khi đặt đúng phía Bắc (`Z_engine < −74 m`) vẫn phản chiếu trọn vẹn Sun Wheel (`Z_engine = −38 m`) khi người xem đứng ở bờ Bắc hoặc nhìn góc cao hướng Nam, thay vì đặt nước tràn qua chân tháp chữ A như hiện tại.

### 7.3. Sông Hàn, Cổng vào & Lối đi chính
- **Bờ Tây Sông Hàn**: chạy dọc góc nghiêng ~NNW–SSE ở phía Đông rìa công viên (`X_engine ≈ +430…+550 m`). Có thể dựng dải mặt nước Sông Hàn (`y = −0.2`) và dải đường nhựa bao quanh (Phan Đăng Lưu ở Nam, đường nội bộ/Bạch Đằng ở Tây, Đỗ Pháp Thuận/Thuận Yến ở Đông) làm khung bối cảnh.
- **Cổng vào & Trục lối đi chính**: từ vòng xoay Phan Đăng Lưu (`+16, +172`) đi thẳng qua **Cổng Thành / Ngọ Môn** (`+16, +139`), qua **Quảng trường Tháp Đồng Hồ** (`+11, +62`), hướng thẳng tới **Sun Wheel** (`0, −38`) và **Thủy đình trên mặt hồ** (`−5, −108`), cùng một nhánh lối đi chính rẽ Tây qua khu trò chơi (`Sky Tower`, `Monorail`).

---

## 8. Phân tích riêng bài toán dời Queen Cobra KHÔNG SCALE (ràng buộc Mục 4)

- Cụm `Queen Cobra` trong `RollerCoaster.h` có khung bao **35.3 m (Đông–Tây) × 147.7 m (Bắc–Nam)** (`X_eng ∈ [−103.0, −67.7]`, `Z_eng ∈ [−76.0, +71.7]`, tâm `(−85.4, −2.2)`).
- Trong khi đó, bán đảo thực tế của `Queen Cobra` trên ảnh vệ tinh chỉ dài **~104 m** theo phương Bắc–Nam (từ lối đi phía Nam ở `Z_loc ≈ −282 m` tới mép nước Bắc ở `Z_loc ≈ −404 m`, và mép hồ phía Đông nằm tại `X_loc ≈ −201 m`).
- **Kết quả kiểm tra bằng script (280 điểm lấy mẫu Bézier, không scale):**
  1. **Nếu dời thẳng tâm cụm tới đúng tâm thực tế `(−227.9, −386.6)` (góc xoay `0°`):**
     - Phần đường ray dài 147.7 m sẽ phủ `Z_loc ∈ [−422.4, −274.7]`, làm **59 / 280 điểm mẫu rơi ra ngoài đất khô** — cụ thể **mũi vòng lượn phía Bắc bị thò ra mặt nước hồ ~17 m** (qua mốc `Z_loc = −404 m`), đồng thời mũi Nam chạm sát mép lối đi bộ (`−282 m`).
  2. **Nếu cho phép tịnh tiến + xoay nhẹ cả cụm (vẫn giữ nguyên 100% hình học/kích thước, không scale) để né hoàn toàn mặt nước, đường và ranh giới:**
     - Kết quả thử nghiệm tìm thấy nếu **xoay cụm ~`−25°`** (hướng NNW–SSE) và dời tâm tới `Local (−255, −320)` (`Engine (−255, −358)`), cả 280 điểm tránh được mặt hồ, nhưng lúc đó cụm sẽ **lệch xuống ~30 m về phía Nam–Tây** so với vị trí thật trên ảnh (chồng lên vùng cỏ/nhà phụ cận phía Tây và sát khu Angkor).
- **Đề xuất chờ bạn quyết định:** Giữ đúng tọa độ thật `(−227.9, −386.6)` (chấp nhận phần đường ray Bắc vươn nhẹ ~17 m lên mặt hồ như trò chơi nhô ra mặt nước — nhìn khá đẹp và vẫn giữ đúng tâm RCDB `#14194`), **hoặc** dời nhẹ về Nam–Tây theo góc nghiêng `−25°` để mọi trụ cột nằm hoàn toàn trên đất khô, **hoặc** bạn cho phép cập nhật lại waypoint ở bước sau.

---

## 9. Các quyết định đã chốt để triển khai 3 đợt

1. **Mốc hiện trạng & phạm vi cảnh quan:** Mô hình biểu diễn hiện trạng khoảng **năm 2019**. Nửa phía Đông của ranh giới chỉ dựng mặt phẳng nền cỏ đơn giản. Không dựng/thêm cây cối, bụi cây hay chi tiết cảnh quan trang trí. Không gian xung quanh chỉ gồm: hồ nước phía Bắc Sun Wheel, đường bao quanh công viên (Phan Đăng Lưu, Đỗ Pháp Thuận/Thuận Yến, đường phía Tây), dải Sông Hàn, và **hệ thống lối đi phẳng** (`quad-strip` nâng cao `3.5–4 cm` tránh Z-fighting, rộng `5–6 m` cho lối chính và `3–3.5 m` cho lối phụ).
2. **Queen Cobra (Đợt 3):** Chọn **Phương án B** — chỉ tịnh tiến tâm cụm về `Engine (−255.0, 0, −358.0)` (`Local (−255.0, −320.0)`) và xoay `−25°` quanh trục Y, **KHÔNG scale**, giữ nguyên 100% hình học và animation.
3. **Đu quay Ngựa gỗ (Đợt 3):** Dùng tạm vị trí `Engine (−159.4, 0, +102.5)` (`16.03923°N, 108.22689°E`), gắn ghi chú `"chưa xác minh"` trong `src/core/ParkLayout.h` (`CAROUSEL_POS_UNVERIFIED`) và tài liệu.
4. **Cổng Ngọ Môn & Tháp Đồng Hồ (Đợt 3):** Đặt Cổng Ngọ Môn tại `(+16.1, 0, +139.1)` với `scale = 0.76`. Tách Tháp Đồng Hồ (nếu không ảnh hưởng hình học) đặt tại `(+10.7, 0, +61.6)` với `scale = 1.0`.
5. **Thủy đình & 10 trụ đèn lồng (Đợt 1):** Dời Thủy đình về vị trí thực tế `(−5.4, 0, −107.7)` trên mặt hồ phía Bắc Sun Wheel kèm cầu gỗ nối vào bờ Nam (`y = 0.25 m`); đặt 10 trụ đèn lồng dọc bờ kè phía Nam của hồ mới (`Z_eng = −70.0 m`).
6. **Tốc độ Sun Wheel (Đợt 1):** Đặt hằng số tốc độ thực `0.4°/s` (15 phút/vòng) và hệ số tua nhanh trình diễn `×5` (`2.0°/s`) bật/tắt bằng phím `[V]` (hoặc menu chuột phải).
7. **Hệ thống lối đi nội bộ (Đợt 1 — `[Ước lượng từ ảnh vệ tinh]`):** Đã số hóa thành các mảng hằng số trong `src/core/ParkLayout.h` theo đúng vết lối đi trên ảnh vệ tinh Wayback 2019 (`wb19_south.png`, `wb19_mid.png`, `wb19_north.png`), nối liên hoàn:
   - `PATH_MAIN_SPINE` (rộng `6.0 m`): Quảng trường Phan Đăng Lưu `(16.1, 166.0)` → xuyên qua vòm trung tâm Cổng Ngọ Môn `(16.1, 139.1)` → vòng qua phía Đông Tháp Đồng Hồ `(30.0, 74.9 → 48.3)` → Quảng trường Nam Sun Wheel `(0.0, 28.4 → 0.7)` → vòng qua hành lang Tây Sun Wheel `(−34.2, −2.6 → −66.8)` → đầu cầu Thủy đình `(−5.4, −66.8)`.
   - `PATH_SOUTH_LAKE_PROM` (rộng `5.0 m`): Dọc bờ kè Nam hồ nước từ `(−63.5, −66.8)` tới `(+53.5, −65.7)`.
   - `PATH_PAVILION_PIER` (rộng `3.0 m`, cầu cao `y = 0.25 m`): Từ bờ kè Nam `(−5.4, −66.8)` dẫn thẳng ra bệ đá Thủy đình `(−5.4, −102.7)`.
   - `PATH_WEST_SPINE` (rộng `5.0 m`): Tách từ sân trong Ngọ Môn `(16.1, 94.8)` → Ga Monorail Nam `(−24.6, 111.4)` → qua phía Bắc Ngựa gỗ `(−159.4, 81.5)` → trục trò chơi phía Tây `(−174.4, 33.9)` → Quảng trường Đông Sky Tower `(−246.1, −54.6)` → Ga Monorail Nepal `(−231.1, −220.6)` → Quảng trường ngôi sao Angkor `(−214.0, −275.9)` → lối vào Nam Queen Cobra `(−246.1, −334.6)`.
   - `PATH_LAKESIDE_WEST` (rộng `3.0 m`) & `PATH_CENTRAL_CONNECTOR` (rộng `3.5 m`): Tuyến phụ ven bờ Tây hồ nước và tuyến nối ngang từ Quảng trường Sun Wheel sang trục phía Tây.
   - **Kiểm tra va chạm:** Đã kiểm tra bằng script hình học (`0` điểm lối đi mặt đất phạm vào đa giác hồ nước `LAKE_OUTLINE_POLY` hay chân các công trình đề xuất).

---

## 10. Bảng tổng hợp cuối cùng sau Đợt 3 & Danh mục các điểm đơn giản hóa / khác thực tế

### 10.1. Bảng thông số vị trí, góc xoay, tỷ lệ (Scale) & độ tin cậy cuối cùng (trong `ParkLayout.h`)

| STT | Công trình / Hạng mục | Hằng số trong `ParkLayout.h` | Tọa độ Engine `(X, Y, Z)` (m) | Góc xoay `Y` | Scale | Độ tin cậy & Sai số |
|:---:|:---|:---|:---|:---:|:---:|:---|
| 1 | **Vòng quay Mặt Trời (Sun Wheel)** | `SUN_WHEEL_CENTER` | `(0.0, 65.0, −38.0)` *(ĐK 100 m, cao 115 m, tốc độ `0.4°/s` + tua `×5`)* | `0°` | `1.00` | **[Đã xác minh]** kích thước & tốc độ; tọa độ gốc **[Ước lượng từ ảnh vệ tinh] ±8 m** |
| 2 | **Thủy đình trên mặt hồ Bắc** | `LAKESIDE_PAVILION_POS` | `(−5.4, 0.0, −107.7)` *(kèm cầu gỗ dài 35.9 m, `y = 0.25 m`)* | `0°` | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±8 m** |
| 3 | **Tháp rơi tự do Golden Sky Tower** | `SKY_TOWER_POS` | `(−260.0, 0.0, −54.6)` *(cao 47 m)* | `0°` | `1.00` | **[Đã xác minh]** chiều cao 47 m; vị trí **[Ước lượng từ ảnh vệ tinh] ±10 m** |
| 4 | **Tàu lượn Queen Cobra (cả cụm ray + ga)** | `QUEEN_COBRA_TARGET_CENTER` | Tâm cụm tại `(−255.0, 0.0, −358.0)` *(tịnh tiến từ `(−85.35, 0, −2.15)`)* | `−25.0°` | **`1.00` (Không scale)** | Vị trí thật nhà ga **[Đã xác minh]** (RCDB `#14194`); vị trí đặt theo Phương án B để 100% chân trụ nằm trên đất khô |
| 5 | **Tàu điện Monorail (~1.87 km)** | `MONORAIL_WAYPOINTS[20]` | `X ∈ [−294.3, +368.1]`, `Z ∈ [−300.3, +128.0]` *(dài 1 904 m, `v = 9.2 m/s` ≈ 3.45 phút/vòng)* | Theo tiếp tuyến `P'(t)` | `1.00` (1:1 theo GPS) | **[Ước lượng từ ảnh vệ tinh] ±10 m** |
| 6 | **Ga Monorail Nam (Cổng chính)** | `MONORAIL_SOUTH_STATION_POS` | `(−24.6, 0.0, +128.0)` | `−90.0°` | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±12 m** (khớp biển chỉ dẫn Commons) |
| 7 | **Ga Monorail Nepal (Khu Tây-Bắc)** | `MONORAIL_NEPAL_STATION_POS` | `(−217.2, 0.0, −220.6)` | `0.0°` | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±10 m** (khớp ảnh GPS trong ga Commons) |
| 8 | **Cổng Thành chính (Ngọ Môn)** | `NGO_MON_GATE_POS` | `(+16.1, 0.0, +139.1)` *(rộng thực tế ~35 m)* | `0.0°` | `0.76` | **[Ước lượng từ ảnh vệ tinh] ±8 m** (khớp ảnh chính diện Commons) |
| 9 | **Tháp Đồng Hồ Trung Tâm (2019)** | `CLOCK_TOWER_POS` | `(+10.7, 0.0, +61.6)` *(tách rời cách Cổng Ngọ Môn ~77.5 m về phía Bắc)* | `0.0°` | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±8 m** (ảnh Wayback 2019) |
| 10 | **Đu quay Ngựa gỗ (Festival Carousel)** | `CAROUSEL_POS_UNVERIFIED` | `(−159.4, 0.0, +102.5)` *(mái nón kẻ sọc ĐK ~16.5 m)* | `0.0°` | `1.00` | **[Không tìm thấy / Chưa xác minh]** (dùng tạm ứng viên ảnh vệ tinh) |
| 11 | **Hồ nước phía Bắc Sun Wheel** | `LAKE_OUTLINE_POLY[11]` | `Z_eng ∈ [−513.8, −73.4]`, `X_eng ∈ [−297.5, +57.8]` (`y = 0.0 m`) | — | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±10 m** |
| 12 | **Ranh giới nền cỏ công viên (~36.1 ha)** | `PARK_BOUNDARY_POLY[16]` | `X_eng ∈ [−381.0, +403.4]`, `Z_eng ∈ [−486.2, +188.9]` (`y = −0.04 m`) | — | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±15 m** |
| 13 | **Hệ thống lối đi nội bộ (6 tuyến)** | `PATH_MAIN_SPINE`, `PATH_WEST_SPINE`, ... | Rộng `5–6 m` (chính), `3–3.5 m` (phụ), `y = 0.035–0.04 m` (cầu Thủy đình `y = 0.25 m`) | — | `1.00` | **[Ước lượng từ ảnh vệ tinh] ±5 m** |

---

### 10.2. Danh sách các điểm đơn giản hóa / khác biệt so với thực tế (phục vụ thuyết minh báo cáo đồ án)

1. **Mốc thời gian hiện trạng mô hình (`SNAPSHOT_YEAR = 2019`):**
   - Toàn bộ bố cục đại diện cho hiện trạng công viên khoảng **năm 2019** (thời điểm còn nguyên **Tháp Đồng Hồ** tại quảng trường trung tâm và cây cối chưa che khuất tuyến dầm Monorail). Trên thực tế, Tháp Đồng Hồ đã được tháo dỡ trước tháng 12/2022 để mở rộng quảng trường sự kiện.
   - Khu vực **nửa phía Đông của ranh giới công viên** (nơi tuyến Monorail vươn ra sát Đường Đỗ Pháp Thuận) được biểu diễn đúng như ảnh vệ tinh 2019 là **mặt phẳng nền cỏ đơn giản**, không dựng thêm công trình giả định.
2. **Tàu lượn Queen Cobra — Giữ nguyên hình học & KHÔNG scale (Phương án B):**
   - Để bảo toàn 100% đường cong Bézier bậc 3 khép kín $C^1$, cấu trúc khung ray và thuật toán chuyển động theo bảo toàn cơ năng đã xây dựng, cụm mô hình Queen Cobra (`35.3 × 147.7 m`, dài ray `359.2 m`, cao đỉnh `47.0 m`) **không bị co giãn (scale = 1.0)** so với mặt bằng thực tế của Vekoma SLC 689m (`~50 × 104 m`, dài ray `689 m`, cao `34 m`).
   - Do khung bao mô hình dài hơn bán đảo thực tế ~43 m, cụm Queen Cobra được **tịnh tiến về `Engine (−255.0, 0, −358.0)` và xoay `−25°` quanh trục Y** để toàn bộ `280 / 280` điểm đường ray, chân trụ đỡ và nhà ga đều nằm hoàn toàn trên đất khô (cách mép hồ tối thiểu `5.48 m`).
   - **Kiểu tàu (Train Style):** Mô hình trong code là dạng **tàu lượn chạy bên trên đường ray** (sit-down coaster có bogie bánh xe lăn trên 2 thanh ray), trong khi Queen Cobra thực tế tại Asia Park là dạng **tàu lượn treo dưới đường ray** (Suspended Looping Coaster - ghế treo dưới dầm ray và có 5 vòng đảo ngược).
3. **Tàu điện Monorail — Làm mượt tiếp tuyến $C^1$ & nhịp cầu vượt mặt hồ:**
   - Tuyến Monorail thực tế là dạng tàu điện treo dưới dầm (Suspended Monorail), còn trong mô hình đồ họa được biểu diễn dưới dạng **tàu điện một ray kiểu straddle-beam ôm trên dầm hộp bê tông** để minh họa rõ hình học tiết diện dầm và 4 toa tàu khớp nối trên cao.
   - Do 20 điểm điều khiển số hóa từ ảnh vệ tinh có khoảng cách nhịp không đều (`28.7 m` ở các góc cua đến `177.6 m` ở các đoạn thẳng dài), bộ tạo spline Bézier trong `BezierTrack.h` đã **chuẩn hóa độ dài tay đòn tiếp tuyến theo nhịp ngắn hơn lân cận** ($\|P_1 - P_0\| \le 0.36 \min(d_{\text{prev}}, d_{\text{next}})$) trong khi giữ nguyên phương tiếp tuyến chung tại mỗi đỉnh. Nhờ đó đường ray đạt liên tục $C^0, C^1$ tuyệt đối mà không bị uốn chữ S thừa ở các góc cua hẹp.
   - Tại đoạn dầm Monorail vượt qua mặt hồ trung tâm (`X_eng ∈ [−132, +41]`, `Z_eng ≈ −298 m`), chương trình **tự động lược bỏ các trụ cột bê tông rơi vào lòng hồ** (`ParkLayout::isPointInLake`) và thay bằng **2 trụ mố cầu gia cường trên hai bờ Đông – Tây cùng dầm hộp cầu vượt chịu lực dưới đáy ray**, đảm bảo không có chân trụ nào cắm xuống giữa mặt nước.
   - Vận tốc hành trình được đặt ở `9.2 m/s` (`~33.1 km/h`), hoàn thành 1 vòng `1 904 m` trong khoảng **`3.45 phút`** (`~207 giây`).
4. **Đu quay Ngựa gỗ (Festival Carousel) — Vị trí `"chưa xác minh"`:**
   - Đặt tại `Engine (−159.4, 0.0, +102.5)` tương ứng với công trình mái bạt hình nón kẻ sọc đường kính `~16.5 m` ở khu trò chơi Tây-Nam trên ảnh vệ tinh (`16.03923°N, 108.22689°E`). Do chưa tìm thấy ảnh chụp mặt đất có biển tên xác nhận chính xác đây là Festival Carousel, vị trí này được đánh dấu rõ là **`[Chưa xác minh]`** trong cả mã nguồn và tài liệu.
5. **Tốc độ Vòng quay Mặt Trời (Sun Wheel) — Thực tế `0.4°/s` kèm phím tua nhanh trình diễn `×5`:**
   - Tốc độ góc mặc định tuân thủ đúng thực tế `15 phút/vòng` (`0.4°/s`). Vì tốc độ thật quay rất chậm khi thuyết trình đồ án trực tiếp, mô phỏng cung cấp phím tắt **`[V]`** (và mục menu chuột phải) để bật/tắt chế độ tua nhanh trình diễn **`×5` (`2.0°/s`)**.
6. **Phạm vi cảnh quan & các trò chơi phụ trợ:**
   - Mặt nền công viên được giản lược thành mặt phẳng cỏ một màu theo đúng biên đa giác `~36.1 ha`, kết hợp hồ nước phía Bắc, dải Sông Hàn, 3 trục đường bao quanh và hệ thống lối đi phẳng nâng cao `3.5–4 cm` (không dựng cây cối hay tiểu cảnh trang trí).
   - Ba mô hình trò chơi phụ trợ từ nhánh thành viên nhóm (`Playground`, `BumperCars`, `PirateShip`) và các nhân vật đi bộ (`Human`) được giữ nguyên hình học & animation ở khu vực sân chơi trung tâm để phục vụ chấm điểm hoạt ảnh phân cấp của nhóm. Cả 3 vị trí này đều gắn nhãn **`[Chưa xác minh]`** trên ảnh Wayback 2019; riêng **`PirateShip` (`src/rides/PirateShip.h`)** là trò thuyền lắc giữ nguyên từ PR #1, **KHÔNG phải** mô hình đại diện cho `The Flying Kirins` (`docs/img/ref/1795453378911_image.png`).

---

### 10.3. Hình ảnh kết quả sau Đợt 3

- [`img/wave3_vs_overlay.png`](img/wave3_vs_overlay.png) — Ảnh chụp trực giao từ trên xuống của scene 3D sau Đợt 3 chồng trực tiếp lên ảnh vệ tinh Wayback 2019 có lưới mét `50 m / 100 m` và nhãn tọa độ từng công trình.
- [`img/wave3_overview_high.png`](img/wave3_overview_high.png) — Ảnh chụp phối cảnh toàn cảnh công viên từ góc cao phía Nam-Tây-Nam.
- [`img/wave3_sunwheel_lake.png`](img/wave3_sunwheel_lake.png) — Ảnh chụp cận cảnh Sun Wheel, hồ nước phía Bắc, Thủy đình kèm cầu gỗ, 10 trụ đèn lồng và ảnh phản xạ qua mặt hồ `y = 0`.
- [`img/wave3_queencobra_closeup.png`](img/wave3_queencobra_closeup.png) — Ảnh chụp cận cảnh cụm tàu lượn Queen Cobra tại bán đảo Tây-Bắc `(−255, 0, −358)`, xoay `−25°`, nằm trọn trên đất khô sát bờ Tây hồ nước.

---

## 12. Mở rộng Đợt 4 → Đợt 6 (Cổng Ngọ Môn liền khối, Quảng trường, Sân khấu Sun Wheel & Khu trò chơi phía Tây)

### 12.1. Bảng đối chiếu độ lệch giữa GPS gốc (RCDB / Wikimedia Commons) và tâm khung bao đã chọn (5 hạng mục đã xác minh)

Theo yêu cầu điều chỉnh số 1 & 2, cả 5 hạng mục dưới đây được gắn nhãn **`[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]`** trong [`src/core/ParkLayout.h`](../src/core/ParkLayout.h) và tài liệu này: tên trò chơi và vị trí khu đất được xác minh bằng GPS thực địa từ RCDB / EXIF Wikimedia Commons, trong khi hình học đường ray & khối nhà trong mô phỏng được đơn giản hóa.

> **Giải thích nguyên nhân độ lệch `~22.2 m` của Garuda Valley (và 4 hạng mục còn lại):**
> Ghim GPS trên RCDB thường đặt tại **một điểm biên của vòng ray hoặc cổng vào** thay vì tâm hình học của cả khu đất trò chơi (`nhà ga + vùng đường ray`). Khi đặt mô hình 3D vào scene, ta lấy **tâm khung bao (bounding-box center)** của toàn bộ công trình trên ảnh vệ tinh Zoom 19 để khối mô hình không bị tràn sang lối đi hay đè lên công trình kế cận:
> - Với **Garuda Valley** (RCDB `#12676`): ghim RCDB `(16.038689, 108.227418)` quy đổi ra `Engine (−102.9, +162.4)`, nằm ở **mút rìa phía Đông của vòng ray** (khớp ảnh thực địa Wikimedia Commons `49356303773.jpg` chụp tại `16.038722, 108.227402`). Trên ảnh vệ tinh Zoom 19 (`z19_south_plaza.png`), toàn bộ khu Garuda Valley (gồm nhà ga mái dài ở phía Tây `lon ≈ 108.22708`, cổng vào tại Commons `49374160882.jpg` `16.038783, 108.227194`, và vòng ray vươn sang Đông tới `108.22742`) trải rộng trên `X_eng ∈ [−142, −103]`, `Z_eng ∈ [+143, +169]`. Do đó **tâm khung bao `34 × 26 m`** của cả cụm trò chơi nằm tại **`(−124.1, +155.7)`**, lệch `(ΔX = −21.2 m, ΔZ = −6.7 m, |d| = 22.2 m)` về phía Tây so với ghim rìa Đông của RCDB.

| # | Hạng mục & Nhãn độ tin cậy | Nguồn GPS gốc | Tọa độ GPS gốc `(lat, lon)` | Quy đổi trực tiếp `Engine_GPS (X, Z)` (m) | Tâm khung bao đã chọn `Engine_Chosen (X, Z)` (m) | Độ lệch `(ΔX, ΔZ)` (m) | Khoảng cách lệch `|d|` (m) | Nguyên nhân lệch giữa ghim GPS và tâm khung bao |
|:-:|:---|:---|:---|:---:|:---:|:---:|:---:|:---|
| 1 | **Tàu lượn Garuda Valley** *(34 × 26 m)*<br>`[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` | RCDB `#12676` + Commons `49374160882`, `49356303773` | `16.038689, 108.227418` | `(−102.9, +162.4)` | **`(−124.1, +155.7)`** | `(−21.2, −6.7)` | **`22.2 m`** | Ghim RCDB nằm ở **mút vòng ray phía Đông**; tâm khung bao `34 × 26 m` bao gồm cả nhà ga phía Tây (`lon 108.22708`) và vòng ray nên dịch `21.2 m` sang Tây. |
| 2 | **Tàu lượn Port of Sky Treasure** *(36 × 52 m)*<br>`[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` | RCDB `#13603` + Commons `49358003766` | `16.039907, 108.226363` | `(−215.8, +27.6)` | **`(−224.0, +22.0)`** | `(−8.2, −5.6)` | **`9.9 m`** | Ghim RCDB đặt tại **mép Đông-Nam nhà ga** gần trục lối đi; tâm hình học của toàn vùng ray + nhà ga trên ảnh vệ tinh nằm lệch `9.9 m` về Tây-Bắc. |
| 3 | **Tòa nhà đá Paradise Fall** *(24 × 14 m)*<br>`[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` | RCDB `#15548` + Commons `49358210737` | `16.040998, 108.226822` | `(−166.7, −93.1)` | **`(−179.8, −91.1)`** | `(−13.1, +2.0)` | **`13.2 m`** | Ghim RCDB và ảnh Commons `49358210737` đặt tại **cổng vào phía Đông** nhìn ra lối đi chính; tâm khối nhà giả núi đá nằm lùi `13.1 m` vào phía Tây. |
| 4 | **Đường ray số 8 Paradise Fall** *(34 × 24 m)*<br>**`[Ước lượng từ ảnh vệ tinh; hình dạng đơn giản hóa]`** | RCDB `#15548` (cùng ghim khu Paradise Fall) | `16.040998, 108.226822` | `(−166.7, −93.1)` | **`(−199.0, −119.9)`** | `(−32.3, −26.8)` | **`41.9 m`** | RCDB chỉ dùng 1 ghim chung tại cổng Đông nhà ga, còn **vòng ray lượn số 8 ngoài trời** nằm nối tiếp về phía Tây-Bắc tòa nhà đá (lệch `41.9 m` nên gắn nhãn **`[Ước lượng từ ảnh vệ tinh]`**). |
| 5 | **Tàu lượn Highway Boat** *(34 × 26 m)*<br>`[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` | RCDB `#15549` + Commons `49374702231` | `16.042034, 108.226667` | `(−183.3, −207.8)` | **`(−176.0, −214.0)`** | `(+7.3, −6.2)` | **`9.6 m`** | Ghim RCDB nằm gần mép Tây-Nam sát Ga Monorail Nepal; tâm vùng ray dịch `9.6 m` sang Đông-Bắc để không chạm hành lang Ga Nepal `(−217.2, −220.6)`. |

---

### 12.2. Kiểm tra va chạm 3D: Ga Monorail Nam & Dầm Monorail Nam với Cổng Ngọ Môn + 2 Cánh Tường Thành (Điều chỉnh số 3)

Đã kiểm tra tọa độ 3D chi tiết giữa tuyến Monorail phía Nam, Ga Monorail Nam và tổ hợp Cổng Ngọ Môn + 2 cánh tường thành mới:

1. **Khung bao 3D của Ga Monorail Nam & Dầm Monorail Nam:**
   - **Ga Monorail Nam (`MONORAIL_SOUTH_STATION_POS = (−24.6, 0, +128.0)`, xoay `−90°` quanh trục Y):**
     Trong hệ tọa độ cục bộ của `drawMonorailStation` (`CurveManager.h`), mặt sàn và mái che nhà ga trải trên `local X ∈ [−2.6, +7.2 m]`, `local Z ∈ [−12.0, +12.0 m]`, cao `Y ∈ [0.0, 18.0 m]`. Khi xoay `−90°` quanh trục Y, trục `+X` cục bộ chuyển thành trục `+Z` thế giới, nên Ga Nam chiếm vùng không gian:
     $$X_{\text{station}} \in [-36.6,\ -12.6]\text{ m},\quad Z_{\text{station}} \in [+125.4,\ +135.2]\text{ m},\quad Y_{\text{station}} \in [0.0,\ 18.0]\text{ m}.$$
   - **Dầm Monorail & Đoàn tàu đoạn phía Nam (`WP19 → WP0 → WP1 → WP2`):**
     Giữa `WP0 (+16.1, 12.0, +128.0)` và `WP1 (−24.6, 12.0, +128.0)`, đường cong Bézier $C^1$ hơi võng nhẹ về phía Nam tới `Z_beam_max = +129.67 m` (mép ngoài toa tàu rộng `2.0 m` đạt **`Z_train_max = +130.7 m`**, ở độ cao `Y ∈ [10.9, 14.1 m]`).
     Ba chân trụ bê tông đỡ dầm gần cổng nhất nằm tại:
     - Trụ Đông sân cổng: `(X = +34.4 m, Z = +127.7 m)`
     - Trụ giữa sân trong Bắc cổng: `(X = +8.2 m, Z = +129.6 m)`
     - Trụ Tây cạnh Ga Nam: `(X = −19.2 m, Z = +128.8 m)`
2. **Khung bao 3D của Cổng Ngọ Môn & 2 Cánh Tường Thành (`NGO_MON_GATE_POS = (+16.1, 0, +139.1)`, `scale = 0.76`):**
   - **Thân cổng chính & Lầu Ngũ Phụng** (`46 × 15 m × 0.76 = 34.96 × 11.40 m`, cao tổng `17.5 m`):
     $$X_{\text{gate}} \in [-1.38,\ +33.58]\text{ m},\quad Z_{\text{gate}} \in [+133.40,\ +144.80]\text{ m},\quad Y_{\text{gate}} \in [0.0,\ 17.5]\text{ m}.$$
     - Khoảng cách ngang từ mặt tường Bắc thân cổng (`Z = +133.40 m`) tới mép Nam đoàn tàu Monorail (`Z = +130.7 m`) là **`2.70 m`** (và ở tầng mái Lầu Ngũ Phụng `Y = 11–14 m`, diềm mái Bắc tại `Z = +133.8 m` cách toa tàu **`3.1 m`**, không xuyên cắt hình học nhưng chạy khá sát phía sau cổng).
   - **Cánh tường thành phía Tây (`West Wing Wall`, cao `~6.8 m`, trải từ `X = −1.38 m` sang `X = −49.2 m`):**
     - Trong bản dựng Đợt 4 hiện tại (chưa tự ý dời cổng hay dời waypoint theo đúng chỉ dẫn), cánh tường Tây được bố trí chạy thẳng theo phương Đông–Tây tại `Z_wall ∈ [+138.4, +141.6 m]`, để chừa khe hở **`3.2 m`** phía Nam Ga Monorail Nam (`Z_station_max = +135.2 m`).
     - Tuy nhiên, nếu đối chiếu với ảnh chụp flycam thực tế (`media_1795356545507_984436ca.jpg`), hai cánh tường thành thực tế hơi ôm cong về phía Bắc (`Z ≈ 133–148 m`) và tuyến đường ray Monorail phía sau lưng cổng cách mặt Bắc cổng tới `~12–15 m` (để nhường chỗ cho hàng cây và đường dạo nội bộ sau tường thành). Với tọa độ `WP0/WP1` hiện tại (`Z = +128.0 m`), nhà ga Nam (`Z_max = +135.2 m`) **chồng lấn `~2.2 m` vào vùng hành lang Bắc của cánh tường Tây (`Z ≈ 133–148 m`)** và dầm Monorail chạy rất sát (`2.7–3.1 m`) mặt Bắc Lầu Ngũ Phụng.
3. **Kết quả áp dụng PHƯƠNG ÁN A đã duyệt:**
   - Đã dịch `MONORAIL_WAYPOINTS[0]` → `(+16.1, 12.0, +121.8)`, `MONORAIL_WAYPOINTS[1]` → `(−24.6, 12.0, +121.8)`, `MONORAIL_WAYPOINTS[2]` → `(−78.1, 12.2, +114.0)` và `MONORAIL_SOUTH_STATION_POS` → `(−24.6, 0.0, +121.8)`, đồng thời giữ nguyên 100% Cổng Ngọ Môn `(+16.1, 0.0, +139.1)` và 2 hồ sen.
   - **Kiểm tra hình học sau khi áp dụng Phương án A:**
     - **0 điểm dầm và 0 chân trụ Monorail** nằm trong vùng Quảng trường lát gạch (`X ∈ [−26.9, +59.1]`, `Z ≤ +115.0 m`).
     - Khoảng cách tối thiểu từ dầm Monorail tới mặt Bắc thân Cổng Ngọ Môn (`Z = +133.40 m`) là **`10.26 m`**.
     - Ga Monorail Nam chiếm `X ∈ [−36.6, −12.6 m]`, **`Z ∈ [+119.2, +129.0 m]`**: cách mép Nam Quảng trường lát gạch (`Z = +115.0 m`) **`4.20 m`** và cách cánh tường thành phía Tây (`Z = +138.4 m`) **`9.40 m`**.
     - Đường cong Bézier $C^1$ của Monorail giữ độ trơn mượt tuyệt đối (tổng chiều dài vòng `1 884.5 m`, tốc độ `9.2 m/s` ≈ `3.41 phút/vòng`).
     - Đã chỉnh điểm `PATH_WEST_SPINE[1]` về `(−24.6, +112.5)` để lối đi chạy ngay trước sảnh Bắc của Ga Monorail Nam mới với khoảng đệm `4.20 m`.

---

### 12.3. Kiểm tra không va chạm giữa Sân khấu Sun Wheel và chân tháp chữ A của Sun Wheel (Điều chỉnh số 4)

- Trong `SunWheel.h`, 4 bệ móng bê tông chịu lực của chân tháp chữ A đặt tại `X = ±27.0 m` (rộng `5.4 m` → `|X| ∈ [24.30, 29.70 m]`), tâm Sun Wheel tại `Z = −38.0 m` với nửa khoảng mở chân `legSpreadZ = 12.5 m` (sâu `5.4 m`):
  - **2 bệ móng chân phía Nam (`sz = +1`):** chiếm `Z ∈ [−28.20, −22.80 m]`, `|X| ∈ [24.30, 29.70 m]`, `Y ∈ [0.0, 3.55 m]`.
  - **2 ống thép chân chữ A phía Nam:** nghiêng từ đỉnh bệ `(±27.0, 3.2, −25.50)` thu dần vào trục ổ quay `(±6.2, 65.0, −31.8)`.
- Trong `ParkLayout.h` & `ParkArchitecture.h`, cụm **Sân khấu Sun Wheel (`SUN_WHEEL_STAGE_POS = (0.0, 0.0, −15.9)`)** đã được lùi thêm `1.8 m` về phía Nam với chiều rộng `52.0 m` (`X ∈ [−26.0, +26.0 m]`) và chiều sâu `9.0 m` (**`Z ∈ [−20.40, −11.40 m]`**):
  - Mặt lưng phía Bắc của sân khấu và 2 tháp màn hình tròn dừng tại **`Z = −20.40 m`**, nằm hoàn toàn ở phía Nam của mép Nam bệ bê tông chân Sun Wheel (`Z = −22.80 m`) với **khe hở thoáng `2.40 m` so với mép đế bê tông và `5.10 m` so với ống thép chân chữ A** (`Z = −25.50 m`).
  - Hai nhà chòi phụ hai bên sân khấu (`SUN_WHEEL_SIDE_PAVILION_WEST_POS / EAST_POS`) đặt tại `(±40.0, 0.0, −22.2)` với bề rộng `12.0 m` (`|X| ∈ [34.0, 46.0 m]`), cách mép ngoài bệ móng chân Sun Wheel (`|X| = 29.70 m`) **`4.30 m`**.
  - **Kết luận:** **Không có bất kỳ điểm giao cắt hình học nào** giữa tổ hợp sân khấu và chân chữ A của Sun Wheel.

---

### 12.4. Bảng hạng mục kiến trúc đã hoàn thành trong ĐỢT 4 (`NgoMonGate.h` & `ParkArchitecture.h`)

| STT | Hạng mục Đợt 4 | Hằng số trong `ParkLayout.h` | Tọa độ Engine `(X, Y, Z)` (m) | Kích thước `(Rộng × Sâu × Cao)` | Đặc điểm hình học liền khối (`LIỀN KHỐI`) |
|:---:|:---|:---|:---|:---:|:---|
| 1 | **Cổng Ngọ Môn (vòm 3 cửa, Lầu Ngũ Phụng 5 tháp mái cong, cầu thang chữ A, 4 ô cửa tròn phát sáng)** | `NGO_MON_GATE_POS` (`scale = 0.76`) | `(+16.1, 0.0, +139.1)` | `35.0 × 11.4 × 17.5 m` (sau scale) | Mái cong 4 mặt đối xứng (`createCurvedAsianRoof`) có cổ diềm đáy ăn khớp trực tiếp lên đầu cột và tường đỡ; 0 chi tiết lơ lửng. |
| 2 | **Hai cánh tường thành răng cưa + cổng vòm phụ Đông/Tây** | `NGO_MON_WING_WALL_SPAN`, `NGO_MON_WING_WALL_HEIGHT` | Trải sang hai bên cổng trên `X ∈ [−49.2, +81.4]`, `Z ≈ +140.0` | Dài `~47.8 m`/cánh × dày `3.2 m` × cao `~5.2 m` | Nối liền khối vào hai đầu hồi Đông – Tây của đài cổng chính; có trụ bổ tường trắng, bờ thành răng cưa và 2 cổng vòm phụ. |
| 3 | **Hai hồ sen đối xứng trước cổng + tiền sảnh** | `LOTUS_POND_WEST_POS`, `LOTUS_POND_EAST_POS` | `(−14.0, 0.0, +154.0)` & `(+46.2, 0.0, +154.0)` | `26.0 × 14.0 m` mỗi hồ | Thành đá bao quanh mặt nước xanh đậm, lá sen và búp sen hồng nổi trên mặt hồ đối xứng qua đường dẫn trung tâm. |
| 4 | **Quảng trường trung tâm lát gạch** | `CENTRAL_PLAZA_CENTER` | `(+16.1, 0.028, +56.0)` | `86.0 × 118.0 m` | Mặt sân lát đá kem sáng có mạch gạch caro `6 m` và bó vỉa đá bao quanh, nối từ sân trong Ngọ Môn tới thềm Sân khấu Sun Wheel. |
| 5 | **Tháp Đồng Hồ Trung Tâm (2019)** | `CLOCK_TOWER_POS` | `(+10.7, 0.0, +61.6)` | `8.4 × 8.4 × 25.5 m` | Giữ nguyên vị trí đã tách riêng ở Đợt 3 giữa quảng trường; mái ngói cong liền khối trên đỉnh tháp 4 mặt đồng hồ. |
| 6 | **Hai nhà chòi mái ngói hai bên Quảng trường** | `PLAZA_PAVILION_WEST_POS`, `PLAZA_PAVILION_EAST_POS` | `(−24.0, 0.0, +65.0)` & `(+48.0, 0.0, +65.0)` | `16.0 × 9.0 × 8.2 m` | Bệ đá tam cấp, 8 cột gỗ lim đỏ, dầm cổ diềm và mái ngói cong 2 tầng liền khối đối xứng hai bên quảng trường. |
| 7 | **Tổ hợp Sân khấu chân Sun Wheel + 2 màn hình tròn + 2 chòi bên** | `SUN_WHEEL_STAGE_POS`, `STAGE_PAVILION_WEST/EAST_POS` | `(0.0, 0.0, −17.7)` và `(±40.0, 0.0, −24.0)` | Sân khấu `52.0 × 9.0 × 12.5 m` (tổng ngang `92 m`) | Khán đài trung tâm, cổng vòm vàng chạm nổi, màn hình LED giữa và 2 tháp mái ngói 2 tầng gắn **2 màn hình tròn phát sáng** (`R = 3.1 m`). |
| 8 | **Hai nhà chòi ven bờ Nam hồ nước** | `SOUTH_LAKE_PAVILION_WEST_POS`, `SOUTH_LAKE_PAVILION_EAST_POS` | `(−49.2, 0.0, −88.0)` & `(+40.7, 0.0, −91.1)` | `10.0 × 8.0 × 7.5 m` | Đặt trên bệ đá sát mặt nước phía Bắc Sun Wheel kèm cầu gỗ nối vào đường dạo bờ kè Nam (`Z = −66.8 m`). |

---

### 12.5. Hiệu năng (FPS trước và sau Đợt 4) & Bộ ảnh kiểm tra Đợt 4

- **Tối ưu hóa bằng OpenGL Display Lists:** Toàn bộ hình học tĩnh của Cổng Ngọ Môn + 2 cánh tường thành + 2 hồ sen (`NgoMonGate.h`) và toàn bộ Quảng trường + Sân khấu Sun Wheel + 6 nhà chòi mái ngói (`ParkArchitecture.h`) được biên dịch sẵn vào **OpenGL Display Lists** (`glGenLists` / `glNewList(..., GL_COMPILE)`) ngay ở khung hình đầu tiên.
- **Kết quả đo FPS (`1280 × 768`, 200 khung hình liên tiếp tại thời điểm Hoàng hôn/Đêm bật đầy đủ nguồn sáng):**
  - **Trước Đợt 4 (cuối Đợt 3):** `~252.6 FPS` (`200 frames / 0.792 s`).
  - **Sau Đợt 4 (thêm toàn bộ Cổng Ngọ Môn mới, 2 cánh tường thành, 2 hồ sen, Quảng trường lát gạch, Sân khấu 2 màn hình tròn và 6 nhà chòi mái ngói):** **`248.3 FPS`** (`200 frames / 0.805 s` — chỉ giảm `~1.7%`, giữ tốc độ dựng hình cực mượt).
- **Bộ ảnh kết quả Đợt 4:**
  - [`img/wave4_overview_high.png`](img/wave4_overview_high.png) — Ảnh 1: Toàn cảnh góc cao từ phía Nam-Đông-Nam bao quát Cổng Ngọ Môn, 2 cánh tường thành, 2 hồ sen, Quảng trường lát gạch, Tháp Đồng Hồ, Sân khấu 2 màn hình tròn và Sun Wheel.
  - [`img/wave4_ngomon_3angles.png`](img/wave4_ngomon_3angles.png) *(và 3 ảnh rời [`wave4_ngomon_front.png`](img/wave4_ngomon_front.png), [`wave4_ngomon_angle.png`](img/wave4_ngomon_angle.png), [`wave4_ngomon_back.png`](img/wave4_ngomon_back.png))* — Ảnh 2: Cận cảnh Cổng Ngọ Môn từ 3 góc (Chính diện mặt Nam, Góc xiên Tây-Nam 45°, Mặt Bắc nhìn từ sân trong sau khi áp dụng Phương án A cho Monorail Nam).
  - [`img/wave4_plaza_to_sunwheel.png`](img/wave4_plaza_to_sunwheel.png) — Ảnh 3: Quảng trường trung tâm nhìn về phía Bắc hướng tới Tháp Đồng Hồ, 2 nhà chòi quảng trường, Sân khấu Sun Wheel (`Z = −15.9 m`) với 2 màn hình tròn phát sáng và Vòng quay Mặt Trời.
  - [`img/wave4_topdown_overlay.png`](img/wave4_topdown_overlay.png) — Ảnh chụp trực giao từ trên xuống sau Đợt 4 chồng lên ảnh vệ tinh Wayback 2019.

---

### 12.6. Bảng hạng mục đã hoàn thành trong ĐỢT 5A (4 Tàu lượn nhỏ dùng chung lớp `MiniCoaster` & Dời 3 trò chơi PR #1 về khu Tây)

- **Thiết kế lớp dùng chung [`src/rides/MiniCoaster.h`](../src/rides/MiniCoaster.h):**
  - Không sửa đổi `RollerCoaster.h` (Queen Cobra).
  - Cả 4 trò tàu lượn nhỏ (`Garuda Valley`, `Port of Sky Treasure`, `Paradise Fall`, `Highway Boat`) được khởi tạo từ **một lớp `AsiaParkRides::MiniCoaster` duy nhất** qua hàm `initWave5aMiniCoasters(gMiniCoasters)`, tái sử dụng `BezierTrack` ($C^1$ cubic Bézier spline) để dựng đường ray đôi, ống xương sống, tà-vẹt, các trụ đỡ nối đất `y = 0`, nhà ga đón khách (hoặc Tòa nhà giả núi đá Paradise Fall) và 1 đoàn tàu 3 toa chạy theo độ dài cung.
  - Toàn bộ hình học tĩnh của mỗi `MiniCoaster` được biên dịch sẵn vào **OpenGL Display List** riêng.
- **Dời 3 trò chơi từ PR #1 của nhóm về vị trí khu Tây (giữ nguyên 100% hình học và animation):**
  - `Playground` (`src/rides/Playground.h`): đặt tại `PLAYGROUND_TARGET_POS (−104.9, 0.0, −24.7)`.
  - `BumperCars` (`src/rides/BumperCars.h`): đặt tại `BUMPER_CARS_TARGET_POS (−206.5, 0.0, +108.1)`.
  - `PirateShip` (`src/rides/PirateShip.h`): đặt tại `PIRATE_SHIP_TARGET_POS (−172.0, 0.0, −2.0)`.
- **Điều chỉnh lối đi `PATH_WEST_SPINE` và `PATH_CENTRAL_CONNECTOR`:**
  - Chỉnh nhánh `PATH_WEST_SPINE` đi qua `(−24.6, +112.5)` (trước Ga Monorail Nam mới) → `(−159.4, +86.5)` (giữa Ngựa gỗ và dầm Monorail) → `(−188.0, +42.0)` → **`(−195.0, −20.0)`** (hành lang giữa `Port of Sky Treasure` và `Pirate Ship`) → `(−244.0, −54.6)` (sân Đông `Sky Tower`).
  - Đã kiểm tra bằng script hình học: **0 va chạm** giữa 4 đường ray + 37 trụ đỡ `MiniCoaster`, Tòa nhà đá Paradise Fall, 3 trò dời vị trí (`Playground`, `BumperCars`, `PirateShip`), 6 tuyến lối đi, đa giác hồ nước, dầm & trụ Monorail và các công trình hiện có.

| STT | Hạng mục Đợt 5A | Hằng số trong `ParkLayout.h` | Tọa độ Engine `(X, Y, Z)` (m) | Kích thước / Chiều dài ray | Nhãn độ tin cậy |
|:---:|:---|:---|:---:|:---:|:---|
| 1 | **Tàu lượn Garuda Valley** (`MiniCoaster[0]`) | `GARUDA_VALLEY_CENTER` | `(−124.1, 0.0, +155.7)` | Khung `34 × 26 m`, dài ray `88.6 m` (8 trụ), 1 đoàn tàu 3 toa | `[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` |
| 2 | **Tàu lượn Port of Sky Treasure** (`MiniCoaster[1]`) | `PORT_SKY_TREASURE_CENTER` | `(−224.0, 0.0, +22.0)` | Khung `36 × 52 m`, dài ray `126.6 m` (12 trụ), 1 đoàn tàu 3 toa | `[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` |
| 3 | **Tòa nhà đá Paradise Fall** (ga `MiniCoaster[2]`) | `PARADISE_FALL_BLDG_POS` | `(−179.8, 0.0, −91.1)` | Khối núi đá + vòm hang + mái cổ `24 × 14 × 11.7 m` | `[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` |
| 4 | **Đường ray số 8 Paradise Fall** (`MiniCoaster[2]`) | `PARADISE_FALL_TRACK_CENTER` | `(−199.0, 0.0, −119.9)` | Khung `34 × 24 m`, dài ray `95.0 m` (9 trụ), 1 đoàn tàu 3 toa | **`[Ước lượng từ ảnh vệ tinh; hình dạng đơn giản hóa]`** |
| 5 | **Tàu lượn Highway Boat** (`MiniCoaster[3]`) | `HIGHWAY_BOAT_CENTER` | `(−176.0, 0.0, −214.0)` | Khung `34 × 26 m`, dài ray `92.4 m` (8 trụ), 1 đoàn tàu 3 toa | `[Đã xác minh tên/vị trí; hình dạng đơn giản hóa]` |
| 6 | **Sân chơi trẻ em (`Playground` dời vị trí)** | `PLAYGROUND_TARGET_POS` | `(−104.9, 0.0, −24.7)` | `23.6 × 12.8 m` (xích đu, cầu trượt, bập bênh, hố cát) | `[Ước lượng từ ảnh vệ tinh — mô phỏng, chưa xác minh]` |
| 7 | **Xe điện đụng (`BumperCars` dời vị trí)** | `BUMPER_CARS_TARGET_POS` | `(−206.5, 0.0, +108.1)` | Sân tròn đường kính `20.4 m`, 6 xe chạy quỹ đạo | `[Ước lượng từ ảnh vệ tinh — mô phỏng, chưa xác minh]` |
| 8 | **Thuyền cướp biển (`PirateShip` dời vị trí)** | `PIRATE_SHIP_TARGET_POS` | `(−172.0, 0.0, −2.0)` | `18.0 × 13.0 × 11.0 m` (con lắc thuyền Viking) | `[Ước lượng từ ảnh vệ tinh — mô phỏng, chưa xác minh]` |

- **Hiệu năng (FPS trước và sau Đợt 5A tại độ phân giải `1280 × 768`, 200 khung hình liên tiếp):**
  - **Trước Đợt 5A (cuối Đợt 4):** `248.3 FPS` (`200 frames / 0.805 s`).
  - **Sau Đợt 5A (thêm 4 tàu lượn `MiniCoaster` với tổng `402.6 m` đường ray ống đôi 3D, 37 trụ đỡ, 4 nhà ga/núi đá và 4 đoàn tàu 12 toa chuyển động liên tục):** **`206.8 FPS`** (`200 frames / 0.967 s`).
- **Bộ ảnh kết quả Đợt 5A:**
  - [`img/wave4_ngomon_back.png`](img/wave4_ngomon_back.png) — Ảnh kiểm tra mặt Bắc Cổng Ngọ Môn sau khi áp dụng Phương án A (dầm & trụ Monorail và Ga Nam nằm hoàn toàn ngoài Quảng trường lát gạch `Z ≤ 115 m`, cách lưng cổng `> 10.2 m`, không che Cổng Ngọ Môn).
  - [`img/wave5a_west_overview.png`](img/wave5a_west_overview.png) — Ảnh 1 Đợt 5A: Toàn cảnh khu Tây từ trên cao bao quát cả 4 tàu lượn `MiniCoaster`, 3 trò chơi dời vị trí (`BumperCars`, `PirateShip`, `Playground`), `Carousel`, `Sky Tower`, `Queen Cobra` và trục lối đi `PATH_WEST_SPINE`.
  - [`img/wave5a_coasters_closeup.png`](img/wave5a_coasters_closeup.png) *(và 2 ảnh rời [`wave5a_coaster_port_sky.png`](img/wave5a_coaster_port_sky.png), [`wave5a_coaster_paradise_fall.png`](img/wave5a_coaster_paradise_fall.png))* — Ảnh 2 Đợt 5A: Cận cảnh các tàu lượn nhỏ (`Port of Sky Treasure` + `Pirate Ship` + lối đi `(−195, −20)`; `Paradise Fall` núi đá nhiều tầng + vòng ray số 8 + `Highway Boat`).
  - [`img/wave5a_topdown_overlay.png`](img/wave5a_topdown_overlay.png) — Ảnh 3 Đợt 5A: Nhìn từ trên xuống chồng trực tiếp lên ảnh vệ tinh Wayback 2019 có gắn nhãn tọa độ.

---

### 12.7. Bảng hạng mục đã hoàn thành trong ĐỢT 5B (Nhà mái ngói tham số hóa, Đu quay tròn tham số hóa, Dãy quầy mái sọc, Quảng trường Ngôi sao Angkor & Nâng cấp Núi đá Paradise Fall)

1. **Nâng cấp Tòa nhà Paradise Fall (`PARADISE_FALL_BLDG_POS = (−179.8, 0.0, −91.1)`) thành Núi đá nhiều tầng gồ ghề (`drawStoneMountainBuilding` trong `src/rides/MiniCoaster.h`):**
   - Thay thế 2 khối hộp phẳng bằng **tổ hợp 10 khối vách đá chồng 3 tầng** với kích thước, góc xoay (`−21°..+24°`) và độ nghiêng (`±3°..±7°`) khác nhau, kết hợp **3 chỏm núi nhọn** trên đỉnh (`y` lên tới `~13.4 m`), màu đá nâu-xám nhiều sắc độ, kèm **2 hang đá & cổng vòm** (`createArchVault`) ở mặt Đông và mặt Tây-Bắc.
2. **Đánh giá kích thước & độ mảnh của `Playground`, `PirateShip`, `BumperCars` (PR #1 của thành viên nhóm — giữ nguyên 100% hình học & không tự sửa):**
   - **`Playground` (`src/rides/Playground.h`, tâm `(−104.9, 0, −24.7)`):** Khung bao tổng `23.6 × 12.8 m`, cao `5.0 m` là hợp lý về mặt diện tích đất (`1 unit = 1 m`), nhưng **các thanh khung rất mảnh** (chân xích đu `0.12 × 0.12 m`, dây treo `0.04 m`, trụ bập bênh `0.15 m`) và **không có tấm nền sân chơi lót dưới mặt đất**, nên khi nhìn từ camera toàn cảnh độ cao `170 m` chỉ thấy rõ ô cát vàng `5 × 5 m`.
   - **`PirateShip` (`src/rides/PirateShip.h`, tâm `(−172.0, 0, −2.0)`):** Thân thuyền dài `16 m`, khung chữ A rộng `18 m × cao 11 m` đúng tỷ lệ thật `1:1`, nhưng **4 chân trụ chữ A chỉ dày `0.30 × 0.30 m`**, cần treo mảnh (`R = 0.15 m`), thân thuyền màu nâu sẫm đặt trực tiếp trên nền cỏ xanh đậm (không có bệ sàn nhà ga) nên nhìn từ xa khá mảnh và độ tương phản thấp.
   - **`BumperCars` (`src/rides/BumperCars.h`, tâm `(−206.5, 0, +108.1)`):** Sân tròn đường kính `20.4 m` có 6 xe chạy rõ ràng khi nhìn từ trên xuống, nhưng chỉ cao `0.90 m` (**không có mái vòm nhà che** như sân xe điện đụng thực tế cao `~5–6 m`) nên nhìn nghiêng trông mỏng như một đĩa phẳng.
3. **Các hàm tham số hóa dùng chung trong [`src/rides/ParkArchitecture.h`](../src/rides/ParkArchitecture.h) (Đợt 5B):**
   - **MỘT hàm vẽ nhà mái ngói tham số hóa `appendTileRoofedHall(target, pos, w, d, wallH, rotYDeg, roofCol, roofStyle)`:** dựng bệ đá bậc tam cấp, thân tường đặc, hàng cột hiên, cổ diềm, mái ngói cong 4 mặt hai tầng liền khối (`createCurvedAsianRoof`) và sảnh hiên đón khách. Dùng chung cho 4 nhà mái ngói xanh/teal (`#7`, `#8`, `#15`, `#18`) và nhà mái ngói đỏ lớn ven hồ (`#12`).
   - **MỘT hàm vẽ đu quay tròn có animation quay tham số hóa `drawSpinningRoundRide(listIdx, pos, radius, height, primaryCol, secondaryCol, numSeats, spinDeg, hasOuterColonnade)`:** dựng bệ tròn, trụ tâm, tán mái nón 16 múi sọc màu quay quanh trục Y và dàn ghế treo nghiêng ly tâm (`12..16` ghế). Dùng chung cho `#9`, `#10`, `#14` và biến thể có vòng cột hành lang bao quanh `#13`.
   - **Dãy gian hàng mái sọc (`#6` — `appendStripedStallsComplex`):** 3 dãy quầy nhỏ song song Bắc–Nam (mỗi dãy 6 gian) có mái sọc đỏ – trắng xen kẽ trên nền sân `16 × 32 m`.
   - **Quảng trường Ngôi sao & 4 tháp chòi Angkor (`#19` — `appendAngkorStarPlazaTowers` + `drawTiledCentralPlaza`):** nền đá vuông `24 × 24 m` lát **ngôi sao 8 cánh** vàng-đỏ ở tâm, đài tháp trung tâm và **4 tháp chòi nhọn kiểu Prang Angkor** ở 4 góc (kèm sân lát hình **bông hoa 8 cánh** tại `FLOWER_PLAZA_CENTER (−108.0, 0, +76.0)` giữa 2 nhà mái xanh `#7` và `#8`).
   - **Toàn bộ 11 hạng mục Đợt 5B đều gắn nhãn `[Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]`** và đã kiểm tra bằng script hình học (**0 va chạm** với 6 tuyến lối đi, mặt hồ, dầm/trụ Monorail và các công trình Đợt 1–5A).

| Mã | Hạng mục Đợt 5B (`[Ước lượng từ ảnh vệ tinh; mô phỏng, chưa xác minh]`) | Hằng số trong `ParkLayout.h` | Tọa độ Engine `(X, Y, Z)` (m) | Kích thước / Thông số | Hàm dựng trong `ParkArchitecture.h` |
|:--:|:---|:---|:---:|:---:|:---|
| `#6` | **Dãy 3 hàng gian hàng mái sọc đỏ-trắng** | `STRIPED_STALLS_POS` | `(−50.0, 0.0, +12.0)` | `16 × 32 × 3.3 m` (3 dãy × 6 quầy) | `appendStripedStallsComplex` |
| `#7` | **Nhà mái ngói xanh Bắc sân bông hoa** | `BLUE_ROOF_HALL_NORTH_POS` | `(−113.4, 0.0, +55.0)` | `26 × 12 × 8.8 m`, xoay `+12°`, mái xanh teal 2 tầng | `appendTileRoofedHall(..., style=0)` |
| `#8` | **Nhà mái ngói xanh Nam sân bông hoa** | `BLUE_ROOF_HALL_SOUTH_POS` | `(−102.7, 0.0, +118.0)` | `22 × 11 × 8.6 m`, xoay `−15°`, mái xanh teal 2 tầng | `appendTileRoofedHall(..., style=0)` |
| — | **Sân lát hình Bông hoa 8 cánh** | `FLOWER_PLAZA_CENTER` | `(−108.0, 0.044, +76.0)` | Đường kính `21.6 m`, hoa văn 8 cánh vàng | `drawTiledCentralPlaza` |
| `#9` | **Đu quay tròn 1 (mái sọc cam-trắng)** | `ROUND_SWING_RIDE_1_POS` | `(−132.7, 0.0, +12.9)` | Đường kính `17.0 m` (`R = 8.5 m`), cao `8.2 m`, `16` ghế quay | `drawSpinningRoundRide(..., col=false)` |
| `#10` | **Đu quay tròn 2 (mái sọc xanh-vàng)** | `ROUND_SPIN_RIDE_2_POS` | `(−94.2, 0.0, +17.3)` | Đường kính `13.0 m` (`R = 6.5 m`), cao `6.4 m`, `12` ghế quay | `drawSpinningRoundRide(..., col=false)` |
| `#12` | **Nhà ngói đỏ lớn ven hồ** | `RED_ROOF_LAKESIDE_HALL_POS` | `(−94.2, 0.0, −71.2)` | `22 × 18 × 9.0 m`, mái ngói đỏ 2 tầng | `appendTileRoofedHall(..., style=0)` |
| `#13` | **Công trình tròn có hành lang cột khu giữa** | `CENTRAL_ROTUNDA_RIDE_POS` | `(−141.3, 0.0, −137.6)` | Đường kính `18.0 m` (`R = 9.0 m`), cao `8.8 m`, hành lang 12 cột + `16` ghế quay | `drawSpinningRoundRide(..., col=true)` |
| `#14` | **Đu quay tròn 3 khu giữa (mái ngọc-vàng)** | `ROUND_SPIN_RIDE_3_POS` | `(−143.4, 0.0, −179.6)` | Đường kính `15.0 m` (`R = 7.5 m`), cao `7.4 m`, `14` ghế quay | `drawSpinningRoundRide(..., col=false)` |
| `#15` | **Nhà mái xanh-trắng khu Tây** | `WEST_CANOPY_THEATER_POS` | `(−256.0, 0.0, −128.7)` | `20 × 20 × 9.2 m`, tường trắng, mái cong vòm rộng xanh teal | `appendTileRoofedHall(..., style=1)` |
| `#18` | **Nhà mái ngói xanh khu Angkor ven hồ** | `ANGKOR_LAKESIDE_HALL_POS` | `(−138.0, 0.0, −242.0)` | `20 × 14 × 8.8 m`, xoay `−22°`, mái xanh teal 2 tầng | `appendTileRoofedHall(..., style=0)` |
| `#19` | **Quảng trường Ngôi sao & 4 tháp chòi Angkor** | `ANGKOR_STAR_PLAZA_POS` | `(−185.0, 0.0, −254.0)` | Sân `24 × 24 m` lát sao 8 cánh + 4 tháp nhọn góc & tháp tâm | `appendAngkorStarPlazaTowers` |

- **Hiệu năng (FPS trước và sau Đợt 5B ở `1280 × 768`, 200 khung hình liên tiếp):**
  - **Trước Đợt 5B (cuối Đợt 5A):** `206.8 FPS` (`200 frames / 0.967 s`).
  - **Sau Đợt 5B (thêm toàn bộ 11 cụm công trình tĩnh & 4 đu quay tròn có animation quay, dùng Display List cho cả khối kiến trúc tĩnh và cụm tán quay):** **`163.7 FPS`** (`200 frames / 1.222 s` — đảm bảo yêu cầu `> 150 FPS`).
- **Bộ ảnh kết quả Đợt 5B:**
  - [`img/wave5b_west_overview.png`](img/wave5b_west_overview.png) — Ảnh 1 Đợt 5B: Toàn cảnh khu Tây từ trên cao bao quát toàn bộ các công trình Đợt 5A + 5B.
  - [`img/wave5b_blue_roof_halls.png`](img/wave5b_blue_roof_halls.png) — Ảnh 2 Đợt 5B: Cận cảnh cụm nhà mái ngói xanh (`#7`, `#8`) quanh sân bông hoa 8 cánh và dãy 3 hàng quầy mái sọc đỏ-trắng (`#6`).
  - [`img/wave5b_round_rides.png`](img/wave5b_round_rides.png) — Ảnh 3 Đợt 5B: Cận cảnh khu đu quay tròn (`#9`, `#10`, `#13`, `#14`), nhà ngói đỏ lớn ven hồ (`#12`) và Tòa nhà núi đá gồ ghề nhiều tầng của `Paradise Fall`.
  - [`img/wave5b_topdown_overlay.png`](img/wave5b_topdown_overlay.png) — Ảnh chụp trực giao từ trên xuống sau Đợt 5B chồng trực tiếp lên ảnh vệ tinh Wayback 2019.
  - [`img/wave4_ngomon_back.png`](img/wave4_ngomon_back.png) — Ảnh chụp Cổng Ngọ Môn nhìn từ phía Bắc xác nhận dầm Monorail đã lùi về `Z = +121.8 m` (cách mặt Bắc cổng `11.6 m`) và không còn che khuất Cổng Ngọ Môn.

---

### 12.8. Bảng hạng mục đã hoàn thành trong ĐỢT 6 (6a: Bệ sàn đá, Mái che BumperCars, Nền đa sắc độ, Đèn đường, Ghế nghỉ, Bồn hoa, Cột cờ, Biển chỉ dẫn & Đèn lồng; 6b: 123 Cây xanh & Cập nhật lộ trình Người đi dạo)

#### 12.8.1. Hạng mục Đợt 6a (`ParkLayout.h` & `ParkArchitecture.h` — không sửa code bên trong `Playground.h`, `PirateShip.h`, `BumperCars.h`)
1. **Lót bệ sàn đá/gạch mỏng dưới `Playground`, `PirateShip`, `BumperCars` (không phóng to mô hình gốc):**
   - `Playground` (`(−105.7, 0, −20.5)`): bệ sàn đá-gạch 2 lớp `26.8 × 15.8 m` (`y = 0.024..0.032 m`) kèm gờ bó vỉa gạch thấp ở hai biên Bắc – Nam.
   - `PirateShip` (`(−172.0, 0, −2.0)`): bệ sàn đá đón khách `21.6 × 15.2 m` (`y = 0.024..0.032 m`) kèm 2 tường lan can đá thấp ở hai cạnh Đông – Tây (ngoài hành trình văng thuyền).
   - `BumperCars` (`(−206.5, 0, +108.1)`): bệ sàn đá `25.4 × 25.4 m` (`y = 0.006 m`, nằm dưới sàn sân xe `y = 0.010 m`) và **nhà vòm 16 cột + mái ngói cong xanh 2 tầng (`23 × 23 m`, thông 4 mặt tường)** dựng bằng chính hàm tham số hóa `appendTileRoofedHall(..., openColonnadeOnly = true)` để bao quanh sân xe điện đụng mà vẫn thấy trọn vẹn 6 xe chạy bên trong.
2. **Đa dạng hóa mặt nền (không dùng texture nặng):**
   - **8 mảng thảm cỏ sắc độ đậm/nhạt** (`y = −0.022 m`) phân vùng tự nhiên giữa khu Tây-Nam, khu Sân bông hoa, khu Port of Sky Treasure, khu ven bờ Tây hồ nước, khu Nepal – Angkor và trảng cỏ nửa Đông.
   - **8 mảng sân lát đá/gạch phẳng** (`y = +0.022 m`) bao quanh chân các cụm trò chơi và nhà mái ngói (`Garuda Valley`, `Carousel`, `Port of Sky Treasure`, cụm `Đu quay tròn #9–#10`, `Nhà ngói đỏ ven hồ #12`, cụm `Paradise Fall`, cụm `Công trình tròn #13–#14`, `Highway Boat`).
3. **Hệ thống tiểu cảnh đường dạo & chiếu sáng emissive (`appendWave6aPropsAndLanterns`):**
   - **24 Trụ đèn lối đi & quảng trường (`PARK_PATH_LAMP_POS[24]`, cao `5.1 m`):** đặt dọc rìa Đông – Tây Quảng trường Trung tâm, sân đón Cổng Ngọ Môn và trục `PATH_WEST_SPINE`. Phần bóng đèn phát sáng vàng ấm ở chế độ Hoàng hôn / Đêm **chỉ dùng vật liệu `GL_EMISSION`** (biên dịch trong `emissiveLampList`), không dùng quá 8 nguồn sáng `GL_LIGHTi` của OpenGL.
   - **16 Ghế nghỉ công viên (`PARK_BENCH_POS[16]`)** bằng gỗ-sắt dọc rìa quảng trường, bờ hồ Nam và các lối đi chính.
   - **16 Bồn hoa hộp màu (`PARK_FLOWER_PLANTER_POS[16]`, `2.8 × 1.4 × 1.08 m`)** với 3 màu hoa (đỏ, vàng, hồng tím) bố trí đối xứng ở sân trước/sau Ngọ Môn, Quảng trường Trung tâm và khu Tây.
   - **6 Cột cờ lễ hội (`PARK_FLAGPOLE_POS[6]`, cao `11.8 m`, cờ đỏ sao vàng):** 2 cột cờ trước Cổng Ngọ Môn (`(−3.0, +148.0)`, `(+35.2, +148.0)`) và 4 cột cờ hai bên thềm Sân khấu Sun Wheel (`(±21.0, −7.5)`, `(±28.0, −7.5)`).
   - **6 Biển chỉ dẫn đa hướng (`PARK_SIGNBOARD_POS[6]`, cao `3.4 m`, 3 bảng mũi tên màu)** tại 6 ngã ba chính.
   - **34 Đèn lồng đỏ phát sáng emissive** treo dọc hai cánh tường thành Cổng Ngọ Môn và dưới 4 góc mái của 6 nhà chòi Quảng trường / Sun Wheel / Bờ hồ.

#### 12.8.2. Hạng mục Đợt 6b (123 Cây xanh bằng Display List & Lộ trình Người đi dạo)
1. **123 Cây xanh đơn giản (`PARK_TREES[123]` trong `ParkLayout.h`, dựng bằng `appendWave6bTrees` trong Display List `archSmoothList`):**
   - Gồm 3 kiểu xen kẽ với hệ số tỷ lệ `0.88..1.25` và 4 sắc độ lá xanh:
     - **Kiểu 0 (Cây bóng mát tán tròn 2 tầng):** thân trụ nâu + 2 khối tán đa giác tròn chồng nhau.
     - **Kiểu 1 (Cây thông / tùng tán tháp 3 tầng):** thân trụ nâu + 3 tầng tán chóp nhọn.
     - **Kiểu 2 (Cây cọ / dừa nhiệt đới):** thân trụ cao + búp đỉnh và 4 tàu lá vươn 4 hướng.
   - **Kiểm tra va chạm bằng script:** toàn bộ `123 / 123` cây nằm trên đất khô (cách bờ hồ `≥ 2.2 m`, **0 cây dưới hồ**), nằm ngoài biên của cả 6 tuyến lối đi (**0 cây trên lối đi**), nằm ngoài toàn bộ 37 khung bao công trình/trò chơi, và **cách dầm cũng như mọi chân trụ Monorail `≥ 3.6 m`** (vượt yêu cầu `≥ 3.0 m`). Nửa phía Đông ranh giới (`X > +76 m`) giữ trống đúng hiện trạng 2019.
2. **Cập nhật lộ trình 18 người đi dạo (`createDefaultCrowd()` trong [`src/actors/Human.h`](../src/actors/Human.h)):**
   - Thay toàn bộ các tâm đi bộ cũ (vốn đi xuyên qua Sân khấu Sun Wheel `(0, −15)` hoặc vùng trống cũ) bằng **18 du khách** đi bộ tuần hoàn trên 8 khu vực an toàn: Quảng trường Nam Sân khấu Sun Wheel `(0, 0, +4.0)`, Quảng trường Đông & Tây-Bắc Tháp Đồng Hồ `(+28, 0, +34)` & `(−6, 0, +38)`, Quảng trường Nam Tháp Đồng Hồ `(+16.1, 0, +92)`, đường dẫn Nam Cổng Ngọ Môn `(+16.1, 0, +156)`, Sân bông hoa `(-108, 0, +76)`, ngã ba Port of Sky Treasure `(-188, 0, +38)`, sân Đông Sky Tower `(-238, 0, -54.6)`, hành lang khu giữa `(-120, 0, -108)` và Quảng trường Angkor `(-212, 0, -274)`.

#### 12.8.3. Hiệu năng (FPS trước và sau Đợt 6) & Bộ ảnh kiểm tra Đợt 6
- **Đo FPS ở độ phân giải `1280 × 768` (200 khung hình liên tiếp, chế độ Hoàng hôn bật đầy đủ đèn & hoạt ảnh của toàn bộ công viên):**
  - **Trước Đợt 6 (cuối Đợt 5B):** `163.7 FPS` (`200 frames / 1.222 s`).
  - **Sau Đợt 6 (thêm toàn bộ bệ đá, mái che BumperCars, 16 mảng cỏ/sân đá, 24 trụ đèn, 16 ghế nghỉ, 16 bồn hoa, 6 cột cờ, 6 biển chỉ dẫn, 34 đèn lồng, 123 cây xanh và 18 người đi dạo):** **`118.5 FPS`** (`200 frames / 1.688 s` — cao hơn ngưỡng `100 FPS` nên không cần cắt giảm số cây hay chi tiết).
- **Bộ ảnh kết quả Đợt 6:**
  - [`img/wave6_overview_high.png`](img/wave6_overview_high.png) — Ảnh 1 Đợt 6: Toàn cảnh góc cao toàn công viên lúc hoàng hôn.
  - [`img/wave6_gate_plaza_sunwheel.png`](img/wave6_gate_plaza_sunwheel.png) — Ảnh 2 Đợt 6: Cận Quảng trường Cổng Ngọ Môn – Tháp Đồng Hồ – Sân khấu Sun Wheel với cột cờ, bồn hoa, trụ đèn phát sáng, đèn lồng, cây xanh và người đi dạo.
  - [`img/wave6_west_trees.png`](img/wave6_west_trees.png) — Ảnh 3 Đợt 6: Khu phía Tây phủ nhiều cây xanh, mái che + hàng cột quanh `BumperCars`, bệ đá dưới `Playground` & `PirateShip`, các mảng cỏ đậm/nhạt và sân lát đá quanh các trò chơi.
  - [`img/wave6_eye_level.png`](img/wave6_eye_level.png) — Ảnh 4 Đợt 6: Góc nhìn ngang tầm mắt du khách (`y = 3.5 m`) từ Quảng trường Trung tâm nhìn về Tháp Đồng Hồ, Sân khấu 2 màn hình tròn, Sun Wheel, chòi mái ngói treo đèn lồng, bồn hoa, ghế nghỉ và người đi bộ.
  - [`img/wave6_ngomon_from_north.png`](img/wave6_ngomon_from_north.png) — Ảnh chụp Cổng Ngọ Môn nhìn từ phía Bắc xác nhận dầm Monorail (`Z = +121.8 m`) không che khuất Cổng Ngọ Môn.
  - [`img/wave6_topdown_overlay.png`](img/wave6_topdown_overlay.png) — Ảnh chụp trực giao từ trên xuống sau Đợt 6 chồng lên ảnh vệ tinh Wayback 2019.

---

### 12.9. Chỉnh bề mặt ĐỢT 7 (Mặt đất mờ dần ra đường chân trời & Màn hình xanh dịu có chữ/hoa văn tại Sân khấu Sun Wheel)

1. **Mở rộng nền ngoài ranh giới thành mặt đất mờ dần kèm đường chân trời (`drawParkGroundAndLake()` trong `src/main.cpp`):**
   - Thay tấm nền tối hẹp `[-620..+640]` bằng vành địa hình hướng tâm mở rộng tới bán kính `R = 2 150 m` (`y = −0.10 m`), chuyển sắc mượt từ màu đất cỏ ngoại vi (`R = 280 m`) qua lớp sương mờ trung cảnh (`R = 820 m`) và hòa hoàn toàn vào màu bầu trời tại đường chân trời (`R = 2 150 m`) theo cả 3 chế độ Ngày / Hoàng hôn / Đêm, kết hợp dải núi mờ xa ở chân trời (`R = 1 980 m`). Triệt tiêu hoàn toàn viền đen ngoài ranh giới công viên.
2. **Đổi màn hình Sân khấu Sun Wheel từ trắng phẳng sang nền xanh dịu có chữ & hoa văn (`drawStageCircularScreensAndLights()` trong `src/rides/ParkArchitecture.h`):**
   - Màn hình LED chính giữa sân khấu được vẽ với nền chuyển sắc xanh ngọc dịu → xanh biển trầm, kèm dải sóng nước ở chân màn hình, biểu tượng Vòng quay Mặt Trời 12 nan hoa vàng ở tâm và dòng chữ **`"ASIA PARK"`** phía trên; 2 màn hình tròn hai bên dùng nền xanh dịu đồng bộ kèm hoa văn mặt trời/hoa sen 8 cánh vàng ở tâm.
3. **Hiệu năng & Bộ 3 ảnh toàn cảnh Đợt 7:**
   - **FPS tại `1280 × 768` (200 khung hình liên tiếp):** **`118.7 FPS`** (`200 frames / 1.684 s` — vượt ngưỡng `≥ 100 FPS`).
   - [`img/wave7_panorama_sunset.png`](img/wave7_panorama_sunset.png) — Ảnh toàn cảnh 1 (Góc cao Nam – Tây Nam lúc hoàng hôn, thấy rõ mặt đất mờ dần ra đường chân trời và màn hình sân khấu xanh dịu).
   - [`img/wave7_panorama_west_day.png`](img/wave7_panorama_west_day.png) — Ảnh toàn cảnh 2 (Góc cao khu Tây ban ngày với đường chân trời xanh dịu và toàn bộ công viên).
   - [`img/wave7_panorama_stage_sunwheel.png`](img/wave7_panorama_stage_sunwheel.png) *(kèm cận cảnh sân khấu [`img/wave7_stage_closeup.png`](img/wave7_stage_closeup.png))* — Ảnh toàn cảnh 3 (Trục Quảng trường Trung tâm nhìn về Sân khấu Sun Wheel có chữ `"ASIA PARK"` & biểu tượng vòng quay trên màn hình xanh dịu cùng đường chân trời phía Bắc).

---

## 13. Kiểm toán 18 ảnh tham chiếu (`docs/img/ref/`), kiểm chứng Wayback 2019 & Kế hoạch Đợt 8a/8b (Chốt Phương án 1)

> **Trạng thái:** Đã hoàn thành toàn bộ **Đợt 8a + Đợt 8b + Đợt 8c**, bao phủ **100% (17/17 công trình + 1 bản vẽ tổng mặt bằng)** trong thư mục `docs/img/ref/`. Phần 8 hạng mục quy hoạch tổng thể nửa phía Đông (**Đợt 8c**) được đóng gói riêng trong [`src/rides/MasterPlan8c.h`](../src/rides/MasterPlan8c.h), có phím bật/tắt **`[K]`** (và mục menu chuột phải) kèm nhãn hiển thị rõ trên HUD: **`"Quy hoach tong the, khong phai hien trang 2019"`**.

### 13.1. Đánh giá bộ 18 ảnh `docs/img/ref/` & liệt kê mâu thuẫn số liệu bản vẽ

Toàn bộ 18 ảnh PNG (`1024×572 px`) trong [`docs/img/ref/`](img/ref/) đều là ảnh **sinh bằng AI** mô phỏng tổng mặt bằng và bản vẽ kỹ thuật CAD. Do đó:
- **Tổng mặt bằng [`1795453337010_image.png`](img/ref/1795453337010_image.png):** Chỉ mang tính minh họa tương quan phân khu, **không đúng tỷ lệ và không đúng hệ tọa độ WGS84** (lỗi văn bản `MASTEN PLAN`, `LEASNE`, `PANOA RESTAURANT`, `OANUDA VALLEY`, `PARADIBE FALL` / `PARADISE PALL`; trùng số hiệu `(2)`, `(3)`, `(8)`, `(12)` xuất hiện 2 lần, `(10)` xuất hiện 3 lần; đặt `VƯỜN TRÚC` 2 lần ở hai góc khác nhau; đánh `(1)` ở lối Tây trong khi `CỔNG THÀNH` phía Nam không số). Mọi tọa độ chuẩn vẫn lấy theo [`src/core/ParkLayout.h`](../src/core/ParkLayout.h).
- **Quy tắc dùng 17 bản vẽ kỹ thuật:** Kích thước mặt bằng lấy theo **khung bao đo trên ảnh vệ tinh / số liệu đã xác minh trong tài liệu này**, còn **hình dáng kiến trúc và tỷ lệ chi tiết** mới lấy từ bản vẽ sau khi đã lọc bỏ các kích thước tự mâu thuẫn của AI.
- **Liệt kê cụ thể mâu thuẫn trên bản vẽ Đền Angkor Wat ([`1795453359463_image.png`](img/ref/1795453359463_image.png)):**
  1. **Mâu thuẫn chiều rộng (`X`):** Mặt đứng chính ghi `38.00 m` ở dải kích thước dưới nhưng ghi `36.00 m` ở dải trên; mặt đứng sau ghi `36.0 m`; mặt bằng tổng thể ghi `26.0 m` (nét mờ dễ đọc lẫn `36.0 m`) nhưng chuỗi phân đoạn trên lại cộng ra `39.0 m`; khung tên dọc ghi `36.0 m` (nét chữ mảnh dễ lẫn `96.0 m`).
  2. **Mâu thuẫn chiều sâu (`Z`):** Khung tên ghi `24.0 m`; cạnh phải mặt bằng ghi `20–21 m`; mặt đứng bên ghi `30.0 m`.
  3. **Mâu thuẫn chiều cao (`Y`):** Các mốc cao độ ghi tổng `+20.00 m` (nền `+4.00 m`, hành lang cột `+8.50 m`, 4 tháp góc `+14.50 m`, tháp tâm `+20.00 m`), nhưng chuỗi kích thước dọc hai bên mặt đứng lại cộng sai lên `26 m` / `31 m`.
- **Các mâu thuẫn chính ở những bản vẽ liên quan Đợt 8a/8b khác:**
  - **Tháp Đồng Hồ ([`1795453346086_image.png`](img/ref/1795453346086_image.png)):** Mặt đứng chính ghi đế `28.0 m` (chuỗi con `5+5+5 = 15 m`), mặt bằng ghi `550 m²` (`~23.5×23.5 m`) nhưng hai cạnh mặt bằng chỉ ghi `12.5×12.0/13.0 m`; chiều cao ghi `29.0 m` ở mặt đứng chính nhưng chuỗi con cộng ra `44.2 m` và mặt đứng bên/mặt cắt ghi `20.0 m` / `19.0–28.0 m`; mốc cao độ lẫn `0.00 / -6.00 / 0.00 / 1.00 / 5.0 / 0.0` (chữ `FROT ELEVATION`, `OROUND FLOOR PLAN`, `21/18/2018`). Khung bao chuẩn lấy từ ảnh vệ tinh Wayback 2019 ở Mục 5.5 là **`~25 × 24 m`**, cao **`~29 m`**.
  - **Festival Carousel ([`1795453381958_image.png`](img/ref/1795453381958_image.png)):** Vẽ đu quay Venice 2 tầng `ĐK 24.0 m`, cao `14.0 m`, nhưng chuỗi dọc cộng `0.9+14+1.5+6 = 22.4 m`, chuỗi mặt bằng ghi `3.5+4.5 = 24.0 m`. Theo Quyết định 2, giữ tâm & đường kính vệ tinh **`~17.0 m`** và thu hình dạng từ bản vẽ theo tỷ lệ **`~0.71`** (cao **`~10.0 m`**).

### 13.2. Bảng kiểm chứng tồn tại trên ảnh Wayback 2019 (Mốc `SNAPSHOT_YEAR = 2019`)

> **Minh bạch file ảnh trong repo:** Các file tile thô `wb19_south.png`, `wb19_mid.png`, `wb19_north.png`, `wb2019_z18_park`, `wb2019_east`, `z19_south_plaza.png` được dẫn ở các mục trước không lưu rời trong thư mục repo. Trên đĩa tại `docs/img/`, ảnh vệ tinh **Esri Wayback 2019 (`release 4756`, 2019-12-12)** hiện diện trực tiếp làm nền của các ảnh ghép chuẩn hóa:
> - [`img/layout_overlay.png`](img/layout_overlay.png) (và [`img/wave3_vs_overlay.png`](img/wave3_vs_overlay.png), [`img/wave4_topdown_overlay.png`](img/wave4_topdown_overlay.png), [`img/wave5a_topdown_overlay.png`](img/wave5a_topdown_overlay.png), [`img/wave5b_topdown_overlay.png`](img/wave5b_topdown_overlay.png), [`img/wave6_topdown_overlay.png`](img/wave6_topdown_overlay.png))
> - [`img/step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) (phủ vùng khu Tây & trục Nam–Bắc 2019 với khung ứng viên `#1..#19`).

| Công trình / Hạng mục | Bằng chứng Wayback 2019 có trong repo & đối chứng thực địa | Trạng thái 2019 | Ghi chú kỹ thuật |
|:---|:---|:---|:---|
| **Sun Wheel** (`(0, 65, -38)`) | Thấy rõ 4 móng chữ A + vành tròn trên nền Wayback 2019 của [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + Sun World / VnExpress / Wiki. | **Có trong 2019 (Đã xác minh)** | Không sửa `SunWheel.h`. |
| **Thủy đình & Hồ Bắc** (`(-5.4, 0, -107.7)`) | Thấy rõ chòi bát giác trên mặt nước, cầu gỗ nối bờ Nam và bờ kè hồ trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png). | **Có trong 2019 (Đã ước lượng từ ảnh vệ tinh)** | Đã đặt đúng Đợt 1. |
| **Cổng Ngọ Môn + 2 cánh tường thành + 2 hồ sen** (`(+16.1, 0, +139.1)`) | Thấy rõ toàn bộ cụm cổng 3 cửa, 2 cánh tường thành và 2 hồ sen trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + ảnh Commons `49356303688`. | **Có trong 2019 (Đã ước lượng/xác minh)** | Code `NgoMonGate.h` tốt hơn BP2; giữ nguyên. |
| **Tháp Đồng Hồ Trung Tâm** (`(+10.7, 0, +61.6)`) | Thấy nguyên vẹn **khối đế ~25×24 m + mái tháp + bóng đổ dài hướng NNW** giữa Quảng trường Trung tâm trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) (khớp ghi chú `wb19_south.png`; đã bị tháo dỡ trên ảnh 2022 và [`sat_grid.png`](img/sat_grid.png) 2026). | **Có trong 2019 (Xác nhận trên Wayback 2019)** | Code hiện tại mới là 3 hộp `8.4→6.4m`; nâng cấp lên đế `~25×24m`, cao `~29m` ở **Đợt 8a**. |
| **Golden Sky Tower** (`(-260.0, 0, -54.6)`) | Thấy rõ chân tròn + bóng tháp trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + ảnh Commons `49358003906`, `49374882021`. | **Có trong 2019 (Đã xác minh cao 47m / vị trí từ Wayback 2019)** | — |
| **Queen Cobra** (`(-255.0, 0, -358.0)`) | Thấy rõ bán đảo đường ray thực tế ở góc Tây-Bắc trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + RCDB `#14194`. | **Có trong 2019 (Đã xác minh)** | Không sửa `RollerCoaster.h`. |
| **Tuyến Monorail + Ga Nam + Ga Nepal** | Dầm bê tông toàn tuyến (~1.87 km, kể cả đoạn vượt hồ & nhánh Đông) + 2 nhà ga thấy rõ trên [`layout_overlay.png`](img/layout_overlay.png) + Commons `49370578306`, `49373368582`, `49374702231`. | **Có trong 2019 (Đã ước lượng/xác minh)** | Không dời waypoint. |
| **Garuda Valley** (`(-124.1, 0, +155.7)`) | Thấy rõ vòng ray + nhà ga dài trong khung `#1 (34×26m)` trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + RCDB `#12676` & Commons `49374160882`, `49356303773`. | **Có trong 2019 (Đã xác minh tên/vị trí; hình dạng đơn giản hóa)** | — |
| **Port of Sky Treasure** (`(-224.0, 0, +22.0)`) | Thấy rõ cụm ray chữ S/T và nhà ga trong khung `#2 (36×56m)` trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + RCDB `#13603` & Commons `49358003766`. | **Có trong 2019 (Đã xác minh tên/vị trí; hình dạng đơn giản hóa)** | — |
| **Tòa nhà đá Paradise Fall (`#3`) & Ray số 8 (`#4`)** | Tòa nhà `#3 (-179.8, -91.1)` thấy trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + RCDB `#15548` & Commons `49358210737`; vòng ray `#4 (-199.0, -119.9)` thấy hình ray trên ảnh Wayback 2019 nhưng không có ghim RCDB riêng. | `#3`: **Có trong 2019 (Đã xác minh)**<br>`#4`: **Thấy vệt ray trên Wayback 2019 — `[Chưa xác minh]` tên riêng** | — |
| **Highway Boat (`#5`)** (`(-176.0, 0, -214.0)`) | Thấy rõ cụm ray trong khung `#5` trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) + RCDB `#15549` & Commons `49374702231`. | **Có trong 2019 (Đã xác minh tên/vị trí; hình dạng đơn giản hóa)** | — |
| **Quảng trường Ngôi sao & Cụm tháp Angkor (`#19`)** (`(-185.0, 0, -254.0)`) | Trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) **thấy rõ quảng trường lát ngôi sao 8 cánh (`~24–28 m`) và các khối kiến trúc/tháp tròn kề cận**; tuy nhiên chưa có ảnh mặt đất EXIF GPS hay nguồn văn bản xác nhận tên gọi chính thức `"Đền Angkor Wat"` và chi tiết bản vẽ BP5. | **`[Chưa xác minh]`** *(khung sân sao `28×28m` thấy trên Wayback 2019; tên gọi/kiến trúc đền chưa xác minh)* | Nâng cấp kiến trúc theo BP5 nằm gọn trong khung bao `28×28m` ở **Đợt 8a**. |
| **Festival Carousel** (`(-159.4, 0, +102.5)`) | Trên [`layout_overlay.png`](img/layout_overlay.png) & [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) thấy rõ **mái nón kẻ sọc ĐK `~16.5 m`**, nhưng không có ảnh mặt đất biển tên xác nhận là Festival Carousel. | **`[Chưa xác minh]`** | Giữ tâm & `ĐK ~17 m`, thu hình dạng BP10 tỷ lệ `~0.71` (cao `~10 m`) ở **Đợt 8b**. |
| **Các cụm nhà mái ngói & đu quay Đợt 5B** (`#6..#10, #12..#15, #18`) | Có vệt mái/đĩa tròn tương ứng trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png) nhưng chưa có ảnh mặt đất định danh từng hạng mục. | **`[Chưa xác minh]`** | Giữ nguyên theo Đợt 5B. |
| **3 trò chơi PR #1 (`Playground #11`, `BumperCars #16`, `PirateShip #17`)** | Trên [`step0_wave4_6_candidates.png`](img/step0_wave4_6_candidates.png), các tọa độ này không có ảnh mặt đất chứng minh khớp 3 trò chơi này trong thực tế 2019 (tọa độ `(-172,-2)` trên ảnh là sân/cây xanh). | **`[Chưa xác minh]`** | Giữ nguyên 3 file không sửa. **`PirateShip` KHÔNG phải đại diện của `The Flying Kirins`.** |
| **8 hạng mục nửa Đông / Quy hoạch mới** (`Panda`, `Symphony of River`, `Nhà hát Âu Cơ`, `Dino Island`, `Vườn Trúc`, `Merlion`, `Dragon Boat`, `The Flying Kirins`) | Trên [`layout_overlay.png`](img/layout_overlay.png) (Wayback 2019), nửa phía Đông (`X > +65 m`, ngoài dầm Monorail) chỉ là **nền cỏ trống và đường đất**, không có công trình nào trong 8 hạng mục này (ví dụ `Symphony of River` ra mắt năm 2024). | **Không tồn tại trong hiện trạng 2019** | Tách sang **Đợt 8c** (`"quy hoạch tổng thể, không phải hiện trạng 2019"`), **chưa làm**. |

### 13.3. Quy hoạch riêng Đợt 8c & Ghi chú `PirateShip` vs. `The Flying Kirins`

- **Ghi chú `The Flying Kirins` vs. `PirateShip`:** Theo Quyết định 3, **không** dựng `The Flying Kirins` ([`1795453378911_image.png`](img/ref/1795453378911_image.png)) ở Đợt 8a/8b. Mô hình [`PirateShip.h`](../src/rides/PirateShip.h) (`(-172.0, 0, -2.0)`) của thành viên nhóm được giữ nguyên 100% và **chỉ là hạng mục mô phỏng phục vụ hoạt ảnh phân cấp trong đồ án, không phải đại diện cho `The Flying Kirins`**.
- **Phạm vi Đợt 8c (tách riêng, chưa thực hiện):** Khi triển khai ở giai đoạn sau, 8 công trình trong bộ ref gồm **Nhà hàng Panda** (`1795453362382`), **Sân khấu nổi Symphony of River** (`1795453392187`), **Nhà hát Âu Cơ** (`1795453395295`), **Đảo Khủng Long / Dino Island** (`1795453400510`), **Vườn Trúc / Bamboo Garden** (`1795453403998`), **Tượng & Hồ Merlion** (`1795453365678`), **Thuyền Rồng / Dragon Boat** (`1795453369127`) và **The Flying Kirins** (`1795453378911`) sẽ được gom vào một nhánh/chế độ tách biệt có **phím bật/tắt**, mặc định TẮT ở trạng thái `SNAPSHOT_YEAR = 2019`, và gắn nhãn hiển thị rõ **`"quy hoạch tổng thể, không phải hiện trạng 2019"`**.

### 13.4. Kiểm tra va chạm hình học cho Đợt 8a & 8b (đã tính trực tiếp trên `ParkLayout.h` & đường cong `BezierTrack.h`)

1. **Tháp Đồng Hồ (`CLOCK_TOWER_POS = (+10.7, 0.0, +61.6)`) — khung bao `25.0 × 24.0 m`, cao `~29.0 m`:**
   - **Với 2 nhà chòi quảng trường:** Cách mép Đông `PLAZA_PAVILION_WEST_POS (-24.0, 0, +65.0)` (`16×9 m`, mép Đông `X = -16.0`) là **`14.20 m`**; cách mép Tây `PLAZA_PAVILION_EAST_POS (+48.0, 0, +65.0)` (`16×9 m`, mép Tây `X = +40.0`) là **`16.80 m`**.
   - **Với `PATH_MAIN_SPINE` (rộng `6.0 m`, nửa rộng `3.0 m`):**
     - Đoạn `[2→3]` (`(16.1, 94.8) → (30.0, 74.9)`): cách khung `25×24 m` là `6.32 m` tới tim (**`3.32 m`** tới mép đường).
     - Đoạn `[3→4]` (`(30.0, 74.9) → (30.0, 48.3)`): cách khung `25×24 m` là `6.80 m` tới tim (**`3.80 m`** tới mép đường).
     - Đoạn `[4→5]` (`(30.0, 48.3) → (0.0, 28.4)`): điểm gần nhất tại góc Đông-Bắc khối đế `(23.2, 49.6)` cách tim đường `4.84 m` (**`1.84 m`** tới mép đường).
   - **Với tiểu cảnh & người đi bộ (`Human.h`):** Cách bồn hoa gần nhất `PARK_FLOWER_PLANTER_POS[6] (-8.0, 42.0)` là **`9.81 m`**; cách 3 vòng tuần hoàn người đi dạo gần quảng trường (`(28, 34) R=6.5`, `(-6, 38) R=6.0`, `(16.1, 92) R=7.5`) lần lượt **`9.82 m`**, **`6.34 m`**, **`10.90 m`**; `0` cây xanh hay trụ đèn bị đè.
   - **Ràng buộc hình học Đợt 8a:** Toàn bộ **4 cầu thang trục + bệ đá giật cấp phải nằm gọn hoàn toàn bên trong `25.0 × 24.0 m`** (lõi bệ chính `~19 × 18 m`, 4 cầu thang nhô ra tới biên `±12.5 m` theo X và `±12.0 m` theo Z; có thể vát/giật cấp góc bệ để tăng thêm khoảng thoáng tới `PATH_MAIN_SPINE[4→5]`).
2. **Quần thể Đền & Quảng trường Angkor (`ANGKOR_STAR_PLAZA_POS = (-185.0, 0.0, -254.0)`) — khung bao vệ tinh `28.0 × 28.0 m` (`#19`):**
   - **Với đường cong Bézier Monorail (KHÔNG dời bất kỳ waypoint nào):** Với khung bao `28.0 × 28.0 m` (`X ∈ [-199, -171], Z ∈ [-268, -240]`), khoảng cách ngang nhỏ nhất từ AABB đến tim đường cong Bézier `MONORAIL_WAYPOINTS` (`seg 9` quanh `(-200.1, 14.0, -273.9)`) là **`5.95 m`** (`~4.95 m` tới mép dầm/tàu); nếu thu lõi khối đền + cầu thang về **`26.0 × 26.0 m`** trên nền sân sao `28×28 m` khoảng cách này tăng lên **`7.08 m`** (hoặc **`8.23 m`** với `24.0 × 24.0 m`).
   - **Với lối đi, công trình lân cận & tiểu cảnh:** Cách mép `PATH_LAKESIDE_WEST[4→5]` **`10.36 m`** (`11.86 m` tới tim); cách `PATH_WEST_SPINE[11]` (`(-214, -275.9)`) **`> 14 m`**; cách `HIGHWAY_BOAT_CENTER (-176, -214)` **`13.0 m`**; cách `ANGKOR_LAKESIDE_HALL_POS (-138, -242)` **`~21.0 m`**; cách vòng đi dạo `Human` (`(-212, -274) R=4.2`) **`10.12 m`**; cách `tree[121]` **`11.00 m`**; cách `benches[15]` **`9.22 m`**.
   - **Ràng buộc hình học Đợt 8a:** Toàn bộ kiến trúc đền giật cấp + hành lang cột + 4 cầu thang chữ thập + 5 tháp búp sen nằm trọn trong khung `26–28 × 26–28 m` (lõi khối đền `~20 × 20 m`, 4 cầu thang nhô trong giới hạn `±13..±14 m`), tổng cao `~15.0 m` (thu tỷ lệ `~0.74` so với `36–38 m` trong bản vẽ, đồng bộ tỷ lệ với mặt bằng vệ tinh `28×28 m`).
3. **Festival Carousel (`CAROUSEL_POS_UNVERIFIED = (-159.4, 0.0, +102.5)`) — giữ `ĐK ~17.0 m` (`platformRadius = 8.5 m`), thu tỷ lệ `~0.71`, cao `~10.0 m`:**
   - **Với `PATH_WEST_SPINE`:** Đường chạy phía Bắc tại `Z = 86.5 m` (rộng `5.0 m` → mép Nam `Z = 89.0 m`). Vành Bắc của đu quay `R = 8.5 m` ở `Z = 94.0 m` → khe hở thoáng **`5.00 m`** (không sửa `PATH_WEST_SPINE`, không dời tâm).
   - **Với Monorail:** Khoảng cách tới đường cong Bézier Monorail (`seg 2→3`) là **`25.10 m`** từ tâm (**`16.60 m`** từ vành ngoài `R = 8.5 m`).
   - **Ràng buộc hình học Đợt 8b:** Toàn bộ bệ bậc, hàng rào bảo vệ, 2 tầng sàn, cầu thang lên tầng 2 và chi tiết phụ phải thu gọn trong bán kính `<= 8.8 m` (`ĐK <= 17.6 m`) để bảo toàn khe thoáng `>= 4.7 m` tới `PATH_WEST_SPINE`.

### 13.5. Kế hoạch Đợt 8a & 8b (Nộp chờ duyệt — chưa chỉnh sửa mã nguồn)

#### Đợt 8a — Nâng cấp Tháp Đồng Hồ & Quần thể Angkor (theo Quyết định 4)
1. **Tái cấu trúc helper dựng lưới dùng chung (`src/geometry/MeshModel.h` hoặc `src/geometry/GeometryBuilders.h` mới):**
   - Gom `appendBoxByYRange` và `appendRotatedTranslatedMesh` (hiện đang trùng lặp giữa `NgoMonGate.h` và `ParkArchitecture.h`) về 1 nơi dùng chung, bổ sung helper `appendStaircase` (bậc thang + tay vịn liền khối) và `appendOctagonalTieredTower` (thân tháp bát giác thu nhỏ dần kèm mái đao/mái bát giác) — tuân thủ nghiêm **không dùng `glutSolid*`**, chuẩn hóa pháp tuyến `computeNormals()`.
2. **Nâng cấp Tháp Đồng Hồ Trung Tâm (`src/rides/NgoMonGate.h` — `buildClockTowerGeometry`):**
   - Vị trí giữ nguyên `ParkLayout::CLOCK_TOWER_POS (+10.7, 0.0, +61.6)`, `clockTowerScale = 1.00f`.
   - Hình học mới bám BP3 (`1795453346086_image.png`) & khung bao vệ tinh Wayback 2019:
     - **Tầng đế đá giật cấp (`25.0 × 24.0 m` toàn cụm kể cả 4 cầu thang):** bệ dưới `~19.5 × 18.5 m` cao `1.6 m`, bệ thượng `~15.5 × 15.0 m` cao tới `3.4 m`, viền lan can đá bao quanh sân thượng bệ, **4 cầu thang trục (Bắc, Nam, Đông, Tây)** dẫn lên sân bệ và dừng đúng trong giới hạn `±12.5 m (X)` × `±12.0 m (Z)`.
     - **Thân tháp bát giác 4 tầng, 5 lớp mái:** dựng thân tháp bát giác thu nhỏ theo từng tầng từ mặt bệ `y = 3.4 m` lên đỉnh chóp **`~29.0 m`**, mỗi tầng có gờ cổ diềm và mái ngói cong bát giác liền khối, tầng mặt đồng hồ gắn **4 mặt đồng hồ phát sáng emissive** ban đêm (giữ nguyên cơ chế không thêm `GL_LIGHTi`).
     - Biên dịch toàn bộ hình học tĩnh vào `clockSmoothList` / `clockFlatList` để không làm giảm FPS.
3. **Nâng cấp Quần thể Đền & Quảng trường Angkor (`src/rides/ParkArchitecture.h` — `appendAngkorStarPlazaTowers` & `drawTiledCentralPlaza`):**
   - Vị trí giữ nguyên `ParkLayout::ANGKOR_STAR_PLAZA_POS (-185.0, 0.0, -254.0)`. Giữ nhãn **`[Chưa xác minh tên/kiến trúc chính thức; khung sao 28×28m thấy trên Wayback 2019]`**.
   - Giữ nền quảng trường lát sao 8 cánh phía ngoài/dưới (`28.0 × 28.0 m`), bên trên dựng tổ hợp đền giật cấp theo hình dạng BP5 (`1795453359463_image.png`) thu gọn vừa khung `26.0 × 26.0 m` (toàn khối + cầu thang nằm lọt trong `28.0 × 28.0 m`):
     - **Nền đá giật cấp 2 tầng** (`~20 × 20 m` → `~15 × 15 m`, cao tổng `~3.0 m`) + **4 cầu thang chữ thập** vươn tới `±13.0 m` (cách tim Monorail `>= 5.95 m`).
     - **Hành lang cột bao quanh** (`colonnade gallery` cao tới `~6.3 m`) bằng cột đá + xà mái đá liền khối.
     - **5 tháp búp sen (Prang Khmer)** nhiều tầng thuôn nhọn: 4 tháp góc cao `~10.8 m` và 1 tháp trung tâm cao `~15.0 m` gắn chóp vàng.
     - Biên dịch toàn bộ vào `archSmoothList` / `archFlatList`.
4. **Kiểm tra & nghiệm thu Đợt 8a:**
   - Biên dịch không lỗi/cảnh báo với `g++ -std=c++17 -O2 -Wall -Wextra`; script kiểm tra 0 điểm giao cắt với `PLAZA_PAVILION_WEST/EAST`, `PATH_MAIN_SPINE`, `PATH_LAKESIDE_WEST`, `MONORAIL_WAYPOINTS` và `Human`; chụp ảnh cận cảnh + toàn cảnh + đối chiếu FPS (`>= 100 FPS` ở `1280×768`).

#### Đợt 8b — Nâng cấp Carousel 2 tầng tỷ lệ ~0.71 & Tối ưu hiệu năng (theo Quyết định 2)
1. **Nâng cấp Festival Carousel (`src/rides/Carousel.h`) — giữ `(-159.4, 0.0, +102.5)`, `ĐK ~17 m` (`R = 8.5 m`), cao `~10.0 m`:**
   - **Giữ nguyên hoàn toàn:** `position = ParkLayout::CAROUSEL_POS_UNVERIFIED`, `platformRadius = 8.5f`, không sửa `PATH_WEST_SPINE`, không đụng Monorail, giữ ghi chú `[Chưa xác minh]`.
   - **Cấu trúc mới lấy theo bản vẽ BP10 (`1795453381958_image.png`) thu tỷ lệ `~0.71` (`17 / 24`):**
     - **Phần tĩnh ngoài (`R <= 8.8 m`):** Bệ bậc tròn ngoài, hàng rào bảo vệ thấp chia lối vào và **quầy vé nhỏ** đặt sát rìa Nam-Đông (giữ toàn bộ trong `R <= 8.8 m` để bảo toàn khe `>= 4.7 m` tới `PATH_WEST_SPINE`).
     - **Phần quay 2 tầng (biến đổi phân cấp cha → con giữ đúng nguyên lý Chương 4):**
       - Sàn tầng 1 (`R = 8.5 m`, `y = 0..0.55 m`) + trụ lõi trung tâm trang trí (`R ≈ 1.6 m`) + **2 cầu thang cong đối xứng** nối từ sàn tầng 1 lên **sàn tầng 2 (`R ≈ 6.2 m`, `y ≈ 4.1 m`)** kèm lan can tầng 2.
       - **Tầng 1:** `12` ngựa gỗ chia 2 vòng quay + cột đồng thẳng đứng, nhấp nhô sin `y(t)`.
       - **Tầng 2:** `8` ngựa gỗ vòng nhỏ hơn (`R ≈ 4.5 m`) di chuyển cùng sàn tầng 2 + chuyển động nhấp nhô ngược pha.
       - **Hệ mái 2 tầng kiểu Venice (`y ≈ 7.2..10.0 m`):** Cổ diềm trang trí + mái nón đa múi/sọc tầng 2 (`ParametricShapes`) + vương miện/đỉnh cầu vương miện chạm mốc **`~10.0 m`**, kèm dải đèn LED `GL_EMISSION` quanh cả 2 vành diềm mái tầng 1 và tầng 2 khi trời tối/hoàng hôn.
     - Biên dịch các phần tĩnh của mái/trụ/hàng rào vào **OpenGL Display List** cục bộ trong `Carousel` để bù chi phí đa giác khi lên 2 tầng.
2. **Tối ưu hiệu năng bổ sung & Bảo toàn quy tắc nhóm:**
   - Cache phần khung toa xe trong `MiniCoaster.h` (`drawMovingTrain`) và mặt lưới caro của `ParkArchitecture::drawTiledCentralPlaza()` vào Display List để đảm bảo FPS toàn cảnh vẫn `>= 100 FPS` sau khi thêm chi tiết Carousel 2 tầng, Tháp Đồng Hồ & Angkor.
   - **Tuyệt đối không chỉnh sửa** `SunWheel.h`, `RollerCoaster.h`, `Playground.h`, `BumperCars.h`, `PirateShip.h`.
3. *(Mục hỏi thêm khi duyệt 8a/8b — chưa tự đưa vào nếu chưa được đồng ý):* Có giữ nguyên hay muốn nâng cấp chi tiết hình dáng (trong cùng khung bao vệ tinh) cho 3 công trình đã xác minh 2019 là **Golden Sky Tower** (chóp hoa sen + sàn `+4.2m`), **Garuda Valley** (mái yên ngựa Minangkabau) và **Port of Sky Treasure** (nhà ga bến cảng gỗ) ở cuối Đợt 8b không?

### 13.6. Kết quả thực hiện & kiểm chứng Đợt 8a (Đã hoàn thành)

1. **Các file mã nguồn đã thêm / cập nhật:**
   - [`src/geometry/MeshBuilders.h`](../src/geometry/MeshBuilders.h) *(mới)*: gom `appendBoxByYRange` và `appendRotatedTranslatedMesh` dùng chung (xóa trùng lặp giữa `NgoMonGate.h` & `ParkArchitecture.h`), bổ sung `appendOrientedPolygon` (Newell winding check), `appendStaircase`, `appendOctagonalPrism`, `appendOctagonalRoof` (mái bát giác cong khép kín đáy) và `appendOctagonalTieredTower`.
   - [`src/rides/NgoMonGate.h`](../src/rides/NgoMonGate.h): nâng cấp `CentralClockTower2019` tại `CLOCK_TOWER_POS (+10.7, 0.0, +61.6)` lên bệ đá giật cấp 3 lớp + 4 cầu thang trục + lan can đá (nằm gọn trong `25.00 × 24.00 m`) và tháp bát giác 5 tầng mái cao **`28.97 m`**; điều chỉnh 4 mặt đồng hồ phát sáng (`clockFaceCenterY_`, `clockFaceOffset_`, tỷ lệ `k = 0.72f`) gắn khít lên mặt phẳng trục tầng 3; biên dịch vào Display List `clockSmoothList` / `clockFlatList`.
   - [`src/rides/ParkArchitecture.h`](../src/rides/ParkArchitecture.h): thay `appendAngkorStarPlazaTowers` bằng `appendAngkorTempleComplex` tại `ANGKOR_STAR_PLAZA_POS (-185.0, 0.0, -254.0)` gồm nền đá 2 tầng, 4 cầu thang trục chữ thập, hành lang cột 20 trụ, 4 tháp Prang góc (`10.8 m`) và 1 tháp Prang trung tâm (`15.0 m`); mở rộng nền sân sao 8 cánh ở `drawTiledCentralPlaza()` thành **`27.8 × 27.8 m`** (4 cánh chéo tỏa ra 4 góc sân).
   - **Không chỉnh sửa** `SunWheel.h`, `RollerCoaster.h`, `Playground.h`, `BumperCars.h`, `PirateShip.h` hay bất kỳ waypoint nào của `MONORAIL_WAYPOINTS` / `PATH_*`.
2. **Kết quả kiểm tra hình học & va chạm 3D (đo trên đỉnh lưới thật sau chỉnh sửa cuối):**
   - **Tháp Đồng Hồ:** `2 545` đỉnh, `1 035` mặt (`0` pháp tuyến lỗi/NaN), khung bao cục bộ `X ∈ [-12.50, +12.50]` (`25.00 m`), `Y ∈ [0.00, 28.97]`, `Z ∈ [-12.00, +12.00]` (`24.00 m`); cách tim `PATH_MAIN_SPINE` `4.84 m` (**`1.84 m`** tới mép đường), cách `PLAZA_PAVILION_WEST` **`14.20 m`**, `PLAZA_PAVILION_EAST` **`16.80 m`**, cách vòng người đi dạo `Human` tối thiểu **`6.34 m`**, cách cây `PARK_TREES` tối thiểu **`29.20 m`**.
   - **Quần thể Angkor:** `3 234` đỉnh, `2 228` mặt (`0` pháp tuyến lỗi/NaN), khung bao thế giới `X ∈ [-198.00, -172.00]` (`26.00 m`), `Y ∈ [0.00, 15.00]`, `Z ∈ [-267.00, -241.00]` (`26.00 m`) nằm trọn trên sân sao `27.8 × 27.8 m`; cách tim đường cong Bézier Monorail **`7.15 m`**, cách `PATH_LAKESIDE_WEST` `13.10 m` (**`11.60 m`** tới mép đường), cách `PATH_WEST_SPINE` **`17.92 m`**, cách người đi dạo `Human` **`11.45 m`**, cách cây gần nhất **`12.00 m`**.

### 13.7. Kết quả thực hiện & kiểm chứng Đợt 8b (Đã hoàn thành)

1. **Nâng cấp Festival Carousel 2 tầng kiểu Venice tỷ lệ `~0.71` ([`src/rides/Carousel.h`](../src/rides/Carousel.h)):**
   - Giữ nguyên vị trí `CAROUSEL_POS_UNVERIFIED (-159.4, 0.0, +102.5)`, `platformRadius = 8.5f` (`ĐK 17.0 m`), nâng tổng chiều cao lên **`~10.0 m`** (`canopyHeight = 10.0f` — thu tỷ lệ `~0.71` từ bản vẽ BP10 `Ø24 m × 14 m`):
     - **Phần tĩnh ngoài:** Bệ tròn 2 cấp, hàng rào bảo vệ (`R = 8.68 m`) chừa cổng vào phía Bắc và **quầy vé bát giác kiểu Venice** ở góc Đông-Nam sân lát (`R_center = 9.35 m`, góc `+45°`, tách rời hoàn toàn khỏi sàn quay `R = 8.48 m` và nằm về phía Nam nên giữ nguyên khe hở `5.00 m` tới `PATH_WEST_SPINE` ở phía Bắc).
     - **Khối quay 2 tầng (Parent) & 20 Ngựa gỗ (Children):** Trụ lõi 2 tầng, **2 cầu thang cong đối xứng** dẫn từ sàn tầng 1 (`y = 0.55 m`) lên sàn tầng 2 (`R = 6.38 m`, `y = 4.02 m`), lan can con tiện tầng 2, `12` ngựa gỗ tầng 1 (`R = 6.15 m` & `4.35 m`) + **`8` ngựa gỗ tầng 2** (`R = 4.55 m`, thu tỷ lệ `0.82`, nhấp nhô ngược pha), mái vòm nón tầng 2 + lầu đèn vương miện đạt đỉnh `10.04 m`, cùng 2 vòng đèn LED `GL_EMISSION` (28 bóng tầng 1 + 20 bóng tầng 2); biên dịch toàn bộ phần tĩnh/khung quay/3 mẫu ngựa vào OpenGL Display Lists (`staticBaseList_`, `rotatingStructureList_`, `horseLists_[3]`).
2. **Nâng cấp Golden Sky Tower — Đỉnh búp sen vàng & Sàn đón khách `+4.20 m` ([`src/rides/SkyTower.h`](../src/rides/SkyTower.h)):**
   - Giữ nguyên `SKY_TOWER_POS (-260.0, 0.0, -54.6)`, `totalHeight = 47.0f`, `towerRadius = 1.65f`, `gondolaRadius = 3.35f`.
   - **Tổ hợp chân tháp & nhà ga `+4.20 m` (gói gọn trong `R <= 7.88 m`, cách mép `PATH_WEST_SPINE[7]` `>= 5.62 m`):** Bệ bát giác tầng trệt, phòng điều hành bát giác dưới sàn, 8 cột trụ đỏ đỡ **sàn đón khách trên cao `y = 4.20 m`** (có lan can vàng bao quanh), **2 cầu thang trục phía Đông** dẫn lên sảnh đón và **chòi mái cong đón khách** tại `x = +5.55 m` (nằm ngoài hoàn toàn bán kính ghế Gondola `R = 3.85 m`, điều chỉnh `minY = 4.30 m` để ghế dừng đón khách ngay trên mặt sàn `+4.20 m`, 0% va chạm).
   - **Đỉnh búp sen vàng nhiều lớp (`y = 47.0 .. 52.75 m`, đèn chớp đỏ ở `y = 52.95 m`):** Đài sen 16 cánh loe rộng (`R = 1.92 → 2.95 m`) đỡ búp sen vàng 16 múi khép kín thu nhọn lên đỉnh, biên dịch vào `staticTowerList_`.
3. **Nâng cấp Garuda Valley & Port of Sky Treasure trong cùng khung bao vệ tinh ([`src/rides/MiniCoaster.h`](../src/rides/MiniCoaster.h) & [`src/core/ParkLayout.h`](../src/core/ParkLayout.h)):**
   - **Garuda Valley (`MiniCoaster[0]`, `stationStyle = 1`):** Giữ nguyên 100% tọa độ mặt bằng `(X, Z)` của 8 waypoint (`W = 29.29 m × D = 21.63 m`, dài ray `91.7 m`), nâng đỉnh đồi kéo phía Bắc lên **`12.52 m`**; thay nhà ga thường bằng **nhà ga mái yên ngựa cong kiểu Minangkabau / Batak (Indonesia)** theo BP15 (sàn cao `2.45 m`, cầu thang đá phía Nam, 6 cột đỏ, mái yên ngựa võng giữa `y = 7.75 m` và vút cao hai đầu sừng lên `10.65 m` kèm dải mào bờ nóc mạ vàng).
   - **Port of Sky Treasure (`MiniCoaster[1]`, `stationStyle = 2`):** Giữ nguyên 100% tọa độ mặt bằng `(X, Z)` của 8 waypoint (`W = 27.97 m × D = 43.04 m`, dài ray `128.9 m`), nâng đỉnh xoắn helix phía Bắc lên **`13.44 m`**; thay nhà ga thường bằng **nhà ga bến cảng phương Tây khung gỗ mái đầu hồi** theo BP12 (bệ cầu cảng có 8 cọc gỗ bến tàu bọc đai đồng, cầu thang gỗ, tường khung gỗ half-timbered, mái đầu hồi dốc 2 phía có vì kèo tam giác king-post và **tháp canh cảng biển cupola** trên sống mái cao `11.00 m`).
4. **Tối ưu hiệu năng Display List:**
   - Biên dịch thân/viền/ghế toa tàu trong `MiniCoaster::drawMovingTrain` vào `carListSmooth_` / `carListFlat_` (loại bỏ 36 lần cấp phát `PolygonMesh::createBox` mỗi khung hình) và biên dịch toàn bộ mặt nền lát gạch/sân sao/thảm cỏ trong `ParkArchitecture::drawTiledCentralPlaza()` vào `groundPlazaList_`.

### 13.8. Kết quả thực hiện & kiểm chứng Đợt 8c — Hoàn thiện 100% bộ bản vẽ `docs/img/ref/` (Đã hoàn thành)

1. **Nâng cấp Paradise Fall (`BP11` — [`1795453385446_image.png`](img/ref/1795453385446_image.png)) trong [`src/rides/MiniCoaster.h`](../src/rides/MiniCoaster.h) & [`src/core/ParkLayout.h`](../src/core/ParkLayout.h):**
   - **Phân tích mâu thuẫn bản vẽ AI:** Mặt đứng chính chỉ ghi `34.0 m` cho riêng phần đường ray (bỏ quên bề ngang nhà ga), mặt bằng ghi `7.8 + 34.0 + 7.5 = 49.3 m` nhưng dòng tổng dưới vẫn ghi `34.0 m`; chiều sâu ghi `4.0 + 16.8 + 4.2 = 25.0 m` nhưng tổng ghi `28.0 m`; mốc cao độ ghi hai lần `+12.00 m` và hai cột trái ghi hai lần `16.0 m`.
   - **Triển khai chuẩn hóa:** Giữ nguyên tâm & khung bao vệ tinh 2019 của tòa nhà núi đá `PARADISE_FALL_BLDG_POS (-179.8, 0.0, -91.1)` (`24 × 14 m`) và vòng ray số 8 `PARADISE_FALL_TRACK_CENTER (-199.0, 0.0, -119.9)` (`34 × 24 m`); nâng đỉnh đồi kéo vòng ray số 8 lên **`14.4 m`** và dựng thêm **Lầu Nhà Ga Cổ Điển 2 tầng mái ngói cong + cầu thang đá mặt Nam lên sàn đón `+4.80 m`** ngự trên đỉnh khối núi đá (tổng cao **`12.98 m`**).
2. **Triển khai 8 công trình Quy hoạch Tổng thể Đợt 8c trong [`src/rides/MasterPlan8c.h`](../src/rides/MasterPlan8c.h) (`9 979` đỉnh, `7 080` mặt, `0` pháp tuyến lỗi/NaN):**
   - Có phím tắt **`[K]`** và mục menu chuột phải để bật/tắt tức thời, hiển thị nhãn **`"[K] Dot 8c : BAT/TAT (Quy hoach tong the, khong phai hien trang 2019)"`** trên bảng HUD ([`src/ui/HUD.h`](../src/ui/HUD.h)):
   - **① Nhà hàng Panda (`BP6` — [`1795453362382_image.png`](img/ref/1795453362382_image.png)) tại `(+148.0, 0.0, -86.0)`:** Lọc bỏ chuỗi kích thước lỗi `7+8.5+7+8+12+8+7.5+7.8+8 = 73.8 m ≠ 26.0 m` trên bản vẽ AI để chuẩn hóa về **`26.0 (W) × 18.0 (D) × 14.0 (H) m`** (bệ đá tam cấp + 4 cầu thang, 2 tầng mái ngói lưu ly vàng cong, hệ đấu củng, ban công gỗ tầng 2, đôi rồng chầu bờ nóc và đôi tượng Gấu Trúc Panda trước sảnh); cách tim Monorail **`59.58 m`** (khoảng hở mép **`45.08 m`**).
   - **② Tượng & Hồ phun nước Merlion (`BP7` — [`1795453365678_image.png`](img/ref/1795453365678_image.png)) tại `(+102.0, 0.0, -45.0)`:** Lọc bỏ chuỗi dọc cộng sai `2.0+1.0+9.0+6.0 = 18.0 m ≠ 10.0 m` để chuẩn hóa về **hồ tròn đồng tâm `ĐK 28.0 m` (`R = 14.0 m`), tổng cao `10.0 m`** (thành hồ đá 2 tầng, bệ sóng biển `y = 2.0..3.2 m`, tượng Ngư Sư Merlion thân cá uốn chữ S + đầu bờm sư tử `y = 3.2..10.0 m` phun chùm tia nước động parabol cùng 12 vòi phun đồng tâm); cách tim Monorail **`114.30 m`** (khoảng hở mép **`100.30 m`**).
   - **③ Thuyền Rồng Hoàng Gia (`BP8` — [`1795453369127_image.png`](img/ref/1795453369127_image.png)) nổi trên mặt hồ tại `(+18.0, 0.0, -152.0)`:** Lọc bỏ chuỗi dài cộng sai `7+8+9+8+7 = 39.0 m ≠ 22.0 m` và chiều ngang `7+7 = 14.0 m ≠ 7.0 m` để chuẩn hóa về **`22.0 (L) × 7.0 (W) × 11.4 (H) m`** (thân thuyền chạm vẩy rồng, đầu rồng có sừng ở mũi, đuôi rồng ngọn lửa ở lái, lầu đình 2 tầng mái cong lưu ly vàng-đỏ ở giữa thuyền); nằm hoàn toàn trong lòng hồ (`inLake = 1`), cách cầu Monorail vượt hồ **`64.20 m`** (khoảng hở **`53.20 m`**) và cách Thủy đình `> 48 m`.
   - **④ Tàu lượn Thuyền Rồng Đu Đưa "The Flying Kirins" (`BP9` — [`1795453378911_image.png`](img/ref/1795453378911_image.png)) tại `(+66.0, 0.0, -118.0)`:** Lọc bỏ chuỗi ngang `5+5+18+5+5 = 38.0 m ≠ 18.0 m` và mốc cao độ vô lý `+20.0 m` ở sàn thấp để chuẩn hóa về **`18.0 (L) × 12.0 (W) × 15.0 (H) m`** (bến cầu cảng gỗ trên cọc bê tông ven bờ Đông hồ, 4 trụ thép chữ A chụm lên ổ trục cơ khí `y = 14.5 m`, thuyền chạm hai đầu Kỳ Lân/Rồng vàng đối xứng lắc lư điều hòa `±58°`). **Giữ nguyên 100% [`PirateShip.h`](../src/rides/PirateShip.h) ở khu Tây và tách biệt hoàn toàn theo Quyết định 3**; cách tim Monorail **`49.98 m`** (khoảng hở **`40.48 m`**).
   - **⑤ Sân khấu nổi & Khán đài bán nguyệt Symphony of River (`BP13` — [`1795453392187_image.png`](img/ref/1795453392187_image.png)) tại `(+84.0, 0.0, -238.0)`:** Lọc bỏ chuỗi ngang `4.8+4.8+42+4.9+4 = 60.5 m` và `1.8+3.8+5.0+2.5+1.8 = 14.9 m` để chuẩn hóa về **`42.0 (W) × 20.0 (D) × 12.0 (H) m`** (sàn biểu diễn bán nguyệt, khán đài 6 bậc hình quạt, cổng chào vòm bán nguyệt giàn thép cao `12.05 m` gắn đèn LED phát sáng ban đêm và 2 vọng lầu bát giác 2 tầng mái cong ở hai đầu cánh); cách tim Monorail **`32.26 m`** (khoảng hở mép **`14.26 m`**).
   - **⑥ Nhà hát Âu Cơ — Múa rối nước Việt (`BP14` — [`1795453395295_image.png`](img/ref/1795453395295_image.png)) tại `(+135.0, 0.0, +12.0)`:** Lọc bỏ chuỗi bên `8+5+18+2.1 = 33.1 m ≠ 18.0 m` và mốc cao độ lặp hai lần `+12.00 m` / `+4.80 m` để chuẩn hóa về **`24.0 (W) × 18.0 (D) × 12.0 (H) m`** (khối trung tâm + 2 cánh bậc thang, hệ lam gỗ dọc mặt tiền, cổng vòm bán nguyệt có hồ thủy đình rối nước và 3 tầng mái ngói đỏ đầu đao cong); cách tim Monorail **`49.00 m`** (khoảng hở **`36.00 m`**).
   - **⑦ Đảo Khủng Long — Dino Island (`BP16` — [`1795453400510_image.png`](img/ref/1795453362382_image.png)) tại `(+268.0, 0.0, -72.0)`:** Lọc bỏ chuỗi dọc cộng `10+10+10 = 30.0 m ≠ 10.0 m` và mặt đứng bên ghi `22.0 m` vào trục chiều cao để chuẩn hóa về **`28.0 (W) × 22.0 (D) × 10.0 (H) m`** (cổng chào gỗ-đá gắn đuốc lửa, núi lửa giả tạo cao `10.0 m` có hồ dung nham phát sáng ban đêm, 3 mô hình khủng long 3D gồm cổ dài Brachiosaurus cao `8.95 m`, bạo chúa T-Rex cao `6.15 m`, Triceratops 3 sừng và 4 cây cọ tiền sử); cách tim Monorail **`52.84 m`** (khoảng hở **`37.84 m`**).
   - **⑧ Vườn Trúc Nhật Bản — Bamboo Garden (`BP17` — [`1795453403998_image.png`](img/ref/1795453403998_image.png)) tại `(+226.0, 0.0, -35.0)`:** Lọc bỏ chuỗi ngang `2+3+1+8+1+3+1 = 19.0 m ≠ 24.0 m` để chuẩn hóa về **`24.0 (W) × 20.0 (D) × 9.0 (H) m`** (cổng thần đạo Torii đỏ cao `6.8 m`, dòng suối cá Koi, cầu gỗ cong Taiko-bashi sơn đỏ, nhà trà đạo Nhật Bản mái cong cao `6.5 m`, 2 đèn đá Tōrō và 4 rặng trúc xanh cao `9.0 m` ở 4 góc vườn); cách tim Monorail **`63.30 m`** (khoảng hở **`50.30 m`**).

---

## 11. Danh mục nguồn bổ sung

1. [Sun World — Vòng quay Mặt Trời Sun Wheel](https://sunworld.vn/vi/check-in/vong-quay-sun-wheel-tai-asia-park-niem-tu-hao-cua-da-nang-6125)
2. [VnExpress — Ngắm vòng quay khổng lồ tại Đà Nẵng](https://vnexpress.net/ngam-vong-quay-khong-lo-tai-da-nang-3017385.html)
3. [Wikipedia — Sun Wheel (Da Nang)](https://en.wikipedia.org/wiki/Sun_Wheel_(Da_Nang))
4. [RCDB #14194 — Queen Cobra (tọa độ & thông số)](https://rcdb.com/14194.htm) & [RCDB #12589 — Sun World Danang Downtown](https://rcdb.com/12589.htm)
5. [Thành Niên Mới — Tàu lượn Queen Cobra](https://thanhnienmoi.com/blogs/tau-luon-queen-cobra-tro-choi-mao-hiem-hang-dau-o-da-nang-574.html) & [Viện Kỷ Lục Việt Nam — Đề xuất kỷ lục Queen Cobra](https://vienkyluc.vn/tin-tuc/tin-tuc/vietmaster-de-xuat-ky-luc-p-72-queen-cobra-cong-vien-asia-park-da-nang-tau-luon-dang-treo-lon-nhat-viet-nam)
6. Wikimedia Commons (ảnh mặt đất gắn EXIF GPS & hướng la bàn): [49356303688 (Cổng Ngọ Môn)](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49356303688).jpg), [49370578306 (Biển ga Monorail)](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49370578306).jpg), [49374882021 (Biển chỉ khu Tây)](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49374882021).jpg), [49358003906 (Góc nhìn Sky Tower)](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49358003906).jpg), [49374702231 (Tàu Monorail qua ga Nepal)](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49374702231).jpg), [49373970611](https://commons.wikimedia.org/wiki/File:Sun_World_Asia_Park,_Da_Nang_(49373970611).jpg)
7. OpenStreetMap / Nominatim (`way 667422708`, `1331565206`, `1392384112`, `1392384120`, `1392384128`, `667422707`, `302765736`, `1280089217`, `302765742`, `643263435`, `node 11882946634`, `relation 19756034`)
8. Esri World Imagery & Wayback Historical Releases (`release 4756` 2019-12-12, `release 45134` 2022-12-14, current `2026-03-01`)

---

## Phụ lục A. Đỉnh polygon OSM `way 667422708` (Da Nang Downtown, tham chiếu)

*(Giữ nguyên dữ liệu gốc 64 đỉnh từ OSM qua Nominatim `polygon_geojson=1`; lưu ý phần tọa độ local cũ trong bảng dưới dùng gốc `16.040627, 108.228132` nên lệch ~25 m Đông-Tây so với tâm nadir mới `16.040500, 108.228380`).*

| # | lat | lon | X_local (m) | Z_local (m) | X_engine (m) | Z_engine (m) |
|:-:|:--|:--|--:|--:|--:|--:|
| 0 | 16.0438823 | 108.2247149 | -365.6 | -360.2 | -365.6 | -398.2 |
| 1 | 16.0423949 | 108.2248635 | -349.7 | -195.6 | -349.7 | -233.6 |
| 2 | 16.0420964 | 108.2248982 | -346.0 | -162.6 | -346.0 | -200.6 |
| 3 | 16.0421526 | 108.2254379 | -288.3 | -168.8 | -288.3 | -206.8 |
| 4 | 16.0417637 | 108.2254834 | -283.4 | -125.8 | -283.4 | -163.8 |
| 5 | 16.0416956 | 108.2249568 | -339.7 | -118.2 | -339.7 | -156.2 |
| 6 | 16.0398685 | 108.2251697 | -317.0 | 83.9 | -317.0 | 45.9 |
| 7 | 16.0398990 | 108.2257868 | -250.9 | 80.6 | -250.9 | 42.6 |
| 8 | 16.0382865 | 108.2259899 | -229.2 | 259.0 | -229.2 | 221.0 |
| 9 | 16.0384700 | 108.2276898 | -47.3 | 238.7 | -47.3 | 200.7 |
| 10 | 16.0392597 | 108.2276295 | -53.7 | 151.3 | -53.7 | 113.3 |
| 11 | 16.0393914 | 108.2289392 | 86.4 | 136.7 | 86.4 | 98.7 |
| 12 | 16.0386521 | 108.2290072 | 93.7 | 218.5 | 93.7 | 180.5 |
| 13 | 16.0387771 | 108.2297098 | 168.9 | 204.7 | 168.9 | 166.7 |
| 14 | 16.0404312 | 108.2294459 | 140.7 | 21.7 | 140.7 | -16.3 |
| 15 | 16.0404978 | 108.2292668 | 121.5 | 14.3 | 121.5 | -23.7 |
| 16 | 16.0410237 | 108.2290693 | 100.4 | -43.9 | 100.4 | -81.9 |
| 17 | 16.0410297 | 108.2289937 | 92.3 | -44.6 | 92.3 | -82.6 |
| 18 | 16.0410103 | 108.2288509 | 77.0 | -42.4 | 77.0 | -80.4 |
| 19 | 16.0410030 | 108.2288062 | 72.2 | -41.6 | 72.2 | -79.6 |
| 20 | 16.0409639 | 108.2284156 | 30.4 | -37.3 | 30.4 | -75.3 |
| 21 | 16.0409598 | 108.2283719 | 25.7 | -36.8 | 25.7 | -74.8 |
| 22 | 16.0409176 | 108.2279817 | -16.0 | -32.2 | -16.0 | -70.2 |
| 23 | 16.0409120 | 108.2279378 | -20.7 | -31.5 | -20.7 | -69.5 |
| 24 | 16.0409014 | 108.2278134 | -34.0 | -30.4 | -34.0 | -68.4 |
| 25 | 16.0411198 | 108.2277836 | -37.2 | -54.5 | -37.2 | -92.5 |
| 26 | 16.0413238 | 108.2277520 | -40.6 | -77.1 | -40.6 | -115.1 |
| 27 | 16.0414956 | 108.2277295 | -43.0 | -96.1 | -43.0 | -134.1 |
| 28 | 16.0415615 | 108.2277244 | -43.6 | -103.4 | -43.6 | -141.4 |
| 29 | 16.0415817 | 108.2277228 | -43.7 | -105.6 | -43.7 | -143.6 |
| 30 | 16.0416335 | 108.2276568 | -50.8 | -111.4 | -50.8 | -149.4 |
| 31 | 16.0417308 | 108.2276906 | -47.2 | -122.1 | -47.2 | -160.1 |
| 32 | 16.0419659 | 108.2276768 | -48.7 | -148.2 | -48.7 | -186.2 |
| 33 | 16.0421309 | 108.2276364 | -53.0 | -166.4 | -53.0 | -204.4 |
| 34 | 16.0422502 | 108.2275984 | -57.1 | -179.6 | -57.1 | -217.6 |
| 35 | 16.0423556 | 108.2275509 | -62.1 | -191.3 | -62.1 | -229.3 |
| 36 | 16.0425023 | 108.2274515 | -72.8 | -207.5 | -72.8 | -245.5 |
| 37 | 16.0425612 | 108.2273960 | -78.7 | -214.0 | -78.7 | -252.0 |
| 38 | 16.0426140 | 108.2273341 | -85.3 | -219.9 | -85.3 | -257.9 |
| 39 | 16.0427101 | 108.2271667 | -103.2 | -230.5 | -103.2 | -268.5 |
| 40 | 16.0427596 | 108.2270936 | -111.1 | -236.0 | -111.1 | -274.0 |
| 41 | 16.0428290 | 108.2270250 | -118.4 | -243.7 | -118.4 | -281.7 |
| 42 | 16.0429104 | 108.2269653 | -124.8 | -252.7 | -124.8 | -290.7 |
| 43 | 16.0431166 | 108.2266812 | -155.2 | -275.5 | -155.2 | -313.5 |
| 44 | 16.0431613 | 108.2266329 | -160.4 | -280.4 | -160.4 | -318.4 |
| 45 | 16.0432184 | 108.2266041 | -163.5 | -286.8 | -163.5 | -324.8 |
| 46 | 16.0432685 | 108.2265926 | -164.7 | -292.3 | -164.7 | -330.3 |
| 47 | 16.0433216 | 108.2265877 | -165.2 | -298.2 | -165.2 | -336.2 |
| 48 | 16.0434927 | 108.2265682 | -167.3 | -317.1 | -167.3 | -355.1 |
| 49 | 16.0435461 | 108.2265390 | -170.4 | -323.0 | -170.4 | -361.0 |
| 50 | 16.0435486 | 108.2264114 | -184.1 | -323.3 | -184.1 | -361.3 |
| 51 | 16.0435996 | 108.2263506 | -190.6 | -328.9 | -190.6 | -366.9 |
| 52 | 16.0436010 | 108.2262534 | -201.0 | -329.1 | -201.0 | -367.1 |
| 53 | 16.0436222 | 108.2262039 | -206.3 | -331.4 | -206.3 | -369.4 |
| 54 | 16.0436588 | 108.2261667 | -210.3 | -335.5 | -210.3 | -373.5 |
| 55 | 16.0437091 | 108.2261515 | -211.9 | -341.1 | -211.9 | -379.1 |
| 56 | 16.0439610 | 108.2261138 | -215.9 | -368.9 | -215.9 | -406.9 |
| 57 | 16.0440668 | 108.2260135 | -226.7 | -380.6 | -226.7 | -418.6 |
| 58 | 16.0440831 | 108.2259905 | -229.1 | -382.4 | -229.1 | -420.4 |
| 59 | 16.0441573 | 108.2258853 | -240.4 | -390.7 | -240.4 | -428.7 |
| 60 | 16.0440072 | 108.2258051 | -249.0 | -374.0 | -249.0 | -412.0 |
| 61 | 16.0438715 | 108.2257369 | -256.3 | -359.0 | -256.3 | -397.0 |
| 62 | 16.0438231 | 108.2256739 | -263.0 | -353.7 | -263.0 | -391.7 |
| 63 | 16.0439766 | 108.2254730 | -284.5 | -370.7 | -284.5 | -408.7 |

(Đỉnh 64 trùng đỉnh 0 để khép kín polygon, đã lược bỏ. Nguồn: OSM way 667422708 qua Nominatim `polygon_geojson=1`.)
