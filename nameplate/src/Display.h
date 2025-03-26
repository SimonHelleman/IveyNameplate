#pragma once

#include <string>
#include "Util/Color.h"

#include "Reaction.h"

namespace nameplate {

class Display
{
public:
    Display(unsigned int width, unsigned int height, const char* name)
        : m_width(width), m_height(height), m_name(name)
    {
    }

    virtual ~Display() = default;

    unsigned int Width() const
    {
        return m_width;
    }

    unsigned int Height() const
    {
        return m_height;
    }

    const char* Name() const
    {
        return m_name;
    }

    // To be implemented by specific implementation
    virtual void HandleEvents() = 0;
    
    virtual void DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const std::string& text) = 0;
    virtual void FillRectangle(
        const float posX, const float posY, const float width, const float height,
        const RGBA fill, const RGBA outlineColor, const float outlineThickness
    ) = 0;

    // Probably should move to a DrawImage approach but this saves time for now
    virtual void DrawReaction(
        const unsigned int posX, const unsigned int posY,
        Reaction r
    ) = 0;

    virtual const unsigned int ReactionWidth(const Reaction r) const = 0;
    virtual const unsigned int ReactionHeight(const Reaction r) const = 0;

    virtual void Clear(RGB color) = 0;
    virtual void Show() = 0;


protected:
    unsigned int m_width;
    unsigned int m_height;
    const char* m_name;
};
}