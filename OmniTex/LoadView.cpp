
#include "LoadView.h"

LoadView::LoadView(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
		connect(ui.loadButton, &QPushButton::clicked, this, &LoadView::loadImage);
}

LoadView::~LoadView()
{
}

void LoadView::loadImage() {
	auto path = QFileDialog::getOpenFileName(this, "Load image", QDir::homePath(), tr("Image(*.jpg *.jpeg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;Bitmap(*.bmp)"));
	if (path.isEmpty() || path.isNull()) return;
		QImage input(path);

	if (input.isNull()) return;
	emit imageLoaded(input); 
}
