#pragma once

#include <QWidget>
#include <QComboBox>
#include <QFileDialog>
#include <array>
#include <thread>
#include <utility>
#include "Sharpen.h"
#include "ui_NormalMapWidget.h"
#include "shared.h"
#include "Sobel.h"
#include "Prewitt.h"
#include "Schaar.h"


class NormalMapWidget : public QWidget
{
	Q_OBJECT

public:
	NormalMapWidget(QWidget *parent = nullptr);
	~NormalMapWidget();
	void setImage(QImage&);
	QImage getImage() { return finalImage; };
private:
	Ui::NormalMapWidget ui; 		
	std::vector<QRgb> original, sharpened, results;
	std::vector<QVector3D> sharpenedVec, normalized;
	std::vector<std::thread> threads; 	
	int processImageSize, threadNumber, width, height, imageSize; 	
	QImage finalImage; 	
	Sharpen sharpen{}; 	
	std::unique_ptr<EdgeDetection> edgeDetection; 	
	void applyFinalImage(); 	
	void preprocessImage(); 	
	void applyNormalMap(); 	
	void applyIntensity(); 	
	void applySharpen(); 
signals: 
	void finishedComputing(); 	
	void imageComputed();
private slots: 	
	void saveImg(); 	
	void intensityChanged(int value);  	
	void sharpenChanged(int value); 	
	void edgeDetectionChanged(int value); 
};
