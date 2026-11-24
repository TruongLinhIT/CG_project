#ifndef MESH_MODEL_H
#define MESH_MODEL_H

/**
 * @file MeshModel.h
 * @brief Chương 4 & 6: Biểu diễn bề mặt 3D: Mô hình Khung dây (WireFrame) & Lưới đa giác (PolygonMesh)
 *
 * Triển khai các cấu trúc dữ liệu hình học nền tảng:
 *  - Chương 4 (Mô hình hóa hình học & Quan sát 3D):
 *      1. Mô hình Khung dây - WireFrame (Đỉnh + Cạnh):
 *         - vertices: std::vector<Vec3>
 *         - edges:    std::vector<std::pair<int, int>>
 *         - Vẽ bằng glLineWidth & glBegin(GL_LINES)
 *      2. Mô hình Lưới đa giác / Đỉnh-Mặt - PolygonMesh (Đỉnh + Mặt + Pháp tuyến):
 *         - vertices:      std::vector<Vec3>
 *         - vertexNormals: std::vector<Vec3> (tô bóng mịn nội suy Gouraud)
 *         - faces:         std::vector<Face> (chỉ số đỉnh, pháp tuyến mặt faceNormal, màu sắc/vật liệu tùy chọn)
 *      3. Các phép biến đổi thuần nhất 4x4:
 *         - transform(Mat4), translate(), scale(), rotateX(), rotateY(), rotateZ()
 *         - append(PolygonMesh) để ghép nối các bộ phận kiến trúc phức hợp
 *  - Chương 6 (Chiếu sáng, Tô bóng & Khử mặt khuất):
 *      1. Tự động tính toán vector pháp tuyến (computeNormals()):
 *         - Pháp tuyến mặt qua tích có hướng: N_face = (A x B) / ||A x B||
 *           với A = P1 - P0, B = P2 - P0
 *         - Trung bình cộng pháp tuyến đỉnh: n_p = (1 / k) * sum_{i=1..k}(n_i), sau đó chuẩn hóa
 *      2. Hai chế độ tô bóng:
 *         - Tô bóng phẳng - Flat Shading (GL_FLAT / Lambert): 1 pháp tuyến mặt cho mỗi đa giác
 *         - Tô bóng mịn - Smooth Shading (GL_SMOOTH / Gouraud): pháp tuyến trung bình tại mỗi đỉnh
 *      3. Hiển thị trực quan vector pháp tuyến để gỡ lỗi (Whiskers):
 *         - Vector pháp tuyến mặt màu vàng vẽ từ trọng tâm đa giác
 *         - Vector pháp tuyến đỉnh màu xanh lơ (cyan) vẽ từ các đỉnh
 *      4. Các khối kiến trúc cơ sở sinh tự động bằng thuật toán (Procedural):
 *         - Hình hộp (Box), Lăng trụ vát (Tapered Prism), Hình chóp (Pyramid), Lăng trụ đa giác (Cylinder/Prism), Vòm cổng (Arch Tunnel), Mái cong Á Đông (Asian Curved Roof)
 */

#include <GL/freeglut.h>
#include <vector>
#include <utility>
#include <cmath>
#include <string>
#include <algorithm>
#include <set>
#include "core/MathCG.h"
#include "geometry/ParametricShapes.h"

namespace Geometry {

// ============================================================================
// 1. WIREFRAME MODEL (MÔ HÌNH KHUNG DÂY - CHƯƠNG 4)
// ============================================================================

struct WireFrame {
    std::vector<MathCG::Vec3>        vertices;
    std::vector<std::pair<int, int>> edges;
    MathCG::ColorRGB                 color     = MathCG::ColorRGB(1.0f, 1.0f, 1.0f);
    float                            lineWidth = 1.0f;

    WireFrame() = default;

    void clear() {
        vertices.clear();
        edges.clear();
    }

    int addVertex(const MathCG::Vec3& v) {
        vertices.push_back(v);
        return static_cast<int>(vertices.size()) - 1;
    }

    void addEdge(int v0, int v1) {
        if (v0 != v1) {
            edges.push_back({ v0, v1 });
        }
    }

    void transform(const MathCG::Mat4& mat) {
        for (auto& v : vertices) {
            v = mat.transformPoint(v);
        }
    }

    void translate(float dx, float dy, float dz) {
        MathCG::Vec3 d(dx, dy, dz);
        for (auto& v : vertices) {
            v += d;
        }
    }

    void scale(float sx, float sy, float sz) {
        for (auto& v : vertices) {
            v.x *= sx;
            v.y *= sy;
            v.z *= sz;
        }
    }

    void draw(float r = -1.0f, float g = -1.0f, float b = -1.0f) const {
        if (edges.empty() || vertices.empty()) return;

        GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
        if (lightingWasOn) glDisable(GL_LIGHTING);

        glLineWidth(lineWidth);
        if (r >= 0.0f) {
            glColor3f(r, g, b);
        } else {
            glColor3f(color.r, color.g, color.b);
        }

        glBegin(GL_LINES);
        for (const auto& edge : edges) {
            if (edge.first >= 0 && edge.first < static_cast<int>(vertices.size()) &&
                edge.second >= 0 && edge.second < static_cast<int>(vertices.size())) {
                const auto& p0 = vertices[edge.first];
                const auto& p1 = vertices[edge.second];
                glVertex3f(p0.x, p0.y, p0.z);
                glVertex3f(p1.x, p1.y, p1.z);
            }
        }
        glEnd();

        glLineWidth(1.0f);
        if (lightingWasOn) glEnable(GL_LIGHTING);
    }
};

// ============================================================================
// 2. ĐỊNH NGHĨA MẶT ĐA GIÁC - POLYGON FACE (CHƯƠNG 4 & 6)
// ============================================================================

struct Face {
    std::vector<int> vertexIndices;               // Danh sách chỉ số tham chiếu tới PolygonMesh::vertices (ngược chiều kim đồng hồ CCW)
    MathCG::Vec3     faceNormal = MathCG::Vec3(0.0f, 1.0f, 0.0f); // Pháp tuyến mặt N = (P1-P0) x (P2-P0)
    MathCG::ColorRGB color      = MathCG::ColorRGB(1.0f, 1.0f, 1.0f);
    bool             hasCustomColor = false;

    Face() = default;

    Face(std::initializer_list<int> idxs) : vertexIndices(idxs) {}

    Face(const std::vector<int>& idxs, const MathCG::ColorRGB& col)
        : vertexIndices(idxs), color(col), hasCustomColor(true) {}

    Face(int i0, int i1, int i2, const MathCG::ColorRGB& col = MathCG::ColorRGB(1.0f, 1.0f, 1.0f), bool customCol = false)
        : vertexIndices{ i0, i1, i2 }, color(col), hasCustomColor(customCol) {}

    Face(int i0, int i1, int i2, int i3, const MathCG::ColorRGB& col = MathCG::ColorRGB(1.0f, 1.0f, 1.0f), bool customCol = false)
        : vertexIndices{ i0, i1, i2, i3 }, color(col), hasCustomColor(customCol) {}
};

// ============================================================================
// 3. MÔ HÌNH LƯỚI ĐA GIÁC - POLYGON MESH (CHƯƠNG 4 & 6)
// ============================================================================

class PolygonMesh {
public:
    std::string               name;
    std::vector<MathCG::Vec3> vertices;
    std::vector<MathCG::Vec3> vertexNormals; // Pháp tuyến đỉnh Gouraud n_p
    std::vector<Face>         faces;
    MathCG::ColorRGB          defaultColor = MathCG::ColorRGB(0.85f, 0.85f, 0.88f);
    float                     shininess    = 32.0f;
    float                     specular     = 0.35f;

    PolygonMesh() = default;
    explicit PolygonMesh(const std::string& meshName) : name(meshName) {}

    void clear() {
        vertices.clear();
        vertexNormals.clear();
        faces.clear();
    }

    int addVertex(const MathCG::Vec3& v) {
        vertices.push_back(v);
        return static_cast<int>(vertices.size()) - 1;
    }

    int addVertex(float x, float y, float z) {
        return addVertex(MathCG::Vec3(x, y, z));
    }

