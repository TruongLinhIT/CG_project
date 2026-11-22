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

        for (const auto& face : faces) {
            if (face.vertexIndices.size() < 3) continue;

            if (face.hasCustomColor) {
                glColor3f(face.color.r, face.color.g, face.color.b);
            } else {
                glColor3f(defaultColor.r, defaultColor.g, defaultColor.b);
            }

            // Ở chế độ tô bóng phẳng (Flat Shading) hoặc khi chưa có pháp tuyến đỉnh, truyền 1 pháp tuyến mặt
            if (!smoothShading || !hasVertexNormals) {
                glNormal3f(face.faceNormal.x, face.faceNormal.y, face.faceNormal.z);
            }

            GLenum mode = GL_POLYGON;
            if (face.vertexIndices.size() == 3) mode = GL_TRIANGLES;
            else if (face.vertexIndices.size() == 4) mode = GL_QUADS;

            glBegin(mode);
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
            glEnd();
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

        // Vòng đỉnh đáy dưới [0 .. sides-1]
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy, cz + radius * std::sin(u));
        }

        // Vòng đỉnh đáy trên [sides .. 2*sides-1]
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy + height, cz + radius * std::sin(u));
        }

        // Đỉnh tâm đáy dưới (2*sides) & Đỉnh tâm đáy trên (2*sides + 1)
        int bCenter = mesh.addVertex(cx, cy, cz);
        int tCenter = mesh.addVertex(cx, cy + height, cz);

        // Các mặt tứ giác xung quanh thân lăng trụ
        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            int b0 = i;
            int b1 = iNext;
            int t0 = sides + i;
            int t1 = sides + iNext;
            mesh.addQuad(b0, b1, t1, t0, col, true);
        }

        // Nắp đáy dưới & nắp đáy trên ghép từ các tam giác chung tâm
        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            mesh.addTriangle(bCenter, (i + 1) % sides, i, col, true);       // Đáy dưới (pháp tuyến -Y)
            mesh.addTriangle(tCenter, sides + i, sides + iNext, col, true);  // Đáy trên (pháp tuyến +Y)
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
     * @brief Mái cong truyền thống Á Đông (Mái cong Cổng Ngọ Môn)
     *        Sinh lưới mái ngói uốn cong vểnh lên ở bốn góc bằng hàm bậc hai.
     */
    static PolygonMesh createCurvedAsianRoof(
        float cx, float cy, float cz,
        float baseW, float baseD, float height, float flareOut,
        const MathCG::ColorRGB& tileCol = MathCG::ColorRGB(0.92f, 0.65f, 0.12f)
    ) {
        PolygonMesh mesh("AsianCurvedRoof");
        mesh.defaultColor = tileCol;

        const int uSteps = 8;
        const int vSteps = 6;

        float hw = (baseW + flareOut) * 0.5f;
        float hd = (baseD + flareOut) * 0.5f;

        // Sinh lưới bề mặt mái cong với độ uốn vểnh lên về phía các góc mái
        for (int j = 0; j <= vSteps; ++j) {
            float v = static_cast<float>(j) / vSteps; // 0 (diềm mái) -> 1 (đỉnh nóc mái)
            float rz = (v - 0.5f) * 2.0f;             // -1 .. 1

            for (int i = 0; i <= uSteps; ++i) {
                float u = static_cast<float>(i) / uSteps;
                float rx = (u - 0.5f) * 2.0f;         // -1 .. 1

                // Độ uốn cong bậc hai: bốn góc mái vểnh cao lên
                float cornerLift = (rx * rx + rz * rz) * 0.35f * height;
                float ridgeTaper = (1.0f - v * 0.72f);

                float px = cx + rx * hw * ridgeTaper;
                float py = cy + v * height + (1.0f - v) * cornerLift;
                float pz = cz + rz * hd * ridgeTaper;

                mesh.addVertex(px, py, pz);
            }
        }

        for (int j = 0; j < vSteps; ++j) {
            for (int i = 0; i < uSteps; ++i) {
                int row0 = j * (uSteps + 1);
                int row1 = (j + 1) * (uSteps + 1);

                int v0 = row0 + i;
                int v1 = row0 + i + 1;
                int v2 = row1 + i + 1;
                int v3 = row1 + i;

                mesh.addQuad(v0, v1, v2, v3, tileCol, true);
            }
        }

        mesh.computeNormals();
        return mesh;
    }
};

} // namespace Geometry

#endif // MESH_MODEL_H
