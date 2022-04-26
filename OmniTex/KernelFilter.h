#pragma once
#include <vector>
#include <QColor>


class KernelFilter
{
public:
	KernelFilter() = default; 								
	void virtual apply(std::vector<QRgb>& input, int startY, int width, int size, const float amount) = 0;
};

