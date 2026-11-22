# Tài liệu đối chiếu thông số & bố cục — Asia Park (Đà Nẵng)

> Tài liệu phục vụ chuẩn hóa bố cục và tỷ lệ dự án **Mô phỏng 3D Công viên Châu Á (Asia Park / Da Nang Downtown)**.
> **Mốc hiện trạng mô hình:** Khoảng **năm 2019** (`SNAPSHOT_YEAR = 2019` — thời điểm công viên còn nguyên Tháp Đồng Hồ tại quảng trường trung tâm, đầy đủ các trò chơi phía Tây và khu vực nửa phía Đông ranh giới là nền cỏ đơn giản nơi tuyến Monorail chạy qua).
> Cập nhật: 2026-11-22 (đã cập nhật 5 quyết định duyệt kế hoạch 3 đợt & hoàn thành Đợt 1).

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

## 10. Kế hoạch chỉnh sửa code theo 3 đợt (Bước 3 — Chờ duyệt, CHƯA SỬA CODE)

> **Nguyên tắc bắt buộc:** Giữ nguyên cấu trúc hình học, lưới đa giác (`PolygonMesh` / `ParametricShapes`) và logic animation đang chạy tốt của mọi trò chơi. Tách toàn bộ thông số vị trí, tỷ lệ, hướng xoay và ranh giới ra thành struct/namespace hằng số riêng (ví dụ `src/core/ParkLayout.h`) để dễ chỉnh sửa và đối chiếu báo cáo.

### Đợt 1 — Gom hằng số, chuẩn hóa Hồ nước, Mặt đất bối cảnh & Tốc độ Sun Wheel
- **Tạo `src/core/ParkLayout.h`** (chỉ chứa `constexpr` tọa độ, góc xoay, scale, đa giác hồ thật, đa giác ranh giới công viên ~36.1 ha, vị trí các tuyến đường bao quanh & sông Hàn, và hệ số quy đổi GPS để tiện giải thích trong code).
- **`src/rides/SunWheel.h` & `src/main.cpp`**:
  - Tham chiếu tâm Sun Wheel tới `ParkLayout::SUN_WHEEL_CENTER` `(0, 65, −38)`.
  - Cập nhật `rotationSpeed` theo phương án bạn chọn ở mục 9 (`0.4f` deg/s nếu chuẩn 15 phút/vòng).
- **`src/main.cpp` (`drawParkGroundAndLake`, `drawLakesidePavilionMesh`, `displayCallback`)**:
  - Sửa vùng mặt hồ từ hình chữ nhật `[-58..58, -62..16]` (đè qua chân Sun Wheel) sang **bờ hồ thực tế phía Bắc Sun Wheel** (`Z_eng <= -74 m`), tách riêng phần nền cỏ để không đè Z-fighting với mặt nước.
  - Bổ sung dải **Sông Hàn** ở phía Đông (`X_eng >= +430 m`), **ranh giới công viên / vỉa hè bao quanh** và **3 trục đường chính** (Phan Đăng Lưu ở Nam, đường nội bộ/Bạch Đằng ở Tây, Đỗ Pháp Thuận / Thuận Yến ở Đông), kèm **trục lối đi lát gạch** nối từ Cổng chính tới Quảng trường Sun Wheel và sang khu trò chơi phía Tây.
  - Dời `drawLakesidePavilionMesh()` từ `(0, 0, 28)` sang vị trí thủy đình thật phía Bắc Sun Wheel `(−5.4, 0, −107.7)` kèm cầu nối ra bờ Nam của hồ; dời 10 trụ đèn lồng dọc bờ kè quảng trường/hồ mới để không đứng dưới nước.

### Đợt 2 — Mở rộng Mặt phẳng nền, Camera, Drone & Minimap 2D
- **`src/core/Camera.h`**:
  - Mở rộng `gluPerspective` `zNear = 1.0f`, `zFar = 2200.0f` để nhìn xuyên suốt công viên dài ~750 m mà không mất nền/công trình ở xa.
  - Nới `orbit` `distanceR` tối đa từ `380.0f` lên `850.0f`; nới `panTarget` theo biên `ParkLayout` (`X ∈ [−420, 420]`, `Z ∈ [−480, 250]`, `Y ∈ [2, 160]`).
  - Cập nhật camera mặc định lúc mở app (`distanceR ~ 380–440 m`, `target` hướng vào vùng trung tâm Sun Wheel & hồ) để thấy toàn cảnh hợp lý.
  - Cập nhật các góc chiếu trực giao `ORTHO_TOP / FRONT / SIDE` (`orthoSize ~ 380 m`, `glOrtho(..., -1200, 1200)`, mắt cách tâm ~`550–650 m`) để không bị cắt đầu/đuôi công viên.
  - Nới vùng bay Drone (`X ∈ [−550, 550]`, `Z ∈ [−560, 360]`, `Y ∈ [1.2, 360]`) và nâng dải tốc độ (`12 / 40 / 100 m/s`, `adjustDroneSpeed` tối đa `220 m/s`).
- **`src/ui/Minimap2D.h` & `src/main.cpp`**:
  - Mở rộng khung WCS của radar Minimap (từ `[-135, 135]` sang vuông bao trùm công viên, ví dụ `wcsMinX = -460, wcsMaxX = 460, wcsMinZ = -520, wcsMaxZ = 400` giữ tỷ lệ 1:1 tránh méo hình).
  - Vẽ đúng đa giác hồ mới và đường bao công viên trên Minimap; nới kẹp `radarEye` trong `main.cpp` tương ứng.

### Đợt 3 — Cập nhật vị trí, hướng xoay & scale các công trình còn lại
- **`src/rides/SkyTower.h`**:
  - Chỉ cập nhật `position = ParkLayout::SKY_TOWER_POS` `(−260.0, 0, −54.6)` (chiều cao 47 m đã chuẩn 1:1, giữ nguyên hình học & chu kỳ 5 pha rơi tự do).
- **`src/rides/NgoMonGate.h`** *(theo lựa chọn của bạn ở mục 9)*:
  - Cập nhật `position` về vị trí Cổng chính `(+16.1, 0, +139.1)` và điều chỉnh `glScalef` đồng nhất (~`0.76` theo bề rộng ~35 m thực tế, hoặc giữ 1.0 tùy bạn chốt) + hướng quay `0°` (đang quay mặt về Nam `+Z` ra Đường Phan Đăng Lưu — khớp thực tế!).
- **`src/curves/RollerCoaster.h` (`AsiaParkCurvesSystem::init`)**:
  - **Monorail**: thay `monorailWaypoints` bằng **20 điểm thực tế** đã đo dọc dầm bê tông khép kín trên ảnh vệ tinh (`~1.87 km`), giữ nguyên chiều cao `12–14.5 m`, cấu hình dầm `isMonorailBeam = true`, hình học toa và animation; cập nhật vị trí 2 nhà ga về **Ga Nam** `(−24.6, 0, +128.0)` và **Ga Nepal** `(−217.2, 0, −220.6)` (xoay khớp tiếp tuyến tại ga).
  - **Queen Cobra**: **KHÔNG scale**, giữ nguyên `queenCobraWaypoints` gốc + `track` + `CoasterTrain`; chỉ áp dụng phép tịnh tiến + xoay cho toàn bộ cụm (cả đường ray + nhà ga) theo phương án **(A)** hoặc **(B)** bạn chọn ở mục 9.
- **`src/rides/Carousel.h`**:
  - Giữ nguyên cho tới khi bạn xác nhận câu hỏi #1 ở mục 9 (`16.03923, 108.22689` hay giữ nguyên/cung cấp khác).

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
