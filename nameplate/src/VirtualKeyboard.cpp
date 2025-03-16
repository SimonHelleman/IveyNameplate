#include <ctype.h>
#include <string.h>
#include "Util/MiscUtil.h"
#include "VirtualKeyboard.h"

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{

static const char* s_layout[] = {
    { "QWERTYUIOP<" },
    { "ASDFGHJKL" },
    { "^ZXCVBNM_" }
};

static constexpr char SHIFT_SYMBOL = '^';
static constexpr char SPACE_SYMBOL = '_';
static constexpr char BACKS_SYMBOL = '<';

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
                break;
            }

            if (k.label == BACKS_SYMBOL)
            {
                if (!m_text.empty())
                {
                    m_text.pop_back();
                }

                break;
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

