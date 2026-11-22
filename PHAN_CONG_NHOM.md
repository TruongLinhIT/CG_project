# BẢNG PHÂN CÔNG CÔNG VIỆC & HƯỚNG DẪN KIẾN THỨC ĐỒ ÁN ĐỒ HỌA MÁY TÍNH
**Đề tài:** Mô phỏng 3D Công viên Châu Á – Asia Park (Đà Nẵng)  
**Công nghệ:** C++17 & FreeGLUT (Legacy OpenGL)  
**Quy mô nhóm:** 2 Thành viên (Làm việc song song, tách biệt thư mục để không bị xung đột code / Merge Conflict)

---

## 1. TỔNG QUAN KIẾN TRÚC THƯ MỤC & QUYỀN SỞ HỮU FILE

Để 2 thành viên có thể code độc lập trên máy cá nhân rồi gộp code (Git Merge) mượt mà, dự án được chia thành các module riêng biệt:

```text
d:/BTL_CG/
├── CMakeLists.txt & Makefile     # [Dùng chung] Cấu hình biên dịch dự án
├── PHAN_CONG_NHOM.md             # [Tài liệu nhóm] Bảng phân công & Hướng dẫn kiến thức
└── src/
    ├── main.cpp                  # [Dùng chung] Khởi tạo GLUT, Vòng lặp 60FPS, Menu, Phím/Chuột
    │
    ├── core/                     # 👤 THÀNH VIÊN 1 PHỤ TRÁCH
    │   ├── MathCG.h              # Toán học đồ họa (Vector, Ma trận 4x4, HSV->RGB, WCS->NCS->DCS, Bernstein)
    │   ├── Camera.h              # Hệ thống quan sát 3D (Tọa độ cầu, Ride Cameras, Phép chiếu)
    │   └── Lighting.h            # Chiếu sáng (Ngày/Hoàng hôn/Đêm), Tô bóng Lambert/Gouraud, Suy giảm 1/D^2
    │
    ├── curves/                   # 👤 THÀNH VIÊN 1 PHỤ TRÁCH
    │   ├── BezierTrack.h         # Đường cong tham số Bézier, Đa giác kiểm soát, Khung tiếp tuyến P'(t)
    │   └── RollerCoaster.h       # Tàu lượn siêu tốc Queen Cobra & Tàu điện Monorail chạy trên đường Bézier
    │
    ├── geometry/                 # 👤 THÀNH VIÊN 2 PHỤ TRÁCH
    │   ├── ParametricShapes.h    # Mặt kẻ & Mặt tròn xoay (Trụ, Nón, Cầu, Xuyến) sinh bằng tham số (u, v)
    │   └── MeshModel.h           # Cấu trúc WireFrame (Đỉnh-Cạnh) & PolygonMesh (Đỉnh-Mặt + Pháp tuyến)
    │
    ├── rides/                    # 👤 THÀNH VIÊN 2 PHỤ TRÁCH
    │   ├── SunWheel.h            # Vòng quay Mặt Trời (Biến đổi phân cấp giữ cabin thẳng đứng + LED HSV) Linh lam phan nay roi
    │   ├── SkyTower.h            # Tháp rơi tự do Golden Sky Tower (Trụ + Vòng ghế Toroid trượt/xoay)
    │   ├── Carousel.h            # Đu quay ngựa gỗ (Mái nón + Ngựa chuyển động phân cấp xoay & nhấp nhô)
    │   └── NgoMonGate.h          # Cổng Ngọ Môn & Tháp Đồng Hồ (Dựng bằng lưới đa giác PolygonMesh)
    │
    └── ui/                       # 👤 THÀNH VIÊN 2 PHỤ TRÁCH
        ├── Minimap2D.h           # Bản đồ 2D (WCS->NCS->DCS, Cắt xén Cohen-Sutherland 4-bit, Bresenham/Midpoint)
        └── HUD.h                 # Giao diện hiển thị thông số trạng thái bằng glutBitmapCharacter
```

---

## 2. BẢNG PHÂN CHIA CÔNG VIỆC CHI TIẾT CHO 2 THÀNH VIÊN

