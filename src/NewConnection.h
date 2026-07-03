#pragma once
#include "Base.h"
#include "TextInput.h"
#include "Button.h"
#include "ErrorAlert.h"

struct NewConnectionScreen : BaseScreen {
	TextInput m_nameInput;
	Button m_connectButton;
	ErrorAlert m_errorAlert;

	// name captured when LOGIN is sent; used to build the CONNECT token once the
	// server replies with AUTHENTICATE (and sent verbatim in the CONNECT packet)
	std::string m_pendingName;

	NewConnectionScreen();

	void Update(float dt) override;
	void Draw() override;
	void EndFrame() override;

	void PacketHandler(const nlohmann::json& packet) override;
};