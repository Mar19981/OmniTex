#pragma once
#include <functional>
#include <iterator>
#include <algorithm>
#include <array>
#include <QColor>
#include <QVector3d>



template<typename T>
void applyLambda(std::vector<T>& data, int start, int size, std::function<void(T&)> lambda) {
		auto begin = data.begin();
		std::advance(begin, start);
		auto end = begin;
		std::advance(end, size);
		std::for_each(begin, end, lambda);
	}
template<typename T, typename R>
void convertVector(std::vector<T>& source, std::vector<R>& destination, int index, int size, std::function<void(T&, R&)> lambda) {
	for (; size; size--, index++) {
		lambda(source.at(index), destination.at(index));
	}
}
template <typename T>
void generateLUT (std::array<uint8_t, 256>& arr, int index, int size, std::function<void(uint8_t&, uint8_t)> lambda) {
	for (; size ; size--, index++)
		lambda(arr.at(index), index);
}

template <typename T>
inline T remap(T value, T low1, T high1, T low2, T high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}
