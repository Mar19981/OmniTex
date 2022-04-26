
#include "DisplacementMapWidget.h"

DisplacementMapWidget::DisplacementMapWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	threadNumber = std::thread::hardware_concurrency();
	lutSize = 256 / threadNumber; 	threads.resize(threadNumber);
	ui.brightnessValue->setText(QString::number(ui.brightnessSlider->value()));
	ui.sharpenValue->setText(QString::number(ui.sharpenSlider->value()));
	ui.contrastValue->setText(QString::number(ui.contrastSlider->value()));
	ui.blurValue->setText(QString::number(ui.blurSlider->value()));
	ui.preview->setFixedSize(128, 128);


	connect(ui.brightnessSlider, &QSlider::valueChanged, this, &DisplacementMapWidget::brightnessChanged);
	connect(ui.contrastSlider, &QSlider::valueChanged, this, &DisplacementMapWidget::contrastChanged);
	connect(ui.sharpenSlider, &QSlider::valueChanged, this, &DisplacementMapWidget::sharpenChanged);
	connect(ui.blurSlider, &QSlider::valueChanged, this, &DisplacementMapWidget::blurChanged);
	connect(ui.sharpenSlider, &QSlider::sliderReleased, this, &DisplacementMapWidget::applySharpen);
	connect(ui.contrastSlider, &QSlider::sliderReleased, this, &DisplacementMapWidget::applyContrast);
	connect(ui.brightnessSlider, &QSlider::sliderReleased, this, &DisplacementMapWidget::applyBrightness);
	connect(ui.blurSlider, &QSlider::sliderReleased, this, &DisplacementMapWidget::applyBlur);
	connect(ui.loadButton, &QPushButton::clicked, this, &DisplacementMapWidget::loadImg);
	connect(ui.saveButton, &QPushButton::clicked, this, &DisplacementMapWidget::saveImg);
	connect(ui.invertCheck, &QCheckBox::stateChanged, this, &DisplacementMapWidget::invertChanged);
	connect(ui.originalCheck, &QCheckBox::stateChanged, this, &DisplacementMapWidget::imageSwitch);
}

DisplacementMapWidget::~DisplacementMapWidget()
{
}
void DisplacementMapWidget::setImage(QImage& img)
{
	originalImage = img;
	ui.originalCheck->setEnabled(false);
	width = img.width();
	height = img.height();
	imageSize = width * height;
	processImageSize = imageSize / threadNumber;

	finalImage = QImage{ width, height, img.format() };

	original.clear();
	brightnessImage.clear();
	results.clear();
	sharpenImage.clear();
	blurImage.clear();
	original.resize(imageSize);
	brightnessImage.resize(imageSize);
	results.resize(imageSize);
	sharpenImage.resize(imageSize);
	blurImage.resize(imageSize);

	std::memcpy(original.data(), img.bits(), imageSize * sizeof(QRgb));

	preprocessImage();
}
void DisplacementMapWidget::loadImg()
{
	auto path = QFileDialog::getOpenFileName(this, "Load image", QDir::homePath(), tr("Image(*.jpg *.jpeg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;Bitmap(*.bmp)"));
	if (path.isEmpty() || path.isNull()) return;
	QImage input(path);
	if (input.isNull()) throw InvalidImageException("Failed to load image!"); 	
	setImage(input); 		
	ui.originalCheck->setChecked(true);
	ui.originalCheck->setEnabled(true);
	imageSwitch(true);
}

void DisplacementMapWidget::saveImg()
{
	if (finalImage.isNull()) return;
	auto path = QFileDialog::getSaveFileName(this, "Save image", QDir::homePath(), tr("JPEG(*.jpg);;PNG(*.png);;Bitmap(*.bmp)"));
	if (path.isEmpty() || path.isNull()) return;
	finalImage.save(path);
}
void DisplacementMapWidget::brightnessChanged(int value)
{
	ui.brightnessValue->setText(QString::number(value));
	if (finalImage.isNull()) return;
}

void DisplacementMapWidget::contrastChanged(int value)
{
	ui.contrastValue->setText(QString::number(value));
	if (finalImage.isNull()) return;
}

void DisplacementMapWidget::sharpenChanged(int value)
{
	ui.sharpenValue->setText(QString::number(value));
	if (finalImage.isNull()) return;
}

void DisplacementMapWidget::blurChanged(int value)
{
	ui.blurValue->setText(QString::number(value));
	if (finalImage.isNull()) return;
}

