#pragma once
#include "EdgeDetection.h"

class Prewitt :
    public EdgeDetection
{
public:
	Prewitt() = default;
	void virtual apply(std::vector<QVector3D>&input, int startY, int width, int size, bool flipR, bool flipG);
};

