#include "EditorView.h"

#ifdef NDEBUG
const bool dbg = false;
#else
const bool dbg = true;
#endif

EditorView::EditorView(QWidget *parent)
	: QWidget(parent), normal(new NormalMapWidget(this)), specular(new SpecularMapWidget(this)), 
	displacement(new DisplacementMapWidget(this)), ao(new AOMapWidget(this)), verticalLayout(new QVBoxLayout()),
	horizontalLayout(new QHBoxLayout()), materialsLayout(new QVBoxLayout()), 
	lightLayout(new QVBoxLayout()), mappingLayout(new QVBoxLayout()), buttonsLayout(new QVBoxLayout()),
	backgroundColorLayout(new QHBoxLayout()), uLayout(new QHBoxLayout()), vLayout(new QHBoxLayout()),
	lightIntensityLayout(new QHBoxLayout()), lightColorLayout(new QHBoxLayout()), displacementLayout(new QHBoxLayout()), 
	shininessLayout(new QHBoxLayout()), backgroundColor(QColor()), lightColor(QColor(255, 255, 255)), lightColorChanging(false)
{
	gridLayout = new QGridLayout(this);
    verticalLayout->setSpacing(6);
    mapsControlWidget = new QStackedWidget(this);
	colorDialog = new QColorDialog(this);
	colorDialog->setOptions(QColorDialog::ColorDialogOption::DontUseNativeDialog);
    verticalLayout->addWidget(mapsControlWidget);
	horizontalLayout->setSpacing(6);
    normalButton = new QPushButton(this);
    normalButton->setText("Normal");

    horizontalLayout->addWidget(normalButton);

    specularButton = new QPushButton(this);
    specularButton->setText("Specular");

	meshSelect = new QComboBox(this);

	meshSelect->addItem("Sphere");
	meshSelect->addItem("Box");
	meshSelect->addItem("Cylinder");
	meshSelect->addItem("Teapot");
	meshSelect->addItem("Plane");
	meshSelect->addItem("Custom");

    horizontalLayout->addWidget(specularButton);

    displacementButton = new QPushButton(this);
    displacementButton->setText("Displacement");

    horizontalLayout->addWidget(displacementButton);

    aoButton = new QPushButton(this);
	aoButton->setText("Occlusion");

    horizontalLayout->addWidget(aoButton);


    verticalLayout->addLayout(horizontalLayout);

    mapsControlWidget->setCurrentIndex(0);

		inst.setLayers(QByteArrayList()
		<< "VK_LAYER_GOOGLE_threading"
		<< "VK_LAYER_LUNARG_parameter_validation"
		<< "VK_LAYER_LUNARG_object_tracker"
		<< "VK_LAYER_LUNARG_core_validation"
		<< "VK_LAYER_LUNARG_image"
		<< "VK_LAYER_LUNARG_swapchain"
		<< "VK_LAYER_GOOGLE_unique_objects");

	if (!inst.create()) qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

	mapsControlWidget->addWidget(normal);
	mapsControlWidget->addWidget(specular);
	mapsControlWidget->addWidget(displacement);
	mapsControlWidget->addWidget(ao);

	groupBox = new QGroupBox(this);
	groupBox->setTitle("Parameters");

	paramsGridLayout = new QGridLayout(groupBox);

	exportAllButton = new QPushButton(groupBox);
	loadDiffuseButton = new QPushButton(groupBox);
	loadAllButton = new QPushButton(groupBox);
	lightColorButton = new QPushButton(groupBox);
	backgroundColorButton = new QPushButton(groupBox);

	exportAllButton->setText("Export all");
	loadDiffuseButton->setText("Reload Diffuse");
	loadAllButton->setText("Reload All");
	lightColorButton->setStyleSheet("border: 0.25ex solid #e0e0e0; background: #FFFFFF;");
	backgroundColorButton->setStyleSheet("border: 0.25ex solid #e0e0e0; background: #000000;");

	uSpin = new QDoubleSpinBox(groupBox);
	vSpin = new QDoubleSpinBox(groupBox);
	shininessSpin = new QDoubleSpinBox(groupBox);
	displacementSpin = new QDoubleSpinBox(groupBox);
	lightIntesitySpin = new QDoubleSpinBox(groupBox);
	uSpin->setValue(1.0);
	vSpin->setValue(1.0);
	uSpin->setMinimum(0.01);
	vSpin->setMinimum(0.01);
	shininessSpin->setMinimum(0.0);
	shininessSpin->setMaximum(500.0);
	shininessSpin->setValue(150.0);
	displacementSpin->setValue(1.0);
	displacementSpin->setMinimum(-100.0);
	displacementSpin->setMaximum(100.0);
	lightIntesitySpin->setMinimum(0.0);
	lightIntesitySpin->setValue(1.0);
	lightIntesitySpin->setDecimals(1);


	materialPropsLabel = new QLabel(groupBox);
	mappingPropsLabel = new QLabel(groupBox);
	lightPropsLabel = new QLabel(groupBox);
	uLabel = new QLabel(groupBox);
	vLabel = new QLabel(groupBox);
	lightIntensityLabel = new QLabel(groupBox);
	lightLabel = new QLabel(groupBox);
	displacementLabel = new QLabel(groupBox);
	shininessLabel = new QLabel(groupBox);
	backgroundLabel = new QLabel(groupBox);


	materialPropsLabel->setText("Material Properties:");
	mappingPropsLabel->setText("Mapping Properties:");
	lightPropsLabel->setText("Light Properties:");
	uLabel->setText("U:");
	vLabel->setText("V:");
	lightIntensityLabel->setText("Intensity:");
	lightLabel->setText("Light:");	
	displacementLabel->setText("Displacement Factor:");
	shininessLabel->setText("Shininess:");
	backgroundLabel->setText("Background Color:");

	animateCheck = new QCheckBox(groupBox);
	animateCheck->setText("Animate");
	animateCheck->setChecked(false);

	mappingLayout->addWidget(mappingPropsLabel);
	uLayout->addWidget(uLabel);
	uLayout->addWidget(uSpin);
	uLayout->addStretch(0);
	mappingLayout->addLayout(uLayout);	
	vLayout->addWidget(vLabel);
	vLayout->addWidget(vSpin);
	vLayout->addStretch(0);
	mappingLayout->addLayout(vLayout);
	mappingLayout->addStretch(0);
	mappingLayout->addStretch(-1);

	buttonsLayout->addWidget(loadDiffuseButton);
	buttonsLayout->addWidget(loadAllButton);
	buttonsLayout->addWidget(exportAllButton);
	buttonsLayout->addStretch(0);
	buttonsLayout->addStretch(-1);

	materialsLayout->addWidget(materialPropsLabel);
	displacementLayout->addWidget(displacementLabel);
	displacementLayout->addWidget(displacementSpin);
	displacementLayout->addStretch(-1);
	materialsLayout->addLayout(displacementLayout);	
	shininessLayout->addWidget(shininessLabel);
	shininessLayout->addWidget(shininessSpin);
	shininessLayout->addStretch(-1);
	materialsLayout->addLayout(shininessLayout);	
	materialsLayout->addStretch(0);
	materialsLayout->addStretch(-1);
	
	lightLayout->addWidget(lightPropsLabel);
	lightIntensityLayout->addWidget(lightIntensityLabel);
	lightIntensityLayout->addWidget(lightIntesitySpin);
	lightIntensityLayout->addStretch(-1);
	lightLayout->addLayout(lightIntensityLayout);
	lightColorLayout->addWidget(lightLabel);
	lightColorLayout->addWidget(lightColorButton);
	lightColorLayout->addStretch(-1);	
	lightLayout->addLayout(lightColorLayout);
	backgroundColorLayout->addWidget(backgroundLabel);
	backgroundColorLayout->addWidget(backgroundColorButton);
	backgroundColorLayout->addStretch(-1);
	lightLayout->addLayout(backgroundColorLayout);
	lightLayout->addWidget(animateCheck);
	lightLayout->addStretch(0);
	lightLayout->addStretch(-1);

	paramsGridLayout->addLayout(materialsLayout, 0, 0, 2, 2);
	paramsGridLayout->addLayout(mappingLayout, 0, 2, 2, 2);
	paramsGridLayout->addLayout(lightLayout, 2, 0, 2, 2);
	paramsGridLayout->addLayout(buttonsLayout, 2, 2, 2, 2);

	gridLayout->addLayout(verticalLayout, 0, 4, 3, 2);
	gridLayout->addWidget(meshSelect, 2, 0, 1, 4);
	gridLayout->addWidget(groupBox, 0, 6, 3, 2);
	setLayout(gridLayout);
	

	connect(normalButton, &QPushButton::clicked, this, [=] {mapsControlWidget->setCurrentIndex(0); });
	connect(specularButton, &QPushButton::clicked, this, [=] {mapsControlWidget->setCurrentIndex(1); });
	connect(displacementButton, &QPushButton::clicked, this, [=] {mapsControlWidget->setCurrentIndex(2); });
	connect(aoButton, &QPushButton::clicked, this, [=] {mapsControlWidget->setCurrentIndex(3); });
	connect(normal, &NormalMapWidget::finishedComputing, this, [=] {emit finishedComputing(); });
	connect(ao, &AOMapWidget::finishedComputing, this, [=] {emit finishedComputing(); });
	connect(specular, &SpecularMapWidget::finishedComputing, this, [=] {emit finishedComputing(); });
	connect(displacement, &DisplacementMapWidget::finishedComputing, this, [=] {emit finishedComputing(); });	
	connect(animateCheck, &QCheckBox::stateChanged, this, [=] {vulkan->toggleAnimating(); });	
	connect(meshSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorView::meshChanged);
	
	connect(normal, &NormalMapWidget::imageComputed, this, [=] {
		if (!loaded) return;
		QImage img = normal->getImage();
		vulkan->updateTexture(img, MapType::NORMAL);
		
		});
	connect(ao, &AOMapWidget::imageComputed, this, [=] {
		if (!loaded) return;
		QImage img = ao->getImage();
		vulkan->updateTexture(img, MapType::AO);
		});
	connect(specular, &SpecularMapWidget::imageComputed, this, [=] {
		if (!loaded) return;
		QImage img = specular->getImage();
		vulkan->updateTexture(img, MapType::SPECULAR);
		});
	connect(displacement, &DisplacementMapWidget::imageComputed, this, [=] {
		if (!loaded) return;
		QImage img = displacement->getImage();
		vulkan->updateTexture(img, MapType::DISPLACEMENT);
		});

	connect(uSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
		QVector2D uv{ static_cast<float>(value), static_cast<float>(vSpin->value()) };
		vulkan->setUV(uv);
		});	
	connect(vSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
		QVector2D uv{ static_cast<float>(vSpin->value()), static_cast<float>(value) };
		vulkan->setUV(uv);
		});	
	connect(displacementSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
		vulkan->setDisplacementFactor(value);
		});		
	connect(lightIntesitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
		vulkan->setLightIntensity(value);
		});	
	connect(shininessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
		vulkan->setShininess(value);
		});

	connect(lightColorButton, &QPushButton::clicked, this, [=] {
		colorDialog->setCurrentColor(lightColor);
		lightColorChanging = true;
		colorDialog->open(this, SLOT(changeLightColor(QColor)));
		});	
	connect(backgroundColorButton, &QPushButton::clicked, this, [=] {
		colorDialog->setCurrentColor(backgroundColor);
		lightColorChanging = false;
		colorDialog->open(this, SLOT(changeBackgroundColor(QColor)));
		});	
	
	connect(loadDiffuseButton, &QPushButton::clicked, this, [=] {
			auto path = QFileDialog::getOpenFileName(this, "Load image", QDir::homePath(), tr("Image(*.jpg *.jpeg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;Bitmap(*.bmp)"));
			if (path.isEmpty() || path.isNull()) return;
			QImage img(path);
			if (img.isNull()) throw InvalidImageException("Failed to load image!");
			diffuse = img;
			vulkan->updateTexture(img, MapType::DIFFUSE);
			normal->setImage(img);
		});	
	connect(loadAllButton, &QPushButton::clicked, this, [=] {
			auto path = QFileDialog::getOpenFileName(this, "Load image", 
				QDir::homePath(), tr("Image(*.jpg *.jpeg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;Bitmap(*.bmp)"));
			if (path.isEmpty() || path.isNull()) return;
			QImage img(path);
			if (img.isNull()) throw InvalidImageException("Failed to load image!");
			diffuse = img;
			vulkan->updateTexture(img, MapType::DIFFUSE);
			normal->setImage(img);
			specular->setImage(img);
			displacement->setImage(img);
			ao->setImage(img);
		});	
	connect(exportAllButton, &QPushButton::clicked, this, [=] {
			auto path = QFileDialog::getSaveFileName(this, "Save images", 
				QDir::homePath(), tr("Image(*.jpg *.png *.bmp);;JPEG(*.jpg);;PNG(*.png);;Bitmap(*.bmp)"));
			if (path.isEmpty() || path.isNull()) return;
			QImage n = normal->getImage(),
				dp = displacement->getImage(),
				sp = specular->getImage(),
				a = ao->getImage();
			std::regex reg(R"(^(.*)(\.(jpg|png|bmp)))");
			std::smatch match;
			std::string pathStr = path.toStdString();
			std::string extension{};
			if (std::regex_match(pathStr, match, reg))
			{
				extension = "." + match[3].str();
			}
			pathStr = pathStr.substr(0, pathStr.length() - extension.length());
			diffuse.save((pathStr + "_DIFFUSE" + extension).c_str());
			n.save((pathStr + "_NORMAL" + extension).c_str());
			dp.save((pathStr + "_DISPLACEMENT" + extension).c_str());
			sp.save((pathStr + "_SPECULAR" + extension).c_str());
			a.save((pathStr + "_OCCLUSION" + extension).c_str());
		});

	connect(colorDialog, &QColorDialog::currentColorChanged, this, [=] (QColor c) {
		if (lightColorChanging) {
			changeLightColor(c);
			return;
		}
		else
			changeBackgroundColor(c);
		});
	connect(colorDialog, &QColorDialog::rejected, this, [=] () {
		lightColorChanging = false;
		});

}

