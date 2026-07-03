#include "pch.h"
#include "ChatRoom.h"
#include "clay.h"
#include <string>
#include "ConnectionManager.h"

ChatRoomScreen::ChatRoomScreen() {
    this->m_messageInput.m_placeholder = "Enter your message...";
    this->m_messageInput.m_inputID = "message";

    this->m_sendButton.m_label = "Send";
    this->m_sendButton.m_buttonID = "send";
    this->m_sendButton.m_onClick = [this]() {
        TraceLog(LOG_INFO, "Sending message");

        if (!ConnectionManager::m_running) return;

        // TODO: build a message of type CHAT

        // TODO: send the message using ConnectionManager::Send
        
		// reset the text input
        this->m_messageInput.m_content = "";
        this->m_messageInput.m_cursorPos = 0;
    };
}

void ChatRoomScreen::PacketHandler(const nlohmann::json& packet) {
    const std::string& packetType = packet["type"];
    // TODO: handle incoming packets by packetType
    // See documentation for each message type and how to handle it
    if (packetType == "ERROR") {
        this->m_errorAlert.Push(packet["reason"]);
    }
}

void ChatRoomScreen::Update(float dt) {
    // TODO: use ConnectionManager to poll for incoming messages
	// For all received, parse the json and send to PacketHandler for handling



    // detect manual scroll
    Vector2 scroll = GetMouseWheelMoveV();
    this-> m_scrollY += scroll.y * this->m_scrollSensitivity;

    // new message arrived, snap to bottom if auto scrolling
    int currentCount = (int)m_messageHistory.size();
    float scrollMax = std::abs(this->m_contentHeight + this->m_scrollSensitivity) * -1;

    if (currentCount != this->m_lastMessageCount && this->m_contentHeight >= 0.0f) {
        if (this->m_autoScroll) {
            this->m_scrollY = scrollMax;
        }
    }

    if (this->m_scrollY >= 0.0f) this->m_scrollY = 0.0f;
    if (this->m_scrollY <= scrollMax) this->m_scrollY = scrollMax;

    if (this->m_messageInput.m_focused) {
        if (IsKeyPressed(KEY_ENTER) && !this->m_messageInput.m_content.empty()) {
            this->m_sendButton.m_onClick();
        }
    }

    this->m_messageInput.Update(dt);
    this->m_sendButton.Update();
    this->m_errorAlert.Update(dt);
}

