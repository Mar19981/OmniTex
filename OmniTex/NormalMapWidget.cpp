#include "NormalMapWidget.h"


NormalMapWidget::NormalMapWidget(QWidget *parent)
	: QWidget(parent), edgeDetection(std::make_unique<Sobel>())
{
	ui.setupUi(this);
	threadNumber = std::thread::hardware_concurrency();
	threads.resize(threadNumber);
	ui.intensityValue->setText(QString::number(ui.intensitySlider->value()));
	ui.sharpenValue->setText(QString::number(ui.sharpenSlider->value()));
	ui.preview->setFixedSize(128, 128);

	connect(ui.intensitySlider, &QSlider::valueChanged, this, &NormalMapWidget::intensityChanged);
	connect(ui.sharpenSlider, &QSlider::valueChanged, this, &NormalMapWidget::sharpenChanged);
	connect(ui.sharpenSlider, &QSlider::sliderReleased, this, &NormalMapWidget::applySharpen);
	connect(ui.intensitySlider, &QSlider::sliderReleased, this, &NormalMapWidget::applyNormalMap);
	connect(ui.edgeDetectionSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NormalMapWidget::edgeDetectionChanged);
	connect(ui.flipXCheck, &QCheckBox::stateChanged, this, [=](int i) {
		if (finalImage.isNull()) return;
		applyNormalMap(); 		
		});	
	connect(ui.flipYCheck, &QCheckBox::stateChanged, this, [=](int i) {
		if (finalImage.isNull()) return;
		applyNormalMap(); 		
		});
	connect(ui.saveButton, &QPushButton::clicked, this, &NormalMapWidget::saveImg);
}

NormalMapWidget::~NormalMapWidget()
{
}

void NormalMapWidget::setImage(QImage& img)
{
	width = img.width();
	height = img.height();
	imageSize = width * height;
	processImageSize = imageSize / threadNumber;

	finalImage = QImage{ width, height, img.format() };

	original.clear();
	normalized.clear();
	results.clear();
	sharpened.clear();
	original.resize(imageSize);
	normalized.resize(imageSize);
	sharpenedVec.resize(imageSize);
	results.resize(imageSize);
	sharpened.resize(imageSize);

	std::memmove(original.data(), img.bits(), imageSize * sizeof(QRgb));

	preprocessImage();
}

void NormalMapWidget::applyFinalImage()
{
	std::memmove(finalImage.bits(), results.data(), results.size() * sizeof(QRgb));
	ui.preview->setPixmap(QPixmap::fromImage(finalImage).scaled(ui.preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	finalImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
	emit imageComputed();
}

void NormalMapWidget::preprocessImage()
{
	int start{};
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			applyLambda<QRgb>(original, start, processImageSize, [=](QRgb& pixel) -> void {
				uint8_t newValue = QColor::fromRgb(pixel).lightness(); 				
				pixel = qRgb(newValue, newValue, newValue);
				});
			emit finishedComputing();
			});
		start += processImageSize;
	}
	threads.at(threadNumber - 1) = std::thread([=] {
		applyLambda<QRgb>(original, start, imageSize - processImageSize * (threadNumber - 1), [=](QRgb& pixel) -> void {
			uint8_t newValue = QColor::fromRgb(pixel).lightness();
			pixel = qRgb(newValue, newValue, newValue);
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applySharpen(); 
}

void NormalMapWidget::applyNormalMap()
{
	if (finalImage.isNull()) return;
	int start{ 1 }, yStep{ (height - 2) / threadNumber }, process{ imageSize - width - width - height - height }, size{ process / threadNumber };
	normalized = sharpenedVec; 
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			edgeDetection->apply(normalized, start, width, size, ui.flipXCheck->isChecked(), ui.flipYCheck->isChecked()); 			
			emit finishedComputing();
			});
		start += yStep;
	}

	threads.at(threadNumber - 1) = std::thread([=] {
		edgeDetection->apply(normalized, start, width, process - size * (threadNumber - 1), ui.flipXCheck->isChecked(), ui.flipYCheck->isChecked());
		emit finishedComputing();
		});

		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applyIntensity(); 
}