    void addFace(const Face& f) {
        faces.push_back(f);
    }

    void addTriangle(int i0, int i1, int i2, const MathCG::ColorRGB& col = MathCG::ColorRGB(1.0f, 1.0f, 1.0f), bool customCol = false) {
        faces.emplace_back(i0, i1, i2, col, customCol);
    }

    void addQuad(int i0, int i1, int i2, int i3, const MathCG::ColorRGB& col = MathCG::ColorRGB(1.0f, 1.0f, 1.0f), bool customCol = false) {
        faces.emplace_back(i0, i1, i2, i3, col, customCol);
    }

    // ========================================================================
    // CHƯƠNG 6: TỰ ĐỘNG TÍNH TOÁN PHÁP TUYẾN MẶT & ĐỈNH
    // ========================================================================

    /**
     * @brief Tính pháp tuyến mặt giải tích qua tích có hướng N = (P1 - P0) x (P2 - P0)
     *        và pháp tuyến đỉnh Gouraud bằng trung bình cộng các mặt kề:
     *        n_p = (1 / k) * sum_{i=1..k}(N_i)
     */
    void computeNormals() {
        if (vertices.empty()) return;

        // Bước 1: Tính vector pháp tuyến đơn vị cho từng mặt đa giác
        for (auto& face : faces) {
            if (face.vertexIndices.size() >= 3) {
                int i0 = face.vertexIndices[0];
                int i1 = face.vertexIndices[1];
                int i2 = face.vertexIndices[2];

                if (i0 < static_cast<int>(vertices.size()) &&
                    i1 < static_cast<int>(vertices.size()) &&
                    i2 < static_cast<int>(vertices.size())) {
                    face.faceNormal = MathCG::computeFaceNormal(vertices[i0], vertices[i1], vertices[i2]);
                } else {
                    face.faceNormal = MathCG::Vec3(0.0f, 1.0f, 0.0f);
                }
            } else {
                face.faceNormal = MathCG::Vec3(0.0f, 1.0f, 0.0f);
            }
        }

        // Bước 2: Tập hợp pháp tuyến của các mặt liền kề cho từng đỉnh
        std::vector<std::vector<MathCG::Vec3>> vertexAdjacentNormals(vertices.size());
        for (const auto& face : faces) {
            for (int vIdx : face.vertexIndices) {
                if (vIdx >= 0 && vIdx < static_cast<int>(vertices.size())) {
                    vertexAdjacentNormals[vIdx].push_back(face.faceNormal);
                }
            }
        }

        // Bước 3: Tính trung bình pháp tuyến đỉnh Gouraud: n_p = (1/k) * sum(n_i)
        vertexNormals.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertexNormals[i] = MathCG::averageVertexNormal(vertexAdjacentNormals[i]);
        }
    }

    // ========================================================================
    // CHƯƠNG 4: CÁC PHÉP BIẾN ĐỔI THUẦN NHẤT & GHÉP HỢP MÔ HÌNH
    // ========================================================================

    void transform(const MathCG::Mat4& mat) {
        for (auto& v : vertices) {
            v = mat.transformPoint(v);
        }
        for (auto& n : vertexNormals) {
            n = mat.transformVector(n).normalized();
        }
        for (auto& face : faces) {
            face.faceNormal = mat.transformVector(face.faceNormal).normalized();
        }
    }

    void translate(float dx, float dy, float dz) {
        MathCG::Vec3 d(dx, dy, dz);
        for (auto& v : vertices) {
            v += d;
        }
    }

    void scale(float sx, float sy, float sz) {
        for (auto& v : vertices) {
            v.x *= sx;
            v.y *= sy;
            v.z *= sz;
        }
        computeNormals();
    }

    void rotateY(float deg) {
        MathCG::Mat4 mat = MathCG::Mat4::rotateY(deg);
        transform(mat);
    }

    void rotateX(float deg) {
        MathCG::Mat4 mat = MathCG::Mat4::rotateX(deg);
        transform(mat);
    }

    void rotateZ(float deg) {
        MathCG::Mat4 mat = MathCG::Mat4::rotateZ(deg);
        transform(mat);
    }

    /**
     * @brief Ghép một lưới đa giác khác vào lưới hiện tại và tự động dịch chuyển chỉ số đỉnh
     */
    void append(const PolygonMesh& other) {
        int indexOffset = static_cast<int>(vertices.size());

        vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());
        vertexNormals.insert(vertexNormals.end(), other.vertexNormals.begin(), other.vertexNormals.end());

