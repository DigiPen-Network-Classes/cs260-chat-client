#include "pch.h"
#include "raymath.h"
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "clay.h"

#define CLAY_COLOR_TO_RAYLIB_COLOR(color) \
    Color{ (unsigned char)roundf(color.r), (unsigned char)roundf(color.g), (unsigned char)roundf(color.b), (unsigned char)roundf(color.a) }

inline Camera Raylib_camera = {};

enum CustomLayoutElementType {
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
};

struct CustomLayoutElement_3DModel {
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
};

struct CustomLayoutElement {
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    } customData;
};

static const char* overlayShaderCode =
"#version 330\n"
"\n"
"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"\n"
"uniform sampler2D texture0;\n"
"uniform vec4 overlayColor;\n"
"\n"
"out vec4 finalColor;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 texelColor = texture(texture0, fragTexCoord) * fragColor;\n"
"    vec3 blendedRGB = mix(texelColor.rgb, overlayColor.rgb, overlayColor.a);\n"
"    finalColor = vec4(blendedRGB, texelColor.a);\n"
"}";

static Shader overlayShader = {};
static int colorLoc = 0;
static bool overlayEnabled = false;

static void InitOverlay() {
    overlayShader = LoadShaderFromMemory(nullptr, overlayShaderCode);
    colorLoc = GetShaderLocation(overlayShader, "overlayColor");
}

static void SetColorOverlay(Color color) {
    overlayEnabled = true;
    float colorFloat[4] = {
        (float)color.r / 255.0f,
        (float)color.g / 255.0f,
        (float)color.b / 255.0f,
        (float)color.a / 255.0f,
    };
    SetShaderValue(overlayShader, colorLoc, colorFloat, SHADER_UNIFORM_VEC4);
    BeginShaderMode(overlayShader);
}

static void DisableColorOverlay() {
    if (overlayEnabled) {
        EndShaderMode();
        overlayEnabled = false;
    }
}

static Ray GetScreenToWorldPointWithZDistance(Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance) {
    Ray ray = {};

    float x = (2.0f * position.x) / (float)screenWidth - 1.0f;
    float y = 1.0f - (2.0f * position.y) / (float)screenHeight;
    float z = 1.0f;

    Vector3 deviceCoords = { x, y, z };

    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE) {
        matProj = MatrixPerspective(camera.fovy * DEG2RAD, ((double)screenWidth / (double)screenHeight), 0.01f, zDistance);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
        double aspect = (double)screenWidth / (double)screenHeight;
        double top = camera.fovy / 2.0;
        double right = top * aspect;
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    Vector3 nearPoint = Vector3Unproject(Vector3{ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    Vector3 farPoint = Vector3Unproject(Vector3{ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);
    Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

    ray.position = farPoint;
    ray.direction = direction;

    return ray;
}

static inline Font font;

static inline Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* userData) {
    Clay_Dimensions textSize = {};

    float maxTextWidth = 0.0f;
    float lineTextWidth = 0.0f;
    int   maxLineCharCount = 0;
    int   lineCharCount = 0;

    float textHeight = (float)config->fontSize;
    Font  fontToUse = font;

    float scaleFactor = config->fontSize / (float)fontToUse.baseSize;

    for (int i = 0; i < text.length; ++i, lineCharCount++) {
        if (text.chars[i] == '\n') {
            maxTextWidth = fmaxf(maxTextWidth, lineTextWidth);
            maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);
            lineTextWidth = 0.0f;
            lineCharCount = 0;
            continue;
        }
        int index = text.chars[i] - 32;
        if (fontToUse.glyphs[index].advanceX != 0)
            lineTextWidth += (float)fontToUse.glyphs[index].advanceX;
        else
            lineTextWidth += fontToUse.recs[index].width + (float)fontToUse.glyphs[index].offsetX;
    }

    maxTextWidth = fmaxf(maxTextWidth, lineTextWidth);
    maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);

    textSize.width = maxTextWidth * scaleFactor + (float)(lineCharCount * config->letterSpacing);
    textSize.height = textHeight;

    return textSize;
}

inline void Clay_Raylib_Initialize() {
    InitOverlay();

    font = LoadFontEx("resources/fonts/Oxygen-Bold.ttf", 48, 0, 400);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, &font);
}

static char* temp_render_buffer = nullptr;
static int   temp_render_buffer_len = 0;

