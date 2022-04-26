#include "GaussianBlur.h"

void GaussianBlur::apply(std::vector<QRgb>& input, int startY, int width, int size, const float amount)
{
	std::vector<QRgb> inter{ input }; 	int limitWidth = width - 1; 	
	for (int i = 0; i < amount; i++) { 		
		int s = size, y = startY; 				
		while (s) {
			for (int x = 1; x < limitWidth; x++) {
				QColor central = inter.at(y * width + x), top = inter.at((y - 1) * width + x), bottom = inter.at((y + 1) * width + x), left = inter.at(y * width + x - 1),
					right = inter.at(y * width + x + 1), topRight = inter.at((y - 1) * width + x + 1), topLeft = inter.at((y - 1) * width + x - 1),
					bottomLeft = inter.at((y + 1) * width + x - 1), bottomRight = inter.at((y + 1) * width + x + 1);
				int newRed = ((central.red() << 2) + (top.red() << 1) + (bottom.red() << 1) + (left.red() << 1) + (right.red() << 1) + topRight.red() + topLeft.red() + bottomRight.red() + bottomLeft.red()) >> 4,
					newGreen = ((central.green() << 2) + (top.green() << 1) + (bottom.green() << 1) + (left.green() << 1) + (right.green() << 1) + topRight.green() + topLeft.green() + bottomRight.green() + bottomLeft.green()) >> 4,
					newBlue = ((central.blue() << 2) + (top.blue() << 1) + (bottom.blue() << 1) + (left.blue() << 1) + (right.blue() << 1) + topRight.blue() + topLeft.blue() + bottomRight.blue() + bottomLeft.blue()) >> 4;
				central.setRed(newRed);
				central.setGreen(newGreen);
				central.setBlue(newBlue);

				input.at(y * width + x) = central.rgb();
				s--;
				if (!s) break;
			}
			y++;
		}
		inter = input; 	
	}
}
