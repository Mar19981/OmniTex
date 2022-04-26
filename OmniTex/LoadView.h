#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QImage>
#include "ui_LoadView.h"


class LoadView: public QWidget
{
	Q_OBJECT

public:
	LoadView(QWidget *parent = nullptr);
	~LoadView();

private:
	Ui::LoadView ui; signals:
	void imageLoaded(QImage img); 
private slots:
	void loadImage(); 
};
