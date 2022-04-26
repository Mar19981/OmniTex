#pragma once
#include <QVector3D>
#include <utility>
#include <algorithm>

class Light
{
public:
	Light(QVector3D& p);
	Light();
	void setPosition(QVector3D& p);
	void setColor(QVector3D& c);
	void setAttenuation(QVector3D& a);
	void setSpecular(QVector3D& s);
	void setAmbient(QVector3D& a);
	void setIntensity(const float i);
	void setXConstaints(const float min, const float max);
	void setYConstaints(const float min, const float max);
	void setZConstaints(const float min, const float max);
	QVector3D getColor() { return color * intensity; };
	QVector3D getOffset() { return offset; };
	QVector3D getAttenuation() { return attenuation; };
	QVector3D getSpecular() { return specular * intensity; };
	QVector3D getAmbient() { return ambient * intensity; };
	void translate(float x, float y, float z);
private:
	QVector3D offset, color, attenuation, specular, ambient; 	
	float intensity; 	
	std::pair<float, float> xConstraints, yConstraints, zConstraints; 
};

