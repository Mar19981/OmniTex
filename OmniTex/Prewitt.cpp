#include "Prewitt.h"

void Prewitt::apply(std::vector<QVector3D>& input, int startY, int width, int size, bool flipR, bool flipG)
{
	std::vector<QVector3D> inter{ input }; 	
	int limitWidth = width - 1; 	
	float rMul{ flipR ? -1.0f : 1.0f }, gMul{ flipG ? -1.0f : 1.0f }; 		
	while (size) {
		for (int x = 1; x < limitWidth; x++) {
			float topLeft{ inter.at((startY - 1) * width + x - 1).x() }, top{ inter.at((startY - 1) * width + x).x() }, topRight{ inter.at((startY - 1) * width + x + 1).x() },
				bottomLeft{ inter.at((startY + 1) * width + x - 1).x() }, bottom{ inter.at((startY + 1) * width + x).x() }, bottomRight{ inter.at((startY + 1) * width + x + 1).x() },
				left{ inter.at(startY * width + x - 1).x() }, right{ inter.at(startY * width + x + 1).x() };
			input.at(startY * width + x).setX(rMul * (-topLeft - left - bottomLeft + topRight + right + bottomRight));
			input.at(startY * width + x).setY(gMul * (topLeft + top + +topRight - bottomLeft - bottom - bottomRight));
			size--;
			if (!size) return;
		}
		startY++;
	}
}
