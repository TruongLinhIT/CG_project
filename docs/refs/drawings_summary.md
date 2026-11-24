# Tổng hợp & Đối chiếu Bộ Bản vẽ Tham khảo AI (`docs/img/ref/` → `docs/refs/`)

> **QUY TẮC NGUỒN (BẮT BUỘC):**
> 1. Toàn bộ 18 ảnh PNG (`1024×572 px`) hiện có trong [`docs/img/ref/`](../img/ref/) (sơ đồ tổng mặt bằng + 17 bản vẽ kỹ thuật) đều do **một AI khác (Gemini) tạo ra**, **KHÔNG phải tài liệu kỹ thuật chính thức của Sun World / Asia Park**.
> 2. **Thứ tự ưu tiên khi có xung đột:**
>    - **(1) Ưu tiên cao nhất:** Dữ liệu đã xác minh trong [`docs/asia_park_specs.md`](../asia_park_specs.md) (ảnh vệ tinh Esri Wayback 2019/2022, RCDB, Wikimedia Commons EXIF GPS, nguồn báo/web chính thức) **luôn thắng**.
>    - **(2) Tham khảo bản vẽ:** Các bản vẽ chi tiết chỉ dùng để tham khảo **HÌNH DÁNG** và **tỷ lệ tương đối** sau khi đã kiểm tra & loại bỏ các số đo AI tự cộng sai.
>    - **(3) Sơ đồ tổng quát `01` (`1795453337010_image.png`):** Sai tỷ lệ nặng và rối loạn vị trí/la bàn, **chỉ dùng để tham khảo công trình nào nằm gần khu vực nào**, **tuyệt đối KHÔNG suy ra tọa độ hay khoảng cách từ nó**.
> 3. Mọi thông số hình dáng và số đo lấy từ bộ ảnh này đều gắn nhãn **`[Bản vẽ minh họa AI, tham khảo hình dạng]`**.

