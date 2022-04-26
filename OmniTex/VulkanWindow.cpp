
#include "VulkanWindow.h"
#include "VulkanRenderer.h";

constexpr float DELTA_MULTIPLIER = 1.0f / 10.0f; 
void VulkanWindow::mousePressEvent(QMouseEvent* e)
{
	switch (e->button()) {
		case Qt::LeftButton: leftButtonPressed = true; break;
		case Qt::RightButton: rightButtonPressed = true; break;
		case Qt::MiddleButton: middleButtonPressed = true; break;
	}
	lastPos = e->pos(); 
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent* e)
{
		leftButtonPressed = middleButtonPressed = rightButtonPressed = false;
}

void VulkanWindow::mouseMoveEvent(QMouseEvent* e)
{
	if (!leftButtonPressed && !middleButtonPressed && !rightButtonPressed) return;
	int dx = e->pos().x() - lastPos.x(), dy = e->pos().y() - lastPos.y();
	if (leftButtonPressed) {
			if (dy)
				renderer->pitch(dy * DELTA_MULTIPLIER);
			if(dx)
				renderer->yaw(dx * DELTA_MULTIPLIER);
	}
	else if (rightButtonPressed) {
		renderer->translateLight(dx * DELTA_MULTIPLIER, dy * DELTA_MULTIPLIER, 0.0f);
	}
	else if (middleButtonPressed) {
		renderer->translateLight(0.0f, 0.0f, dy * DELTA_MULTIPLIER);
	}
	lastPos = e->pos(); }

void VulkanWindow::keyPressEvent(QKeyEvent* e)
{
	const float amount = 1.0f; 	
	switch (e->key()) 
	{
	    case Qt::Key_W: 			
			renderer->walk(amount);
			break;
		case Qt::Key_S: 			
			renderer->walk(-amount);
			break;
		case Qt::Key_A: 			
			renderer->strafe(-amount);
			break;
		case Qt::Key_D: 			
			renderer->strafe(amount);
			break;
		default:
			break;
	}
}

VulkanWindow::VulkanWindow(bool dbg, QImage& df, QImage& sp, QImage& dp, QImage& n, QImage& a) :
	debug(dbg), diffuse(df), specular(sp), displacement(dp), normal(n), ao(a), leftButtonPressed(false), rightButtonPressed(false), middleButtonPressed(false)
{
}

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
	renderer = new VulkanRenderer(this, diffuse, specular, displacement, normal, ao);
	diffuse.~QImage();
	specular.~QImage();
	displacement.~QImage();
	normal.~QImage();
	ao.~QImage();
	return renderer;
}

void VulkanWindow::updateTexture(QImage& img, MapType type)
{
	renderer->updateTexture(img, type);
}

void VulkanWindow::changeMesh(PrimitiveType type)
{
	renderer->changeMesh(type);
}

PrimitiveType VulkanWindow::getMeshType()
{
	return renderer->getMeshType();
}

void VulkanWindow::loadModel(const std::string& path)
{
	renderer->loadModel(path);
}

void VulkanWindow::setUV(QVector2D u)
{
	renderer->setUV(u);
}

void VulkanWindow::setLightColor(QColor c)
{
	renderer->setLightColor(c);
}

void VulkanWindow::setShininess(float amount)
{
	renderer->setShininess(amount);
}

void VulkanWindow::setDisplacementFactor(float amount)
{
	renderer->setDisplacementFactor(amount);
}

void VulkanWindow::setLightIntensity(float amount)
{
	renderer->setLightIntensity(amount);
}

void VulkanWindow::setBackground(QColor c)
{
	renderer->setBackground(c);
}

void VulkanWindow::toggleAnimating()
{
	renderer->toogleAnimating();
}