inline void Clay_Raylib_Close() {
    if (temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer = nullptr;
    temp_render_buffer_len = 0;
    CloseWindow();
}

inline void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts) {
    for (int j = 0; j < renderCommands.length; j++) {
        Clay_RenderCommand* renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox    boundingBox = renderCommand->boundingBox;

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData* textData = &renderCommand->renderData.text;
                Font                 fontToUse = fonts[textData->fontId];

                int len = textData->stringContents.length + 1;
                if (len > temp_render_buffer_len) {
                    if (temp_render_buffer) free(temp_render_buffer);
                    temp_render_buffer = (char*)malloc(len);
                    temp_render_buffer_len = len;
                }

                if (!temp_render_buffer) break;
                memcpy(temp_render_buffer, textData->stringContents.chars, textData->stringContents.length);
                temp_render_buffer[textData->stringContents.length] = '\0';

                DrawTextEx(
                    fontToUse,
                    temp_render_buffer,
                    Vector2{ boundingBox.x, boundingBox.y },
                    (float)textData->fontSize,
                    (float)textData->letterSpacing,
                    CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor)
                );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D  imageTexture = *(Texture2D*)renderCommand->renderData.image.imageData;
                Clay_Color tintColor = renderCommand->renderData.image.backgroundColor;

                if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 && tintColor.a == 0) {
                    tintColor = Clay_Color{ 255, 255, 255, 255 };
                }

                DrawTexturePro(
                    imageTexture,
                    Rectangle{ 0.0f, 0.0f, (float)imageTexture.width, (float)imageTexture.height },
                    Rectangle{ boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height },
                    Vector2{ 0.0f, 0.0f },
                    0.0f,
                    CLAY_COLOR_TO_RAYLIB_COLOR(tintColor)
                );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                BeginScissorMode(
                    (int)roundf(boundingBox.x),
                    (int)roundf(boundingBox.y),
                    (int)roundf(boundingBox.width),
                    (int)roundf(boundingBox.height)
                );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START: {
                SetColorOverlay(CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->renderData.overlayColor.color));
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END: {
                DisableColorOverlay();
                break; // added missing break
            }

            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleRenderData* config = &renderCommand->renderData.rectangle;
                if (config->cornerRadius.topLeft > 0) {
                    float radius = (config->cornerRadius.topLeft * 2) /
                        (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                    DrawRectangleRounded(
                        Rectangle{ boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height },
                        radius, 8,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                    );
                }
                else {
                    DrawRectangle(
                        (int)boundingBox.x, (int)boundingBox.y,
                        (int)boundingBox.width, (int)boundingBox.height,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                    );
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData* config = &renderCommand->renderData.border;

                if (config->width.left > 0) {
                    DrawRectangleV(
                        Vector2{ boundingBox.x, boundingBox.y + config->cornerRadius.topLeft },
                        Vector2{ (float)config->width.left, boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft },
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->width.right > 0) {
                    DrawRectangleV(
                        Vector2{ boundingBox.x + boundingBox.width - config->width.right, boundingBox.y + config->cornerRadius.topRight },
                        Vector2{ (float)config->width.right, boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight },
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->width.top > 0) {
                    DrawRectangleV(
                        Vector2{ boundingBox.x + config->cornerRadius.topLeft, boundingBox.y },
                        Vector2{ boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight, (float)config->width.top },
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->width.bottom > 0) {
                    DrawRectangleV(
                        Vector2{ boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->width.bottom },
                        Vector2{ boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight, (float)config->width.bottom },
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.topLeft > 0) {
                    DrawRing(
                        Vector2{ roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) },
                        roundf(config->cornerRadius.topLeft - config->width.top), config->cornerRadius.topLeft,
                        180.0f, 270.0f, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.topRight > 0) {
                    DrawRing(
                        Vector2{ roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) },
                        roundf(config->cornerRadius.topRight - config->width.top), config->cornerRadius.topRight,
                        270.0f, 360.0f, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.bottomLeft > 0) {
                    DrawRing(
                        Vector2{ roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) },
                        roundf(config->cornerRadius.bottomLeft - config->width.bottom), config->cornerRadius.bottomLeft,
                        90.0f, 180.0f, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.bottomRight > 0) {
                    DrawRing(
                        Vector2{ roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) },
                        roundf(config->cornerRadius.bottomRight - config->width.bottom), config->cornerRadius.bottomRight,
                        0.1f, 90.0f, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData* config = &renderCommand->renderData.custom;
                CustomLayoutElement* customElement = (CustomLayoutElement*)config->customData;
                if (!customElement) continue;

                switch (customElement->type) {
                    case CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL: {
                        Clay_BoundingBox rootBox = renderCommands.internalArray[0].boundingBox;
                        float            scaleValue = CLAY__MIN(CLAY__MIN(1.0f, 768.0f / rootBox.height) * CLAY__MAX(1.0f, rootBox.width / 1024.0f), 1.5f);
                        Ray              positionRay = GetScreenToWorldPointWithZDistance(
                            Vector2{ renderCommand->boundingBox.x + renderCommand->boundingBox.width / 2.0f,
                                     renderCommand->boundingBox.y + renderCommand->boundingBox.height / 2.0f + 20.0f },
                            Raylib_camera,
                            (int)roundf(rootBox.width),
                            (int)roundf(rootBox.height),
                            140.0f
                        );
                        BeginMode3D(Raylib_camera);
                        DrawModel(customElement->customData.model.model, positionRay.position, customElement->customData.model.scale * scaleValue, WHITE);
                        EndMode3D();
                        break;
                    }
                    default: break;
                }
                break;
            }

            default: {
                printf("Error: unhandled render command.\n");
                exit(1);
            }
        }
    }
}