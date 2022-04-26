
#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.progressBar->setMaximum(std::thread::hardware_concurrency() * 23 + 1); 	
	ui.progressBar->setValue(0); 
}

LoadingScreen::~LoadingScreen()
{
}

void LoadingScreen::incrementProgress()
{
	int val = ui.progressBar->value(); 
	if (val++ == ui.progressBar->maximum()) return; 	
	ui.progressBar->setValue(val); 
	if (val == ui.progressBar->maximum()) 		
		emit loadingFinished();
}
