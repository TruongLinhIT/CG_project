#ifndef LIGHTING_H
#define LIGHTING_H

/**
 * @file Lighting.h
 * @brief Chapter 6: Illumination Models, Point Light Attenuation & Day/Sunset/Night Cycle
 *
 * Implements:
 *  - Ambient light contribution: I_a * k_a
 *  - Point light diffuse & specular illumination with quadratic distance attenuation:
 *      I_d = I_s * k_d * max(0, cos(theta)) / (c_0 + c_1 * D + c_2 * D^2)
 *  - Interactive Day / Sunset / Night cycle + HSV-driven night LED accent light on Sun Wheel
 */

#include <GL/freeglut.h>
#include "core/MathCG.h"

enum class TimeOfDay {
    DAY    = 0,
    SUNSET = 1,
    NIGHT  = 2
};

enum class ShadingModel {
    GOURAUD_SMOOTH = 0, // GL_SMOOTH (Interpolated vertex normals n_p = 1/k sum n_i)
    LAMBERT_FLAT   = 1  // GL_FLAT   (Constant face normal N = A x B)
};

class LightingSystem {
public:
    TimeOfDay    timeOfDay;
    ShadingModel shadingModel;
    bool         lightingEnabled;
    float        ledHueDeg; // Dynamic H in [0, 360) for Sun Wheel HSV->RGB LEDs

    LightingSystem()
        : timeOfDay(TimeOfDay::NIGHT),
          shadingModel(ShadingModel::GOURAUD_SMOOTH),
          lightingEnabled(true),
          ledHueDeg(0.0f) {}

    /**
     * @brief Set sky clear color based on Day / Sunset / Night state.
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
     * @brief Apply Lambert Constant Shading (GL_FLAT) or Gouraud Shading (GL_SMOOTH)
     *        and configure Light 0 (Sun/Moon directional-point light) and Light 1
     *        (Sun Wheel central point light with 1/D^2 distance attenuation).
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
        // Global Ambient Intensity I_a and Light0 (Sun / Moon)
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
            // NIGHT: Low ambient I_a k_a so LED emission and point light attenuation stand out
            globalAmbient[0] = 0.14f; globalAmbient[1] = 0.15f; globalAmbient[2] = 0.24f; globalAmbient[3] = 1.0f;
            light0Pos[0]     = 80.0f;  light0Pos[1]    = 150.0f; light0Pos[2]    = -90.0f; light0Pos[3]    = 1.0f;
            light0Diff[0]    = 0.25f; light0Diff[1]    = 0.30f; light0Diff[2]    = 0.48f; light0Diff[3]    = 1.0f;
            light0Spec[0]    = 0.20f; light0Spec[1]    = 0.25f; light0Spec[2]    = 0.40f; light0Spec[3]    = 1.0f;
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
        // Light 1: Sun Wheel Dynamic HSV->RGB Point Light with 1/D^2 Attenuation
        // Syllabus formula: I = I_s * k_d * cos(theta) / D^2
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

        // Quadratic distance attenuation (1 / D^2 behavior)
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.4f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.004f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.00018f);
    }

    /**
     * @brief Configure Light 2 as a warm directional headlight on the Queen Cobra lead car
     *        during Sunset and Night modes (with quadratic attenuation).
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
     * @brief Set surface specular reflection k_s, Phong shininess exponent n,
     *        and optional self-emission for metallic rails or night LED fixtures.
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
     * @brief Restore default matte-satin material specular & zero emission.
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
