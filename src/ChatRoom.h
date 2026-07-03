#pragma once
#include "Base.h"
#include "TextInput.h"
#include "Button.h"
#include "ErrorAlert.h"

struct ChatRoomScreen : BaseScreen {
	TextInput m_messageInput;
	Button m_sendButton;
	ErrorAlert m_errorAlert;

	bool m_autoScroll = true;
	int m_lastMessageCount = 0;
	float m_scrollY = 0.0f;
	float m_contentHeight = 0.0f;
	float m_scrollSensitivity = 40.0f;

	struct Message {
		const std::string m_content;
		const std::string m_user;
		int64_t m_timestamp;
	};

	std::vector<Message> m_messageHistory;
	std::vector<std::string> m_connectedUsers;

	ChatRoomScreen();

	void Update(float dt) override;
	void Draw() override;
	void EndFrame() override;

	void PacketHandler(const nlohmann::json& packet) override;
};