| STT | Hạng mục công việc | File phụ trách | Người thực hiện | Chương lý thuyết liên quan | Trạng thái |
|:---:|:---|:---|:---:|:---|:---:|
| **1** | **Thư viện Toán học Đồ họa cốt lõi**: Vector 3D, Ma trận đồng nhất $4\times 4$, Chuyển đổi màu `HSVtoRGB`, Ánh xạ `WCS->NCS->DCS`, Đa thức Bernstein, Tích có hướng tính pháp tuyến | `src/core/MathCG.h` | **Thành viên 1** | Chương 1, 3, 4, 5, 6 | Đã có khung chuẩn (Step 1) |
| **2** | **Hệ thống Quan sát 3D (Viewing) & Phép chiếu**: Camera tọa độ cầu $(\theta, \Phi, R, D)$, Camera góc nhìn thứ nhất trên Cabin & Tàu lượn, Phép chiếu Phối cảnh & Song song trực giao (Top/Front/Side) | `src/core/Camera.h` | **Thành viên 1** | Chương 4 | Đã có khung chuẩn (Step 1) |
| **3** | **Hệ thống Chiếu sáng, Tô bóng & Phản xạ mặt hồ**: Chu kỳ Ngày/Hoàng hôn/Đêm, Nguồn sáng điểm suy giảm $1/D^2$, Tô bóng Lambert (`GL_FLAT`) & Gouraud (`GL_SMOOTH`), Mặt hồ bán trong suốt (`GL_BLEND`) + Phản xạ $y=0$ | `src/core/Lighting.h` & `src/main.cpp` | **Thành viên 1** | Chương 1, 4, 6 | Đã có khung chuẩn (Step 1) |
| **4** | **Đường cong tham số Bézier (`BezierTrack`)**: Đường ray khép kín ghép trơn, sinh thanh ray đôi, tà-vẹt, cột trụ đỡ và chế độ Debug vẽ Đa giác kiểm soát $P_0 \dots P_n$ | `src/curves/BezierTrack.h` | **Thành viên 1** | Chương 5 | Cần phát triển ở Step 2 |
| **5** | **Tàu lượn Queen Cobra & Monorail (`RollerCoaster`)**: Đoàn tàu nhiều toa chuyển động theo tham số $t \in [0, 1]$, tự động xoay hướng (Yaw, Pitch, Roll) theo vector tiếp tuyến $P'(t)$ | `src/curves/RollerCoaster.h` | **Thành viên 1** | Chương 4, 5 | Cần phát triển ở Step 2 |
| **6** | **Các Mặt cong Tham số (`ParametricShapes`)**: Tự cài đặt lưới đỉnh & pháp tuyến mặt Trụ (`Cylinder`), Nón (`Cone`), Cầu (`Sphere`), Xuyến (`Toroid`) theo $(u, v)$ (không dùng `glutSolid*`) | `src/geometry/ParametricShapes.h` | **Thành viên 2** | Chương 5, 6 | Cần phát triển ở Step 2 |
| **7** | **Biểu diễn Mô hình 3D (`MeshModel`)**: Cài đặt cấu trúc `WireFrame` (Đỉnh + Cạnh) và `PolygonMesh` (Đỉnh + Mặt + Tự động tính pháp tuyến mặt $\vec{N} = \vec{A}\times\vec{B}$ & pháp tuyến đỉnh $\vec{n}_p$) | `src/geometry/MeshModel.h` | **Thành viên 2** | Chương 4, 6 | Cần phát triển ở Step 2 |
| **8** | **Vòng quay Mặt Trời (`SunWheel`)**: Quay bánh xe góc $\alpha$ quanh tâm $I(x_0, y_0, z_0)$, biến đổi phân cấp quay ngược $-\alpha$ giữ $N$ cabin luôn thẳng đứng, hiệu ứng đèn LED `HSVtoRGB` | `src/rides/SunWheel.h` | **Thành viên 2** | Chương 1, 4, 5 | Cần phát triển ở Step 2 |
| **9** | **Các trò chơi & Kiến trúc Asia Park (`SkyTower`, `Carousel`, `NgoMonGate`)**: Tháp rơi tự do, Đu quay ngựa gỗ (quay + nhấp nhô hình sin), Cổng Ngọ Môn & Tháp Đồng Hồ | `src/rides/SkyTower.h`<br>`src/rides/Carousel.h`<br>`src/rides/NgoMonGate.h` | **Thành viên 2** | Chương 4, 5, 6 | Cần phát triển ở Step 2 |
| **10** | **Bản đồ 2D Minimap & HUD (`Minimap2D`, `HUD`)**: Cắt xén đoạn thẳng **Cohen-Sutherland** (mã vùng 4-bit), thuật toán **Bresenham/Midpoint** vẽ vòng tròn radar, chữ bitmap `glutBitmapCharacter` | `src/ui/Minimap2D.h`<br>`src/ui/HUD.h` | **Thành viên 2** | Chương 1, 3 | Cần phát triển ở Step 2 |