EditorView::~EditorView()
{
}

void EditorView::setImage(QImage& img)
{
	diffuse = img; 		
	normal->setImage(img);
	specular->setImage(img);
	displacement->setImage(img);
	ao->setImage(img);
		QImage n = normal->getImage(),
		dp = displacement->getImage(),
		sp = specular->getImage(),
		a = ao->getImage();
		vulkan = new VulkanWindow(dbg, diffuse, sp, dp, n, a);
	vulkan->setVulkanInstance(&inst);
	QWidget* windowWrapper = QWidget::createWindowContainer(vulkan);
    gridLayout->addWidget(windowWrapper, 0, 0, 2, 4);
	loaded = true; 	emit finishedComputing(); }

void EditorView::changeLightColor(QColor c)
{
	std::stringstream ss;
	ss << "border: 0.25ex solid #e0e0e0; background: rgb(" << c.red() << ", " << c.green() << ", " << c.blue() << ")"; 
	lightColorButton->setStyleSheet(ss.str().c_str());
		vulkan->setLightColor(c);
	lightColor = c;
	lightColorChanging = false;
}

void EditorView::changeBackgroundColor(QColor c)
{

	std::stringstream ss;
	ss << "border: 0.25ex solid #e0e0e0; background: rgb(" << c.red() << ", " << c.green() << ", " << c.blue() << ")";
	backgroundColorButton->setStyleSheet(ss.str().c_str());
		vulkan->setBackground(c);
	backgroundColor = c;
}