        for (auto f : other.faces) {
            for (auto& idx : f.vertexIndices) {
                idx += indexOffset;
            }
            faces.push_back(f);
        }
    }

    // ========================================================================
    // TRÍCH XUẤT MÔ HÌNH KHUNG DÂY - WIREFRAME (CHƯƠNG 4)
    // ========================================================================

    /**
     * @brief Trích xuất tập hợp các cạnh hình học duy nhất từ tất cả các mặt đa giác
     */
    WireFrame extractWireframe(const MathCG::ColorRGB& wireCol = MathCG::ColorRGB(1.0f, 1.0f, 1.0f), float width = 1.0f) const {
        WireFrame wf;
        wf.vertices = vertices;
        wf.color = wireCol;
        wf.lineWidth = width;

        std::set<std::pair<int, int>> uniqueEdges;
        for (const auto& face : faces) {
            size_t n = face.vertexIndices.size();
            for (size_t i = 0; i < n; ++i) {
                int v0 = face.vertexIndices[i];
                int v1 = face.vertexIndices[(i + 1) % n];
                if (v0 != v1) {
                    uniqueEdges.insert(std::minmax(v0, v1));
                }
            }
        }

        wf.edges.assign(uniqueEdges.begin(), uniqueEdges.end());
        return wf;
    }

    // ========================================================================
    // VẼ MÔ HÌNH & HIỂN THỊ VECTOR PHÁP TUYẾN KIỂM TRA (CHƯƠNG 6)
    // ========================================================================

    /**
     * @brief Vẽ lưới đa giác có chiếu sáng và tô bóng:
     * @param smoothShading true để tô bóng nội suy Gouraud (GL_SMOOTH), false để tô bóng phẳng Lambert (GL_FLAT)
     * @param drawNormals true để vẽ các đoạn thẳng biểu diễn pháp tuyến mặt (vàng) & pháp tuyến đỉnh (xanh lơ)
     */
    void draw(bool smoothShading = true, bool drawNormals = false) const {
        if (vertices.empty() || faces.empty()) return;

        bool hasVertexNormals = (vertexNormals.size() == vertices.size());
        MathCG::ColorRGB prevCol(-1.0f, -1.0f, -1.0f);

        auto emitFace = [&](const Face& face) {
            const MathCG::ColorRGB& c = face.hasCustomColor ? face.color : defaultColor;
            if (c.r != prevCol.r || c.g != prevCol.g || c.b != prevCol.b) {
                glColor3f(c.r, c.g, c.b);
                prevCol = c;
            }
            if (!smoothShading || !hasVertexNormals) {
                glNormal3f(face.faceNormal.x, face.faceNormal.y, face.faceNormal.z);
            }
            for (int vIdx : face.vertexIndices) {
                if (vIdx >= 0 && vIdx < static_cast<int>(vertices.size())) {
                    if (smoothShading && hasVertexNormals) {
                        const auto& n = vertexNormals[vIdx];
                        glNormal3f(n.x, n.y, n.z);
                    }
                    const auto& p = vertices[vIdx];
                    glVertex3f(p.x, p.y, p.z);
                }
            }
        };

        bool hasTris = false, hasQuads = false, hasPolys = false;
        for (const auto& face : faces) {
            size_t n = face.vertexIndices.size();
            if (n == 3) hasTris = true;
            else if (n == 4) hasQuads = true;
            else if (n > 4) hasPolys = true;
        }

        // Gộp hàng loạt GL_TRIANGLES và GL_QUADS để giảm glBegin/glEnd khi biên dịch Display List
        if (hasTris) {
            glBegin(GL_TRIANGLES);
            for (const auto& face : faces) {
                if (face.vertexIndices.size() == 3) emitFace(face);
            }
            glEnd();
        }
        if (hasQuads) {
            glBegin(GL_QUADS);
            for (const auto& face : faces) {
                if (face.vertexIndices.size() == 4) emitFace(face);
            }
            glEnd();
        }
        if (hasPolys) {
            for (const auto& face : faces) {
                if (face.vertexIndices.size() > 4) {
                    glBegin(GL_POLYGON);
                    emitFace(face);
                    glEnd();
                }
            }
        }

        // Hiển thị trực quan pháp tuyến để kiểm tra (phục vụ giảng dạy & bảo vệ đồ án Chương 6)
        if (drawNormals) {
            drawDebugNormals();
        }
    }

    /**
     * @brief Vẽ các đoạn thẳng màu vàng cho pháp tuyến mặt và màu xanh lơ (cyan) cho pháp tuyến đỉnh
     */
    void drawDebugNormals(float faceNormalLen = 0.6f, float vertexNormalLen = 0.45f) const {
        GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
        if (lightingWasOn) glDisable(GL_LIGHTING);

        // 1. Pháp tuyến mặt (Màu vàng từ trọng tâm mặt)
        glColor3f(1.0f, 1.0f, 0.1f);
        glBegin(GL_LINES);
        for (const auto& face : faces) {
            if (face.vertexIndices.empty()) continue;
            MathCG::Vec3 centroid(0.0f, 0.0f, 0.0f);
            for (int idx : face.vertexIndices) {
                if (idx >= 0 && idx < static_cast<int>(vertices.size())) {
                    centroid += vertices[idx];
                }
            }
            centroid = centroid / static_cast<float>(face.vertexIndices.size());
            MathCG::Vec3 tip = centroid + face.faceNormal * faceNormalLen;
            glVertex3f(centroid.x, centroid.y, centroid.z);
            glVertex3f(tip.x, tip.y, tip.z);
        }
        glEnd();

        // 2. Pháp tuyến đỉnh (Màu xanh lơ từ mỗi đỉnh)
        if (vertexNormals.size() == vertices.size()) {
            glColor3f(0.1f, 0.9f, 1.0f);
            glBegin(GL_LINES);
            for (size_t i = 0; i < vertices.size(); ++i) {
                const auto& p = vertices[i];
                MathCG::Vec3 tip = p + vertexNormals[i] * vertexNormalLen;
                glVertex3f(p.x, p.y, p.z);
                glVertex3f(tip.x, tip.y, tip.z);
            }
            glEnd();
        }

        if (lightingWasOn) glEnable(GL_LIGHTING);
    }

    // ========================================================================
    // CÁC HÀM SINH KHỐI HÌNH HỌC & CẤU TRÚC KIẾN TRÚC BẰNG THUẬT TOÁN
    // ========================================================================

    /**
     * @brief Tạo hình hộp chữ nhật song song trục tọa độ có tâm tại (cx, cy, cz) và kích thước (sx, sy, sz)
     */
    static PolygonMesh createBox(
        float cx, float cy, float cz,
        float sx, float sy, float sz,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.85f, 0.85f, 0.88f)
    ) {
        PolygonMesh mesh("Box");
        mesh.defaultColor = col;
        float hx = sx * 0.5f;
        float hy = sy * 0.5f;
        float hz = sz * 0.5f;

        // 8 đỉnh của hình hộp
        mesh.addVertex(cx - hx, cy - hy, cz + hz); // 0: Trước-Dưới-Trái
        mesh.addVertex(cx + hx, cy - hy, cz + hz); // 1: Trước-Dưới-Phải
        mesh.addVertex(cx + hx, cy + hy, cz + hz); // 2: Trước-Trên-Phải
        mesh.addVertex(cx - hx, cy + hy, cz + hz); // 3: Trước-Trên-Trái
        mesh.addVertex(cx - hx, cy - hy, cz - hz); // 4: Sau-Dưới-Trái
        mesh.addVertex(cx + hx, cy - hy, cz - hz); // 5: Sau-Dưới-Phải
        mesh.addVertex(cx + hx, cy + hy, cz - hz); // 6: Sau-Trên-Phải
        mesh.addVertex(cx - hx, cy + hy, cz - hz); // 7: Sau-Trên-Trái

        // 6 mặt tứ giác (ngược chiều kim đồng hồ CCW khi nhìn từ ngoài vào)
        mesh.addQuad(0, 1, 2, 3, col, true); // Mặt trước (+Z)
        mesh.addQuad(5, 4, 7, 6, col, true); // Mặt sau (-Z)
        mesh.addQuad(3, 2, 6, 7, col, true); // Mặt trên (+Y)
        mesh.addQuad(4, 5, 1, 0, col, true); // Mặt dưới (-Y)
        mesh.addQuad(4, 0, 3, 7, col, true); // Mặt trái (-X)
        mesh.addQuad(1, 5, 6, 2, col, true); // Mặt phải (+X)

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Tạo khối lăng trụ vát dọc theo trục Z (hữu ích cho thân toa tàu và mái vòm)
     */
    static PolygonMesh createTaperedPrism(
        float z0, float y0, float w0, float h0,
        float z1, float y1, float w1, float h1,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.88f, 0.72f, 0.15f)
    ) {
        PolygonMesh mesh("TaperedPrism");
        mesh.defaultColor = col;

        float hw0 = w0 * 0.5f, hh0 = h0 * 0.5f;
        float hw1 = w1 * 0.5f, hh1 = h1 * 0.5f;

        // Các đỉnh tứ giác mặt trước tại z1 (Z lớn hơn)
        mesh.addVertex(-hw1, y1 - hh1, z1); // 0: Dưới-Trái
        mesh.addVertex( hw1, y1 - hh1, z1); // 1: Dưới-Phải
        mesh.addVertex( hw1, y1 + hh1, z1); // 2: Trên-Phải
        mesh.addVertex(-hw1, y1 + hh1, z1); // 3: Trên-Trái

        // Các đỉnh tứ giác mặt sau tại z0 (Z nhỏ hơn)
        mesh.addVertex(-hw0, y0 - hh0, z0); // 4: Dưới-Trái
        mesh.addVertex( hw0, y0 - hh0, z0); // 5: Dưới-Phải
        mesh.addVertex( hw0, y0 + hh0, z0); // 6: Trên-Phải
        mesh.addVertex(-hw0, y0 + hh0, z0); // 7: Trên-Trái

        mesh.addQuad(0, 1, 2, 3, col, true); // Mặt trước (+Z)
        mesh.addQuad(5, 4, 7, 6, col, true); // Mặt sau (-Z)
        mesh.addQuad(3, 2, 6, 7, col, true); // Mặt trên
        mesh.addQuad(4, 5, 1, 0, col, true); // Mặt dưới
        mesh.addQuad(4, 0, 3, 7, col, true); // Mặt trái
        mesh.addQuad(1, 5, 6, 2, col, true); // Mặt phải

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Tạo hình chóp tứ giác đều (đáy nằm trên mặt phẳng XZ, đỉnh chóp hướng theo +Y)
     */
    static PolygonMesh createPyramid(
        float cx, float cy, float cz,
        float baseW, float baseD, float height,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.88f, 0.28f, 0.18f)
    ) {
        PolygonMesh mesh("Pyramid");
        mesh.defaultColor = col;
        float hw = baseW * 0.5f;
        float hd = baseD * 0.5f;

        // 4 đỉnh mặt đáy
        mesh.addVertex(cx - hw, cy, cz - hd); // 0: Sau-Trái
        mesh.addVertex(cx + hw, cy, cz - hd); // 1: Sau-Phải
        mesh.addVertex(cx + hw, cy, cz + hd); // 2: Trước-Phải
        mesh.addVertex(cx - hw, cy, cz + hd); // 3: Trước-Trái

        // Đỉnh chóp
        int apex = mesh.addVertex(cx, cy + height, cz); // 4

        // Mặt đáy dưới
        mesh.addQuad(0, 1, 2, 3, col, true);

        // 4 mặt bên hình tam giác nghiêng
        mesh.addTriangle(3, 2, apex, col, true); // Mặt trước
        mesh.addTriangle(2, 1, apex, col, true); // Mặt phải
        mesh.addTriangle(1, 0, apex, col, true); // Mặt sau
        mesh.addTriangle(0, 3, apex, col, true); // Mặt trái

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Tạo khối lăng trụ đa giác đều N cạnh / Hình trụ dọc theo trục Y
     */
    static PolygonMesh createPrismY(
        float cx, float cy, float cz,
        float radius, float height, int sides = 16,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.75f, 0.75f, 0.78f)
    ) {
        PolygonMesh mesh("PrismY");
        mesh.defaultColor = col;
        if (sides < 3) sides = 3;

        // Vòng đỉnh đáy dưới [0 .. sides-1] và đáy trên [sides .. 2*sides-1] theo chiều CCW (+Y nhìn xuống -> cz - r*sin(u))
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy, cz - radius * std::sin(u));
        }
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy + height, cz - radius * std::sin(u));
        }

        int bCenter = mesh.addVertex(cx, cy, cz);
        int tCenter = mesh.addVertex(cx, cy + height, cz);

        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            int b0 = i, b1 = iNext, t0 = sides + i, t1 = sides + iNext;
            mesh.addQuad(b0, b1, t1, t0, col, true);                      // Mặt bên CCW hướng ra ngoài
            mesh.addTriangle(bCenter, iNext, i, col, true);               // Đáy dưới (-Y)
            mesh.addTriangle(tCenter, sides + i, sides + iNext, col, true); // Đáy trên (+Y)
        }

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Vòm hầm kiến trúc (Vòm Cổng Ngọ Môn)
     *        Sinh lưới bề mặt vòm bán nguyệt nối từ z0 đến z1
     */
    static PolygonMesh createArchVault(
        float cx, float cy, float z0, float z1,
        float innerR, float outerR, int segments = 12,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.72f, 0.68f, 0.62f)
    ) {
        PolygonMesh mesh("ArchVault");
        mesh.defaultColor = col;
        if (segments < 4) segments = 4;

        // Sinh các cung bán nguyệt trong và ngoài từ góc 0 (bên phải) đến pi (bên trái)
        // tại mặt phẳng z0 (gần) và z1 (xa)
        int baseIdx = 0;
        for (int i = 0; i <= segments; ++i) {
            float th = (static_cast<float>(i) / segments) * MathCG::PI;
            float cTh = std::cos(th);
            float sTh = std::sin(th);

            // Vòng cung tại z0
            mesh.addVertex(cx + innerR * cTh, cy + innerR * sTh, z0); // baseIdx + 0: Cung trong mặt gần
            mesh.addVertex(cx + outerR * cTh, cy + outerR * sTh, z0); // baseIdx + 1: Cung ngoài mặt gần
            // Vòng cung tại z1
            mesh.addVertex(cx + innerR * cTh, cy + innerR * sTh, z1); // baseIdx + 2: Cung trong mặt xa
            mesh.addVertex(cx + outerR * cTh, cy + outerR * sTh, z1); // baseIdx + 3: Cung ngoài mặt xa

            baseIdx += 4;
        }

        for (int i = 0; i < segments; ++i) {
            int k0 = i * 4;
            int k1 = (i + 1) * 4;

            // Mặt trần vòm bên trong (nhìn từ dưới lên -> pháp tuyến hướng vào tâm)
            mesh.addQuad(k0 + 0, k0 + 2, k1 + 2, k1 + 0, col, true);

            // Mặt lưng vòm bên ngoài
            mesh.addQuad(k0 + 1, k1 + 1, k1 + 3, k0 + 3, col, true);

            // Mặt vòm phía gần (z0)
            mesh.addQuad(k0 + 0, k1 + 0, k1 + 1, k0 + 1, col, true);

            // Mặt vòm phía xa (z1)
            mesh.addQuad(k0 + 2, k0 + 3, k1 + 3, k1 + 2, col, true);
        }

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Mái cong truyền thống Á Đông 4 mái dốc đối xứng (4-sided Curved Asian Hipped Roof)
     *        Đặt trực tiếp lên thân tường/xà đỡ tại cao độ y = cy, có cổ diềm dưới khép kín
     *        và pháp tuyến mặt trên hướng chuẩn ra ngoài (+Y, CCW khi nhìn từ ngoài vào).
     */
    static PolygonMesh createCurvedAsianRoof(
        float cx, float cy, float cz,
        float baseW, float baseD, float height, float flareOut,
        const MathCG::ColorRGB& tileCol = MathCG::ColorRGB(0.92f, 0.65f, 0.12f)
    ) {
        PolygonMesh mesh("AsianCurvedRoof");
        mesh.defaultColor = tileCol;

        const int uSteps = 10;
        const int vSteps = 8;

        // Nửa kích thước tại diềm mái đua ra (ngoài) và tại cổ tường đỡ (trong)
        float hw = (baseW + flareOut) * 0.5f;
        float hd = (baseD + flareOut) * 0.5f;
        float innerHW = baseW * 0.5f;
        float innerHD = baseD * 0.5f;

        // 1. Sinh lưới mặt trên của mái 4 dốc có sống nóc ngang ở giữa (|rx| <= 0.36, rz = 0)
        for (int j = 0; j <= vSteps; ++j) {
            float v  = static_cast<float>(j) / vSteps;
            float rz = (v - 0.5f) * 2.0f; // -1 (Bắc) .. +1 (Nam)

            for (int i = 0; i <= uSteps; ++i) {
                float u  = static_cast<float>(i) / uSteps;
                float rx = (u - 0.5f) * 2.0f; // -1 (Tây) .. +1 (Đông)

                // Khoảng cách chuẩn hóa từ sống nóc trung tâm ra 4 cạnh diềm mái:
                // dEdge = 0 tại sống nóc giữa, dEdge = 1 tại cả 4 mép diềm mái Đông-Tây-Nam-Bắc
                float dx = std::max(0.0f, (std::fabs(rx) - 0.36f) / 0.64f);
                float dz = std::fabs(rz);
                float dEdge = std::clamp(std::max(dx, dz), 0.0f, 1.0f);

                // Đường cong võng truyền thống: dốc cao ở đỉnh nóc (dEdge=0) và hạ về 0 tại diềm mái (dEdge=1)
                float s = 1.0f - dEdge;
                float slopeH = (0.72f * s + 0.28f * s * s) * height;

                // Độ vểnh nhẹ ở 4 đầu đao góc mái (|rx| -> 1 và |rz| -> 1)
                float cornerFlare = (rx * rx) * (rz * rz) * 0.22f * height;

                float px = cx + rx * hw;
                float py = cy + slopeH + dEdge * cornerFlare;
                float pz = cz + rz * hd;

                mesh.addVertex(px, py, pz);
            }
        }

        // Nối các ô tứ giác mặt trên mái theo thứ tự CCW khi nhìn từ trên xuống (+Y):
        // v0(-X,-Z) -> v3(-X,+Z) -> v2(+X,+Z) -> v1(+X,-Z)
        for (int j = 0; j < vSteps; ++j) {
            for (int i = 0; i < uSteps; ++i) {
                int row0 = j * (uSteps + 1);
                int row1 = (j + 1) * (uSteps + 1);

                int v0 = row0 + i;
                int v1 = row0 + i + 1;
                int v2 = row1 + i + 1;
                int v3 = row1 + i;

                mesh.addQuad(v0, v3, v2, v1, tileCol, true);
            }
        }

        // 2. Tạo vòng cổ diềm dưới (soffit / fascia) tại cao độ y = cy ôm khít vào thân tường/dầm bên dưới
        //    để phần diềm mái đua ra và 4 góc đao vểnh lên luôn liền khối đặc với thân công trình!
        MathCG::ColorRGB soffitCol(
            std::max(0.0f, tileCol.r * 0.58f),
            std::max(0.0f, tileCol.g * 0.55f),
            std::max(0.0f, tileCol.b * 0.52f)
        );

        // Vòng đỉnh diềm dưới ngoài tại y = cy (ngay dưới mép mái đua) và cổ tường trong tại y = cy - 0.08f
        int baseDropStart = static_cast<int>(mesh.vertices.size());
        for (int j = 0; j <= vSteps; ++j) {
            float v  = static_cast<float>(j) / vSteps;
            float rz = (v - 0.5f) * 2.0f;
            for (int i = 0; i <= uSteps; ++i) {
                float u  = static_cast<float>(i) / uSteps;
                float rx = (u - 0.5f) * 2.0f;
                mesh.addVertex(cx + rx * hw, cy - 0.04f, cz + rz * hd);
            }
        }

        auto topIdx = [&](int i, int j) { return j * (uSteps + 1) + i; };
        auto botIdx = [&](int i, int j) { return baseDropStart + j * (uSteps + 1) + i; };

        // Khép kín 4 cạnh diềm mái (Bắc j=0, Nam j=vSteps, Tây i=0, Đông i=uSteps) xuống y = cy - 0.04f
        for (int i = 0; i < uSteps; ++i) {
            // Cạnh Nam (+Z, j = vSteps): nhìn từ +Z vào
            mesh.addQuad(botIdx(i, vSteps), botIdx(i + 1, vSteps), topIdx(i + 1, vSteps), topIdx(i, vSteps), soffitCol, true);
            // Cạnh Bắc (-Z, j = 0): nhìn từ -Z vào
            mesh.addQuad(botIdx(i + 1, 0), botIdx(i, 0), topIdx(i, 0), topIdx(i + 1, 0), soffitCol, true);
        }
        for (int j = 0; j < vSteps; ++j) {
            // Cạnh Tây (-X, i = 0): nhìn từ -X vào
            mesh.addQuad(botIdx(0, j), botIdx(0, j + 1), topIdx(0, j + 1), topIdx(0, j), soffitCol, true);
            // Cạnh Đông (+X, i = uSteps): nhìn từ +X vào
            mesh.addQuad(botIdx(uSteps, j + 1), botIdx(uSteps, j), topIdx(uSteps, j), topIdx(uSteps, j + 1), soffitCol, true);
        }

        // Mặt đáy phẳng dưới diềm mái (y = cy - 0.04f) nối vào thân tường (innerHW, innerHD)
        int bFL = mesh.addVertex(cx - hw, cy - 0.04f, cz + hd);
        int bFR = mesh.addVertex(cx + hw, cy - 0.04f, cz + hd);
        int bBR = mesh.addVertex(cx + hw, cy - 0.04f, cz - hd);
        int bBL = mesh.addVertex(cx - hw, cy - 0.04f, cz - hd);
        mesh.addQuad(bBL, bBR, bFR, bFL, soffitCol, true);

        // Cổ lót nối liền từ mặt dưới mái xuống thân tường (y = cy - 0.12f .. cy + 0.05f)
        auto collar = createBox(cx, cy - 0.04f, cz, innerHW * 2.0f, 0.16f, innerHD * 2.0f, soffitCol);
        mesh.append(collar);

        // 3. Thanh bờ nóc hoàng lưu ly đặt khít trên sống nóc giữa (y = cy + height)
        MathCG::ColorRGB ridgeCol(
            std::min(1.0f, tileCol.r * 1.08f + 0.05f),
            std::min(1.0f, tileCol.g * 1.08f + 0.05f),
            std::min(1.0f, tileCol.b * 1.05f + 0.02f)
        );
        auto ridgeBeam = createBox(cx, cy + height + 0.06f, cz, hw * 0.74f, 0.22f, 0.34f, ridgeCol);
        mesh.append(ridgeBeam);

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Hàm tiện ích thêm hình hộp theo biên độ cao [yBottom, yTop] đảm bảo chồng khít liền khối
     */
    static void appendBoxByYRange(
        PolygonMesh& target,
        float cx, float yBottom, float yTop, float cz,
        float sx, float sz,
        const MathCG::ColorRGB& col
    ) {
        float sy = yTop - yBottom;
        if (sy <= 1e-4f) return;
        float cy = 0.5f * (yBottom + yTop);
        target.append(createBox(cx, cy, cz, sx, sy, sz, col));
    }

    /**
     * @brief Lăng trụ bát giác đều (tâm đáy tại (cx, yBottom, cz), cao lên yTop)
     *        Độ dài từ tâm tới trung điểm mỗi cạnh phẳng = apothemRadius (mặc định xoay 22.5°
     *        để 4 mặt chính song song với 4 trục Đông/Tây/Nam/Bắc; quy ước CCW ngoài chuẩn).
     */
    static PolygonMesh createRegularOctagonalPrism(
        float cx, float yBottom, float yTop, float cz,
        float apothemRadius,
        const MathCG::ColorRGB& col,
        float rotOffsetDeg = 22.5f
    ) {
        PolygonMesh mesh("RegularOctagonalPrism");
        mesh.defaultColor = col;
        const int sides = 8;
        float outerR = apothemRadius / std::cos(MathCG::PI / 8.0f);
        float rotRad = rotOffsetDeg * MathCG::DEG2RAD;

        for (int i = 0; i < sides; ++i) {
            float a = rotRad + (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + outerR * std::cos(a), yBottom, cz - outerR * std::sin(a));
        }
        for (int i = 0; i < sides; ++i) {
            float a = rotRad + (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + outerR * std::cos(a), yTop, cz - outerR * std::sin(a));
        }
        int bCenter = mesh.addVertex(cx, yBottom, cz);
        int tCenter = mesh.addVertex(cx, yTop, cz);

        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            mesh.addQuad(i, iNext, sides + iNext, sides + i, col, true); // Mặt bên CCW hướng ra ngoài
            mesh.addTriangle(bCenter, iNext, i, col, true);             // Đáy dưới (-Y)
            mesh.addTriangle(tCenter, sides + i, sides + iNext, col, true); // Đáy trên (+Y)
        }
        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Mái bát giác cong Á Đông nhiều tầng [Bản vẽ minh họa AI, tham khảo hình dạng]
     *        Tọa độ đặt trên đỉnh tầng bát giác ở cao độ y = cy; có 8 sống đao góc vểnh lên,
     *        mặt đáy diềm khép kín hoàn toàn (soffit) và cổ lót ôm xuống bên dưới cy - 0.18m.
     */
    static PolygonMesh createOctagonalCurvedRoof(
        float cx, float cy, float cz,
        float innerApothem,
        float outerApothem,
        float height,
        float eaveLift,
        float topApothem,
        const MathCG::ColorRGB& tileCol,
        const MathCG::ColorRGB& soffitCol
    ) {
        PolygonMesh mesh("OctagonalCurvedRoof");
        mesh.defaultColor = tileCol;

        const int sides = 8;
        const int subdivPerSide = 2; // 16 góc quanh chu vi -> tạo cạnh mái hơi võng nhẹ & 8 mũi đao sắc nét
        const int ringSegs = sides * subdivPerSide;
        const int radialSteps = 4;
        const float rotRad = 22.5f * MathCG::DEG2RAD;
        const float cosPi8 = std::cos(MathCG::PI / 8.0f);

        auto octBoundaryPoint = [&](int segIdx, float scaleApothem) {
            int edge = (segIdx / subdivPerSide) % sides;
            float frac = static_cast<float>(segIdx % subdivPerSide) / subdivPerSide;
            float a0 = rotRad + (static_cast<float>(edge) / sides) * MathCG::TWO_PI;
            float a1 = rotRad + (static_cast<float>((edge + 1) % sides) / sides) * MathCG::TWO_PI;
            float cornerR = scaleApothem / cosPi8;
            float x0 = cornerR * std::cos(a0), z0 = -cornerR * std::sin(a0);
            float x1 = cornerR * std::cos(a1), z1 = -cornerR * std::sin(a1);
            return std::pair<float, float>(x0 + (x1 - x0) * frac, z0 + (z1 - z0) * frac);
        };

        // 1. Lưới mặt trên mái bát giác từ đỉnh (j = 0) ra diềm mái (j = radialSteps)
        for (int j = 0; j <= radialSteps; ++j) {
            float v = static_cast<float>(j) / radialSteps; // 0 (nóc) .. 1 (diềm ngoài)
            float curApothem = topApothem + (outerApothem - topApothem) * v;
            float baseSlopeY = cy + height * std::pow(1.0f - v, 1.48f);

            for (int k = 0; k < ringSegs; ++k) {
                bool isCorner = (k % subdivPerSide == 0);
                float flareFactor = isCorner ? 1.07f : 1.0f;
                auto xz = octBoundaryPoint(k, curApothem * (1.0f + (flareFactor - 1.0f) * v * v));
                float lift = isCorner ? (eaveLift * v * v) : (eaveLift * 0.12f * v * v);
                mesh.addVertex(cx + xz.first, baseSlopeY + lift, cz + xz.second);
            }
        }

        for (int j = 0; j < radialSteps; ++j) {
            int row0 = j * ringSegs;
            int row1 = (j + 1) * ringSegs;
            for (int k = 0; k < ringSegs; ++k) {
                int kNext = (k + 1) % ringSegs;
                // Quy ước CCW (+Y): inner0 -> outer0 -> outer1 -> inner1
                mesh.addQuad(row0 + k, row1 + k, row1 + kNext, row0 + kNext, tileCol, true);
            }
        }

        // 2. Mặt đáy diềm mái khép kín (soffit) nối từ diềm ngoài xuống cổ tường bên trong tại cy - 0.14m
        int outerRingStart = radialSteps * ringSegs;
        int innerSoffitStart = static_cast<int>(mesh.vertices.size());
        for (int k = 0; k < ringSegs; ++k) {
            auto xz = octBoundaryPoint(k, innerApothem * 0.94f);
            mesh.addVertex(cx + xz.first, cy - 0.14f, cz + xz.second);
        }
        for (int k = 0; k < ringSegs; ++k) {
            int kNext = (k + 1) % ringSegs;
            mesh.addQuad(outerRingStart + k, innerSoffitStart + k,
                         innerSoffitStart + kNext, outerRingStart + kNext, soffitCol, true);
        }

        // Nắp kín đỉnh mái trên cùng (tránh hở nếu nhìn thẳng từ trên xuống)
        int topCapCenter = mesh.addVertex(cx, cy + height, cz);
        for (int k = 0; k < ringSegs; ++k) {
            int kNext = (k + 1) % ringSegs;
            mesh.addTriangle(topCapCenter, k, kNext, tileCol, true);
        }

        // 3. Gờ dầm cổ mái bát giác ôm chặt vào đỉnh tường bên dưới (cy - 0.22m .. cy + 0.08m)
        mesh.append(createRegularOctagonalPrism(
            cx, cy - 0.22f, cy + 0.08f, cz,
            innerApothem * 1.03f, soffitCol, 22.5f
        ));

        // 4. 8 Đường sống gờ sườn mái nổi uốn cong theo ĐÚNG đường cong mái (không hở cung)
        MathCG::ColorRGB ridgeCol(
            std::min(1.0f, tileCol.r * 1.12f + 0.04f),
            std::min(1.0f, tileCol.g * 1.10f + 0.04f),
            std::min(1.0f, tileCol.b * 1.05f + 0.02f)
        );
        for (int s = 0; s < sides; ++s) {
            int cornerSeg = s * subdivPerSide;
            for (int j = 0; j < radialSteps; ++j) {
                int vIdx0 = j * ringSegs + cornerSeg;
                int vIdx1 = (j + 1) * ringSegs + cornerSeg;
                const auto& p0 = mesh.vertices[vIdx0];
                const auto& p1 = mesh.vertices[vIdx1];
                float dx = p1.x - p0.x, dy = p1.y - p0.y, dz = p1.z - p0.z;
                float segLen = std::sqrt(dx * dx + dy * dy + dz * dz);
                float yaw = std::atan2(dx, dz) * MathCG::RAD2DEG;
                float pitch = -std::atan2(dy, std::sqrt(dx * dx + dz * dz)) * MathCG::RAD2DEG;
                PolygonMesh rib = createBox(0.0f, 0.01f, 0.0f, 0.16f, 0.18f, segLen * 1.04f, ridgeCol);
                rib.rotateX(pitch);
                rib.rotateY(yaw);
                rib.translate(0.5f * (p0.x + p1.x), 0.5f * (p0.y + p1.y), 0.5f * (p0.z + p1.z));
                mesh.append(rib);
            }
        }

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Hàm dùng lại xây dựng cầu thang đá đặc nguyên khối từ chân bệ (tại distFromCenter0, yBottom)
     *        leo dần vào/lên mặt sàn tại (distFromCenter1, yTop) theo góc xoay yawDeg quanh trục Y.
     *        Gồm toàn bộ bậc bước chồng kín, thân dầm đỡ đặc xuống nền và 2 thành lan can vát 2 bên.
     */
    static PolygonMesh createSteppedStaircase(
        float baseX, float baseZ, float yawDeg,
        float yBottom, float yTop,
        float horizRun, float totalWidth, int numSteps,
        const MathCG::ColorRGB& stepCol,
        const MathCG::ColorRGB& railCol
    ) {
        PolygonMesh stair("SteppedStaircase");
        if (numSteps < 2) numSteps = 2;
        float railW = std::min(0.42f, totalWidth * 0.14f);
        float treadW = totalWidth - 2.0f * railW;
        float stepRun = horizRun / static_cast<float>(numSteps);
        float riseH = yTop - yBottom;
        float stepRise = riseH / static_cast<float>(numSteps);

        // Chiều đi lên cầu thang trong tọa độ cục bộ: đi theo trục -Z từ z = horizRun (chân đất) về z = 0 (sân trên)
        for (int i = 0; i < numSteps; ++i) {
            float frac = static_cast<float>(i + 1) / static_cast<float>(numSteps);
            float curTopY = yBottom + riseH * frac;
            float zCenter = horizRun - (i + 0.5f) * stepRun;
            appendBoxByYRange(
                stair, 0.0f, yBottom, curTopY, zCenter,
                treadW, stepRun + 0.08f, stepCol
            );
            // Hai thành đá lan can bậc thang bậc cấp bám sát bậc bước (+-X)
            for (int side : {-1, 1}) {
                appendBoxByYRange(
                    stair, side * (treadW * 0.5f + railW * 0.5f),
                    yBottom, curTopY + 0.65f, zCenter,
                    railW, stepRun + 0.08f, railCol
                );
            }
        }
        // Hai trụ cột vuông nhỏ ở chân cầu thang
        for (int side : {-1, 1}) {
            appendBoxByYRange(
                stair, side * (treadW * 0.5f + railW * 0.5f),
                yBottom, yBottom + stepRise + 1.05f, horizRun + 0.18f,
                railW * 1.35f, railW * 1.35f, railCol
            );
        }

        stair.rotateY(yawDeg);
        stair.translate(baseX, 0.0f, baseZ);
        stair.computeNormals();
        return stair;
    }

    /**
     * @brief Hàm dùng lại tạo Chỏm Hoa Sen 3D nhiều lớp cánh (Golden Sky Tower crown)
     *        [Bản vẽ minh họa AI, tham khảo hình dạng]: búp đài sen gắn liền tại baseY - 0.25m,
     *        vươn cao lên baseY + totalHeight với 2 tầng cánh sen uốn cong & đài nhụy nhọn.
     */
    static PolygonMesh createLotusCrown(
        float cx, float baseY, float cz,
        float shaftRadius, float totalHeight, int petalsPerRing,
        const MathCG::ColorRGB& petalGoldCol,
        const MathCG::ColorRGB& innerGoldCol
    ) {
        PolygonMesh lotus("LotusCrown");
        lotus.defaultColor = petalGoldCol;
        if (petalsPerRing < 6) petalsPerRing = 6;

        // 1. Cổ đài sen nối khít vào thân cột tại baseY - 0.25m .. baseY + 1.15m
        lotus.append(createPrismY(cx, baseY - 0.25f, cz, shaftRadius * 1.18f, 0.75f, 16, innerGoldCol));
        lotus.append(createPrismY(cx, baseY + 0.45f, cz, shaftRadius * 1.36f, 0.55f, 16, petalGoldCol));

        // 2. Hai vòng cánh hoa sen 3D uốn cong (cánh ngoài xòe rộng, cánh trong khum ôm búp)
        auto addPetalRing = [&](int count, float angleOffsetRad, float yStart, float petalH,
                                float rBase, float rMidFlare, float rTip, float halfWidthBase,
                                const MathCG::ColorRGB& col) {
            const float thickness = 0.16f;
            for (int p = 0; p < count; ++p) {
                float a = angleOffsetRad + (static_cast<float>(p) / count) * MathCG::TWO_PI;
                PolygonMesh petal("LotusPetal");

                // Dựng 1 cánh sen cong 3D theo trục cục bộ +Z (xòe ra) và +Y (lên cao), khép kín 2 mặt dày
                const int segs = 4;
                const float yFracs[5]   = { 0.0f, 0.28f, 0.62f, 0.86f, 1.0f };
                const float rVals[5]    = { rBase, rMidFlare, rMidFlare * 0.98f, (rMidFlare + rTip) * 0.52f, rTip };
                const float wVals[5]    = { halfWidthBase * 0.72f, halfWidthBase, halfWidthBase * 0.85f, halfWidthBase * 0.36f, 0.04f };

                for (int s = 0; s <= segs; ++s) {
                    float py = yStart + petalH * yFracs[s];
                    float pr = rVals[s];
                    float hw = wVals[s];
                    float tIn = pr - thickness;
                    petal.addVertex(-hw, py, pr);   // s*4 + 0: ngoài trái
                    petal.addVertex( hw, py, pr);   // s*4 + 1: ngoài phải
                    petal.addVertex( hw, py, tIn);  // s*4 + 2: trong phải
                    petal.addVertex(-hw, py, tIn);  // s*4 + 3: trong trái
                }
                for (int s = 0; s < segs; ++s) {
                    int b = s * 4;
                    int n = (s + 1) * 4;
                    petal.addQuad(b + 0, b + 1, n + 1, n + 0, col, true); // Mặt ngoài cánh (+Z)
                    petal.addQuad(b + 2, b + 3, n + 3, n + 2, col, true); // Mặt trong cánh (-Z)
                    petal.addQuad(b + 3, b + 0, n + 0, n + 3, col, true); // Mép trái
                    petal.addQuad(b + 1, b + 2, n + 2, n + 1, col, true); // Mép phải
                }
                petal.addQuad(0, 3, 2, 1, col, true);                     // Đáy cánh
                int top = segs * 4;
                petal.addQuad(top + 0, top + 1, top + 2, top + 3, col, true); // Đỉnh cánh

                petal.rotateY(a * MathCG::RAD2DEG);
                petal.translate(cx, 0.0f, cz);
                lotus.append(petal);
            }
        };

        // Tầng cánh ngoài (cắm sâu từ đài sen y = baseY + 0.45m, xòe ra ~3.0m)
        addPetalRing(
            petalsPerRing, 0.0f,
            baseY + 0.48f, totalHeight * 0.60f,
            shaftRadius * 1.12f, shaftRadius * 1.80f, shaftRadius * 1.28f,
            shaftRadius * 0.44f, petalGoldCol
        );
        // Tầng cánh trong so le (vươn cao hơn ~76% chiều cao chỏm sen, ôm chặt búp sen)
        addPetalRing(
            petalsPerRing, MathCG::PI / static_cast<float>(petalsPerRing),
            baseY + 0.85f, totalHeight * 0.68f,
            shaftRadius * 0.92f, shaftRadius * 1.38f, shaftRadius * 0.56f,
            shaftRadius * 0.36f, innerGoldCol
        );

        // 3. Búp nhụy sen giật cấp ở chính giữa vươn lên hết chiều cao totalHeight
        lotus.append(createPrismY(cx, baseY + 0.85f, cz, shaftRadius * 0.88f, totalHeight * 0.54f, 14, innerGoldCol));
        lotus.append(createPrismY(cx, baseY + 0.85f + totalHeight * 0.50f, cz, shaftRadius * 0.60f, totalHeight * 0.18f, 14, petalGoldCol));
        lotus.append(createPyramid(
            cx, baseY + 0.85f + totalHeight * 0.66f, cz,
            shaftRadius * 0.96f, shaftRadius * 0.96f, totalHeight * 0.20f, petalGoldCol
        ));

        lotus.computeNormals();
        return lotus;
    }

    /**
     * @brief Hàm dùng lại tạo Mái yên ngựa cong kiểu Minangkabau (Indonesia)
     *        [Bản vẽ minh họa AI, tham khảo hình dạng]: hai đầu sống nóc vểnh cao thành sừng trâu,
     *        bụng mái võng xuống ở giữa, vách tam giác gỗ đầu hồi khép kín và mào nóc trang trí.
     */
    static PolygonMesh createMinangkabauRoof(
        float cx, float cy, float cz,
        float baseW, float baseD,
        float midSaddleH, float hornPeakH, float hornFlareLen,
        int numCrestSpikes,
        const MathCG::ColorRGB& roofCol,
        const MathCG::ColorRGB& trimCol,
        float rotYDeg = 0.0f
    ) {
        PolygonMesh roof("MinangkabauSaddleRoof");
        roof.defaultColor = roofCol;

        const int uSteps = 6;
        const int vSteps = 8;
        const float hw = baseW * 0.5f;
        const float hd = baseD * 0.5f;
        const float thick = 0.16f;

        auto evalRoofXZ = [&](float rx, float rz, float& outX, float& outY, float& outZ) {
            float vAbs = std::fabs(rz);
            float ridgeY = midSaddleH + (hornPeakH - midSaddleH) * std::pow(vAbs, 2.15f);
            float eaveLiftY = 0.28f * hornPeakH * vAbs * vAbs;
            float sideCurve = std::pow(std::max(0.0f, 1.0f - std::fabs(rx)), 1.45f);
            float flare = hornFlareLen * (1.0f - 0.72f * rx * rx) * std::pow(vAbs, 1.75f);
            outX = rx * hw;
            outY = cy + eaveLiftY + (ridgeY - eaveLiftY) * sideCurve;
            outZ = (hd + flare) * rz;
        };

        const int ringStride = uSteps + 1;
        // 1. Mặt trên mái (+Y) [0 .. (vSteps+1)*ringStride - 1]
        for (int j = 0; j <= vSteps; ++j) {
            float rz = (static_cast<float>(j) / vSteps - 0.5f) * 2.0f;
            for (int i = 0; i <= uSteps; ++i) {
                float rx = (static_cast<float>(i) / uSteps - 0.5f) * 2.0f;
                float x, y, z;
                evalRoofXZ(rx, rz, x, y, z);
                roof.addVertex(x, y, z);
            }
        }
        const int botOffset = static_cast<int>(roof.vertices.size());
        // 2. Mặt dưới diềm mái (-Y, dày 0.16m)
        for (int j = 0; j <= vSteps; ++j) {
            float rz = (static_cast<float>(j) / vSteps - 0.5f) * 2.0f;
            for (int i = 0; i <= uSteps; ++i) {
                float rx = (static_cast<float>(i) / uSteps - 0.5f) * 2.0f;
                float x, y, z;
                evalRoofXZ(rx, rz, x, y, z);
                roof.addVertex(x, y - thick, z);
            }
        }

        auto tIdx = [&](int i, int j) { return j * ringStride + i; };
        auto bIdx = [&](int i, int j) { return botOffset + j * ringStride + i; };

        for (int j = 0; j < vSteps; ++j) {
            for (int i = 0; i < uSteps; ++i) {
                roof.addQuad(tIdx(i, j), tIdx(i, j + 1), tIdx(i + 1, j + 1), tIdx(i + 1, j), roofCol, true);
                roof.addQuad(bIdx(i, j), bIdx(i + 1, j), bIdx(i + 1, j + 1), bIdx(i, j + 1), trimCol, true);
            }
        }
        // Khép kín viền mép diềm trái/phải và 2 đầu hồi Bắc/Nam
        for (int j = 0; j < vSteps; ++j) {
            roof.addQuad(bIdx(0, j), bIdx(0, j + 1), tIdx(0, j + 1), tIdx(0, j), trimCol, true);
            roof.addQuad(bIdx(uSteps, j + 1), bIdx(uSteps, j), tIdx(uSteps, j), tIdx(uSteps, j + 1), trimCol, true);
        }
        for (int i = 0; i < uSteps; ++i) {
            roof.addQuad(bIdx(i + 1, 0), bIdx(i, 0), tIdx(i, 0), tIdx(i + 1, 0), trimCol, true);
            roof.addQuad(bIdx(i, vSteps), bIdx(i + 1, vSteps), tIdx(i + 1, vSteps), tIdx(i, vSteps), trimCol, true);
        }

        // 3. Vách gỗ tam giác đặc kín tại 2 đầu hồi Bắc (j=0) & Nam (j=vSteps) từ cao độ xà y = cy lên sống mái
        //    Giúp mặt dưới đầu hồi không hở trống và gắn khít lên cột nhà ga bên dưới
        for (int endIdx : { 0, vSteps }) {
            bool isSouth = (endIdx == vSteps);
            float rz = isSouth ? 1.0f : -1.0f;
            float baseZ = rz * hd * 0.94f;
            int gBaseStart = static_cast<int>(roof.vertices.size());
            for (int i = 0; i <= uSteps; ++i) {
                float rx = (static_cast<float>(i) / uSteps - 0.5f) * 2.0f;
                roof.addVertex(rx * hw * 0.94f, cy - 0.14f, baseZ);
            }
            int gBackStart = static_cast<int>(roof.vertices.size());
            for (int i = 0; i <= uSteps; ++i) {
                float rx = (static_cast<float>(i) / uSteps - 0.5f) * 2.0f;
                roof.addVertex(rx * hw * 0.94f, cy - 0.14f, baseZ + (isSouth ? -0.22f : 0.22f));
            }
            for (int i = 0; i < uSteps; ++i) {
                if (isSouth) {
                    roof.addQuad(gBaseStart + i, gBaseStart + i + 1, bIdx(i + 1, endIdx), bIdx(i, endIdx), trimCol, true);
                    roof.addQuad(gBackStart + i + 1, gBackStart + i, bIdx(i, endIdx), bIdx(i + 1, endIdx), trimCol, true);
                } else {
                    roof.addQuad(gBaseStart + i + 1, gBaseStart + i, bIdx(i, endIdx), bIdx(i + 1, endIdx), trimCol, true);
                    roof.addQuad(gBackStart + i, gBackStart + i + 1, bIdx(i + 1, endIdx), bIdx(i, endIdx), trimCol, true);
                }
            }
        }

        // 4. Mào trang trí chạm trổ dọc sống mái yên ngựa (theo bản vẽ #16)
        MathCG::ColorRGB crestCol(0.95f, 0.78f, 0.20f);
        if (numCrestSpikes > 0) {
            for (int k = 0; k < numCrestSpikes; ++k) {
                float rz = (numCrestSpikes == 1) ? 0.0f : (-0.82f + 1.64f * (static_cast<float>(k) / (numCrestSpikes - 1)));
                float rx, ry, rzz;
                evalRoofXZ(0.0f, rz, rx, ry, rzz);
                float spikeH = 0.42f + 0.32f * std::fabs(rz);
                roof.append(createPyramid(0.0f, ry - 0.12f, rzz, 0.24f, 0.34f, spikeH, crestCol));
            }
        }

        if (std::fabs(rotYDeg) > 1e-3f) roof.rotateY(rotYDeg);
        roof.translate(cx, 0.0f, cz);
        roof.computeNormals();
        return roof;
    }

    /**
     * @brief Hàm dùng lại tạo Mái gỗ 2 dốc có vì kèo tam giác (Port of Sky Treasure / nhà ga bến cảng gỗ)
     *        [Bản vẽ minh họa AI, tham khảo hình dạng]: hai mái nghiêng kín, thanh bờ nóc và
     *        các khung vì kèo tam giác (king-post truss) tại các nhịp đầu hồi.
     */
    static PolygonMesh createGableRoofWithTruss(
        float cx, float cy, float cz,
        float spanW, float lengthD,
        float ridgeH, float eaveOverhang,
        const MathCG::ColorRGB& roofCol,
        const MathCG::ColorRGB& woodCol,
        const MathCG::ColorRGB& trussCol,
        int numTrusses = 3,
        float rotYDeg = 0.0f
    ) {
        PolygonMesh mesh("GableRoofWithTruss");
        mesh.defaultColor = roofCol;

        const float hs = spanW * 0.5f + eaveOverhang;
        const float hl = lengthD * 0.5f + eaveOverhang;
        const float rafterLen = std::sqrt(hs * hs + ridgeH * ridgeH);
        const float slopeAngleDeg = std::atan2(ridgeH, hs) * MathCG::RAD2DEG;
        const float thick = 0.16f;

        // 1. Hai cánh mái 2 dốc (mặt phẳng kép kín từ đỉnh nóc xuống diềm mái trái/phải)
        for (int side : {-1, 1}) {
            PolygonMesh slab = createBox(0.0f, 0.0f, 0.0f, rafterLen + 0.08f, thick, hl * 2.0f, roofCol);
            slab.rotateZ(-side * slopeAngleDeg);
            slab.translate(side * hs * 0.5f, cy + ridgeH * 0.5f, 0.0f);
            mesh.append(slab);
        }
        // Thanh bờ nóc trên đỉnh giao hai mái
        mesh.append(createBox(0.0f, cy + ridgeH + 0.08f, 0.0f, 0.30f, 0.22f, hl * 2.0f + 0.06f, woodCol));

        // 2. Hệ vì kèo gỗ tam giác lộ thiên (Bottom Tie Beam + King-Post + Diagonal Struts + Rafters)
        const int trusses = std::max(2, numTrusses);
        const float trussSpan = spanW;
        const float tHS = trussSpan * 0.5f;
        const float tAngleDeg = std::atan2(ridgeH, tHS) * MathCG::RAD2DEG;
        const float tRafterLen = std::sqrt(tHS * tHS + ridgeH * ridgeH);

        for (int t = 0; t < trusses; ++t) {
            float tz = -lengthD * 0.46f + (lengthD * 0.92f) * (static_cast<float>(t) / (trusses - 1));
            // Xà ngang đáy vì kèo (tie beam)
            mesh.append(createBox(0.0f, cy + 0.04f, tz, trussSpan + 0.10f, 0.20f, 0.22f, trussCol));
            // Thanh trụ đứng giữa (king-post) từ xà ngang lên đỉnh nóc
            appendBoxByYRange(mesh, 0.0f, cy + 0.04f, cy + ridgeH + 0.02f, tz, 0.20f, 0.20f, trussCol);
            // Hai thanh kèo chéo chính + hai thanh chống xiên bên trong tam giác vì kèo
            for (int side : {-1, 1}) {
                PolygonMesh rafter = createBox(0.0f, 0.0f, 0.0f, tRafterLen, 0.16f, 0.18f, woodCol);
                rafter.rotateZ(-side * tAngleDeg);
                rafter.translate(side * tHS * 0.5f, cy + ridgeH * 0.5f - 0.06f, tz);
                mesh.append(rafter);

                float strutW = std::sqrt((tHS * 0.48f) * (tHS * 0.48f) + (ridgeH * 0.48f) * (ridgeH * 0.48f));
                float strutAng = std::atan2(ridgeH * 0.48f, tHS * 0.48f) * MathCG::RAD2DEG;
                PolygonMesh strut = createBox(0.0f, 0.0f, 0.0f, strutW, 0.13f, 0.14f, trussCol);
                strut.rotateZ(side * strutAng);
                strut.translate(side * tHS * 0.26f, cy + ridgeH * 0.28f, tz);
                mesh.append(strut);
            }
        }

        if (std::fabs(rotYDeg) > 1e-3f) mesh.rotateY(rotYDeg);
        mesh.translate(cx, 0.0f, cz);
        mesh.computeNormals();
        return mesh;
    }
};

} // namespace Geometry

#endif // MESH_MODEL_H
