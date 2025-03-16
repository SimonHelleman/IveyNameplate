namespace nameplate
{

bool RectOverlapTest(
    const float x1, const float y1, const float w1, const float h1,
    const float x2, const float y2, const float w2, const float h2
)
{
    const bool overlapX = x1 + w1 >= x2 && x2 + w2 >= x1;
    const bool overlapY = y1 + h1 >= y2 && y2 + h2 >= y1;

    return overlapX && overlapY;
}


}