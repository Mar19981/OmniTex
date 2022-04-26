#pragma once
#include "EdgeDetection.h"

class Schaar :
    public EdgeDetection
{
public:
    Schaar() = default;
    virtual void apply(std::vector<QVector3D>&input, int startY, int width, int size, bool flipR, bool flipG) override;
};

