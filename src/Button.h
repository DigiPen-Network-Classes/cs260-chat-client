#pragma once
#include "clay.h"
#include <string>
#include <functional>
#include "helpers.h"
#include "color.h"

using namespace ColorLibrary;

struct Button {
    std::string m_label;
    std::string m_buttonID = "";

    float m_cornerRadius = 4.0f;

    struct ColorConfig {
        Clay_Color background;
        Clay_Color background_hover;
        Clay_Color background_press;
        Clay_Color text;
    };

    ColorConfig m_colors = {
        Get(Hue::Violet, Shade::S600),
        Get(Hue::Violet, Shade::S500),
        Get(Hue::Violet, Shade::S700),
        Get(Hue::Neutral, Shade::S50),
    };

    bool m_hovered = false;
    bool m_pressed = false;

    Rectangle m_bbox = { 0 };

    std::function<void()> m_onClick;

    void Update() {
        // raylib's mouse position and Clay's bounding boxes are both in logical
        // (screen) coordinates, so compare them directly. Scaling by the render/
        // screen DPI ratio breaks hit-testing on HiDPI displays (e.g. Retina,
        // where the ratio is 2) - clicks land at double the coordinates.
        Vector2 mousePos = GetMousePosition();
        float dpiScale = (float)GetRenderWidth() / (float)GetScreenWidth(); // 1.4
		mousePos = { mousePos.x * dpiScale, mousePos.y * dpiScale },
        m_hovered = CheckCollisionPointRec(mousePos, m_bbox);
        bool wasPressed = m_pressed;
        m_pressed = m_hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        if (wasPressed && !m_pressed && m_hovered) {
            if (m_onClick) m_onClick();
        }
    }

    void Draw(
        Clay_SizingAxis widthSizing = CLAY_SIZING_FIT(0),
        Clay_SizingAxis heightSizing = CLAY_SIZING_FIXED(44)
    ) {
        Clay_Color bg = m_pressed ? m_colors.background_press
            : m_hovered ? m_colors.background_hover
            : m_colors.background;

        CLAY(CLAY_SID(C("Button_" + m_buttonID)), {
            .layout = {
                .sizing = { widthSizing, heightSizing },
                .padding = { 16, 16, 10, 10 },
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
            },
            .backgroundColor = bg,
            .cornerRadius = CLAY_CORNER_RADIUS(m_cornerRadius),
            }) {
            CLAY_TEXT(C(m_label), CLAY_TEXT_CONFIG({
                .textColor = m_colors.text,
                .fontSize = 24
            }));
        }
    }

    void UpdateBBOX() {
        Clay_ElementData data = Clay_GetElementData(CLAY_SID(C("Button_" + m_buttonID)));
        if (data.found) {
            m_bbox = {
                data.boundingBox.x, data.boundingBox.y,
                data.boundingBox.width, data.boundingBox.height
            };
        }
    }
};