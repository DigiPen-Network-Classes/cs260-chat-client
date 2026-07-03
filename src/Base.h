#pragma once
#include "json.hpp"

struct BaseScreen {
	inline static unsigned int CurrentScreen = 0;

	virtual ~BaseScreen() = default;

	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;
	virtual void EndFrame() = 0;

	virtual void PacketHandler(const nlohmann::json& packet) = 0;
};