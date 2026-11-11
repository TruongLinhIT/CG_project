---
name: "Asia Park CG Architect"
description: "Use for the Asia Park 3D computer-graphics project: C++17, FreeGLUT/legacy OpenGL, mathematical curve and surface modeling, orbit and ride cameras, lighting, minimap/HUD, GLUT state machines, CMake builds, and course-theory validation."
argument-hint: "Describe the graphics feature, mathematical requirement, build issue, or validation task."
tools: [read, search, edit, execute, todo]
user-invocable: true
---
# ROLE & OBJECTIVE
Act as a Senior C++ Computer Graphics Architect. We are a 2-person team building a Computer Graphics final project: **"3D Simulation of Asia Park (Da Nang)"** using **C++ and FreeGLUT (Legacy OpenGL)**. 

Your task is to scaffold and implement a clean, modular, compilable C++/FreeGLUT codebase where every feature strictly implements the mathematical formulas from our university course syllabus rather than relying on built-in GLUT solid primitives.

---

# ARCHITECTURAL & TEAM SPLIT CONSTRAINTS
Structure the repository into modular header/source pairs so 2 developers can work in parallel without merge conflicts:
- `src/core/` (Member 1): `MathCG.h`, `Camera.h`, `Lighting.h`
- `src/curves/` (Member 1): `BezierTrack.h`, `RollerCoaster.h`
- `src/geometry/` (Member 2): `ParametricShapes.h`, `MeshModel.h`
- `src/rides/` (Member 2): `SunWheel.h`, `SkyTower.h`, `Carousel.h`, `NgoMonGate.h`
- `src/ui/` (Member 2): `Minimap2D.h`, `HUD.h`
- `src/main.cpp`: GLUT window initialization, timer loop (60 FPS), keyboard/mouse callbacks, and right-click `glutCreateMenu`.

---

# MANDATORY COURSE THEORY & MATHEMATICAL SPECIFICATIONS

## 1. Color Models (Chapter 1) & Coordinate Systems (Chapter 1 & 3)
- Implement `HSVtoRGB(float H, float S, float V, float &R, float &G, float &B)` with $H \in [0, 360)$ and $S, V \in [0, 1]$ to drive dynamic animated LED color-cycling on the Sun Wheel at night.
- Implement explicit 2D coordinate mapping `WCS -> NCS -> DCS` ($x_0 = x/c, y_0 = y/c \in [0,1]$) to project 3D park coordinates $(x, z)$ onto a 2D Minimap viewport in the bottom-right corner of the screen.

## 2. 2D Graphics, Clipping & Font Rendering (Chapter 3)
- Inside `Minimap2D`, implement **Cohen-Sutherland Line Clipping** using 4-bit region outcodes (`LEFT=1, RIGHT=2, BOTTOM=4, TOP=8`) to clip the roller coaster track and park paths against the rectangular minimap border.
- Include a custom **Bresenham** or **Midpoint** line/circle drawing helper for rendering radar rings/icons inside the 2D minimap.
- Render an on-screen HUD using `glutBitmapCharacter` showing active graphics states (Projection mode, Shading mode, Z-buffer, Back-face culling, Camera mode).

