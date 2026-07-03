#include <iostream>
#include "pch.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "renderer.cpp"
#include "color.h"
#include "TextInput.h"

#include "NewConnection.h"
#include "ChatRoom.h"
#include "ConnectionManager.h"

using namespace ColorLibrary;

void HandleClayErrors(Clay_ErrorData errorData) {
    TraceLog(LOG_ERROR, "Clay error: %s", errorData.errorText.chars);
}

int main(int argc, char* argv[]) {
    // TODO: verify correct and valid arguments
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port> <token>" << std::endl;
        return 1;
    }

    // TODO: get the IP address, verify it is valid

    // TODO: get port, verify it is valid

    // TODO: get token
    
    // TODO: set the ip, port, and token in the ConnectionManager


    // Window and renderer setup
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "CS260 Chat Client");
    SetTargetFPS(60);

    // Clay init (UI Layout Engine)
    uint64_t clayMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(static_cast<size_t>(clayMemorySize), malloc(static_cast<size_t>(clayMemorySize)));

    Clay_Initialize(
        clayMemory,
        { static_cast<float>(screenWidth), static_cast<float>(screenHeight) },
        { HandleClayErrors }
    );

    Clay_Raylib_Initialize();

    // array of screens that can be shown, each has its own update and render handler
    // Screen 1: New Connection
    //           Displays the new connection screen to prompt for a username and triggers the connection to the server

    // Screen 2: Chat Room
    //           The main screen of the app, has the chat window and message input + connected users
    std::unique_ptr<BaseScreen> screens[2];
    screens[0] = std::make_unique<NewConnectionScreen>();
    screens[1] = std::make_unique<ChatRoomScreen>();

    bool debug = false;

    while (!WindowShouldClose()) {
        // Press F10 to open the UI debug (like the inspector in browser devtools)
        if (IsKeyPressed(KEY_F10)) {
            debug = !debug;
            Clay_SetDebugModeEnabled(debug);
        }

        float dt = GetFrameTime();

        // Update UI parameters (window size, cursor pos, etc.)
        Clay_SetLayoutDimensions({
            static_cast<float>(GetScreenWidth()),
            static_cast<float>(GetScreenHeight()),
        });

        // raylib draws in logical (screen) coordinates even on HiDPI displays,
        // so Clay's layout/pointer space is logical too. Feed the raw mouse
        // position - scaling by the render/screen ratio (2 on Retina) would put
        // the pointer outside every element and make the UI feel "disabled".
        Vector2 mousePosition = GetMousePosition();
        float dpiScale = (float)GetRenderWidth() / (float)GetScreenWidth();

        Clay_SetPointerState(
            { mousePosition.x * dpiScale, mousePosition.y * dpiScale },
            IsMouseButtonDown(MOUSE_LEFT_BUTTON)
        );

        Vector2 scroll = GetMouseWheelMoveV();
        Clay_UpdateScrollContainers(true, { scroll.x, scroll.y }, GetFrameTime());


        // Update the currently active screen
        screens[BaseScreen::CurrentScreen]->Update(dt);

        // Render the next frame of the current screen
        Clay_BeginLayout();
        screens[BaseScreen::CurrentScreen]->Draw();
        Clay_RenderCommandArray renderCommands = Clay_EndLayout(dt);

        // draw the commands to the screen
        BeginDrawing();
        screens[BaseScreen::CurrentScreen]->EndFrame();
        ClearBackground({ 0, 0, 0, 255 });
        Clay_Raylib_Render(renderCommands, &font);
        EndDrawing();

        ClayStringHelpers::ClearBuffers();
    }

    // leave the chat gracefully before tearing down the socket
    ConnectionManager::Disconnect();

    UnloadFont(font);
    CloseWindow();
    free(clayMemory.memory);
    return 0;
}