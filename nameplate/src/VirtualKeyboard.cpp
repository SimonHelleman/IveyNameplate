#include <ctype.h>
#include <string.h>
#include "VirtualKeyboard.h"
#define LAYOUT_ROWS 3

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{

static const char* s_layout[LAYOUT_ROWS] = {
    { "QWERTYUIOP" },
    { "ASDFGHJKL" },
    { "^ZXCVBNM_" }
};

static constexpr char SHIFT_SYMBOL = '^';
static constexpr char SPACE_SYMBOL = '_';

static bool RectOverlapTest(
    const float x1, const float y1, const float w1, const float h1,
    const float x2, const float y2, const float w2, const float h2 
)
{
    const bool overlapX = x1 + w1 >= x2 && x2 + w2 >= x1;
    const bool overlapY = y1 + h1 >= y2 && y2 + h2 >= y1;

    return overlapX && overlapY;
}


VirtualKeyboard::VirtualKeyboard(const float posX, const float posY,
    const float keyWidth, const float keyHeight, const float keyPadding, const int outlineThickness
)
    : m_keyWidth(keyWidth), m_keyHeight(keyHeight),
    m_keyPadding(keyPadding), m_outlineThickness(outlineThickness)
{
    float currentX = posX;
    float currentY = posY;
    for (size_t row = 0; row < LAYOUT_ROWS; ++row)
    {
        const size_t len = strlen(s_layout[row]);
        for (size_t col = 0; col < len; ++col)
        {
            m_keys.emplace_back(currentX, currentY, s_layout[row][col]);
            // Font size calc might need adjusting

            currentX += m_keyWidth + m_keyPadding;
        }
        currentX = posX;
        currentY += m_keyHeight + m_keyPadding;
    }
}

void VirtualKeyboard::Draw(Display& display, const RGBA fillColor, const RGBA outlineColor) const
{
    for (const auto& k : m_keys)
    {
        display.FillRectangle(k.x, k.y, m_keyWidth, m_keyHeight, fillColor, outlineColor, m_outlineThickness);
        display.DrawText(k.x + m_keyWidth / 4, k.y + m_keyHeight / 4, m_keyWidth / 4, outlineColor, std::string(1, k.label));
    }
}

void VirtualKeyboard::Update(const Touch& touch)
{
    const auto pos = touch.GetTouchPos();
    const bool currentTouchState = touch.IsTouched();
    for (const auto& k : m_keys)
    {
        const bool overlap = RectOverlapTest(k.x, k.y, m_keyWidth, m_keyHeight, pos.first, pos.second, 1, 1);

        if (overlap && currentTouchState && !m_lastTouchState)
        {
            if (k.label == SHIFT_SYMBOL)
            {
                m_shift = !m_shift; // toggle shift
            }

            if (k.label == SPACE_SYMBOL)
            {
                m_text += ' ';
            }
            else
            {
                m_text += m_shift ? (char)toupper(k.label) : (char)tolower(k.label);
            }

            LOG_DEBUG("[VirtualKeyboard] " + m_text);
        }
    }
    m_lastTouchState = currentTouchState;

}

void VirtualKeyboard::ClearText()
{
    m_text.clear();
}

}