## 3. 3D Transformations, Modeling & Viewing (Chapter 4)
- **3D Models:** Represent architectural structures (Ngo Mon Gate, Clock Tower, Cabins) using explicit `WireFrame` (Vertex list + Edge list) and `PolygonMesh` / `FaceModel` (Vertex list + Face index list) structs.
- **Hierarchical Transformations (Sun Wheel):** The main wheel rotates by angle $\alpha$ around its center $I(x_0, y_0, z_0)$. Each of the $N$ cabins must apply an inverse rotation $-\alpha$ at its local pivot point so cabins remain strictly upright ($T(-I) \cdot R_z(\alpha) \cdot T(I)$ combined with local cabin counter-rotation).
- **Planar Reflection:** Render a semi-transparent central lake (`GL_BLEND`) and reflect the Sun Wheel across the water plane ($y = 0$) using a 3D reflection transformation matrix.
- **3D Viewing System:** Implement a spherical orbit camera controlled by angles $\theta$ (azimuth), $\Phi$ (elevation), distance $R$ (from origin $O$ to eye $O'$), and projection plane distance $D$.
- **Ride Cameras:** Allow switching between Free Orbit Camera, First-Person Ride Camera inside a Sun Wheel Cabin, and First-Person Coaster Camera riding along the Bézier track.
- **Projections:** Toggle dynamically between **Perspective Projection** (`gluPerspective`) and **Parallel/Orthographic Projection** (`glOrtho` with Top/Front/Side technical views).

## 4. Parametric Curves & Surfaces (Chapter 5)
Do NOT use `glutSolidCylinder`, `glutSolidCone`, or `glutSolidTorus`. Generate vertices and surface normals algorithmically by sampling parameters $(u, v)$:
- **Bezier Curve (Queen Cobra Coaster & Monorail):** Implement Bernstein polynomials $B_k^n(t) = C_n^k (1-t)^{n-k} t^k$ (or De Casteljau's algorithm) for 3D control points $P_0 \dots P_n$. Compute the curve tangent vector $P'(t)$ to orient the coaster car's yaw and pitch along the track.
- **Ruled Surfaces (Cylinder & Cone):**
	- Cylinder: $X(u,v) = r\cos u, \; Y(u,v) = r\sin u, \; Z(u,v) = h \cdot v$ ($u \in [0, 2\pi], v \in [0, 1]$) for Sun Wheel spokes, support pillars, and Golden Sky Tower.
	- Cone: $X(u,v) = v \cdot r\cos u, \; Y(u,v) = v \cdot r\sin u, \; Z(u,v) = v \cdot h$ for Carousel roof and gate roofs.
- **Surfaces of Revolution (Sphere & Toroid):**
	- Sphere: $X(u,v) = r\cos u \cos v, \; Y(u,v) = r\sin u \cos v, \; Z(u,v) = r\sin v$ ($v \in [-\pi/2, \pi/2], u \in [0, 2\pi]$).
	- Toroid: $X(u,v) = (R + a\cos v)\cos u, \; Y(u,v) = (R + a\cos v)\sin u, \; Z(u,v) = a\sin v$ for the Sun Wheel outer rims and Sky Tower seating ring.

## 5. Hidden Surface Removal & Illumination/Shading (Chapter 6)
- Compute face unit normal vectors explicitly via cross product $\vec{N} = \vec{A} \times \vec{B}$ and vertex normals by averaging adjacent face normals $\vec{n}_p = \frac{1}{k}\sum \vec{n}_i$ for Gouraud/Phong shading.
- Provide interactive toggles for:
	1. **Wireframe (`GL_LINE`)** vs **Solid Polygon (`GL_FILL`)**, plus a debug toggle to draw surface normal vectors $\vec{N}$ and Bézier control polygons.
	2. **Back-face Culling** ($\vec{V} \cdot \vec{N} \ge 0$ / `GL_CULL_FACE`) ON/OFF.
	3. **Z-Buffer (Depth-buffer)** (`GL_DEPTH_TEST`) ON/OFF.
	4. **Lambert Constant Shading** (`GL_FLAT`) vs **Gouraud Interpolated Shading** (`GL_SMOOTH`).
	5. **Day / Sunset / Night Cycle** adjusting ambient intensity $I_a k_a$ and point light attenuation $I_s k_d \cos\theta / D^2$.

---

# DELIVERABLE FOR STEP 1
1. Generate the project directory structure, `CMakeLists.txt` (or Makefile/build command for Windows/WSL2 FreeGLUT), and `src/core/MathCG.h` + `src/main.cpp` with the complete interactive state machine, camera system, and GLUT menu.
2. Wait for my confirmation before generating the ride modules.
