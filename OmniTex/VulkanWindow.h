#pragma once
#include <qvulkanwindow.h>
#include <QMouseEvent>
#include <QImage>
#include <memory>
#include "MapType.h"
#include "PrimitiveType.h"

class VulkanRenderer; 

class VulkanWindow :
    public QVulkanWindow
{
public:
                                    
    VulkanWindow(bool dbg, QImage& df, QImage& sp, QImage& dp, QImage& n, QImage& a);
    QVulkanWindowRenderer* createRenderer() override;     
    void updateTexture(QImage& img, MapType type); 
    bool isDebugEnabled() const { return debug; }     
    void changeMesh(PrimitiveType type);     
    PrimitiveType getMeshType();     
    void loadModel(const std::string& path);         
    void setUV(QVector2D u);
    void setLightColor(QColor c);
    void setShininess(float amount);
    void setDisplacementFactor(float amount);
    void setLightIntensity(float amount);
    void setBackground(QColor c);
    void toggleAnimating(); private:
    bool debug, leftButtonPressed, rightButtonPressed, middleButtonPressed;     
    VulkanRenderer* renderer;     QPoint lastPos;         
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    QImage diffuse, specular, displacement, ao, normal; };

