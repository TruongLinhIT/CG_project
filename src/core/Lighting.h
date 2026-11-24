#ifndef LIGHTING_H
#define LIGHTING_H

/**
 * @file Lighting.h
 * @brief Chương 6: Các Mô hình Chiếu sáng, Suy giảm Nguồn sáng Điểm & Chu kỳ Ngày/Hoàng hôn/Đêm
 *
 * Cài đặt:
 *  - Thành phần ánh sáng môi trường (Ambient light): I_a * k_a
 *  - Chiếu sáng khuếch tán (Diffuse) & phản xạ gương (Specular) từ nguồn sáng điểm với suy giảm bậc hai theo khoảng cách:
 *      I_d = I_s * k_d * max(0, cos(theta)) / (c_0 + c_1 * D + c_2 * D^2)
 *  - Chu kỳ Ngày / Hoàng hôn / Đêm tương tác + Hệ thống đèn LED đổi màu theo HSV vào ban đêm trên Vòng quay Sun Wheel
 */

#include <GL/freeglut.h>
#include "core/MathCG.h"

enum class TimeOfDay {
    DAY    = 0,
    SUNSET = 1,
    NIGHT  = 2
};

enum class ShadingModel {
    GOURAUD_SMOOTH = 0, // GL_SMOOTH (Nội suy pháp tuyến đỉnh n_p = 1/k sum n_i)
    LAMBERT_FLAT   = 1  // GL_FLAT   (Pháp tuyến mặt hằng số N = A x B)
};

class LightingSystem {
public:
    TimeOfDay    timeOfDay;
    ShadingModel shadingModel;
    bool         lightingEnabled;
    float        ledHueDeg; // Góc màu H động thuộc [0, 360) cho dải đèn LED HSV->RGB trên Sun Wheel

    LightingSystem()
        : timeOfDay(TimeOfDay::NIGHT),
          shadingModel(ShadingModel::GOURAUD_SMOOTH),
          lightingEnabled(true),
          ledHueDeg(0.0f) {}

    /**
     * @brief Thiết lập màu nền bầu trời (clear color) dựa theo trạng thái Ngày / Hoàng hôn / Đêm.
     */
    void applyClearColor() const {
        switch (timeOfDay) {
            case TimeOfDay::DAY:
                glClearColor(0.52f, 0.77f, 0.96f, 1.0f);
                break;
            case TimeOfDay::SUNSET:
                glClearColor(0.88f, 0.44f, 0.28f, 1.0f);
                break;
            case TimeOfDay::NIGHT:
            default:
                glClearColor(0.04f, 0.06f, 0.14f, 1.0f);
                break;
        }
    }

