#pragma once
#include "clay.h"
#include "helpers.h"
#include "color.h"
#include "Button.h"
#include <string>
#include <vector>
using namespace ColorLibrary;

struct ErrorAlert {
    struct Error {
        std::string m_message;
        float m_lifetime = 5.0f;
    };

    std::vector<Error> m_errors;

    void Push(const std::string& message) {
        m_errors.push_back({ message });
    }

    void Update(float dt) {
        for (auto& error : m_errors) {
            error.m_lifetime -= dt;
        }
        m_errors.erase(
            std::remove_if(m_errors.begin(), m_errors.end(),
                [](const Error& e) { return e.m_lifetime <= 0.0f; }),
            m_errors.end()
        );
    }

    void Draw() {
        if (m_errors.empty()) return;

        unsigned int index = 0;
        CLAY(CLAY_ID("ErrorAlertContainer"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0)
                },
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            },
        }) {
            for (const Error& error : m_errors) {
                CLAY(CLAY_IDI_LOCAL("ErrorAlert", ++index), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIT(0)
                        },
                        .padding = CLAY_PADDING_ALL(12),
                        .childGap = 8,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT
                    },
                    .backgroundColor = Get(Hue::Red, Shade::S900),
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .border = {
                        .color = Get(Hue::Red, Shade::S700),
                        .width = { 2, 2, 2, 2, 0 }
                    },
                }) {
                    CLAY_TEXT(C(error.m_message), CLAY_TEXT_CONFIG({
                        .textColor = Get(Hue::Red, Shade::S50),
                        .fontSize = 20
                    }));
                }
            }
        }
    }
};