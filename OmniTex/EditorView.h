#pragma once

#include <QWidget>
#include <QColorDialog>
#include <QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <memory>
#include <regex>
#include <sstream>
#include <QVulkanInstance>
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "SpecularMapWidget.h"
#include "NormalMapWidget.h"
#include "DisplacementMapWidget.h"
#include "AOMapWidget.h"
#include "InvalidImageException.h"


class EditorView : public QWidget
{
	Q_OBJECT

public:
	EditorView(QWidget *parent = nullptr);
	~EditorView();
	void setImage(QImage& img);
signals:
	void finishedComputing(); 
private:
	bool loaded{}; 	
	QImage diffuse; 	
	QColor backgroundColor, lightColor; 	
	QVulkanInstance inst; 	
	NormalMapWidget* normal; 	
	SpecularMapWidget* specular; 	
	DisplacementMapWidget* displacement; 	
	AOMapWidget* ao; 	
	VulkanWindow* vulkan; 	
	QVBoxLayout* verticalLayout, *materialsLayout, *lightLayout, *mappingLayout, *buttonsLayout; 	
	QStackedWidget* mapsControlWidget; 	QHBoxLayout* horizontalLayout, * uLayout, * vLayout, * lightIntensityLayout, * lightColorLayout, 		
		* backgroundColorLayout, * displacementLayout, * shininessLayout;
	QPushButton* normalButton, * specularButton, * displacementButton, * aoButton, * exportAllButton, * loadDiffuseButton, 		
		* loadAllButton, * lightColorButton, * backgroundColorButton;
	QComboBox* meshSelect; 	
	QDoubleSpinBox* uSpin, * vSpin, * shininessSpin, * displacementSpin, * lightIntesitySpin; 	
	QGroupBox* groupBox; 	
	QCheckBox* animateCheck; 	
	QLabel* materialPropsLabel, * mappingPropsLabel, * lightPropsLabel, * uLabel, * vLabel, * lightIntensityLabel, 		
		* lightLabel, * displacementLabel, * shininessLabel, * backgroundLabel;
	QGridLayout* gridLayout, * paramsGridLayout; 	
	QColorDialog* colorDialog; 	
	bool lightColorChanging;
private slots:
	void meshChanged(int index); 	
	void changeLightColor(QColor c); 	
	void changeBackgroundColor(QColor c); 
};
