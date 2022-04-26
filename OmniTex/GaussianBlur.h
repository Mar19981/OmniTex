#pragma once
#include "KernelFilter.h"

class GaussianBlur :
    public KernelFilter
{
public:
    GaussianBlur() = default;
    virtual void apply(std::vector<QRgb>& input, int startY, int width, int size, const float amount) override;
};

