
#pragma once
#include "KernelFilter.h"

class Sharpen :
    public KernelFilter
{
public:
    Sharpen() = default;
    virtual void apply(std::vector<QRgb>& input, int startY, int width, int size, const float amount) override;
};