void EditorView::meshChanged(int index)
{
	PrimitiveType type{};
	switch (index) {
		case 0: type = PrimitiveType::SPHERE; break;
		case 1: type = PrimitiveType::BOX; break;
		case 2: type = PrimitiveType::CYLINDER; break;
		case 3: type = PrimitiveType::TEAPOT; break;
		case 4: type = PrimitiveType::PLANE; break;
		case 5: type = PrimitiveType::CUSTOM; break;
	}
	if (type != PrimitiveType::CUSTOM)
		vulkan->changeMesh(type); 

	else {
		auto path = QFileDialog::getOpenFileName(this, "Select model", QDir::homePath(), "Obj (*.obj)");
		if (path.isEmpty() || path.isNull()) {
			switch (vulkan->getMeshType()) {
				case PrimitiveType::SPHERE: meshSelect->setCurrentIndex(0); break;
				case PrimitiveType::BOX: meshSelect->setCurrentIndex(1); break;
				case PrimitiveType::CYLINDER: meshSelect->setCurrentIndex(2); break;
				case PrimitiveType::TEAPOT: meshSelect->setCurrentIndex(3); break;
				case PrimitiveType::PLANE: meshSelect->setCurrentIndex(4); break;
			}
			return;
		}

		try { 			
			vulkan->loadModel(path.toStdString());
		}

		catch (InvalidModelException& e) { 			
			meshSelect->setCurrentIndex(0);
			vulkan->changeMesh(PrimitiveType::SPHERE);
			throw e;
		}

	}

}