void ChatRoomScreen::Draw() {
    CLAY(CLAY_ID("OuterContainer"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
        .backgroundColor = ColorLibrary::Get(Hue::Neutral, Shade::S800)
    }) {
        CLAY(CLAY_ID("MainContent"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 32,
                .childAlignment = {
                    .y = CLAY_ALIGN_Y_CENTER
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            },
            .backgroundColor = ColorLibrary::Get(Hue::Neutral, Shade::S800)
        }) {
            CLAY(CLAY_ID("MessageHistory"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0)
                    },
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 6,
                    .childAlignment = {
                        .y = CLAY_ALIGN_Y_TOP
                    },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .cornerRadius = CLAY_CORNER_RADIUS(4),
                .clip = {.vertical = true, .childOffset = {.y = this->m_scrollY } },
                .border = {
                    .color = ColorLibrary::Get(Hue::Neutral, Shade::S700),
                    .width = { 2, 2, 2, 2, 0 }
                },
            }) {
                unsigned int index = 0;
                std::string lastUser = "";
                for (const Message& message : this->m_messageHistory) {
                    CLAY(CLAY_IDI_LOCAL("Message", ++index), {
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIT(0)
                            },
                            .padding = {
                                .left = 4
                            },
                            .childGap = 4,
                            .childAlignment = {
                                .y = CLAY_ALIGN_Y_CENTER
                            },
                            .layoutDirection = CLAY_TOP_TO_BOTTOM
                        },
                    }) {
                        if (message.m_user == "SYSTEM") {
                            CLAY_TEXT(C(message.m_content), CLAY_TEXT_CONFIG({
                                .textColor = ColorLibrary::Get(Hue::Purple, Shade::S400),
                                .fontSize = 20
                            }));
                        } else {
                            if (lastUser != message.m_user) {
                                CLAY(CLAY_ID_LOCAL("UserHeader"), {
                                    .layout = {
                                        .sizing = {
                                            .width = CLAY_SIZING_GROW(0),
                                            .height = CLAY_SIZING_FIT(0)
                                        },
                                        .childGap = 8,
                                        .childAlignment = {.y = CLAY_ALIGN_Y_CENTER },
                                        .layoutDirection = CLAY_LEFT_TO_RIGHT
                                    }
                                }) {
                                    CLAY_TEXT(C(message.m_user), CLAY_TEXT_CONFIG({
                                        .textColor = ColorLibrary::Get(Hue::Purple, Shade::S300),
                                        .fontSize = 16
                                    }));

                                    // Format the Unix ms timestamp into a readable string
                                    std::time_t seconds = message.m_timestamp / 1000;
                                    std::tm tm_info{};
                                    localtime_s(&tm_info, &seconds);
                                    char timeBuf[16];
                                    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", &tm_info);
                                    std::string timeStr(timeBuf);

                                    CLAY_TEXT(C(timeStr), CLAY_TEXT_CONFIG({
                                        .textColor = ColorLibrary::Get(Hue::Neutral, Shade::S500),
                                        .fontSize = 13
                                        }));
                                }
                            }

                            CLAY(CLAY_ID_LOCAL("Inner"), {
                                .layout = {
                                    .sizing = {
                                        .width = CLAY_SIZING_FIT(0),
                                        .height = CLAY_SIZING_FIT(0)
                                    },
                                    .padding = {
                                        .left = 16
                                    }
                                },
                            }) {
                                CLAY_TEXT(C(message.m_content), CLAY_TEXT_CONFIG({
                                    .textColor = ColorLibrary::Get(Hue::Purple, Shade::S50),
                                    .fontSize = 20
                                }));
                            }
                        }
                    }

                    lastUser = message.m_user;
                }
            }

            Clay_ScrollContainerData messageHistoryData = Clay_GetScrollContainerData(CLAY_ID("MessageHistory"));
            this->m_contentHeight = messageHistoryData.contentDimensions.height - messageHistoryData.scrollContainerDimensions.height;
            int currentCount = (int)m_messageHistory.size();
            this->m_lastMessageCount = currentCount;

            m_errorAlert.Draw();

            CLAY(CLAY_ID("Inputs"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_FIT()
                    },
                    .childGap = 16,
                    .childAlignment = {
                        .y = CLAY_ALIGN_Y_CENTER
                    },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT
                },
            }) {
                this->m_messageInput.Draw(
                    CLAY_SIZING_GROW(),
                    CLAY_SIZING_FIXED(44)
                );

                this->m_sendButton.Draw(CLAY_SIZING_FIXED(100), CLAY_SIZING_FIXED(44));
            }
        }
        CLAY(CLAY_ID("SidebarOuter"), {
           .layout = {
               .sizing = {
                   .width = CLAY_SIZING_PERCENT(0.15f),
                   .height = CLAY_SIZING_GROW(0)
               },
               .padding = {
                    .top = 16,
                    .bottom = 16
                },
            },
        }) {
            CLAY(CLAY_ID("Sidebar"), {
               .layout = {
                   .sizing = {
                       .width = CLAY_SIZING_GROW(0),
                       .height = CLAY_SIZING_GROW(0)
                   },
                   .padding = CLAY_PADDING_ALL(16),
                   .childGap = 16,
                   .childAlignment = {
                       .y = CLAY_ALIGN_Y_TOP
                   },
                   .layoutDirection = CLAY_TOP_TO_BOTTOM
                },

                .backgroundColor = ColorLibrary::Get(Hue::Neutral, Shade::S800),
                .cornerRadius = CLAY_CORNER_RADIUS(4),
                .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset() },
                .border = {
                    .color = ColorLibrary::Get(Hue::Neutral, Shade::S700),
                    .width = { 2, 2, 2, 2, 0 }
                },
            }) {
                CLAY_TEXT(C("Connected Users"), CLAY_TEXT_CONFIG({
                    .textColor = ColorLibrary::Get(Hue::Purple, Shade::S400),
                    .fontSize = 26
                }));

                for (const std::string& user : this->m_connectedUsers) {
                    CLAY_TEXT(C(user), CLAY_TEXT_CONFIG({
                        .textColor = ColorLibrary::Get(Hue::Purple, Shade::S50),
                        .fontSize = 24
                    }));
                }
            }
        }
    }
}

void ChatRoomScreen::EndFrame() {
    this->m_messageInput.UpdateBBOX();
    this->m_sendButton.UpdateBBOX();
}
