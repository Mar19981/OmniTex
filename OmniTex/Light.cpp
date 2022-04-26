#include "Light.h"

Light::Light(QVector3D& p): Light()
{
	offset = p;
}

Light::Light(): offset(QVector3D(0.0f, 0.0f, 0.0f)), color(QVector3D(1.0f, 1.0f, 1.0f)), attenuation(QVector3D(1.0f, 0.09, 0.032)),
	specular(QVector3D(1.0f, 1.0f, 1.0f)), ambient(QVector3D(0.2f, 0.2f, 0.2f)), intensity(1.0f), xConstraints(std::make_pair<float, float>(-20.0f, 20.0f)),
	yConstraints(std::make_pair<float, float>(-20.0f, 20.0f)), zConstraints(std::make_pair<float, float>(0.0f, 40.0f))
{
}

void Light::setPosition(QVector3D& p)
{
	offset = p;
}

void Light::setColor(QVector3D& c)
{
	color = c;
}

void Light::setAttenuation(QVector3D& a)
{
	attenuation = a;
}

void Light::setSpecular(QVector3D& s)
{
	specular = s;
}

void Light::setAmbient(QVector3D& a)
{
	ambient = a;
}

void Light::setIntensity(const float i)
{
	intensity = i;
}

void Light::setXConstaints(const float min, const float max)
{
	xConstraints.first = min;
	xConstraints.second = max;
}

void Light::setYConstaints(const float min, const float max)
{
	yConstraints.first = min;
	yConstraints.second = max;
}

void Light::setZConstaints(const float min, const float max)
{
	zConstraints.first = min;
	zConstraints.second = max;
}

void Light::translate(float x, float y, float z = 0.0f)
{
	QVector3D translation{ x, y, z };
	offset += translation;
	offset.setX(std::clamp(offset.x(), xConstraints.first, xConstraints.second));
	offset.setY(std::clamp(offset.y(), yConstraints.first, yConstraints.second));
	offset.setZ(std::clamp(offset.z(), zConstraints.first, zConstraints.second));
}