> **MINH BẠCH NGUỒN ẢNH TRÊN ĐĨA (18/19 ẢNH):**
> - Thư mục `docs/refs/` trước đây chưa tồn tại và **không có các file đặt tên `01_*` đến `19_*`** trên đĩa.
> - Tài liệu này tổng hợp trực tiếp từ **18 file ảnh hiện có trong `docs/img/ref/`** (`1795453337010_image.png` đến `1795453403998_image.png`, commit `b1ed25e`).
> - **Ghi chú thiếu ảnh thứ 19 (`19_*`):** Bộ ảnh trên repo hiện có **18 ảnh**; ảnh thứ 19 hiện **chưa có trên đĩa** (nếu đối chiếu với 4 tàu lượn nhỏ Đợt 5A thì `Highway Boat` chưa có bản vẽ riêng, hoặc bản vẽ #19 chưa được tải vào repo).

---

## 1. Phân tích chi tiết từng bản vẽ (Kiểm tra kích thước, hình dáng & phép cộng mâu thuẫn)

### `#01` — Sơ đồ tổng mặt bằng toàn công viên (`1795453337010_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]` *(sơ đồ định tính lân cận, không dùng tỷ lệ/tọa độ)*.
- **Thông số / văn bản đọc được:**
  - Tiêu đề: `ASIA PARK - ĐÀ NẴNG DOWNTOWN / ARCHITECTURAL MASTEN PLAN & SITE LAYOUT / ĐÀ NẴNG, VIỆT NAM`.
  - Thanh thước: `0..200m` ở góc Đông-Bắc nhưng **không ăn khớp với mặt bằng**; hoa gió chỉ `N` hướng lên trên.
  - Danh mục nhãn trên bản vẽ: `Đường 2/9`, `Đường Bạch Đằng`, `BÃI ĐỖ XE`, `CỔNG THÀNH (MAIN GATE)`, `Đường Phan Đăng Lưu`, `THÁP ĐỒNG HỒ (CLOCK TOWER)`, `QUẢNG TRƯỜNG SUN WHEEL`, `VÒNG QUAY MẶT TRỜI (SUN WHEEL)`, `ĐỀN ANGKOR WAT`, `THÁP RƠI TỰ DO (GOLDEN SKY TOWER)`, `TÀU LƯỢN QUEEN COBRA`, `SÂN KHẤU NỔI (WATERFRONT STAGE / SYMPHONY OF RIVER)`, `NHÀ HÀNG PANDA (PANOA RESTAURANT)`, `TÀU LƯỢN THE FLYING KIRINS`, `VƯỜN TRÚC (BAMBOO GARDEM)`, `THÁC NƯỚC PARADISE FALL`, `TÀU LƯỢN GARUDA VALLEY (OANUDA VALLEY)`, `PORT OF SKY TREASURE`, `ĐU QUAY FESTIVAL CAROUSEL`, `KIDDIE RIDES`.
- **Mô tả hình dáng & bố cục:** Vẽ dải vòng Monorail khép kín chạy sát Sông Hàn ở phía trên và bên phải, Cổng Thành ở cạnh dưới, Sun Wheel lệch hẳn sang cạnh phải, các cụm trò chơi phân tán thành các ô lối đi uốn cong.
- **Số đo/ký hiệu mâu thuẫn & không rõ ràng:**
  - Lỗi chính tả AI hàng loạt: `MASTEN PLAN`, `LEASNE`, `PANOA RESTAURANT`, `OANUDA VALLEY`, `PARADIBE FALL` / `PARADISE PALL`, `FESTWAL CAROUSEL`, `GMIÉAEY`.
  - **Đánh số trùng lặp & thiếu:** Trong bảng `LEASNE` và mặt bằng, các số `(2)`, `(3)`, `(8)`, `(12)` xuất hiện 2 lần ở hai công trình khác nhau; số `(10)` xuất hiện 3 lần (`Port of Sky Treasure`, `Paradise Fall`, `Garuda Valley`, khu gần Sun Wheel); `Cổng Thành` không số còn `(1)` đặt bên lối Tây; **`VƯỜN TRÚC (BAMBOO GARDEM)` bị đặt 2 lần** ở hai góc khác nhau (góc Tây-Nam ngoài khu và chính giữa công viên).
  - **Sai định hướng so với địa lý thực tế (OSM/vệ tinh):** Đặt `Sông Hàn` bọc cả cạnh Bắc lẫn cạnh Đông, đặt `Sun Wheel` ở cạnh Đông thay vì nằm thẳng trục Bắc–Nam của `Cổng Thành` → `Tháp Đồng Hồ` → `Sun Wheel`.
- **Đối chiếu dự án (cuối Đợt 7, `b1ed25e`):** Trong [`ParkLayout.h`](../src/core/ParkLayout.h), tọa độ & ranh giới ~36.1 ha đã được số hóa trực tiếp từ Esri Wayback 2019 + OSM (độ tin cậy cao hơn hẳn sơ đồ 01).

---

### `#02` — Vòng quay Mặt Trời (`1795453339526_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Đường kính vành quay: **`Ø100.0 m`**; tổng cao ghi **`115.0 m`**.
  - Kích thước móng mặt bằng: dài tổng dưới ghi `11000` (mm = `11.0 m`?), các đoạn `1000 + 3000 + ... + 4500 + ... + 3200 + 1000`, rộng `2500 + 2000`.
  - Chi tiết cabin bên phải: chiều cao cabin `400` / `4.00` (mờ).
- **Mô tả hình dáng:** Bánh xe 2 vành đồng tâm có hệ thanh giằng không gian và nan hoa hướng tâm từ trục trung tâm, đặt trên **2 cặp chân tháp chữ A** giàn thép chéo X (4 tầng thanh giằng ngang) xuống 4 bệ móng bê tông (`CONCRETE ANCHOR`); phía cạnh chân tháp có cầu thang dẫn lên **nhà ga đón khách** mái bằng ngang (`NHÀ GA ĐÓN KHÁCH`).
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mốc cao độ bên phải mặt đứng chính ghi **`115.0 m` bị lặp 2 lần** ở đỉnh và ở vành trên; dưới đó là `100.0 m`, `88.9 m`, `86.8 m`, `+3.05 m`, và bệ chân dưới cùng ghi **`41.0 m`**.
  - Chuỗi kích thước đứng bên trái cộng lộn xộn (`15.0`? + `95.0`? + `99.0`? + `8.5` = không ra `115.0 m`). Mặt đứng bên ghi chân trái `-20.5 m` còn nhà ga bên phải `-0.55 m`.
  - Chữ kỹ thuật bị lỗi AI: `TECHMICAL DRAWING`, `TRUC KHACH`, `BASEPPELATES`, `TOEX`. Số cabin vẽ trên vòng đếm khoảng `~32–36` chiếc (ít hơn thực tế `64` cabin).
- **Đối chiếu dự án (`b1ed25e`):** **ĐÃ CÓ** trong [`src/rides/SunWheel.h`](../src/rides/SunWheel.h), vị trí [`SUN_WHEEL_CENTER (0.0, 65.0, -38.0)`](../src/core/ParkLayout.h#L53) (`Ø100 m`, tổng cao `115 m`, `64` cabin, `32` nan hoa, `0.4°/s` — khớp các nguồn web đã xác minh). Phần nhà ga đón khách đã dựng cụm Sân khấu & Nhà ga chân Sun Wheel tại [`SUN_WHEEL_STAGE_POS`](../src/core/ParkLayout.h#L386) trong [`ParkArchitecture.h`](../src/rides/ParkArchitecture.h). **Giữ nguyên vị trí & không cần sửa `SunWheel.h`.**

---

### `#03` — Cổng Thành / Cổng Ngọ Môn (`1795453342175_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Chiều rộng mặt đứng chính: ghi **`25.000`** / khung tên `25.000`? Chuỗi ngang dưới: `6.50 + 3.50 + 6.00 + 3.50 + 6.50 = 26.00 m` (nhưng dòng tổng bên dưới ghi mờ `19.50m` hoặc `29.50m`). Mặt bằng tầng 1 ghi phía trên `5.50 + 3.50 + 3.50 + 4.00 + 8.50 = 25.00 m` (nhưng ngoặc tổng của 3 nhịp giữa ghi `14.50 m` trong khi `3.5+3.5+4.0 = 11.00 m`).
  - Chiều sâu mặt bên: `2.00 + 3.20 + 6.20 = 11.40 m`.
  - Mốc cao độ: `±0.00`, `+5.40` (trên đài thành), **`+12.60`** (đỉnh mái).
- **Mô tả hình dáng:** Khối đài thành hình chữ nhật bằng đá xây gạch thẻ có **3 cửa vòm cuốn** ở tầng trệt, lan can đá trắng bao quanh mặt sân thượng tầng 1 (`+5.40 m`); bên trên là lầu trung tâm **2 tầng mái ngói cong** kèm 2 lầu chòi nhỏ hai bên ở tầng 1 (chưa thể hiện Lầu Ngũ Phụng đủ 5 khối độc lập hay 2 cánh tường thành dài có cổng vòm phụ như thực tế); bên trong đài thành có cầu thang đi lên tầng trên (thấy ở Mặt cắt A-A).
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Hai mốc **`+5.40`** đặt lặp cho cả lan can lẫn chân tường tầng trên; hai mốc **`+12.60`** đặt ở hai tầng mái khác nhau và Mặt cắt A-A gán nhầm `+12.60` xuống mái tầng dưới.
  - Chuỗi cao bên trái mặt đứng chính (`5.50 + 6.50 + 6.50 + 6.50 = 25.0 m`) mâu thuẫn với đỉnh `+12.60 m` và chữ mờ `19.50m`. Một số chữ mặt bằng/mặt cắt bị nhòe (`Raflern`, `Màng vịt`).
- **Đối chiếu dự án (`b1ed25e`):** **ĐÃ CÓ VÀ CHI TIẾT HƠN BẢN VẼ AI** trong [`src/rides/NgoMonGate.h`](../src/rides/NgoMonGate.h) tại [`NGO_MON_GATE_POS (+16.1, 0.0, +139.1)`](../src/core/ParkLayout.h#L66) (`gateScale = 0.76`, rộng sau scale `35.0 m`, 3 vòm cuốn, tiền sảnh 4 cột, Lầu Ngũ Phụng 5 lầu mái cong, 2 cánh tường thành răng cưa dài 58 m local có 2 cổng vòm phụ và 2 hồ sen). Khớp ảnh mặt đất Wikimedia Commons `49356303688` và Wayback 2019 → **giữ nguyên hoàn toàn vị trí và hình học Cổng Ngọ Môn**.

---

### `#04` — Tháp Đồng Hồ Trung Tâm (`1795453346086_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Mặt đứng chính: dòng tổng ngang dưới ghi **`28.0 m`**; mặt bằng ghi diện tích **`550 m²`** (~`23.5 × 23.5 m`).
  - Chiều cao ghi trên mặt đứng chính (cột ngoài cùng bên trái): **`29.0 m`** *(gần khớp bóng tháp & ước lượng ~29 m trong specs mục 5.5)*.
- **Mô tả hình dáng:** Khối tháp gồm:
  1. **Phần đế đá vuông giật cấp rộng** có tường xây đá hộc, 4 cổng vòm giữa 4 mặt, **4 cầu thang trục** ở Đông/Tây/Nam/Bắc dẫn lên sân thượng bệ, lan can đá trắng bao quanh chu vi sân bệ.
  2. **Tầng chuyển tiếp (tầng 1 trên bệ):** Khối giữa cao vươn lên và 2 mái phụ hai bên đối xứng kiểu đầu hồi/mái cong nhỏ.
  3. **Thân tháp hình BÁT GIÁC** gồm **5 tầng mái ngói cong thu nhỏ dần theo chiều cao** (`5` lớp mái rõ rệt trên mặt đứng & mặt cắt A-A, lõi cầu thang xoắn bên trong mặt cắt).
  4. **4 mặt đồng hồ tròn** gắn tại **tầng thứ 4** (tầng áp đỉnh, giữa lớp mái 3 và lớp mái 4); đỉnh trên cùng là **mái bát giác tầng 5 + chóp nhọn nhiều tầng**.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - **Mâu thuẫn mặt bằng:** Dòng tổng mặt đứng ghi `28.0 m` nhưng 3 đoạn con ở trên cộng lại chỉ ra `5.0 + 5.0 + 5.0 = 15.0 m`; mặt bằng tầng trệt ghi `550 m²` nhưng hai cạnh lại ghi `12.5 m` (ngang) và `12.0 m / 13.0 m` (dọc — `12.5 × 12.5 = 156 m²`, sai gấp 3.5 lần so với `550 m²`!).
  - **Mâu thuẫn chiều cao:** Mặt đứng chính ghi tổng `29.0 m`, nhưng chuỗi dọc cộng `5.0 + 8.0 + 12.0 + 8.0 + 8.0 + 5.0 = 46.0 m`; mặt đứng bên (`FROT ELEVATION`) ghi tổng `20.0 m` (chuỗi con cộng `30.0 m`); mặt cắt A-A ghi `19.0 m` ở dưới và `28.0 m` ở trên; cột mốc cao độ bên phải viết lộn xộn `0.00 → -6.00 → 0.00 → 0.00 → 1.00 → 5.0 → 0.0 m`.
  - Ngày bản vẽ AI ghi `21/18/2018` (tháng 18), chữ `OROUND FLOOR PLAN`, `CONCTE FLOOR`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Vị trí đã xác minh trên Wayback 2019: [`CLOCK_TOWER_POS (+10.7, 0.0, +61.6)`](../src/core/ParkLayout.h#L69), khối đế vệ tinh **`~25 × 24 m`**, cao **`~29 m`** ([`asia_park_specs.md` Mục 5.5](../asia_park_specs.md#L129)).
  - **Khác biệt trong [`NgoMonGate.h`](../src/rides/NgoMonGate.h#L499-L541):** Hiện tại tháp chỉ gồm **3 khối hộp vuông** đơn giản (`8.4 → 7.2 → 6.4 m`) + 1 mái cong trên đỉnh, **chưa có bệ đá giật cấp rộng (~25×24 m) với 4 cầu thang trục & lan can đá**, và **chưa có thân tháp bát giác 5 tầng mái cong** như hình dáng bản vẽ AI. *(Giữ nguyên vị trí `(+10.7, 0, +61.6)` và lấy kích thước đế `25 × 24 m` theo vệ tinh — vì dùng `28 m` như bản vẽ AI sẽ chạm mép đường `PATH_MAIN_SPINE` `-0.65 m`!)*.

---

### `#05` — Tháp Rơi Tự Do Golden Sky Tower (`1795453348581_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Chiều cao thân tháp thép: **`+47.00 m`** *(khớp số liệu thực tế 47 m từ Sun World)*.
  - Đỉnh trang trí hoa sen bên trên cột: ghi cao `6.5 m` (mặt đứng chính) / `6.2 m` (mặt bên & chi tiết A-A).
  - Bề rộng thân cột kỹ thuật (chi tiết góc phải trên): **`3.0 m`** *(khớp với `towerRadius = 1.65 m → ĐK 3.3 m` trong code)*.
  - Sàn đón khách trên cao: **`+4.20 m`**.
- **Mô tả hình dáng:**
  - Trụ thép đứng thon gọn có 4 thanh ray dẫn hướng & giàn thép bên trong.
  - Vòng ghế hành khách (`PASSERER SEAT RING` + `OVERHEAD SAFETY HARNESS` + `MECHANICAL CATCH-CAR`) treo quanh trụ.
  - **Điểm nhận dạng nổi bật 1 — Chỏm hoa sen vàng:** Đỉnh tháp (`y = 47 m`) không phải chóp nón trơn mà là **đài sen loe cánh đỡ búp hoa sen vàng nhiều lớp cánh xòe + nhụy chóp nhọn**.
  - **Điểm nhận dạng nổi bật 2 — Tổ hợp nhà ga & sàn đón khách `+4.20 m`:** Bệ tròn/bát giác dưới đất có **buồng điều khiển (`CONTROL ROOM`) và lối xếp hàng tầng trệt**, bên trên là **sàn đón khách tròn `+4.20 m`** có hàng rào bảo vệ, **2 nhánh cầu thang bên ngoài** đi lên và **1 chòi mái ngói nhỏ (trạm đón/bảo trì)** cạnh đầu cầu thang.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mốc **`+47.00 m`** bị dán nhầm **cùng lúc ở 3 cao độ khác nhau** (đỉnh búp sen, cổ búp sen và mái chòi chờ thấp gần mặt đất!).
  - Chiều ngang mặt đứng chính ghi `47.0 m` / `47.2 m` (`1.2 + 42.0? + 2.0 + 2.2 + 1.2`) còn mặt bằng ghi `ĐK ~60.0m? / 9.0m` mờ nhòe — không khớp kích thước thực tế chân đế trên ảnh vệ tinh (chỉ rộng `~14–16 m`, `R ≈ 7.2–8.0 m`). Ngày bản vẽ AI ghi `23/11/5820`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Đã có [`src/rides/SkyTower.h`](../src/rides/SkyTower.h) tại [`SKY_TOWER_POS (-260.0, 0.0, -54.6)`](../src/core/ParkLayout.h#L62) (`totalHeight = 47.0f`, `towerRadius = 1.65f`, `gondolaRadius = 3.35f`, 16 ghế + rơi tự do 5 pha — đã xác minh vị trí & chiều cao 47 m trên Wayback 2019 + Commons `49358003906`).
  - **Khác biệt so với bản vẽ AI:** (a) Đỉnh tháp hiện dùng `drawCylinderY` + `drawConeY` đơn giản, **chưa có chỏm búp hoa sen nhiều cánh**; (b) Chân tháp hiện chỉ là bục bát giác cao `1.2 m` với 8 cọc lan can, **chưa có sàn đón `+4.20 m`, buồng điều khiển tầng trệt, cầu thang lên và chòi mái ngói** (chú ý nâng `minY` của Gondola từ `3.5 m` lên `~4.3 m` khớp sàn đón `+4.20 m` và đặt chòi đón ngoài `R = 3.85 m` để ghế không va quẹt). **Giữ nguyên vị trí `(-260.0, 0, -54.6)` và bán kính gọn `R ≤ 8.0 m`.**

---

### `#06` — Đền Angkor Wat (`1795453359463_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 36.0m x 24.0m` (nét mảnh dễ đọc thành `96.0m`), `CAO ĐIỂM: 20.0m`.
  - Mốc cao độ: `0.00 m`, `+4.00 m` (nền giật cấp), `+8.50 m` (mái hành lang), `+14.50 m` (4 tháp góc), `+20.00 m` (tháp trung tâm).
- **Mô tả hình dáng:** Tổ hợp đền đá Khmer giật cấp đối xứng 4 hướng gồm:
  1. **Nền đá giật cấp 5 bậc** hình chữ nhật mở rộng chữ thập ở 4 lối vào.
  2. **4 cầu thang đá trục** (Đông/Tây/Nam/Bắc) vươn ra 4 phía từ mặt đất lên sân đền `+4.00 m`.
  3. **Hành lang cột đá bao quanh (`colonnade gallery`)** với **4 cổng tiền sảnh tam giác đầu hồi (gopura)** nhô ra ở 4 mặt trục.
  4. **5 tháp búp sen (Prang Khmer)** nhiều tầng gờ đá thuôn nhọn: 4 tháp góc cao vừa và 1 tháp trung tâm vươn cao nổi bật.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn nghiêm trọng):**
  - **Rộng (`X`):** Mặt đứng chính ghi tổng dưới `38.00 m` nhưng phía trên ghi `36.00 m` và dải trên cùng `5.5 + 5.0 + 5.0 + 5.0 = 20.5 m`; mặt đứng sau ghi `5.0 + 13.0 + 5.0` dưới tổng `36.0 / 38.0 m`; mặt bằng mái ghi tổng `26.0 / 36.0 m` nhưng các đoạn cộng `5.0 + 12.0 + 5.0 + 12.0 + 5.0 = 39.0 m`.
  - **Sâu (`Z`):** Khung tên ghi `24.0 m`; cạnh phải mặt bằng ghi `5.0+5.0+5.0` (tổng `20.0 m`) còn cạnh trái `6.0+6.0+9.0 = 21.0 m` (tổng `26.0 m`); mặt đứng bên ghi `12.0+9.0+12.0 = 33.0 m` dưới tổng `36.0/30.0 m`.
  - **Cao (`Y`):** Mốc cao độ tổng `+20.00 m`, nhưng chuỗi dọc bên trái lại cộng `5.0 + 4.0 + 12.0 + 5.0 = 26.0 m` (mặt bên ghi `5.0+6.0+15.0+5.0 = 31.0 m`).
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ SÂN SAO NHƯNG THIẾU TOÀN BỘ ĐỀN:**
  - Tại [`ANGKOR_STAR_PLAZA_POS (-185.0, 0.0, -254.0)`](../src/core/ParkLayout.h#L484), ảnh vệ tinh Wayback 2019 (`step0_wave4_6_candidates.png` `#19`) thấy rõ **sân lát ngôi sao 8 cánh `~28×28 m` và các khối kiến trúc**, tuy nhiên **chưa có nguồn văn bản/ảnh mặt đất EXIF xác nhận tên chính thức và thiết kế chi tiết**.
  - Trong [`ParkArchitecture.h`](../src/rides/ParkArchitecture.h#L356-L415) (`appendAngkorStarPlazaTowers`) và [`drawTiledCentralPlaza`](../src/rides/ParkArchitecture.h#L1014-L1038) hiện chỉ có **nền sao phẳng `23.6×23.6 m` và 5 tháp nhỏ đứng rời trên mặt đất**, chưa có bệ đá giật cấp, 4 cầu thang chữ thập, hành lang cột bao quanh và hệ 5 tháp Prang bề thế như bản vẽ. *(Lưu ý: nếu nâng cấp lên đền Angkor đầy đủ, phải thu gọn trong khung **`26–28 × 26–28 m`** để cách tim dầm Monorail **`6.1–7.2 m`**, không dời Monorail).*

---

### `#07` — Nhà hàng Panda (`1795453362382_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `CHIỀU CAO: 14.0m | CHIỀU RỘNG: 26.0m (hoặc 38.0m) | CHIỀU SÂU: 18.0m`.
- **Mô tả hình dáng:** Lâu đài / nhà hàng Á Đông **2 tầng mái ngói lưu ly cong đầu đao vểnh**, có **hệ đấu củng (`bracket complex`)** dưới diềm mái, tầng trệt có cột hiên và hệ cửa gỗ mắt cáo đặt trên bệ đá có **bậc thềm chính trước/sau + 2 cầu thang nghiêng hai bên hông**, tầng 2 có ban công lan can gỗ bao quanh, đỉnh bờ nóc mái trên có **đôi rồng chầu bảo châu**.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng chính ghi tổng `26.0 m`, nhưng chuỗi 9 đoạn con ở trên cộng lại tới `7.0+8.5+7.0+8.0+12.0+8.0+7.5+7.8+8.0 = 73.8 m` (mặt sau cộng ra `73.0 m`).
  - Mặt đứng bên ghi ở giữa `18.0 m` nhưng cộng hai đầu thang ra `7.5 + 18.0 + 8.0 = 33.5 m`.
  - Mốc cao độ ghi lặp **`+5.20 m`** hai lần ở mặt đứng chính và **`+14.00 m`** ở cả cổ tầng 2 lẫn đỉnh mái trên mặt đứng sau; chuỗi dọc trái cộng `0.9+4.2+2.0+4.2+5.0+7.0 = 23.3 m` không khớp tổng `14.0 m`. Lỗi chữ `PANDA REETAURANT`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không có trên ảnh vệ tinh Wayback 2019** (bài toán bổ sung thuộc **GĐ C** nếu duyệt mở rộng).

---

### `#08` — Tượng & Hồ Merlion (`1795453365678_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Đường kính hồ nước tròn: **`28.0 m`** (chi tiết bệ đầu tượng rộng `3.0 m`).
  - Tổng chiều cao tượng + bệ từ mặt đất: **`10.0 m`** (mốc đỉnh `+10.00 m`).
- **Mô tả hình dáng:** Hồ nước tròn giật cấp đồng tâm (thành ngoài 2 bậc, đảo tròn bệ sóng ở giữa), có **bậc thang dẫn lên mép hồ** ở một phía và vòng **vòi phun nước (`Fountain nozzles`)** quanh lòng hồ; trên bệ sóng là **tượng Ngư Sư Merlion** thân cá có vẩy uốn lượn + đầu sư tử có bờm phun tia nước vòng cung xuống mặt hồ.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Cột dọc bên trái và bên phải đều ghi tổng `10.0 m`, nhưng các đoạn thành phần lại ghi `2.0 + 1.0 + 9.0 + 6.0 = 18.0 m`!
  - Mốc cao độ bên trái lặp **`+3.20 m`** hai lần ở 2 gờ khác nhau; khung tên ghi `DATE: 31/11/2016` (ngày 31 tháng 11) và nhầm `31.90M` vào ô ngày. Lỗi chữ `Funtian pools`, `Tuongnhieu Ngu su`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không có trên ảnh vệ tinh Wayback 2019** (nếu dựng sẽ thuộc **GĐ C**).

---

### `#09` — Thuyền Rồng (`1795453369127_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 22.0m (DÀI) x 7.0m (RỘNG) x 9.0m (CAO)`.
- **Mô tả hình dáng:** Du thuyền rồng hoàng gia dài thuôn hai đầu, thân tàu chạm vảy rồng, **mũi tàu tạc đầu rồng lớn có cặp sừng và bờm**, **lái tàu là đuôi rồng cong vươn cao**, chính giữa boong tàu là **lầu vọng cảnh 2 tầng mái ngói cong** có cột gỗ và lan can bao quanh.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Tổng dài ghi `22.0 m` nhưng chuỗi phân đoạn phía trên cộng `7.0 + 8.0 + 9.0 + 8.0 + 7.0 = 39.0 m`!
  - Mặt đứng trước ghi rộng `7.0 m` nhưng 2 đoạn trên đầu ghi `7.0 + 7.0 = 14.0 m`!
  - Chiều cao ghi `9.0 m` ở cột trái mặt đứng bên & khung tên, nhưng chuỗi trái mặt đứng trước lại cộng `1.0 + 2.0 + 4.0 + 2.0 + 6.0 = 15.0 m` (dòng tổng vẫn ghi `9.0 m`) và mốc cao độ bên phải mặt đứng bên ghi `+12.0 m` (lệch từ `-0.0 → +5.0 → +11.0 → +12.0 m`).
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không có trên ảnh vệ tinh Wayback 2019** (nếu dựng trên mặt hồ sẽ thuộc **GĐ C**).

---

### `#10` — Tàu lượn The Flying Kirins (`1795453378911_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `CHIỀU CAO THIẾT BỊ: 15.0m | CHIỀU DÀI THUYỀN: 18.0m | CHIỀU RỘNG: 12.0m`.
- **Mô tả hình dáng:** Trò chơi thuyền rồng/kỳ lân đu đưa kiểu con lắc:
  - Đặt trên **bến cầu cảng gỗ trên mặt/bờ nước** (`Waterfront`, `Cầu cảng ven sông`) có sàn xếp hàng (`Queue rails`), cầu thang và mái/cổng lên xuống (`Boarding gates`).
  - **4 chân trụ thép chữ A** cắm vào 4 bệ móng vuông (`18.0 × 12.0 m` tâm cột) chụm lên ổ trục cơ khí (`Mechanical hub`) cao `15.0 m`.
  - **Thân thuyền rồng hai đầu đối xứng** có cột buồm ngắn ở giữa dao động qua lại hình cung.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Chuỗi dưới mặt đứng chính ghi `5.0 + 5.0 + 18.0 + 5.0 + 5.0 = 38.0 m` (cả sải cầu cảng, lớn hơn con số `18.0 m`).
  - Mặt đứng sau ghi nhầm cao độ sàn thấp thành **`+20.0 m`** dù đỉnh trục phía trên ghi `+15.0 m`. Lỗi chữ `31/18/2018`, `Queue nines`, `Watertrong`.
- **Đối chiếu dự án (`b1ed25e`):**
  - Trong code hiện có [`src/rides/PirateShip.h`](../src/rides/PirateShip.h) của thành viên nhóm tại [`PIRATE_SHIP_TARGET_POS (-172.0, 0.0, -2.0)`](../src/core/ParkLayout.h#L468) (`18 × 12 m`, trụ chữ A cao `~11 m`, thuyền lắc đơn giản không bến cầu cảng, không đầu rồng).
  - **Nguyên tắc:** Giữ nguyên `PirateShip.h` tại vị trí hiện tại (không sửa file thành viên và không coi `PirateShip` là `The Flying Kirins` ngoài thực địa vì tọa độ `(-172, -2)` không có dấu vết cầu cảng ven sông). Nếu bổ sung `The Flying Kirins` đúng cảnh quan cầu cảng ven hồ theo bản vẽ thì xếp vào **GĐ C** (công trình mới ở bờ Đông hồ).

---

### `#11` — Festival Carousel (`1795453381958_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `ĐƯỜNG KÍNH: Ø24.0m | CHIỀU CAO: 14.0m` (mặt đứng ghi `+14.00 m`, sàn tầng 2 ở `+5.20 m`, bệ ngoài `+1.20 m`).
- **Mô tả hình dáng:** **Đu quay ngựa gỗ 2 tầng kiểu Venice**:
  - Ngoài cùng là bệ bậc tròn, **hàng rào bảo vệ** bao quanh và **1 quầy vé nhỏ** đặt sát rìa cổng vào.
  - Khối quay gồm sàn tầng 1, trụ lõi trung tâm, **cầu thang cong dẫn lên sàn tầng 2 có lan can**, **2 tầng ngựa gỗ & cột đồng**, hệ **diềm mái trang trí lượn sóng**, **mái che hình nón 14–16 múi sọc** và **chóp lầu đèn hình vương miện** trên đỉnh.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Chuỗi chiều cao bên trái mặt đứng chính cộng `0.9 + 14.0 + 1.5 + 6.0 = 22.4 m` (trong khi tổng vẫn ghi `14.0 m`); chữ `MẶT ELEVATION` bị trộn Việt-Anh; mặt bằng ghi 2 đoạn `3.5 + 4.5 = 8.0 m` nhưng ngoặc tổng ghi `24.0 m`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Hiện ở [`src/rides/Carousel.h`](../src/rides/Carousel.h) tại [`CAROUSEL_POS_UNVERIFIED (-159.4, 0.0, +102.5)`](../src/core/ParkLayout.h#L73) (`[Chưa xác minh]`, khớp vệt mái nón kẻ sọc `ĐK ~16.5 m` trên Wayback 2019), đang là **1 tầng** (`platformRadius = 8.5 m → ĐK 17.0 m`, `canopyHeight = 8.8 m`, `12` ngựa gỗ), chưa có tầng 2, cầu thang, hàng rào bảo vệ, quầy vé hay mái chia múi sọc chi tiết.
  - **Ràng buộc hình học khi nâng cấp (GĐ A):** Phải **giữ nguyên tâm `(-159.4, 0, +102.5)` và đường kính vệ tinh `~17 m` (`R = 8.5 m`)** (thu tỷ lệ hình dáng bản vẽ theo **`17 / 24 ≈ 0.71`**, cao tổng **`~10.0 m`**), vì nếu phóng ra `Ø24.0 m` sẽ ép hẹp lối đi `PATH_WEST_SPINE` xuống chỉ còn `1.50 m` và lệch khỏi kích thước vệ tinh.

---

### `#12` — Tàu lượn Paradise Fall & Nhà Ga (`1795453385446_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `CHIỀU CAO TỐI ĐA: 16.0m | CHIỀU RỘNG: 34.0m | CHIỀU SÂU: 28.0m`. Mốc cao độ: `+5.00 m` (sàn ga), `+12.00 m`, `+13.00 m`, `+16.00 m` (đỉnh ray).
- **Mô tả hình dáng:** Đường ray 2 vòng lồng uốn lượn (nhô dốc cao tới 16 m ở đầu cánh trái) đỡ bằng hệ cột thép/xiên, liên thông trực tiếp vào **nhà ga sàn cao `+5.00 m` có cầu thang 2 nhánh đối xứng ở mặt tiền và lầu 2 tầng mái ngói cong**.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng chính ghi tổng ngang `34.0 m` (nhưng đường ray phía trái tách rời phần nhà ga); mặt bằng ghi `7.8 + 34.0 + 7.5 = 49.3 m` dưới con số tổng `34.0 m`! Mặt đứng bên ghi tổng ngang lên tới `34.0 + 16.0 + 2.2 = 52.2 m`.
  - Mặt bằng ghi chiều sâu `4.0 + 16.8 + 4.2 = 25.0 m` nhưng ngoặc tổng ghi `28.0 m`.
  - Cột cao bên trái ghi `16.0 m` lặp hai lần kèm `8.0 m` cộng không khớp; ngày vẽ ghi `31/16/2019`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Đã xác minh trên Wayback 2019 + RCDB `#15548` + Wikimedia Commons `49358210737`: cụm gồm **Tòa nhà giả núi đá `PARADISE_FALL_BLDG_POS (-179.8, 0.0, -91.1)` (`24×14 m`)** và **vòng ray số 8 `PARADISE_FALL_TRACK_CENTER (-199.0, 0.0, -119.9)` (`34×24 m`, cao `8.8 m`)** trong [`MiniCoaster.h`](../src/rides/MiniCoaster.h) (`coasters[2]`).
  - **Khác biệt:** Ảnh thực địa Commons `49358210737` xác nhận có khối kiến trúc giả núi đá nên cần **giữ khối núi đá `drawStoneMountainBuilding`**; có thể nâng cấp thêm chi tiết **lầu nhà ga 2 tầng mái cong + cầu thang** kết hợp với khối núi đá và nâng chiều cao ray số 8 từ `8.8 m` lên `~12–14 m` trong cùng khung bao `(X, Z)` vệ tinh (`34×24 m`).

---

### `#13` — Tàu lượn Port of Sky Treasure (`1795453389310_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 14.0m (CAO) x 40.0m (RỘNG) x 30.0m (SÂU)`.
- **Mô tả hình dáng:** Đường ray **số 8 vắt chéo ở giữa** (một đầu nâng xoắn dốc cao, đầu kia ôm vòng ngang qua khu hàng đợi chữ U) kết hợp **nhà ga bến cảng gỗ phương Tây mái đầu hồi 2 dốc (`gable roof`) để lộ vì kèo tam giác (`king-post truss`) và cột gỗ trên bệ cầu cảng**.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng chính ghi `4.0 + 28.0 + 4.0 = 36.0 m` nhưng dòng tổng ghi `40.00 m`; mặt bằng cộng dọc `1.0 + 1.0 + 3.0 + 4.0 = 9.0 m` trong khi tổng ghi `30.00 m`; cột cao ghi lặp `14.0 + 14.0 + 6.0` (chồng nhau, không khớp `14.0 m`). Ngày vẽ ghi `31/16/2019`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Vị trí & khung bao vệ tinh đã xác minh (`RCDB #13603` + Commons `49358003766`): [`PORT_SKY_TREASURE_CENTER (-224.0, 0.0, +22.0)`](../src/core/ParkLayout.h#L425) (`36 × 52 m`), trong [`MiniCoaster.h`](../src/rides/MiniCoaster.h) (`coasters[1]`).
  - **Khác biệt:** Khác bản vẽ ở **kiến trúc nhà ga** — hiện dùng chung `drawCompactBoardingStation` (mái ngói 4 dốc Á Đông), **chưa có nhà ga gỗ bến cảng mái đầu hồi 2 dốc vì kèo tam giác** và vòng xoắn đỉnh mới cao `9.2 m` (có thể nâng chiều cao vòng xoắn Bắc lên `~13 m` và đổi kiểu nhà ga thành nhà ga bến cảng gỗ trong **cùng khung bao `(X, Z)` vệ tinh**).

---

### `#14` — Sân khấu nổi Symphony of River (`1795453392187_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 42.0m (RỘNG) x 20.0m (SÂU) x 12.0m (CAO)`. Mốc cao độ: `+4.8 m` (bậc trên khán đài), `+12.0 m` (đỉnh vòm bán nguyệt).
- **Mô tả hình dáng:** Sân khấu nổi hướng sông/hồ hình chữ nhật nhô nhẹ bán nguyệt ở mép nước; nổi bật ở chính giữa là **cổng chào bán nguyệt lớn (giàn thép vòm kép trang trí đèn)**; phía sau ôm lấy sân khấu là **khán đài bậc thang bán nguyệt hình quạt** kẹp hai bên bởi **2 vọng lầu mái cong 2 tầng** đối xứng (`VỌNG LẦU NHÀ MÁI VÒM`).
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng chính trên cùng ghi `4.8 + 4.8 + 42.0 + 4.9 + 4.0 = 60.5 m` nhưng ngoặc tổng vẫn ghi `42.0 m`! Mặt bằng trên cộng `1.8 + 3.8 + 5.0 + 2.5 + 1.8 = 14.9 m` dưới tổng `42.0 m`! Mặt đứng sau ghi ở đáy `3.0 × 4 = 12.0 m` dưới tổng `42.0 m`.
  - Chuỗi cao bên trái mặt đứng chính ghi `4.8 + 13.5` (= `18.3 m`) không khớp đỉnh `+12.0 m`. Lỗi chữ `MẶTERIOR TOP PLAN VIEW`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không tồn tại trong hiện trạng 2019** (ngoài đời ra mắt năm 2024 tại bờ Đông/hồ phía Bắc, thuộc **GĐ C** nếu dựng mô phỏng).

---

### `#15` — Nhà hát Âu Cơ / Múa rối nước (`1795453395295_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 24.0m (RỘNG) x 18.0m (SÂU) x 12.0m (CAO)`.
- **Mô tả hình dáng:** Nhà hát kiến trúc truyền thống Việt Nam gồm **3 khối giật cấp đối xứng với 3 tầng mái ngói cong**, chính giữa mặt tiền là **cổng vòm bán nguyệt có mái vòm đón khách uốn cong mềm**, hai cánh trang trí **hệ lam gỗ đứng (`lam trang trí dọc`)**, phía trước có bậc thềm tam cấp và hai bên có đường dốc tiếp cận.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng bên ghi tổng ngang dưới `18.0 m`, nhưng chuỗi phía trên cộng `8.0 + 5.0 + 18.0 + 3.1 = 34.1 m`!
  - Mặt đứng sau ghi mốc **`+12.00 m` ở cả đỉnh tầng 2 lẫn tầng 3**; chuỗi đứng bên trái cộng `2.0 + 6.0 + 4.0 + 4.0 = 16.0 m` dưới tổng `12.0 m`; mặt đứng bên lại ghi tổng cao `16.0 m` dù khung tên ghi `12.0 m`. Mặt bằng mái chồng chữ `24.0m / 18.0m` ở cạnh trái. Ngày vẽ `31/16/2019`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không xác minh được trên Wayback 2019 / Wikimedia Commons** (nếu bổ sung thuộc **GĐ C**).

---

### `#16` — Tàu lượn Garuda Valley & Nhà Ga (`1795453397846_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `CHIỀU CAO TỐI ĐA: 14.00m | CHIỀU RỘNG: 30.00m | CHIỀU SÂU: 22.00m` (riêng nhà ga dài `18.0 m`, sàn cao `+5.00 m`).
- **Mô tả hình dáng:** Đường ray 2 vòng tròn giao nhau lượn dốc cao bên phải kết hợp đặc trưng nhận dạng độc đáo là **nhà ga mái yên ngựa cong vút hai đầu sừng (phong cách Minangkabau / Batak Indonesia)** có **hàng mào chạm trổ dọc sống mái** và bậc thang đi lên chính diện.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mốc cao độ bên phải mặt đứng chính và mặt đứng bên ghi **`+14.00 m` lặp lại tới 3 lần** ở 3 tầng cao khác nhau (`nóc sừng`, `giữa cột`, và `giữa mái`).
  - Mặt đứng bên ghi nhầm `22.0 m` (chiều sâu) vào thước đo **chiều cao**, cạnh mặt bằng ghi vừa `14.0 m` vừa `22.0 m`.
- **Đối chiếu dự án (`b1ed25e`) — ĐÃ CÓ NHƯNG KHÁC BẢN VẼ:**
  - Đã xác minh tên & vị trí trên Wayback 2019 + RCDB `#12676` + Commons `49374160882`: [`GARUDA_VALLEY_CENTER (-124.1, 0.0, +155.7)`](../src/core/ParkLayout.h#L424) (`34 × 26 m`) trong [`MiniCoaster.h`](../src/rides/MiniCoaster.h) (`coasters[0]`).
  - **Khác biệt:** Nhà ga hiện tại dùng chung `drawCompactBoardingStation` (mái cong 4 dốc Á Đông thông thường), **chưa có mái yên ngựa cong 2 đầu sừng đặc trưng Minangkabau** (và đỉnh đồi ray hiện cao `8.2 m`, có thể nâng lên `~12 m` trong cùng khung bao vệ tinh `34×26 m`).

---

### `#17` — Đảo Khủng Long / Dino Island (`1795453400510_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 28.0m (RỘNG) x 22.0m (SÂU) x 10.0m (CAO)`, cổng chào rộng `~7.6 m`.
- **Mô tả hình dáng:** Khu tham quan ngoài trời khép kín bao quanh bởi đường đi bộ uốn cong, chính giữa/bên phải có **núi lửa giả tạo cao 10 m**, góc trái là **cổng chào Đảo Khủng Long bằng trụ gỗ-đá**, rải rác bên trong là **tượng các loài khủng long (`Brachiosaurus` cổ dài, 2 `T-Rex`, `Triceratops`)**, hàng rào gỗ thấp và cây cọ/dương xỉ.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Chuỗi trên mặt bằng ghi `1.1 + 1.8 + 3.0 + 8.0 = 13.9 m` nhưng tổng ngang vẫn ghi `28.0 m`!
  - Cột dọc mặt đứng chính ghi `10.0 + 10.0 = 20.0 m` dưới tổng `10.0 m`; mặt đứng bên ghi tổng cao **`22.0 m`** (cộng `10.8 + 10.0 = 20.8 m`, bị lấy nhầm số chiều sâu!). Lỗi chữ `SAFETY GIÁ TẠNT`, `CIAO PHUNG DANA`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không có trên ảnh vệ tinh Wayback 2019** (thuộc **GĐ C** nếu dựng thêm).

---

### `#18` — Vườn Trúc Nhật Bản / Bamboo Garden (`1795453403998_image.png`)
- **Nhãn nguồn:** `[Bản vẽ minh họa AI, tham khảo hình dạng]`.
- **Kích thước đọc được trên bản vẽ:**
  - Khung tên: `KÍCH THƯỚC: 24.0m (RỘNG) x 20.0m (SÂU) x 9.0m (CAO)`. Mốc cao độ: `+6.50 m` (nhà trà), `+6.8 m` (cổng Torii), `+9.00 m` (rặng trúc).
- **Mô tả hình dáng:** Khu vườn cảnh quan Nhật Bản gồm **Cổng thần đạo Torii**, **dòng suối cá Koi (`KOIN SREM`) uốn lượn ở giữa**, **cầu gỗ cong Nhật Bản** (`8–9 m`) bắc qua suối, **2 đèn đá Tōrō**, **nhà trà mái ngói cong** nhỏ, đường đi bộ đá uốn vòng và các **rặng trúc xanh cao 9 m**.
- **Số đo không đọc rõ / không cộng khớp (mâu thuẫn):**
  - Mặt đứng chính ghi tổng ngang `24.0 m`, nhưng chuỗi trên cộng `1+2+3+3+3+3+2 = 17.0 m` và chuỗi dưới cộng `2+3+1+8+1+3+1 = 19.0 m` — cả hai đều không bằng `24.0 m`!
  - Mặt bằng tổng thể ghi cạnh trái `1.0 + 5.9 + 1.8 = 8.7 m` nhưng ngoặc tổng ghi `20.0 m`! Chiều cao mặt đứng bên cộng `6.0 + 1.0 = 7.0 m` dưới tổng `9.0 m`. Lỗi chữ `STONE BABE`, `KOIN SREM`, `SIOE ELEVATION`, `31/18/2018`.
- **Đối chiếu dự án (`b1ed25e`):** **CHƯA CÓ** trong mã nguồn và **không có trên ảnh vệ tinh Wayback 2019** (thuộc **GĐ C** nếu bổ sung).

---

### `#19` — `[Thiếu ảnh thứ 19]`
- Trong repo tại thời điểm hiện tại chỉ có **18 ảnh** trong [`docs/img/ref/`](../img/ref/). Chưa có file ảnh thứ 19 (`19_*`) trên đĩa để đọc kích thước hay đối chiếu hình dáng. *(Ghi chú: 1 trò chơi còn lại đã dựng trong Đợt 5A nhưng chưa thấy bản vẽ trong 18 ảnh trên là **Highway Boat** tại `(-176.0, 0, -214.0)` — cần bạn xác nhận nếu có bổ sung ảnh thứ 19).*

---

## 2. Bảng đối chiếu tổng hợp: Bản vẽ AI vs. Hiện trạng Code (Mốc Đợt 7, commit `b1ed25e`)

| STT | Công trình / Khu vui chơi | Trạng thái trong Code (`b1ed25e`) | Vị trí hiện tại (`ParkLayout.h` / code) | Đánh giá khác biệt giữa Code và Bản vẽ AI `[Bản vẽ minh họa AI, tham khảo hình dạng]` | Giữ nguyên vị trí? |
|:---:|:---|:---|:---|:---|:---|
| 01 | **Tổng mặt bằng** | Đã có bố cục chuẩn (`~36.1 ha`) | Toàn [`ParkLayout.h`](../src/core/ParkLayout.h) | Sơ đồ AI 01 sai hướng Đông–Bắc và lặp tên; **vệ tinh Wayback 2019 + OSM luôn thắng**. | Giữ nguyên |
| 02 | **Vòng quay Sun Wheel** | Đã có (`SunWheel.h`, `ParkArchitecture.h`) | `SUN_WHEEL_CENTER (0, 65, -38)` | Đạt tỷ lệ thật 1:1 (115 m, ĐK 100 m, 64 cabin — chính xác hơn bản vẽ AI). | **KHÔNG DỜI** (đã xác minh) |
| 03 | **Cổng Ngọ Môn** | Đã có (`NgoMonGate.h`) | `NGO_MON_GATE_POS (+16.1, 0, +139.1)` | Code hiện tại chi tiết & giống ảnh thật hơn bản vẽ AI (có đủ Lầu Ngũ Phụng + 2 cánh tường thành + 2 hồ sen). | **KHÔNG DỜI** (đã xác minh) |
| 04 | **Tháp Đồng Hồ** | **Đã có nhưng khác bản vẽ** (`NgoMonGate.h`) | `CLOCK_TOWER_POS (+10.7, 0, +61.6)` | Code mới là 3 hộp `8.4→6.4 m`; bản vẽ có bệ rộng (~`25×24 m` theo vệ tinh), 4 cầu thang và **thân bát giác 5 tầng mái cong (`~29 m`)**. | **KHÔNG DỜI** (đã xác minh Wayback 2019) |
| 05 | **Golden Sky Tower** | **Đã có nhưng khác bản vẽ** (`SkyTower.h`) | `SKY_TOWER_POS (-260.0, 0, -54.6)` | Đúng vị trí & 47 m; thiếu **chỏm hoa sen nhiều cánh** ở đỉnh và **sàn đón `+4.20 m` + buồng điều khiển + cầu thang + chòi đón**. | **KHÔNG DỜI** (đã xác minh) |
| 06 | **Đền Angkor Wat** | **Đã có sân sao, thiếu đền đầy đủ** (`ParkArchitecture.h`) | `ANGKOR_STAR_PLAZA_POS (-185.0, 0, -254.0)` | Hiện chỉ có 5 tháp đứng rời trên sân sao phẳng `23.6 m`; thiếu **bệ đá giật cấp, 4 cầu thang chữ thập, hành lang cột và 5 tháp Prang**. | **KHÔNG DỜI** (nằm gọn khung `#19` Wayback 2019) |
| 07 | **Nhà hàng Panda** | **CHƯA CÓ** | — | Chưa tồn tại trong scene & không có trong ảnh 2019. | — *(Ứng viên Khu Đông)* |
| 08 | **Tượng & Hồ Merlion** | **CHƯA CÓ** | — | Chưa tồn tại trong scene & không có trong ảnh 2019. | — *(Ứng viên Khu Đông)* |
| 09 | **Thuyền Rồng** | **CHƯA CÓ** | — | Chưa tồn tại trong scene & không có trong ảnh 2019. | — *(Ứng viên Mặt hồ)* |
| 10 | **The Flying Kirins** | **CHƯA CÓ bản vẽ cầu cảng** *(có `PirateShip.h` riêng của nhóm)* | `PirateShip` đang ở `(-172.0, 0, -2.0)` | `PirateShip.h` giữ nguyên ở khu Tây (không phải `The Flying Kirins`); bản vẽ Kirins là cầu cảng + thuyền rồng chữ A ven sông/hồ. | Giữ `PirateShip`; Kirins *(Ứng viên bờ Đông hồ)* |
| 11 | **Festival Carousel** | **Đã có nhưng khác bản vẽ** (`Carousel.h`) | `CAROUSEL_POS_UNVERIFIED (-159.4, 0, +102.5)` | Hiện mới 1 tầng (ĐK 17 m, cao 8.8 m); bản vẽ là **2 tầng kiểu Venice + cầu thang + hàng rào + quầy vé**. | **KHÔNG DỜI** (giữ tâm tạm & ĐK 17 m vệ tinh) |
| 12 | **Paradise Fall** | **Đã có nhưng khác bản vẽ** (`MiniCoaster.h`) | Núi đá `(-179.8, 0, -91.1)`, ray `(-199.0, 0, -119.9)` | Đã có núi đá & ray số 8 (cao 8.8 m); có thể bổ sung lầu ga 2 tầng mái cong & nâng ray lên `~12–14 m` trong khung vệ tinh. | **KHÔNG DỜI** (đã xác minh RCDB/Commons) |
| 13 | **Port of Sky Treasure** | **Đã có nhưng khác bản vẽ** (`MiniCoaster.h`) | `PORT_SKY_TREASURE_CENTER (-224.0, 0, +22.0)` | Đã có ray `36×52 m`, nhưng nhà ga hiện dùng mái Á Đông; bản vẽ dùng **nhà ga bến cảng gỗ mái đầu hồi 2 dốc + vì kèo**. | **KHÔNG DỜI** (đã xác minh RCDB/Commons) |
| 14 | **Symphony of River** | **CHƯA CÓ** | — | Ra mắt 2024, không có trong hiện trạng 2019. | — *(Ứng viên bờ Đông hồ)* |
| 15 | **Nhà hát Âu Cơ** | **CHƯA CÓ** | — | Không có nguồn xác minh 2019; chỉ có trong bản vẽ AI. | — *(Ứng viên Khu Đông)* |
| 16 | **Garuda Valley** | **Đã có nhưng khác bản vẽ** (`MiniCoaster.h`) | `GARUDA_VALLEY_CENTER (-124.1, 0, +155.7)` | Đã có ray `34×26 m`, nhưng nhà ga đang là mái thường; bản vẽ có **mái yên ngựa cong Minangkabau** đặc trưng. | **KHÔNG DỜI** (đã xác minh RCDB/Commons) |
| 17 | **Đảo Khủng Long** | **CHƯA CÓ** | — | Không có trên ảnh vệ tinh 2019. | — *(Ứng viên Khu Đông)* |
| 18 | **Vườn Trúc Nhật Bản** | **CHƯA CÓ** | — | Không có trên ảnh vệ tinh 2019 (bản vẽ 01 còn vẽ lặp 2 lần). | — *(Ứng viên Khu Đông)* |

*(Các hạng mục Đợt 5A/5B khác đã có trong code gồm `Highway Boat (-176, -214)`, `Playground (-104.9, -24.7)`, `BumperCars (-206.5, +108.1)`, `Thủy đình (-5.4, -107.7)`, 2 nhà ga Monorail, dãy quầy sọc `#6`, 5 nhà mái ngói `#7,#8,#12,#15,#18` và 4 đu quay tròn `#9,#10,#13,#14` được giữ nguyên vị trí.)*

---

## 3. Phân tích vùng trống & Đề xuất vị trí ứng viên cho các công trình chưa có

> **Xác nhận độ tin cậy:** Toàn bộ 8 hạng mục mới khu Đông/trên hồ (`Nhà hàng Panda`, `Tượng & Hồ Merlion`, `Thuyền Rồng`, `The Flying Kirins`, `Sân khấu nổi Symphony of River`, `Nhà hát Âu Cơ`, `Đảo Khủng Long / Dino Island`, `Vườn Trúc`) **KHÔNG có nguồn xác minh tọa độ trong `docs/asia_park_specs.md` ngoài bộ sơ đồ AI này** (trên ảnh vệ tinh Wayback 2019 `layout_overlay.png`, nửa phía Đông `X > +65 m` chỉ là bãi cỏ trống). Riêng **Đền Angkor Wat đầy đủ** chỉ thấy khung sân sao 8 cánh `~28×28 m` trên Wayback 2019 (`#19`) nhưng **chưa có ảnh mặt đất biển tên xác minh kiến trúc đền**.

Dựa trên kiểm tra hình học trực tiếp trên [`ParkLayout.h`](../src/core/ParkLayout.h) và đường cong Bézier `MONORAIL_WAYPOINTS` (`pillarSpacing = 19.0 m`, `pillarRadius = 0.56 m`), khu vực **nửa phía Đông (`X ∈ [+65, +340 m], Z ∈ [-270, +80 m]`)** và **mặt hồ Bắc (`Z < -73.4 m`)** đang hoàn toàn trống. Các vị trí ứng viên dưới đây đều đã được đo kiểm bằng script (`audit_free_zones.cpp`):

- **Ảnh minh họa chồng lớp ứng viên trên nền Wave 7 + Wayback 2019:** [`candidates_overlay.png`](candidates_overlay.png)

| Công trình chưa có | Kích thước chuẩn hóa (`W×D`) | Vị trí ứng viên `(X, Y, Z)` & Góc `Y` | Khoảng cách nhỏ nhất tới dầm Monorail | Khoảng cách tới trụ Monorail gần nhất | Khoảng cách tới lối đi / bờ hồ / cây / ranh giới | Độ tin cậy |
|:---|:---:|:---|:---:|:---:|:---|:---|
| **Đền Angkor Wat đầy đủ** *(GĐ B)* | **`26.0 × 26.0 m`** trên sân `28×28 m` | `(-185.0, 0, -254.0)`, `0°` *(tại `#19` hiện có)* | **`7.15 m`** *(hở `≥ 3 m`)* | **`> 7.0 m`** | Cách `PATH_LAKESIDE_WEST` **`11.60 m`**, hồ `36.04 m`, `0` đè cây (`12 m`) | Khung sao thấy trên Wayback 2019; đền `[Bản vẽ minh họa AI]` |
| **Thuyền Rồng Hoàng Gia** *(GĐ C)* | `22.0 × 7.0 m` | `(+18.0, 0, -152.0)`, `-15°` *(nổi trên mặt hồ Bắc)* | **`53.90 m`** | `53.69 m` | Nổi trọn trong `LAKE_OUTLINE_POLY`; cách bờ hồ **`21.61 m`**, cách Thủy đình **`37.85 m`** | `[mô phỏng, chưa xác minh]` |
| **The Flying Kirins (bến hồ Đông)** *(GĐ C)* | `18.0 × 12.0 m` | `(+74.0, 0, -118.0)`, `-90°` *(bờ Đông hồ)* | **`35.78 m`** | `35.46 m` | Cách mép hồ `13.11 m`, cách ranh giới cỏ `8.90 m`, cách Chòi bờ hồ Đông `26.15 m` | `[mô phỏng, chưa xác minh]` |
| **Sân khấu nổi Symphony of River** *(GĐ C)* | `42.0 × 20.0 m` | `(+88.0, 0, -242.0)`, `-78°` *(bãi Đông-Bắc hướng hồ)* | **`23.57 m`** | `22.96 m` | Cách mép hồ `31.76 m`, cách ranh giới Đông `28.51 m`, cách cây `165.4 m` | `[mô phỏng, chưa xác minh]` |
| **Tượng & Hồ Merlion** *(GĐ C)* | `Ø 28.0 m` (`28×28 m`) | `(+102.0, 0, -45.0)`, `0°` *(bãi cỏ Đông-Nam)* | **`94.78 m`** | `94.42 m` | Cách Chòi SW Đông `40.30 m`, cách lối đi `32.64 m`, cách Panda `~26 m` | `[mô phỏng, chưa xác minh]` |
| **Nhà hát Âu Cơ** *(GĐ C)* | `24.0 × 18.0 m` | `(+135.0, 0, +12.0)`, `-18°` *(bãi cỏ Đông-Nam)* | **`34.04 m`** | `33.88 m` | Cách Chòi QT Đông `77.79 m`, cách Cổng Ngọ Môn `> 55 m`, cách cây `57.9 m` | `[mô phỏng, chưa xác minh]` |
| **Nhà hàng Panda** *(GĐ C)* | `26.0 × 18.0 m` | `(+148.0, 0, -86.0)`, `0°` *(trung tâm khu Đông)* | **`43.98 m`** | `44.91 m` | Cách mép hồ `77.28 m`, cách ranh giới Đông `74.00 m`, cách Merlion `~26 m` | `[mô phỏng, chưa xác minh]` |
| **Vườn Trúc (Bamboo Garden)** *(GĐ C)* | `24.0 × 20.0 m` | `(+226.0, 0, -35.0)`, `0°` *(phía Đông-Nam)* | **`52.29 m`** | `51.85 m` | Cách Dino Island `~22.5 m`, hoàn toàn trong đất khô không đè lối đi/cây | `[mô phỏng, chưa xác minh]` |
| **Đảo Khủng Long (Dino Island)** *(GĐ C)* | `28.0 × 22.0 m` | `(+268.0, 0, -72.0)`, `0°` *(khu Đông)* | **`39.90 m`** | `40.41 m` | Cách ranh giới công viên `89.99 m`, cách Vườn Trúc `~22.5 m` | `[mô phỏng, chưa xác minh]` |

---

## 4. Kế hoạch nâng cấp 3 giai đoạn (Công sức & Rủi ro FPS — Mục tiêu `≥ 100 FPS`)

> **Mốc hiệu năng nền (Đợt 7):** `118.7 FPS` (`1280×768`, 200 khung hình). Yêu cầu giữ FPS **không tụt xuống dưới `100 FPS`**. Biện pháp bắt buộc: mọi khối kiến trúc tĩnh đều dựng bằng `PolygonMesh` + **biên dịch vào OpenGL Display List (`glGenLists`/`glNewList`)**, giữ số phân đoạn vừa phải (`8–16` cạnh cho trụ/mái), đồng thời cache thêm phần nền `drawTiledCentralPlaza()` (`ParkArchitecture.h`) và thân toa xe `MiniCoaster::drawMovingTrain()` để lấy lại quỹ FPS bù cho chi tiết mới.

### Giai đoạn A (`GĐ A`) — Nâng cấp hình dáng công trình đã có (giữ nguyên vị trí & khung bao)
| STT | Hạng mục nâng cấp (`[Bản vẽ minh họa AI, tham khảo hình dạng]`) | Chi tiết thay đổi hình học dự kiến | Mức công sức | Rủi ro FPS & Giải pháp giữ `≥ 100 FPS` |
|:---:|:---|:---|:---:|:---|
| A.1 | **Tháp Đồng Hồ Trung Tâm** (`NgoMonGate.h` tại `(+10.7, 0, +61.6)`) | Đế đá giật cấp 3 lớp + 4 cầu thang trục + lan can đá gọn trong **`25 × 24 m`**, thân **bát giác 5 tầng mái cong cao `~29 m`**, 4 mặt đồng hồ ở tầng áp đỉnh. | **Trung bình (Vừa)** | **Thấp:** Biên dịch toàn bộ ~2 500 đỉnh vào Display List `clockSmoothList`/`clockFlatList` sẵn có (~0.2 ms/frame). |
| A.2 | **Golden Sky Tower** (`SkyTower.h` tại `(-260.0, 0, -54.6)`) | Thêm **chỏm búp sen vàng 16 cánh** ở đỉnh `47..52.8 m`; nâng bệ thành **sàn đón `+4.20 m`**, buồng điều khiển tầng trệt, cầu thang đôi & chòi đón ngoài `R = 3.85 m`; khớp `minY = 4.30 m` cho vòng ghế. | **Vừa** | **Thấp (nếu cache Display List):** Hiện `SkyTower.h` vẽ trực tiếp mỗi frame; chuyển phần trụ + bệ + chỏm sen vào Display List sẽ giảm draw overhead so với hiện tại. |
| A.3 | **Nhà ga các tàu lượn nhỏ** (`MiniCoaster.h` tại các vị trí cũ) | - **Garuda Valley:** Nhà ga **mái yên ngựa cong Minangkabau** (tỉ lệ theo BP16) + nâng đỉnh đồi ray lên `~12 m`.<br>- **Port of Sky Treasure:** Nhà ga **bến cảng gỗ mái đầu hồi + vì kèo tam giác** (BP13) + đỉnh xoắn `~13 m`.<br>- *(Tùy chọn GĐ A+)* **Paradise Fall:** Bổ sung lầu ga 2 tầng mái cong trên khối núi đá (giữ nguyên núi đá thật). | **Vừa → Lớn** | **Thấp → Trung bình:** Cache nhà ga vào `staticList_` và cache 3 toa tàu động (`drawMovingTrain` hiện gọi `createBox` mỗi frame) vào `carList_` để **tăng thêm FPS**. |
| A.4 | **Festival Carousel** (`Carousel.h` tại `(-159.4, 0, +102.5)`, `[Chưa xác minh]`) | Giữ nguyên ĐK vệ tinh `17 m` (`R = 8.5 m`), thu hình dáng **2 tầng kiểu Venice (`~0.71`, cao `~10 m`)** với cầu thang cong lên tầng 2, 12 ngựa tầng 1 + 8 ngựa tầng 2, hàng rào & quầy vé góc SE. | **Lớn** | **Trung bình → Cao nếu không tối ưu:** Lên 20 ngựa gỗ vẽ từng box mỗi frame sẽ làm tụt ~15–25 FPS; **bắt buộc biên dịch 3 mẫu ngựa, phần bệ tĩnh và khung 2 tầng quay vào Display List**. *(Cũng nên cache `drawTiledCentralPlaza()` trong `ParkArchitecture.h` để bù thêm ~10–15 FPS).* |

### Giai đoạn B (`GĐ B`) — Công trình mới / mở rộng có thể đối chiếu bằng ảnh vệ tinh hoặc ảnh thật
| STT | Hạng mục nâng cấp | Bằng chứng đối chiếu thực địa | Đề xuất kỹ thuật | Mức công sức | Rủi ro FPS |
|:---:|:---|:---|:---|:---:|:---|
| B.1 | **Quần thể Đền Angkor Wat đầy đủ** (`ParkArchitecture.h` tại `#19: (-185, 0, -254)`) | Trên `step0_wave4_6_candidates.png` thấy rõ sân sao 8 cánh `~28×28 m` và cụm kiến trúc; hình dạng đền theo `[Bản vẽ minh họa AI, tham khảo hình dạng]` (`#06`), gắn nhãn `[Chưa xác minh tên/chi tiết chính thức]`. | Nền đá 2 bậc + 4 cầu thang chữ thập + hành lang 20 cột + 5 tháp Prang nhiều tầng thu gọn trong **`26 × 26 m`** trên sân sao **`27.8 × 27.8 m`** (cách Monorail `7.15 m`, cao `~15 m`). | **Vừa → Lớn** | **Thấp:** Cộng thêm ~2 500–3 200 đỉnh nhưng nằm trọn trong `archSmoothList` Display List tĩnh. |
| B.2 | **Chi tiết kiến trúc Ga Monorail Nam & Ga Nepal** *(nâng cấp hình dáng ga có sẵn)* | Khớp ảnh mặt đất Commons `49370578306` (Ga Nam) và `49373368582` (Ga Nepal mái cong Á Đông). | Thay hộp phẳng hiện tại của Ga Nepal bằng mái ngói cong đặc trưng Nepal/Á Đông nếu có thêm tư liệu ảnh mặt đất. | **Nhỏ** | **Rất thấp:** Biên dịch vào Display List tĩnh. |

### Giai đoạn C (`GĐ C`) — Công trình mới chỉ có trong bản vẽ AI *(gắn nhãn bắt buộc `"mô phỏng, chưa xác minh"`)*
> **Lưu ý quy hoạch:** Vì 8 hạng mục này **không tồn tại trên ảnh vệ tinh Wayback 2019** (`SNAPSHOT_YEAR = 2019` giữ nửa Đông là bãi cỏ), khi triển khai nên đóng gói thành mô-đun độc lập **có phím bật/tắt riêng** để vừa trình diễn được toàn bộ bộ bản vẽ mới, vừa giữ khả năng bật về nguyên trạng 2019.

| STT | Hạng mục *(gắn nhãn `"mô phỏng, chưa xác minh"`)* | Vị trí ứng viên `(X, Y, Z)` | Đặc điểm hình học chính `[Bản vẽ minh họa AI, tham khảo hình dạng]` | Mức công sức | Rủi ro FPS |
|:---:|:---|:---|:---|:---:|:---|
| C.1 | **Nhà hàng Panda** (`#07`) | `(+148.0, 0, -86.0)` | Nhà hàng Á Đông 2 tầng mái lưu ly vàng cong, đấu củng, 4 cầu thang, đôi rồng nóc & tượng Panda (`26×18×14 m`). | **Vừa** | **Thấp** (tĩnh 100%, Display List) |
| C.2 | **Tượng & Hồ Merlion** (`#08`) | `(+102.0, 0, -45.0)` | Hồ tròn 2 bậc ĐK `28 m`, bệ sóng, tượng Ngư Sư cao `10 m` + tia nước phun động (`GL_LINE_STRIP`). | **Vừa** | **Thấp** (hồ/tượng tĩnh Display List + ~15 đường tia nước) |
| C.3 | **Thuyền Rồng Hoàng Gia** (`#09`) | `(+18.0, 0, -152.0)` *(hồ Bắc)* | Thân thuyền dài `22 m` vẩy rồng, đầu/đuôi rồng, lầu 2 tầng mái cong giữa hồ (`22×7×11.4 m`). | **Nhỏ → Vừa** | **Thấp** (tĩnh/nhấp nhô nhẹ Display List) |
| C.4 | **Nhà hát Âu Cơ (Rối Việt)** (`#15`) | `(+135.0, 0, +12.0)` | 3 tầng mái cong giật cấp, cổng vòm bán nguyệt hồ nước, lam đứng dọc mặt tiền (`24×18×12 m`). | **Vừa** | **Thấp** (tĩnh 100%, Display List) |
| C.5 | **Vườn Trúc (Bamboo Garden)** (`#18`) | `(+226.0, 0, -35.0)` | Cổng Torii, suối Koi, cầu gỗ đỏ Taiko-bashi, nhà trà, 2 đèn đá & 4 rặng trúc 9 m (`24×20×9 m`). | **Vừa** | **Thấp** (tĩnh, Display List) |
| C.6 | **Sân khấu nổi Symphony of River** (`#14`) | `(+88.0, 0, -242.0)` | Sàn bán nguyệt, khán đài 6 bậc hình quạt, cổng vòm giàn thép `12 m` có LED, 2 vọng lầu 2 tầng mái cong (`42×20×12 m`). | **Vừa → Lớn** | **Thấp** (khung tĩnh Display List + vài cầu LED đêm) |
| C.7 | **Đảo Khủng Long (Dino Island)** (`#17`) | `(+268.0, 0, -72.0)` | Cổng gỗ-đá, núi lửa cao `10 m` có hồ dung nham phát sáng, Brachiosaurus, T-Rex, Triceratops, cọ (`28×22×10 m`). | **Lớn** | **Thấp** (tĩnh Display List) |
| C.8 | **The Flying Kirins** *(tách khỏi `PirateShip.h`)* (`#10`) | `(+74.0, 0, -118.0)` | Bến cầu cảng gỗ bờ Đông hồ, 4 chân chữ A cao `15 m`, thuyền rồng đôi lắc con lắc (`18×12×15 m`). | **Vừa** | **Thấp** (bến + khung A tĩnh Display List, thuyền lắc Display List) |

**Ước tính rủi ro FPS tổng thể:** Nếu triển khai cả `GĐ A + B + C` nhưng **tất cả phần tĩnh (kể cả cây/ngựa/sân lát/công trình mới) đều đóng gói vào Display List**, dự kiến FPS tại `1280×768` chỉ giảm khoảng `5–12 FPS` từ mốc `118.7 FPS` (duy trì quanh **`106–113 FPS`**, đạt mục tiêu `≥ 100 FPS`). Ngược lại, nếu giữ cách vẽ `PolygonMesh::createBox` tạo mới mỗi khung hình như `MiniCoaster` / `Carousel` hiện tại thì FPS có thể rớt xuống **`< 80 FPS`** — nên tối ưu Display List là điều kiện tiên quyết trước khi tăng hình học ở GĐ A.