---

## 3. ĐỊNH HƯỚNG & GỢI Ý KIẾN THỨC LÝ THUYẾT THEO TỪNG CHỨC NĂNG

Phần này giúp từng thành viên hiểu rõ **"Tại sao mình code như vậy?"** và **"Công thức nào trong sách giáo trình được áp dụng?"** để vừa code nhanh vừa tự tin trả lời câu hỏi vấn đáp của giảng viên.

---

### PHẦN CỦA THÀNH VIÊN 1: TOÁN HỌC, QUAN SÁT 3D, ĐƯỜNG CONG BÉZIER & CHIẾU SÁNG

#### 1. Chuyển đổi mô hình màu `HSVtoRGB` (Chương 1 – `MathCG.h`)
- **Mục đích:** Tạo dải màu đèn LED chuyển sắc mượt mà trên Vòng quay Sun Wheel vào ban đêm chỉ bằng cách tăng dần góc màu $H$ từ $0^\circ \to 360^\circ$ theo thời gian.
- **Kiến thức cần nắm:**
  - Trong không gian RGB, việc đổi màu cầu vồng rất khó nếu chỉnh trực tiếp $R, G, B$. Trong không gian **HSV (Hue - Sắc thái, Saturation - Độ bão hòa, Value - Độ sáng)**, ta chỉ cần giữ cố định $S \approx 0.9, V = 1.0$ và cho $H = (H + \Delta H) \bmod 360$.
  - **Công thức chia 6 vùng (Sectors):**
    $$h = \frac{H}{60}, \quad k = \lfloor h \rfloor \in \{0,1,2,3,4,5\}, \quad f = h - k$$
    $$p = V(1 - S), \quad q = V(1 - S \cdot f), \quad t = V(1 - S(1 - f))$$
    Tùy thuộc vào $k = 0..5$, bộ $(R, G, B)$ sẽ nhận hoán vị của $(V, t, p)$ hoặc $(q, V, p)$...

#### 2. Hệ thống Quan sát 3D (3D Viewing) & Phép chiếu (Chương 4 – `Camera.h`)
- **Mục đích:** Điều khiển vị trí mắt người quan sát $O'(x, y, z)$ nhìn về tâm công viên $O(x_t, y_t, z_t)$.
- **Kiến thức cần nắm:**
  - **Tọa độ cầu (Spherical Coordinates):** Thay vì di chuyển $(x, y, z)$ rời rạc, vị trí mắt $O'$ được xác định bởi:
    - $\theta$ (Azimuth): Góc xoay ngang trên mặt phẳng $XZ$.
    - $\Phi$ (Elevation): Góc nâng cao so với mặt đất $XZ$ (giới hạn trong $[3^\circ, 87^\circ]$ để không bị lật ngược vector Up).
    - $R$: Khoảng cách từ tâm nhìn $O$ đến mắt $O'$.
    - $D$: Khoảng cách từ mắt $O'$ đến mặt phẳng chiếu (Projection Plane). Khi $D$ tăng $\Rightarrow$ góc mở $\text{FOV} = 2\arctan\left(\frac{h}{D}\right)$ giảm (hiệu ứng Telephoto zoom gần); khi $D$ giảm $\Rightarrow$ góc mở rộng (Wide-angle).
  - **Công thức tính tọa độ mắt $O'$:**
    $$O'_x = O_x + R \cos(\Phi) \sin(\theta), \quad O'_y = O_y + R \sin(\Phi), \quad O'_z = O_z + R \cos(\Phi) \cos(\theta)$$
  - **So sánh 2 phép chiếu (Câu hỏi vấn đáp thường gặp):**
    - **Phép chiếu phối cảnh (`gluPerspective`):** Các đường thẳng song song hội tụ tại điểm tụ, vật ở xa nhỏ hơn vật ở gần (dùng cho góc nhìn thực tế).
    - **Phép chiếu song song trực giao (`glOrtho`):** Các tia chiếu song song nhau, bảo toàn tỷ lệ kích thước thật bất kể khoảng cách (dùng cho bản vẽ kỹ thuật Chiếu đứng Front $XY$, Chiếu bằng Top $XZ$, Chiếu cạnh Side $ZY$).

