#include "Sharpen.h"

void Sharpen::apply(std::vector<QRgb>& input, int startY, int width, int size, const float amount)
{
	std::vector<QRgb> inter { input }; 	int limitWidth = width - 1; 		
	while (size) {
		for (int x = 1; x < limitWidth; x++) {
			QColor central = inter.at(startY * width + x), top = inter.at((startY - 1) * width + x), bottom = inter.at((startY + 1) * width + x), left = inter.at(startY * width + x - 1), 
				right = inter.at(startY * width + x + 1), topRight = inter.at((startY - 1) * width + x + 1), topLeft = inter.at((startY - 1) * width + x - 1), 
				bottomLeft = inter.at((startY + 1) * width + x - 1), bottomRight = inter.at((startY + 1) * width + x + 1);
			int newRed = (central.red() + top.red() + bottom.red() + left.red() + right.red() + topRight.red() + topLeft.red() + bottomRight.red() + bottomLeft.red()) * 0.1111111111,
				newGreen = (central.green() + top.green() + bottom.green() + left.green() + right.green() + topRight.green() + topLeft.green() + bottomRight.green() + bottomLeft.green()) * 0.1111111111, 
				newBlue = (central.blue() + top.blue() + bottom.blue() + left.blue() + right.blue() + topRight.blue() + topLeft.blue() + bottomRight.blue() + bottomLeft.blue()) * 0.1111111111;

			central.setRed(std::clamp(central.red() + (central.red() - newRed) * amount, 0.0f, 255.0f));
			central.setGreen(std::clamp(central.green() + (central.green() - newGreen) * amount, 0.0f, 255.0f));
			central.setBlue(std::clamp(central.blue() + (central.blue() - newBlue) * amount, 0.0f, 255.0f));
			input.at(startY * width + x) = central.rgb();
			size--;
			if (!size) return;
		}
			startY++;
	}
}