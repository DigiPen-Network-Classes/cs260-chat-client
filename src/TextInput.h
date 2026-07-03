#pragma once
#include "clay.h"
#include <string>
#include <functional>
#include "helpers.h"
#include "color.h"

struct ColorConfig {
	Clay_Color background;
	Clay_Color background_hover;
	Clay_Color background_focus;

	Clay_Color border;
	Clay_Color border_focus;

	Clay_Color text;
	Clay_Color placeholder;
	Clay_Color cursor;

};

using namespace ColorLibrary;

struct TextInput {
	/*
		Values
	*/

	// user content
	std::string m_content;

	// text to show before content is given
	std::string m_placeholder;

	std::string m_inputID = "";

	bool m_focused = false;
	bool m_cursorVisible = false;

	float m_cursorBlinkTimer = 0.0f;

	int m_cursorPos = 0;
	int m_maxLength = 64;
		
	/*
		Visuals
	*/

	float m_cornerRadius = 4.0f;
	
	ColorConfig m_colors = {
		 Get(Hue::Neutral, Shade::S800),
		 Get(Hue::Neutral, Shade::S700),
		 Get(Hue::Neutral, Shade::S900),
		 Get(Hue::Neutral, Shade::S600),
		 Get(Hue::Violet, Shade::S300),
		 Get(Hue::Neutral, Shade::S200),
		 Get(Hue::Neutral, Shade::S500),
		 Get(Hue::Violet, Shade::S300),
	};

	Rectangle m_bbox = { 0 };

	// called when the input gains focus
	std::function<void(const std::string&)> m_onFocus;

	// called when the content is changed
	std::function<void(const std::string&)> m_onChange;

	// called when the input loses focus
	std::function<void(const std::string&)> m_onUnfocus;

	void focus() {
		this->m_focused = true;
		this->m_cursorPos = static_cast<int>(this->m_content.size());
		if (this->m_onFocus) this->m_onFocus(this->m_content);
	}
	void unfocus() { 
		this->m_focused = false;
		if (this->m_onUnfocus) this->m_onUnfocus(this->m_content);
	}