    /**
     * @brief Áp dụng mô hình tô bóng hằng Lambert (GL_FLAT) hoặc tô bóng mượt Gouraud (GL_SMOOTH)
     *        và cấu hình Nguồn sáng 0 (Mặt trời/Mặt trăng) cùng Nguồn sáng 1
     *        (Nguồn sáng điểm tại tâm Sun Wheel có suy giảm theo bình phương khoảng cách 1/D^2).
     */
    void applyLights(const MathCG::Vec3& sunWheelCenter) const {
        if (shadingModel == ShadingModel::LAMBERT_FLAT) {
            glShadeModel(GL_FLAT);
        } else {
            glShadeModel(GL_SMOOTH);
        }

        if (!lightingEnabled) {
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHTING);
            return;
        }

        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_NORMALIZE);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
        resetMaterial();

        // --------------------------------------------------------------------
        // Cường độ ánh sáng môi trường toàn cục I_a và Nguồn sáng Light0 (Mặt trời / Mặt trăng)
        // --------------------------------------------------------------------
        GLfloat globalAmbient[4];
        GLfloat light0Pos[4];
        GLfloat light0Diff[4];
        GLfloat light0Spec[4];

        if (timeOfDay == TimeOfDay::DAY) {
            globalAmbient[0] = 0.42f; globalAmbient[1] = 0.42f; globalAmbient[2] = 0.45f; globalAmbient[3] = 1.0f;
            light0Pos[0]     = 120.0f; light0Pos[1]    = 180.0f; light0Pos[2]    = 100.0f; light0Pos[3]    = 1.0f;
            light0Diff[0]    = 0.95f; light0Diff[1]    = 0.93f; light0Diff[2]    = 0.85f; light0Diff[3]    = 1.0f;
            light0Spec[0]    = 0.60f; light0Spec[1]    = 0.60f; light0Spec[2]    = 0.55f; light0Spec[3]    = 1.0f;
        } else if (timeOfDay == TimeOfDay::SUNSET) {
            globalAmbient[0] = 0.32f; globalAmbient[1] = 0.22f; globalAmbient[2] = 0.24f; globalAmbient[3] = 1.0f;
            light0Pos[0]     = -160.0f; light0Pos[1]   = 55.0f;  light0Pos[2]    = 90.0f;  light0Pos[3]    = 1.0f;
            light0Diff[0]    = 1.00f; light0Diff[1]    = 0.58f; light0Diff[2]    = 0.30f; light0Diff[3]    = 1.0f;
            light0Spec[0]    = 0.75f; light0Spec[1]    = 0.45f; light0Spec[2]    = 0.25f; light0Spec[3]    = 1.0f;
        } else {
            // BAN ĐÊM: Ánh sáng môi trường I_a * k_a thấp (ánh trăng xanh lạnh) vừa đủ để khối kiến trúc
            // không bị đen tuyền, đồng thời vẫn làm nổi bật hiệu ứng tự phát sáng LED (GL_EMISSION)
            // và các nguồn sáng điểm suy giảm 1/D^2 của hệ thống đèn công viên (NightIllumination).
            globalAmbient[0] = 0.20f; globalAmbient[1] = 0.22f; globalAmbient[2] = 0.34f; globalAmbient[3] = 1.0f;
            light0Pos[0]     = 80.0f;  light0Pos[1]    = 150.0f; light0Pos[2]    = -90.0f; light0Pos[3]    = 1.0f;
            light0Diff[0]    = 0.34f; light0Diff[1]    = 0.40f; light0Diff[2]    = 0.62f; light0Diff[3]    = 1.0f;
            light0Spec[0]    = 0.26f; light0Spec[1]    = 0.32f; light0Spec[2]    = 0.50f; light0Spec[3]    = 1.0f;
        }

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0Diff);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.0005f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.000005f);

        // --------------------------------------------------------------------
        // Nguồn sáng 1 (Light 1): Nguồn sáng điểm động HSV->RGB tại tâm Sun Wheel có suy giảm 1/D^2
        // Công thức theo đề cương: I = I_s * k_d * cos(theta) / D^2
        // --------------------------------------------------------------------
        glEnable(GL_LIGHT1);
        float r = 1.0f, g = 0.8f, b = 0.3f;
        MathCG::HSVtoRGB(ledHueDeg, 0.85f, 1.0f, r, g, b);

        float intensity = (timeOfDay == TimeOfDay::NIGHT)  ? 1.0f :
                          (timeOfDay == TimeOfDay::SUNSET) ? 0.65f : 0.20f;

        GLfloat light1Pos[4]  = { sunWheelCenter.x, sunWheelCenter.y, sunWheelCenter.z + 8.0f, 1.0f };
        GLfloat light1Diff[4] = { r * intensity, g * intensity, b * intensity, 1.0f };
        GLfloat light1Spec[4] = { r * intensity, g * intensity, b * intensity, 1.0f };
        GLfloat light1Amb[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };

        glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
        glLightfv(GL_LIGHT1, GL_AMBIENT,  light1Amb);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1Diff);
        glLightfv(GL_LIGHT1, GL_SPECULAR, light1Spec);

        // Suy giảm bậc hai theo khoảng cách (mô phỏng quy luật 1 / D^2)
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.4f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.004f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.00018f);
    }

    /**
     * @brief Cấu hình Nguồn sáng 2 (Light 2) làm đèn pha định hướng ánh sáng ấm gắn trên toa đầu tàu Queen Cobra
     *        trong chế độ Hoàng hôn và Ban đêm (có suy giảm bậc hai theo khoảng cách).
     */
    void applyCoasterHeadlight(const MathCG::Vec3& pos, const MathCG::Vec3& forward) const {
        if (!lightingEnabled || timeOfDay == TimeOfDay::DAY) {
            glDisable(GL_LIGHT2);
            return;
        }

        float scale = (timeOfDay == TimeOfDay::NIGHT) ? 1.0f : 0.45f;
        GLfloat pos4[4]  = { pos.x, pos.y, pos.z, 1.0f };
        GLfloat dir3[3]  = { forward.x, forward.y, forward.z };
        GLfloat diff[4]  = { 1.0f * scale, 0.90f * scale, 0.65f * scale, 1.0f };
        GLfloat spec[4]  = { 1.0f * scale, 0.92f * scale, 0.72f * scale, 1.0f };
        GLfloat amb[4]   = { 0.0f, 0.0f, 0.0f, 1.0f };

        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2, GL_POSITION,       pos4);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, dir3);
        glLightf (GL_LIGHT2, GL_SPOT_CUTOFF,    35.0f);
        glLightf (GL_LIGHT2, GL_SPOT_EXPONENT,  14.0f);
        glLightfv(GL_LIGHT2, GL_AMBIENT,        amb);
        glLightfv(GL_LIGHT2, GL_DIFFUSE,        diff);
        glLightfv(GL_LIGHT2, GL_SPECULAR,       spec);
        glLightf (GL_LIGHT2, GL_CONSTANT_ATTENUATION,  0.6f);
        glLightf (GL_LIGHT2, GL_LINEAR_ATTENUATION,    0.015f);
        glLightf (GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.0012f);
    }

    /**
     * @brief Thiết lập hệ số phản xạ gương bề mặt k_s, số mũ độ bóng Phong n,
     *        và độ tự phát sáng (emission) tùy chọn cho đường ray kim loại hoặc đèn LED ban đêm.
     */
    static void setMaterial(
        float specStrength,
        float shininess,
        float emitR = 0.0f,
        float emitG = 0.0f,
        float emitB = 0.0f
    ) {
        GLfloat spec[4] = { specStrength, specStrength, specStrength, 1.0f };
        GLfloat emit[4] = { emitR, emitG, emitB, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spec);
        glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  emit);
    }

    /**
     * @brief Khôi phục thuộc tính phản xạ gương mặc định của vật liệu và tắt tự phát sáng.
     */
    static void resetMaterial() {
        setMaterial(0.32f, 28.0f, 0.0f, 0.0f, 0.0f);
    }

    const char* getTimeOfDayName() const {
        switch (timeOfDay) {
            case TimeOfDay::DAY:    return "Ban ngay (Anh sang moi truong I_a cao)";
            case TimeOfDay::SUNSET: return "Hoang hon Da Nang (Nang vang am)";
            case TimeOfDay::NIGHT:  return "Ban dem LED (HSV + Suy giam 1/D^2)";
            default:                return "Khong xac dinh";
        }
    }

    const char* getShadingModelName() const {
        return (shadingModel == ShadingModel::GOURAUD_SMOOTH)
            ? "To bong Gouraud (GL_SMOOTH)"
            : "To bong hang Lambert (GL_FLAT)";
    }
};

#endif // LIGHTING_H
