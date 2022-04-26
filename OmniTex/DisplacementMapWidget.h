#pragma once

#include <QWidget>
#include <QFileDialog>
#include <array>
#include <thread>
#include <utility>
#include "Sharpen.h"
#include "ui_DisplacementMapWidget.h"
#include "shared.h"
#include "GaussianBlur.h"
#include "InvalidImageException.h"


class DisplacementMapWidget : public QWidget
{
	Q_OBJECT

public:
	DisplacementMapWidget(QWidget *parent = nullptr);
	~DisplacementMapWidget();
	void setImage(QImage&);
	QImage getImage() { return finalImage; };
private:
	Ui::DisplacementMapWidget ui; 	std::array<uint8_t, 256> brightnessLut, contrastLut; 	
	std::vector<QRgb> original, brightnessImage, results, sharpenImage, blurImage; 	
	std::vector<std::thread> threads; 	
	int processImageSize, threadNumber, lutSize, width, height, imageSize; 	
	QImage originalImage, finalImage; 	
	Sharpen sharpen{}; 	
	GaussianBlur blur{}; 	
	void applyFinalImage(); 	
	void preprocessImage(); 	
	void applyBrightness(); 	
	void applyContrast();		
	void applyInvert(); 	
	void applySharpen(); 
	void applyBlur(); 	
	void toogleEnabledInputs(bool state); 
signals: 	
	void finishedComputing(); 	
	void imageComputed(); 
private slots: 	
	void loadImg(); 	
	void saveImg(); 	
	void brightnessChanged(int value); 	
	void contrastChanged(int value); 	
	void sharpenChanged(int value); 	
	void blurChanged(int value); 	
	void invertChanged(int state); 	
	void imageSwitch(int state); 
};