#### 3. Phản xạ mặt phẳng qua hồ nước $y = 0$ (Chương 4 – `MathCG.h` & `main.cpp`)
- **Mục đích:** Vẽ bóng của Vòng quay Sun Wheel lộn ngược dưới mặt hồ trung tâm.
- **Kiến thức cần nắm:**
  - Phép đối xứng qua mặt phẳng nằm ngang $y = 0$ đổi dấu tọa độ $y$: $(x, y, z) \mapsto (x, -y, z)$, tương ứng ma trận tỷ lệ $S(1, -1, 1)$.
  - **Lưu ý kỹ thuật rất quan trọng khi bảo vệ:** Khi nhân ma trận phản xạ có định thức $\det(M) = -1 < 0$, chiều chuẩn của các đỉnh đa giác bị đảo ngược từ Ngược chiều kim đồng hồ (`GL_CCW`) thành Cùng chiều kim đồng hồ (`GL_CW`). Do đó trước khi vẽ ảnh phản xạ, phải gọi `glFrontFace(GL_CW)` và sau khi vẽ xong trả lại `glFrontFace(GL_CCW)` để thuật toán Back-face Culling không cắt nhầm mặt trước của bóng phản chiếu!

#### 4. Đường cong Bézier & Định hướng Tàu lượn (Chương 5 – `BezierTrack.h`, `RollerCoaster.h`)
- **Mục đích:** Tạo đường ray uốn lượn 3D mượt mà cho tàu lượn Queen Cobra và tính góc nghiêng của toa tàu tại mọi thời điểm $t \in [0, 1]$.
- **Kiến thức cần nắm:**
  - **Đa thức Bernstein bậc $n$:**
    $$P(t) = \sum_{k=0}^{n} P_k \cdot B_k^n(t), \quad \text{với } B_k^n(t) = C_n^k (1 - t)^{n - k} t^k, \quad t \in [0, 1]$$
  - **Vector tiếp tuyến (Đạo hàm bậc nhất $P'(t)$):**
    $$P'(t) = n \sum_{k=0}^{n-1} (P_{k+1} - P_k) B_k^{n-1}(t)$$
  - **Cách xoay toa tàu theo tiếp tuyến $\vec{T} = \frac{P'(t)}{\|P'(t)\|}$:**
    - Góc hướng ngang (**Yaw** quanh trục $Y$): $\text{yaw} = \text{atan2}(T_x, T_z)$.
    - Góc chúc/ngóc (**Pitch** quanh trục $X$): $\text{pitch} = -\arcsin(T_y)$.
  - **Mẹo ghép nhiều đoạn Bézier bậc 3 (Piecewise Cubic Bézier):** Để đường ray dài và khép kín không bị gãy khúc tại điểm nối giữa đoạn thứ $i$ (điểm cuối $P_3^{(i)}$) và đoạn thứ $i+1$ (điểm đầu $P_0^{(i+1)}$):
    - Liên tục $C^0$ (nối khít vị trí): $P_3^{(i)} = P_0^{(i+1)}$.
    - Liên tục $C^1$ (mượt tiếp tuyến): Điểm kiểm soát $P_3^{(i)}$ phải là trung điểm của đoạn nối $P_2^{(i)}$ và $P_1^{(i+1)}$, tức là $P_1^{(i+1)} = 2 P_3^{(i)} - P_2^{(i)}$.

#### 5. Khử mặt khuất & Mô hình Chiếu sáng / Tô bóng (Chương 6 – `Lighting.h`)
- **Kiến thức cần nắm:**
  - **Khử mặt sau (Back-face Culling):** Với vector nhìn $\vec{V} = P - O'$ (hướng từ mắt tới mặt) và pháp tuyến ngoài $\vec{N}$, nếu tích vô hướng $\vec{V} \cdot \vec{N} \ge 0$ thì mặt đó quay lưng về phía người nhìn $\Rightarrow$ loại bỏ không vẽ.
  - **Thuật toán Z-Buffer (`GL_DEPTH_TEST`):** Lưu độ sâu $z$ của từng pixel vào bộ đệm Depth Buffer; pixel mới chỉ ghi đè nếu có $z_{\text{mới}} < z_{\text{cũ}}$ (gần mắt hơn).
  - **Phương trình chiếu sáng (Illumination):**
    $$I = I_a k_a + \frac{I_s}{c_0 + c_1 D + c_2 D^2} \left[ k_d (\vec{L} \cdot \vec{N}) + k_s (\vec{R} \cdot \vec{V})^n \right]$$
    Trong đó đại lượng $\frac{1}{c_0 + c_1 D + c_2 D^2}$ chính là **suy giảm cường độ ánh sáng bậc 2 theo khoảng cách $D$** từ đèn LED tâm vòng quay Sun Wheel tới các vật thể xung quanh.
  - **Phân biệt Tô bóng hằng Lambert (`GL_FLAT`) và Tô bóng nội suy Gouraud (`GL_SMOOTH`):**
    - `GL_FLAT` (Lambert): Mỗi mặt đa giác chỉ dùng **1 pháp tuyến mặt duy nhất** $\vec{N} = \vec{A} \times \vec{B} \Rightarrow$ cả mặt có cùng 1 màu phẳng, nhìn rõ các cạnh gãy giữa các mặt.
    - `GL_SMOOTH` (Gouraud): Tại mỗi đỉnh $p$, tính **pháp tuyến đỉnh bằng trung bình cộng pháp tuyến các mặt kề** $\vec{n}_p = \frac{1}{k}\sum_{i=1}^k \vec{n}_i$, sau đó OpenGL nội suy tuyến tính màu sắc giữa các đỉnh $\Rightarrow$ bề mặt trụ/cầu/xuyến trông trơn mượt.

---

### PHẦN CỦA THÀNH VIÊN 2: MẶT CONG THAM SỐ, MÔ HÌNH PHÂN CẤP (RIDES) & ĐỒ HỌA 2D (MINIMAP/HUD)

#### 1. Sinh các Mặt cong Tham số bằng lưới $(u, v)$ (Chương 5 – `ParametricShapes.h`)
- **Nguyên tắc bắt buộc:** Không dùng `glutSolidCylinder`, `glutSolidCone`, `glutSolidSphere`, `glutSolidTorus`. Phải dùng 2 vòng lặp chia lưới tham số $u \in [u_{\min}, u_{\max}]$ và $v \in [v_{\min}, v_{\max}]$, vẽ các dải tứ giác `GL_QUAD_STRIP` kèm vector pháp tuyến `glNormal3f`.
- **4 công thức tham số cần cài đặt:**
  1. **Mặt trụ (Cylinder – Mặt kẻ):** ($u \in [0, 2\pi], v \in [0, 1]$)
     $$X(u,v) = r\cos u, \quad Y(u,v) = r\sin u, \quad Z(u,v) = h \cdot v$$
     *Pháp tuyến mặt bên:* $\vec{N}(u,v) = (\cos u, \sin u, 0)$. *(Ứng dụng: Cột trụ đỡ, nan hoa Sun Wheel, thân tháp Sky Tower).*
  2. **Mặt nón (Cone – Mặt kẻ):** ($u \in [0, 2\pi], v \in [0, 1]$, đỉnh tại $v=0$ hoặc $v=1$)
     $$X(u,v) = v \cdot r\cos u, \quad Y(u,v) = v \cdot r\sin u, \quad Z(u,v) = v \cdot h$$
     *(Ứng dụng: Mái đu quay ngựa gỗ Carousel, chóp tháp).*
  3. **Mặt cầu (Sphere – Mặt tròn xoay):** ($u \in [0, 2\pi], v \in [-\pi/2, \pi/2]$)
     $$X(u,v) = r\cos u \cos v, \quad Y(u,v) = r\sin u \cos v, \quad Z(u,v) = r\sin v$$
     *Pháp tuyến đơn vị:* $\vec{N}(u,v) = (\cos u \cos v, \sin u \cos v, \sin v)$. *(Ứng dụng: Quả cầu trang trí, tán cây, đèn lồng).*
  4. **Mặt xuyến (Toroid – Mặt tròn xoay):** ($u \in [0, 2\pi], v \in [0, 2\pi]$, bán kính lớn $R$, bán kính ống $a$)
     $$X(u,v) = (R + a\cos v)\cos u, \quad Y(u,v) = (R + a\cos v)\sin u, \quad Z(u,v) = a\sin v$$
     *Pháp tuyến đơn vị:* $\vec{N}(u,v) = (\cos v \cos u, \cos v \sin u, \sin v)$. *(Ứng dụng: Vành ngoài Vòng quay Sun Wheel, vòng ghế ngồi Sky Tower).*

#### 2. Cấu trúc dữ liệu Mô hình 3D: `WireFrame` & `PolygonMesh` (Chương 4 & 6 – `MeshModel.h`)
- **Mục đích:** Biểu diễn các công trình kiến trúc có mặt phẳng/đa giác như **Cổng Ngọ Môn**, **Tháp Đồng Hồ**, **Cabin Sun Wheel**.
- **Kiến thức cần nắm:**
  - **Mô hình khung dây (`WireFrame`):** Gồm danh sách đỉnh `vector<Vec3> vertices` và danh sách cạnh `vector<pair<int,int>> edges`.
  - **Mô hình mặt đa giác (`PolygonMesh` / `FaceModel`):**
    - Danh sách đỉnh: `vector<Vec3> vertices`.
    - Danh sách mặt: `vector<Face> faces` (mỗi mặt chứa danh sách chỉ số đỉnh `vertexIndices`, pháp tuyến mặt `faceNormal`).
    - Hàm `computeNormals()` tự động chạy 2 bước:
      1. Duyệt từng mặt, lấy 3 đỉnh đầu $P_0, P_1, P_2$, tính $\vec{A} = P_1 - P_0, \vec{B} = P_2 - P_0 \Rightarrow \vec{N}_{\text{face}} = \frac{\vec{A} \times \vec{B}}{\|\vec{A} \times \vec{B}\|}$.
      2. Duyệt từng đỉnh $p$, gom tất cả $\vec{N}_{\text{face}}$ của các mặt có chứa đỉnh $p$ rồi tính trung bình cộng $\vec{n}_p = \frac{1}{k}\sum \vec{n}_i$ để phục vụ tô bóng Gouraud (`GL_SMOOTH`).

#### 3. Biến đổi Phân cấp (Hierarchical Transformations) cho các Trò chơi (`src/rides/`)
- **Vòng quay Mặt Trời (`SunWheel.h`):**
  - Bánh xe lớn quay quanh tâm $I(x_0, y_0, z_0)$ một góc $\alpha$:
    $$M_{\text{wheel}} = T(x_0, y_0, z_0) \cdot R_z(\alpha) \cdot T(-x_0, -y_0, -z_0)$$
  - **Bài toán giữ Cabin luôn thẳng đứng:** Mỗi cabin thứ $k$ nằm ở góc nan hoa $\beta_k = k \cdot \frac{360^\circ}{N}$, cách tâm khoảng $R$. Khi bánh xe quay thêm góc $\alpha$, nan hoa bị xoay tổng cộng góc $(\beta_k + \alpha)$. Nếu gắn cứng cabin vào nan hoa, cabin sẽ bị lộn ngược khi lên đỉnh!
  - **Giải pháp phân cấp (`glPushMatrix` / `glPopMatrix`):** Tại chốt treo cabin ở đầu nan hoa $(R, 0, 0)$, ta áp dụng **phép quay ngược tại chỗ một góc $-(\beta_k + \alpha)$ quanh trục $Z$** trước khi vẽ cabin. Nhờ đó trọng lực cabin luôn hướng thẳng đứng xuống đất $(0, -1, 0)$!
- **Đu quay ngựa gỗ (`Carousel.h`):**
  - Cấp 1 (Cha): Toàn bộ sàn đu quay và mái hình nón quay quanh trục $Y$ góc $\gamma$.
  - Cấp 2 (Con): Mỗi chú ngựa thứ $i$ vừa thừa hưởng phép quay $\gamma$ của sàn, vừa tịnh tiến lên xuống theo hàm điều hòa $y_i(t) = A \sin(\omega t + \phi_i)$ dọc theo cột trụ riêng của nó.
- **Tháp rơi tự do (`SkyTower.h`):**
  - Trụ trung tâm đứng yên (hình trụ tham số cao $H$).
  - Vòng ghế ngồi (hình xuyến `Toroid`) vừa trượt dọc trục $Y$ theo chu kỳ (kéo lên chậm $\to$ dừng ở đỉnh $\to$ rơi tự do nhanh xuống $\to$ phanh êm), vừa tự xoay quanh trục $Y$.

#### 4. Bản đồ 2D Minimap, Cắt xén Cohen-Sutherland & Bresenham/Midpoint (Chương 1 & 3 – `Minimap2D.h`)
- **Mục đích:** Vẽ bản đồ radar ở góc dưới bên phải màn hình hiển thị vị trí camera, vòng quay, hồ nước và đường ray tàu lượn.
- **Kiến thức cần nắm:**
  1. **Ánh xạ tọa độ `WCS -> NCS -> DCS`:**
     - Lấy tọa độ mặt bằng 3D của công viên $(x, z) \in [x_{\min}, x_{\max}] \times [z_{\min}, z_{\max}]$.
     - Đưa về tọa độ chuẩn hóa $\text{NCS} \in [0, 1]$:
       $$x_0 = \frac{x - x_{\min}}{x_{\max} - x_{\min}}, \quad y_0 = \frac{z - z_{\min}}{z_{\max} - z_{\min}}$$
     - Đưa sang tọa độ thiết bị $\text{DCS}$ (pixel trong khung Minimap):
       $$x_d = X_{\text{vp}} + x_0 \cdot W_{\text{vp}}, \quad y_d = Y_{\text{vp}} + y_0 \cdot H_{\text{vp}}$$
  2. **Thuật toán cắt xén đoạn thẳng Cohen-Sutherland (4-bit Outcode):**
     - Mỗi đầu mút $(x, y)$ được gán mã nhị phân 4-bit so với khung chữ nhật $[x_{\min}, x_{\max}] \times [y_{\min}, y_{\max}]$:
       - `LEFT   = 1` ($0001_2$): $x < x_{\min}$
       - `RIGHT  = 2` ($0010_2$): $x > x_{\max}$
       - `BOTTOM = 4` ($0100_2$): $y < y_{\min}$
       - `TOP    = 8` ($1000_2$): $y > y_{\max}$
     - **Kiểm tra nhanh:**
       - Nếu `(code1 | code2) == 0`: Cả 2 đầu mút đều nằm trong khung $\Rightarrow$ **Chấp nhận hoàn toàn (Trivial Accept)**.
       - Nếu `(code1 & code2) != 0`: Cả 2 đầu mút cùng nằm về 1 phía ngoài khung $\Rightarrow$ **Loại bỏ hoàn toàn (Trivial Reject)**.
       - Ngược lại: Tìm giao điểm với cạnh biên tương ứng ($y = y_1 + m(x_{\text{biên}} - x_1)$ hoặc $x = x_1 + \frac{1}{m}(y_{\text{biên}} - y_1)$) và cập nhật lại đầu mút.
  3. **Thuật toán Bresenham / Midpoint vẽ đường thẳng & đường tròn:**
     - Dùng số nguyên/bán kính để chấm từng điểm `GL_POINTS` tạo các vòng tròn radar đồng tâm và tia quét trên Minimap đúng chuẩn lý thuyết Chương 3.

---

## 4. HƯỚNG DẪN BIÊN DỊCH & QUY TRÌNH PHỐI HỢP NHÓM

### Cách biên dịch chạy chương trình trên Windows (MSYS2 UCRT64)
Mở terminal PowerShell tại thư mục `d:\BTL_CG` và chạy:
```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -O2 -Wall -Wextra -Isrc src/main.cpp -o AsiaPark3D.exe -lfreeglut -lopengl32 -lglu32 -lm
.\AsiaPark3D.exe
```
*(Hoặc dùng lệnh `C:\msys64\ucrt64\bin\mingw32-make.exe run`)*.

### Quy tắc tránh xung đột code (Git) giữa 2 thành viên
1. **Thành viên 1** chỉ tạo và chỉnh sửa các file trong `src/core/` và `src/curves/`.
2. **Thành viên 2** chỉ tạo và chỉnh sửa các file trong `src/geometry/`, `src/rides/` và `src/ui/`.
3. Tất cả các module đều viết dưới dạng Header-only (`.h`) có `#ifndef ... #define ... #endif` rõ ràng. Khi hoàn thành, chỉ cần `#include` vào `src/main.cpp` tại hàm `displayCallback()` là ghép nối xong ngay lập tức!
