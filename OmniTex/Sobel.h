#pragma once
#include "EdgeDetection.h"
#include "shared.h"


class Sobel :
    public EdgeDetection
{
public:
    Sobel() = default;
    virtual void apply(std::vector<QVector3D>& input, int startY, int width, int size, bool flipR, bool flipG) override;
};