	// Call once per frame
	void Update(float dt) {
		// raylib's mouse position and Clay's bounding boxes are both in logical
		// (screen) coordinates, so compare them directly. Scaling by the render/
		// screen DPI ratio breaks hit-testing on HiDPI displays (e.g. Retina,
		// where the ratio is 2) - clicks land at double the coordinates and the
		// field never gains focus.
		Vector2 mousePos = GetMousePosition();
        float dpiScale = (float)GetRenderWidth() / (float)GetScreenWidth();
		mousePos = { mousePos.x * dpiScale, mousePos.y * dpiScale };

		bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
		bool mouseClick = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

		bool clickedInside = mouseClick && CheckCollisionPointRec(mousePos, this->m_bbox);
		bool clickedOutside = mouseClick && !CheckCollisionPointRec(mousePos, this->m_bbox);

		if (clickedInside)  focus();
		if (clickedOutside) unfocus();

		// cursor blink
		if (this->m_focused) {
			this->m_cursorBlinkTimer += dt;
			if (this->m_cursorBlinkTimer >= 0.53f) {
				this->m_cursorBlinkTimer = 0.0f;
				this->m_cursorVisible = !this->m_cursorVisible;
			}
		}
		else {
			this->m_cursorVisible = false;
			this->m_cursorBlinkTimer = 0.0f;
		}

		// input only when focused
		if (!this->m_focused) return;

		// arrow keys
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) this->m_cursorPos = std::max(0, this->m_cursorPos - 1);
		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) this->m_cursorPos = std::min((int)this->m_content.size(), this->m_cursorPos + 1);
		if (IsKeyPressed(KEY_HOME))  this->m_cursorPos = 0;
		if (IsKeyPressed(KEY_END))   this->m_cursorPos = (int)this->m_content.size();

		// delete
		if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && this->m_cursorPos > 0) {
			this->m_content.erase(this->m_cursorPos - 1, 1);
			this->m_cursorPos--;
			this->m_cursorBlinkTimer = 0.0f;
			this->m_cursorVisible = true;

			if (this->m_onChange) this->m_onChange(this->m_content);
		}
		if ((IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && this->m_cursorPos < (int)this->m_content.size()) {
			this->m_content.erase(this->m_cursorPos, 1);
			this->m_cursorBlinkTimer = 0.0f;
			this->m_cursorVisible = true;
			if (this->m_onChange) this->m_onChange(this->m_content);
		}

		// Character input
		int ch = GetCharPressed();
		while (ch > 0) {
			if (ch >= 32 && (int)this->m_content.size() < this->m_maxLength) {
				this->m_content.insert(this->m_cursorPos, 1, (char)ch);
				this->m_cursorPos++;
				this->m_cursorBlinkTimer = 0.0f;
				this->m_cursorVisible = true;
				if (this->m_onChange) this->m_onChange(this->m_content);
			}
			ch = GetCharPressed();
		}

	}

	void Draw(
		Clay_SizingAxis widthSizing = CLAY_SIZING_GROW(0),
		Clay_SizingAxis heightSizing = CLAY_SIZING_FIXED(44)
	) {
		Clay_Color border = this->m_focused ? this->m_colors.border_focus : this->m_colors.border;

		uint16_t borderWidth = this->m_focused ? 2 : 1;

		CLAY(CLAY_SID(C("TextInput_" + this->m_inputID)), {
			.layout = {
				.sizing = { widthSizing, heightSizing },
				.padding = { 12, 12, 10, 10 },
				.childAlignment = {.y = CLAY_ALIGN_Y_CENTER }
			},
			.backgroundColor = this->m_focused ? this->m_colors.background_focus
												: Clay_Hovered() ? this->m_colors.background_hover
												: this->m_colors.background,
			.cornerRadius = CLAY_CORNER_RADIUS(this->m_cornerRadius),
			.border = {
				.color = border,
				.width = { borderWidth, borderWidth, borderWidth, borderWidth, 0 }
			}
		}) {
			if (!this->m_focused) {
				// when unfocused just show content or placeholder as one block
				const std::string& display = this->m_content.empty() ? this->m_placeholder : this->m_content;
				Clay_Color textColor = this->m_content.empty() ? this->m_colors.placeholder : this->m_colors.text;

				CLAY_TEXT(C(display), CLAY_TEXT_CONFIG({
					.textColor = textColor,
					.fontSize = 24
				}));
			}
			else {
				// when focused split content at cursor position and insert cursor between
				std::string before = this->m_content.substr(0, this->m_cursorPos);
				std::string after = this->m_content.substr(this->m_cursorPos);

				if (!before.empty()) {
					CLAY_TEXT(C(before), CLAY_TEXT_CONFIG({
						.textColor = this->m_colors.text,
						.fontSize = 18
						}));
				}

				// cursor
				Clay_Color cursorColor = this->m_cursorVisible ? this->m_colors.cursor : ColorLibrary::Get(Hue::Neutral, Shade::S100, 0);
				CLAY(CLAY_ID("TextInputCursor"), {
					.layout = {
						.sizing = {
							CLAY_SIZING_FIXED(2),
							CLAY_SIZING_FIXED(22)
						}
					},
					.backgroundColor = cursorColor
				}) {
				}

				if (!after.empty()) {
					CLAY_TEXT(C(after), CLAY_TEXT_CONFIG({
						.textColor = this->m_colors.text,
						.fontSize = 18
					}));
				}
			}
		}
	}


	void UpdateBBOX() {
		Clay_ElementData inputData = Clay_GetElementData(CLAY_SID(C("TextInput_" + this->m_inputID)));
		if (inputData.found) {
			this->m_bbox = {
				inputData.boundingBox.x, inputData.boundingBox.y,
				inputData.boundingBox.width, inputData.boundingBox.height
			};
		}

	}
};