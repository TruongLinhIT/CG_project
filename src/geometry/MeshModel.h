#ifndef MESH_MODEL_H
#define MESH_MODEL_H

/**
 * @file MeshModel.h
 * @brief Chapter 4 & 6: 3D Surface Representation: WireFrame & PolygonMesh Models
 *
 * Implements fundamental geometric data structures:
 *  - Chapter 4 (3D Geometric Modeling & Viewing):
 *      1. WireFrame Model (Đỉnh + Cạnh):
 *         - vertices: std::vector<Vec3>
 *         - edges:    std::vector<std::pair<int, int>>
 *         - Renders with glLineWidth & glBegin(GL_LINES)
 *      2. PolygonMesh / Face-Vertex Model (Đỉnh + Mặt + Pháp tuyến):
 *         - vertices:      std::vector<Vec3>
 *         - vertexNormals: std::vector<Vec3> (Gouraud smooth shading)
 *         - faces:         std::vector<Face> (indices, faceNormal, optional color/material)
 *      3. Homogeneous 4x4 Transformations:
 *         - transform(Mat4), translate(), scale(), rotateX(), rotateY(), rotateZ()
 *         - append(PolygonMesh) for multi-part architectural assemblies
 *  - Chapter 6 (Illumination, Shading & Hidden Surface Removal):
 *      1. Automatic Normal Computation (computeNormals()):
 *         - Face normal via cross product: N_face = (A x B) / ||A x B||
 *           where A = P1 - P0, B = P2 - P0
 *         - Vertex normal averaging: n_p = (1 / k) * sum_{i=1..k}(n_i), then normalized
 *      2. Dual Shading Modes:
 *         - Flat Shading (GL_FLAT / Lambert): 1 face normal per polygon
 *         - Smooth Shading (GL_SMOOTH / Gouraud): per-vertex averaged normal
 *      3. Debug Normal Visualization (Whiskers):
 *         - Yellow face normal vectors from centroid
 *         - Cyan vertex normal vectors from vertices
 *      4. Procedural Architectural Primitives:
 *         - Box, Tapered Prism, Pyramid, Cylinder/Prism, Arch Tunnel, Asian Curved Roof
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
// 2. POLYGON FACE DEFINITION (MẶT ĐA GIÁC - CHƯƠNG 4 & 6)
// ============================================================================

struct Face {
    std::vector<int> vertexIndices;               // Indices referencing PolygonMesh::vertices (CCW)
    MathCG::Vec3     faceNormal = MathCG::Vec3(0.0f, 1.0f, 0.0f); // N = (P1-P0) x (P2-P0)
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
// 3. POLYGON MESH (MÔ HÌNH LƯỚI ĐA GIÁC - CHƯƠNG 4 & 6)
// ============================================================================

class PolygonMesh {
public:
    std::string               name;
    std::vector<MathCG::Vec3> vertices;
    std::vector<MathCG::Vec3> vertexNormals; // Gouraud vertex normals n_p
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
    // CHAPTER 6: AUTOMATIC NORMAL COMPUTATION (PHÁP TUYẾN MẶT & ĐỈNH)
    // ========================================================================

    /**
     * @brief Computes analytical face normals via cross product N = (P1 - P0) x (P2 - P0)
     *        and Gouraud vertex normals via adjacent face averaging:
     *        n_p = (1 / k) * sum_{i=1..k}(N_i)
     */
    void computeNormals() {
        if (vertices.empty()) return;

        // Step 1: Compute unit normal vector for every polygon face
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

        // Step 2: Accumulate adjacent face normals for each vertex
        std::vector<std::vector<MathCG::Vec3>> vertexAdjacentNormals(vertices.size());
        for (const auto& face : faces) {
            for (int vIdx : face.vertexIndices) {
                if (vIdx >= 0 && vIdx < static_cast<int>(vertices.size())) {
                    vertexAdjacentNormals[vIdx].push_back(face.faceNormal);
                }
            }
        }

        // Step 3: Compute Gouraud vertex normal average: n_p = (1/k) * sum(n_i)
        vertexNormals.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertexNormals[i] = MathCG::averageVertexNormal(vertexAdjacentNormals[i]);
        }
    }

    // ========================================================================
    // CHAPTER 4: HOMOGENEOUS TRANSFORMATIONS & COMPOSITION
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
     * @brief Merges another polygon mesh into this mesh, offsetting indices
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
    // EXTRACTION: WIREFRAME MODEL (CHƯƠNG 4)
    // ========================================================================

    /**
     * @brief Extracts unique topological edges from all polygon faces
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
    // RENDERING & DEBUG NORMAL VISUALIZATION (CHƯƠNG 6)
    // ========================================================================

    /**
     * @brief Render the polygon mesh with lighting & shading:
     * @param smoothShading true for Gouraud interpolation (GL_SMOOTH), false for Lambert Flat (GL_FLAT)
     * @param drawNormals true to draw yellow face normal & cyan vertex normal whiskers
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

            // In Flat Shading or when vertex normals are absent, emit 1 face normal
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

        // Debug visualization of normals (Chapter 6 teaching & defense)
        if (drawNormals) {
            drawDebugNormals();
        }
    }

    /**
     * @brief Draw yellow whiskers for face normals and cyan whiskers for vertex normals
     */
    void drawDebugNormals(float faceNormalLen = 0.6f, float vertexNormalLen = 0.45f) const {
        GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
        if (lightingWasOn) glDisable(GL_LIGHTING);

        // 1. Face Normals (Yellow from centroid)
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

        // 2. Vertex Normals (Cyan from each vertex)
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
    // PROCEDURAL SHAPE GENERATORS & ARCHITECTURAL BUILDERS
    // ========================================================================

    /**
     * @brief Creates an Axis-Aligned Box centered at (cx, cy, cz) with dimensions (sx, sy, sz)
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

        // 8 Vertices
        mesh.addVertex(cx - hx, cy - hy, cz + hz); // 0: Front-Bottom-Left
        mesh.addVertex(cx + hx, cy - hy, cz + hz); // 1: Front-Bottom-Right
        mesh.addVertex(cx + hx, cy + hy, cz + hz); // 2: Front-Top-Right
        mesh.addVertex(cx - hx, cy + hy, cz + hz); // 3: Front-Top-Left
        mesh.addVertex(cx - hx, cy - hy, cz - hz); // 4: Back-Bottom-Left
        mesh.addVertex(cx + hx, cy - hy, cz - hz); // 5: Back-Bottom-Right
        mesh.addVertex(cx + hx, cy + hy, cz - hz); // 6: Back-Top-Right
        mesh.addVertex(cx - hx, cy + hy, cz - hz); // 7: Back-Top-Left

        // 6 Faces (CCW from outside)
        mesh.addQuad(0, 1, 2, 3, col, true); // Front (+Z)
        mesh.addQuad(5, 4, 7, 6, col, true); // Back (-Z)
        mesh.addQuad(3, 2, 6, 7, col, true); // Top (+Y)
        mesh.addQuad(4, 5, 1, 0, col, true); // Bottom (-Y)
        mesh.addQuad(4, 0, 3, 7, col, true); // Left (-X)
        mesh.addQuad(1, 5, 6, 2, col, true); // Right (+X)

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Creates a Tapered Prism along Z-axis (useful for car bodies and canopies)
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

        // Front quad vertices at z1 (higher Z)
        mesh.addVertex(-hw1, y1 - hh1, z1); // 0: BL
        mesh.addVertex( hw1, y1 - hh1, z1); // 1: BR
        mesh.addVertex( hw1, y1 + hh1, z1); // 2: TR
        mesh.addVertex(-hw1, y1 + hh1, z1); // 3: TL

        // Back quad vertices at z0 (lower Z)
        mesh.addVertex(-hw0, y0 - hh0, z0); // 4: BL
        mesh.addVertex( hw0, y0 - hh0, z0); // 5: BR
        mesh.addVertex( hw0, y0 + hh0, z0); // 6: TR
        mesh.addVertex(-hw0, y0 + hh0, z0); // 7: TL

        mesh.addQuad(0, 1, 2, 3, col, true); // Front (+Z)
        mesh.addQuad(5, 4, 7, 6, col, true); // Back (-Z)
        mesh.addQuad(3, 2, 6, 7, col, true); // Top
        mesh.addQuad(4, 5, 1, 0, col, true); // Bottom
        mesh.addQuad(4, 0, 3, 7, col, true); // Left
        mesh.addQuad(1, 5, 6, 2, col, true); // Right

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Creates a 4-sided Pyramid (base on XZ plane, apex at +Y)
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

        // Base 4 vertices
        mesh.addVertex(cx - hw, cy, cz - hd); // 0: Back-Left
        mesh.addVertex(cx + hw, cy, cz - hd); // 1: Back-Right
        mesh.addVertex(cx + hw, cy, cz + hd); // 2: Front-Right
        mesh.addVertex(cx - hw, cy, cz + hd); // 3: Front-Left

        // Apex vertex
        int apex = mesh.addVertex(cx, cy + height, cz); // 4

        // Bottom face
        mesh.addQuad(0, 1, 2, 3, col, true);

        // 4 Sloped triangular faces
        mesh.addTriangle(3, 2, apex, col, true); // Front
        mesh.addTriangle(2, 1, apex, col, true); // Right
        mesh.addTriangle(1, 0, apex, col, true); // Back
        mesh.addTriangle(0, 3, apex, col, true); // Left

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Creates an N-sided Polygonal Prism / Cylinder (Y-aligned)
     */
    static PolygonMesh createPrismY(
        float cx, float cy, float cz,
        float radius, float height, int sides = 16,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.75f, 0.75f, 0.78f)
    ) {
        PolygonMesh mesh("PrismY");
        mesh.defaultColor = col;
        if (sides < 3) sides = 3;

        // Bottom ring [0 .. sides-1]
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy, cz + radius * std::sin(u));
        }

        // Top ring [sides .. 2*sides-1]
        for (int i = 0; i < sides; ++i) {
            float u = (static_cast<float>(i) / sides) * MathCG::TWO_PI;
            mesh.addVertex(cx + radius * std::cos(u), cy + height, cz + radius * std::sin(u));
        }

        // Bottom center vertex (2*sides) & Top center vertex (2*sides + 1)
        int bCenter = mesh.addVertex(cx, cy, cz);
        int tCenter = mesh.addVertex(cx, cy + height, cz);

        // Side quads
        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            int b0 = i;
            int b1 = iNext;
            int t0 = sides + i;
            int t1 = sides + iNext;
            mesh.addQuad(b0, b1, t1, t0, col, true);
        }

        // Bottom & Top fans
        for (int i = 0; i < sides; ++i) {
            int iNext = (i + 1) % sides;
            mesh.addTriangle(bCenter, (i + 1) % sides, i, col, true);       // Bottom (normal -Y)
            mesh.addTriangle(tCenter, sides + i, sides + iNext, col, true);  // Top (normal +Y)
        }

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Architectural Tunnel Arch (Vòm Cổng Ngọ Môn)
     *        Generates a semi-circular curved vault barrel mesh connecting (z0 .. z1)
     */
    static PolygonMesh createArchVault(
        float cx, float cy, float z0, float z1,
        float innerR, float outerR, int segments = 12,
        const MathCG::ColorRGB& col = MathCG::ColorRGB(0.72f, 0.68f, 0.62f)
    ) {
        PolygonMesh mesh("ArchVault");
        mesh.defaultColor = col;
        if (segments < 4) segments = 4;

        // Generate inner and outer semi-circular arcs from angle 0 (right) to pi (left)
        // at z0 (near) and z1 (far)
        int baseIdx = 0;
        for (int i = 0; i <= segments; ++i) {
            float th = (static_cast<float>(i) / segments) * MathCG::PI;
            float cTh = std::cos(th);
            float sTh = std::sin(th);

            // z0 ring
            mesh.addVertex(cx + innerR * cTh, cy + innerR * sTh, z0); // baseIdx + 0: Inner Near
            mesh.addVertex(cx + outerR * cTh, cy + outerR * sTh, z0); // baseIdx + 1: Outer Near
            // z1 ring
            mesh.addVertex(cx + innerR * cTh, cy + innerR * sTh, z1); // baseIdx + 2: Inner Far
            mesh.addVertex(cx + outerR * cTh, cy + outerR * sTh, z1); // baseIdx + 3: Outer Far

            baseIdx += 4;
        }

        for (int i = 0; i < segments; ++i) {
            int k0 = i * 4;
            int k1 = (i + 1) * 4;

            // Inner ceiling barrel (viewed from below -> normal towards center)
            mesh.addQuad(k0 + 0, k0 + 2, k1 + 2, k1 + 0, col, true);

            // Outer vault top
            mesh.addQuad(k0 + 1, k1 + 1, k1 + 3, k0 + 3, col, true);

            // Near face arch (z0)
            mesh.addQuad(k0 + 0, k1 + 0, k1 + 1, k0 + 1, col, true);

            // Far face arch (z1)
            mesh.addQuad(k0 + 2, k0 + 3, k1 + 3, k1 + 2, col, true);
        }

        mesh.computeNormals();
        return mesh;
    }

    /**
     * @brief Traditional Asian Curved Roof Section (Mái cong Cổng Ngọ Môn)
     *        Generates an authentic tiered curved roof flare using quadratic elevation.
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

        // Generate curved roof grid using upward curvature towards corners
        for (int j = 0; j <= vSteps; ++j) {
            float v = static_cast<float>(j) / vSteps; // 0 (eaves) -> 1 (ridge)
            float rz = (v - 0.5f) * 2.0f;             // -1 .. 1

            for (int i = 0; i <= uSteps; ++i) {
                float u = static_cast<float>(i) / uSteps;
                float rx = (u - 0.5f) * 2.0f;         // -1 .. 1

                // Quadratic flare: corners lift upwards
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