void DisplacementMapWidget::applyFinalImage()
{
	std::memmove(finalImage.bits(), results.data(), results.size() * sizeof(QRgb));
	ui.preview->setPixmap(QPixmap::fromImage(finalImage).scaled(ui.preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	finalImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
	emit imageComputed();
}


void DisplacementMapWidget::preprocessImage()
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
		if (ui.invertCheck->isChecked()) {
		applyInvert();
		return;
		}
	applySharpen();
}

void DisplacementMapWidget::applyBrightness()
{
	if (finalImage.isNull()) return;
	int start{}, startProcessing{}, b = remap<int>(ui.brightnessSlider->value(), 0, 100, 100, 255);
	brightnessImage = blurImage; 

	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			generateLUT<uint8_t>(brightnessLut, start, lutSize, [=](uint8_t& pixel, uint8_t index) {
				pixel = std::clamp(index + b, 0, 255); 				
				});
			emit finishedComputing();
			});
		start += lutSize;
	}

	threads.at(threadNumber - 1) = std::thread([=] {
		generateLUT<uint8_t>(brightnessLut, start, 255 - lutSize * (threadNumber - 1), [=](uint8_t& pixel, uint8_t index) {
			pixel = std::clamp(index + b, 0, 255);
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
		}

		for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			applyLambda<QRgb>(brightnessImage, startProcessing, processImageSize, [=](QRgb& pixel) -> void {
				uint8_t newValue = brightnessLut.at(QColor(pixel).red()); 				
				pixel = qRgb(newValue, newValue, newValue);
				});
			emit finishedComputing();
			});
		startProcessing += processImageSize;
		}
	threads.at(threadNumber - 1) = std::thread([=] {
		applyLambda<QRgb>(brightnessImage, startProcessing, imageSize - processImageSize * (threadNumber - 1), [=](QRgb& pixel) -> void {
			uint8_t newValue = brightnessLut.at(QColor(pixel).red());
			pixel = qRgb(newValue, newValue, newValue);
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
		}
	applyContrast(); 
}


void DisplacementMapWidget::applyContrast()
{
	if (finalImage.isNull()) return;
	int start{}, startProcessing{};
	results = brightnessImage; 
	float a = remap<float>(ui.contrastSlider->value(), 0.0f, 255.0f, 0.5f, 1.5f);

	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			generateLUT<uint8_t>(contrastLut, start, lutSize, [=](uint8_t& pixel, uint8_t index) {
				pixel = std::clamp(static_cast<int>(a * (index - 127) + 127), 0, 255); 				
				});
			emit finishedComputing();
			});

		start += lutSize;
	}
	threads.at(threadNumber - 1) = std::thread([=] {
		generateLUT<uint8_t>(contrastLut, start, 255 - lutSize * (threadNumber - 1), [=](uint8_t& pixel, uint8_t index) {
			pixel = std::clamp(static_cast<int>(a * (index - 127) + 127), 0, 255);
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
		}
		for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			applyLambda<QRgb>(results, startProcessing, processImageSize, [=](QRgb& pixel) -> void {
				uint8_t newValue = contrastLut.at(QColor(pixel).red()); 				
				pixel = qRgb(newValue, newValue, newValue);
				});
			emit finishedComputing();
			});
		startProcessing += processImageSize;
		}

	threads.at(threadNumber - 1) = std::thread([=] {
		applyLambda<QRgb>(results, startProcessing, imageSize - processImageSize * (threadNumber - 1), [=](QRgb& pixel) -> void {
			uint8_t newValue = contrastLut.at(QColor(pixel).red());
			pixel = qRgb(newValue, newValue, newValue);
			});
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applyFinalImage(); 
}


void DisplacementMapWidget::applySharpen()
{
		if (finalImage.isNull()) return;
	int start{ 1 }, yStep{ (height - 2) / threadNumber }, process{ imageSize - 2 * width - 2 * height }, size{ process / threadNumber };

	sharpenImage = original; 
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			sharpen.apply(sharpenImage, start, width, size, ui.sharpenSlider->value()); 			
			emit finishedComputing();
			});
		start += yStep;
	}

	threads.at(threadNumber - 1) = std::thread([=] {
		sharpen.apply(sharpenImage, start, width, process - size * (threadNumber - 1), ui.sharpenSlider->value());
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
		}

	applyBlur(); 
}

void DisplacementMapWidget::applyBlur()
{
	if (finalImage.isNull()) return;
	int start{ 1 }, yStep{ (height - 2) / threadNumber }, process{ imageSize - 2 * width - 2 * height }, size{ process / threadNumber };

	blurImage = sharpenImage; 
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			blur.apply(blurImage, start, width, size, ui.blurSlider->value());  			
			emit finishedComputing();
			});
		start += yStep;
	}

	threads.at(threadNumber - 1) = std::thread([=] {
		blur.apply(blurImage, start, width, process - size * (threadNumber - 1), ui.blurSlider->value());
		emit finishedComputing();
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applyBrightness(); 
}

void DisplacementMapWidget::applyInvert()
{
	int start{};
	for (int i = 0; i < threadNumber - 1; i++) {
		threads.at(i) = std::thread([=] {
			applyLambda<QRgb>(original, start, processImageSize, [=](QRgb& pixel) -> void {
				uint8_t newValue = 255 - QColor::fromRgb(pixel).red(); 				
				pixel = qRgb(newValue, newValue, newValue);
				});
			});
		start += processImageSize;
	}
	threads.at(threadNumber - 1) = std::thread([=] {
		applyLambda<QRgb>(original, start, imageSize - processImageSize * (threadNumber - 1), [=](QRgb& pixel) -> void {
			uint8_t newValue = 255 - QColor::fromRgb(pixel).red();
			pixel = qRgb(newValue, newValue, newValue);
			});
		});
		for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	applySharpen(); 
}

void DisplacementMapWidget::toogleEnabledInputs(bool state)
{
	ui.blurSlider->setEnabled(state);
	ui.sharpenSlider->setEnabled(state);
	ui.brightnessSlider->setEnabled(state);
	ui.contrastSlider->setEnabled(state);
	ui.invertCheck->setEnabled(state);
}
void DisplacementMapWidget::invertChanged(int value)
{
	if (finalImage.isNull()) return;
	applyInvert(); 
}

void DisplacementMapWidget::imageSwitch(int state)
{
	if (state) {
		toogleEnabledInputs(false);
		finalImage = originalImage;
		ui.preview->setPixmap(QPixmap::fromImage(originalImage).scaled(ui.preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		emit imageComputed();
	}
	else {
		applyFinalImage();
		toogleEnabledInputs(true);
	}
}
