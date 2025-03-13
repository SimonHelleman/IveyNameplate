#include <string.h>
#include "VirtualKeyboard.h"
#define LAYOUT_ROWS 3

namespace nameplate
{

static const char* s_layout[LAYOUT_ROWS] = {
	{ "QWERTYUIOP" },
	{ "ASDFGHJKL" },
	{ "*ZXCVBNM_" }
};

VirtualKeyboard::VirtualKeyboard(const float keyWidth, const float keyHeight, const float keyPadding, const int outlineThickness)
	: m_keyWidth(keyWidth), m_keyHeight(keyHeight), m_keyPadding(keyPadding), m_outlineThickness(outlineThickness)
{
}

void VirtualKeyboard::Draw(Display& display, const float posX, const float posY, const RGBA fillColor, const RGBA outlineColor) const
{
	float currentX = posX;
	float currentY = posY;
	for (size_t row = 0; row < LAYOUT_ROWS; ++row)
	{
		const size_t len = strlen(s_layout[row]);
		for (size_t col = 0; col < len; ++col)
		{
			display.FillRectangle(currentX, currentY, m_keyWidth, m_keyHeight, fillColor, outlineColor, m_outlineThickness);
			// Font size calc might need adjusting
			display.DrawText(currentX + m_keyWidth / 4, currentY + m_keyHeight / 4, m_keyWidth / 4, outlineColor, std::string(1, s_layout[row][col]));

			currentX += m_keyWidth + m_keyPadding;
		}
		currentX = posX;
		currentY += m_keyHeight + m_keyPadding;
	}
}



}