void NormalMapWidget::applyIntensity()
{
	if (finalImage.isNull()) return;
	float z = ui.intensitySlider->value() * 0.01f;
	int start{};

	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
						convertVector<QVector3D, QRgb>(normalized, results, start, processImageSize, [=](QVector3D& vector, QRgb& pixel) -> void {
				QColor newPixel{};
				vector.setZ(z); 				
				auto norm = vector.normalized(); 								
				newPixel.setRedF((norm.x() + 1.0f) * 0.5f);
				newPixel.setGreenF((norm.y() + 1.0f) * 0.5f);
				newPixel.setBlueF(norm.z());
				pixel = newPixel.rgb();
				});
			emit finishedComputing();
			});
		start += processImageSize;
	}
	threads.at(threadNumber - 1) = std::thread([=] {
		convertVector<QVector3D, QRgb>(normalized, results, start, imageSize - processImageSize * (threadNumber - 1), [=](QVector3D& vector, QRgb& pixel) -> void {
			QColor newPixel{};
			vector.setZ(z);
			auto norm = vector.normalized();
			newPixel.setRedF((norm.x() + 1.0f) * 0.5f);
			newPixel.setGreenF((norm.y() + 1.0f) * 0.5f);
			newPixel.setBlueF(norm.z());
			pixel = newPixel.rgb();
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applyFinalImage(); 
}

void NormalMapWidget::applySharpen()
{
	if (finalImage.isNull()) return;
	int start{ 1 }, yStep{ (height - 2) / threadNumber }, process{ imageSize - width - width - height - height }, 
		size{ process / threadNumber }, startConvert{};

	sharpened = original; 
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			sharpen.apply(sharpened, start, width, size, ui.sharpenSlider->value()); 			
			emit finishedComputing();
			});
		start += yStep;
	}

	threads.at(threadNumber - 1) = std::thread([=] {
		sharpen.apply(sharpened, start, width, process - size * (threadNumber - 1), ui.sharpenSlider->value());
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}

		for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			convertVector<QRgb, QVector3D>(sharpened, sharpenedVec, startConvert, processImageSize, [=](QRgb& pixel, QVector3D& vector) -> void { 				float newValue = QColor::fromRgb(pixel).redF();
				vector = QVector3D{ newValue, newValue, newValue };
				});
			emit finishedComputing();
			});
		startConvert += processImageSize;
	}
	threads.at(threadNumber - 1) = std::thread([=] {
		convertVector<QRgb, QVector3D>(sharpened, sharpenedVec, startConvert, imageSize - processImageSize * (threadNumber - 1), [=](QRgb& pixel, QVector3D& vector) -> void {
			float newValue = QColor::fromRgb(pixel).redF();
			vector = QVector3D{ newValue, newValue, newValue };
			});
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}

	applyNormalMap(); 
}

void NormalMapWidget::saveImg()
{
	if (finalImage.isNull()) return;
	auto path = QFileDialog::getSaveFileName(this, "Save image", QDir::homePath(), tr("JPEG(*.jpg);;PNG(*.png);;Bitmap(*.bmp)"));
	if (path.isEmpty() || path.isNull()) return;
	finalImage.save(path);
}
void NormalMapWidget::intensityChanged(int value)
{
	ui.intensityValue->setText(QString::number(value));
	if (finalImage.isNull()) return;
}
void NormalMapWidget::sharpenChanged(int value)
{
	ui.sharpenValue->setText(QString::number(value));
	if (finalImage.isNull()) return;

}
void NormalMapWidget::edgeDetectionChanged(int item)
{
	switch (item) {
		case 0: edgeDetection.reset(new Sobel()); break; 	
		case 1: edgeDetection.reset(new Prewitt()); break; 	
		case 2: edgeDetection.reset(new Schaar()); break; 	
	}
		
	if (finalImage.isNull()) return;
	applyNormalMap(); 


}
