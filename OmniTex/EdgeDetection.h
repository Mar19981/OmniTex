#pragma once
#include <vector>
#include <QVector3D>

class EdgeDetection
{
public:
	EdgeDetection() = default; 									
	void virtual apply(std::vector<QVector3D>& input, int startY, int width, int size, bool flipR, bool flipG) = 0;
};

