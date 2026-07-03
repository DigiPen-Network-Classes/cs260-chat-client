#include "pch.h"
#include "NewConnection.h"
#include "clay.h"
#include <string>
#include "json.hpp"
#include "ConnectionManager.h"
#include "sha256.h"

namespace {
    // Trim leading/trailing whitespace so the name we hash matches the name the
    // server trims before recomputing the token.
    std::string Trim(const std::string& s) {
        const auto first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        const auto last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    }
}

NewConnectionScreen::NewConnectionScreen() {
    // set up input components
    this->m_nameInput.m_placeholder = "Enter your name...";
    this->m_nameInput.m_inputID = "name";
    this->m_nameInput.m_maxLength = 32;

    this->m_connectButton.m_label = "Connect";
    this->m_connectButton.m_buttonID = "submit";
    this->m_connectButton.m_onClick = [this]() {
        TraceLog(LOG_INFO, "Opening Socket");

        if (!ConnectionManager::m_running) {
		    // TODO: Connect to the IP and port
        }
		
        // capture the name now; we need it to build the token once the server
        // replies to LOGIN with an AUTHENTICATE nonce
        this->m_pendingName = Trim(this->m_nameInput.m_content);

        // TODO: create step 1 of the handshake: a LOGIN message
        // TODO: send the login message
    };
}

void NewConnectionScreen::PacketHandler(const nlohmann::json& packet) {
    const std::string& packetType = packet["type"];
    // TODO: handle incoming messages here
    // see documentation for the incoming message types and what to do with them
    if (packetType == "ERROR") {
        TraceLog(LOG_ERROR, packet["reason"].get<std::string>().c_str());
        this->m_errorAlert.Push(packet["reason"]);
    }
}

void NewConnectionScreen::Update(float dt) {
    // TODO: poll for incoming messages, parse and send to PacketHandler()

   
    if (this->m_nameInput.m_focused) {
        if (IsKeyPressed(KEY_ENTER) && !this->m_nameInput.m_content.empty()) {
            this->m_connectButton.m_onClick();
        }
    }

    this->m_nameInput.Update(dt);
    this->m_connectButton.Update();
    this->m_errorAlert.Update(dt);
}

void NewConnectionScreen::Draw() {
    CLAY(CLAY_ID("OuterContainer"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
        .backgroundColor = ColorLibrary::Get(Hue::Neutral, Shade::S800)
    }) {
        CLAY(CLAY_ID("ErrorContainer"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3f),
                    .height = CLAY_SIZING_FIT(0)
                },
            },
        }) {
            this->m_errorAlert.Draw();
        }

        this->m_nameInput.Draw(
            CLAY_SIZING_FIXED(320),
            CLAY_SIZING_FIXED(44)
        );

        this->m_connectButton.Draw(CLAY_SIZING_FIXED(320), CLAY_SIZING_FIXED(44));
    }
}

void NewConnectionScreen::EndFrame() {
    this->m_nameInput.UpdateBBOX();
    this->m_connectButton.UpdateBBOX();
}
