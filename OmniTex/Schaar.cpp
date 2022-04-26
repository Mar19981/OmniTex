
#include "Schaar.h"

void Schaar::apply(std::vector<QVector3D>& input, int startY, int width, int size, bool flipR, bool flipG)
{
	std::vector<QVector3D> inter{ input }; 	int limitWidth = width - 1; 	float rMul{ flipR ? -1.0f : 1.0f }, gMul{ flipG ? -1.0f : 1.0f }; 		while (size) {
		for (int x = 1; x < limitWidth; x++) {
						float topLeft{ inter.at((startY - 1) * width + x - 1).x() }, top{ inter.at((startY - 1) * width + x).x() }, topRight{ inter.at((startY - 1) * width + x + 1).x() },
				bottomLeft{ inter.at((startY + 1) * width + x - 1).x() }, bottom{ inter.at((startY + 1) * width + x).x() }, bottomRight{ inter.at((startY + 1) * width + x + 1).x() },
				left{ inter.at(startY * width + x - 1).x() }, right{ inter.at(startY * width + x + 1).x() };
						input.at(startY * width + x).setX(rMul * (-3.0f * topLeft - 10.0f * left - 3.0f * bottomLeft + 3.0f * topRight + 10.0f * right + 3.0f * bottomRight) * 0.03125);
						input.at(startY * width + x).setY(gMul * (3.0f * topLeft + 10.0f * top + 3.0f * topRight - 3.0f * bottomLeft - 10.0f * bottom - 3.0f * bottomRight) * 0.03125);
						size--;
			if (!size) return;
		}
		startY++;
	}
}
