#pragma once

#include <QWidget>
#include <thread>
#include "ui_LoadingScreen.h"


class LoadingScreen : public QWidget
{
	Q_OBJECT

public:
	LoadingScreen(QWidget *parent = nullptr);
	~LoadingScreen();
	void incrementProgress();
signals:
	void loadingFinished(); 
private:
	Ui::LoadingScreen ui; 
};
