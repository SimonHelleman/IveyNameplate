#pragma once
#include "Display.h"

namespace nameplate
{
class VirtualKeyboard
{
public:
	VirtualKeyboard(const float keyWidth, const float keyHeight, const float keyPadding, const int outlineThickness);

	void Draw(Display& display, const float posX, const float posY, const RGBA fillColor, const RGBA outlineColor) const;
private:
	const float m_keyWidth;
	const float m_keyHeight;
	const float m_keyPadding;
	const int m_outlineThickness;
};

}