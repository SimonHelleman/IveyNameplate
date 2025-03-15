#pragma once
#include <vector>
#include <string>
#include "Display.h"
#include "Touch.h"

namespace nameplate
{
class VirtualKeyboard
{
public:
    VirtualKeyboard(const float posX, const float posY,
        const float keyWidth, const float keyHeight, const float keyPadding, const int outlineThickness
    );

    void Draw(Display& display, const RGBA fillColor, const RGBA outlineColor) const;

    void Update(const Touch& touch);
    void ClearText();

    const std::string& Text()
    {
        return m_text;
    }

private:
    struct Key
    {
        Key(const float x, const float y, const char label)
            : x(x), y(y), label(label)
        {
        }
        const float x, y;
        const char label;
    };
private:
    const float m_keyWidth;
    const float m_keyHeight;
    const float m_keyPadding;
    const int m_outlineThickness;

    std::vector<Key> m_keys;
    std::string m_text;
    bool m_lastTouchState = false;
    bool m_shift = false;
};

}