#pragma once
#include "im_main.h"

struct RectangleShadowSettings
{
    bool    linear = false;
    float   sigma = 3;
    ImVec2  padding = ImVec2(50, 50);
    ImVec2  rectPos;
    ImVec2  rectSize;
    ImVec2  shadowOffset;
    ImVec2  shadowSize = ImVec2(120, 50);
    Imcolor_t shadowColor = Imcolor_t(0.0f, 0.0f, 0.0f, 1.0f);

    int  rings = 8;
    int  spacingBetweenRings = 6;
    int  samplesPerCornerSide = 1;
    int  spacingBetweenSamples = 15;
    int totalVertices = 0;
    int totalIndices = 0;
    bool enableDebugVisualization = false;
};

class shadow : public Singleton <shadow>
{
public:

    ImVec4 boxGaussianIntegral(ImVec4 x);
    ImVec4 boxLinearInterpolation(ImVec4 x);
    float boxShadow(ImVec2 lower, ImVec2 upper, ImVec2 point, float sigma, bool linearInterpolation);
    void drawRectangleShadowVerticesAdaptive(RectangleShadowSettings& settings);
    RectangleShadowSettings shadowSettings;
};