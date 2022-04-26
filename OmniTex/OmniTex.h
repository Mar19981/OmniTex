#pragma once

#include <QtWidgets/QMainWindow>
#include <QMessageBox>
#include <memory>
#include "ui_OmniTex.h"
#include "EditorView.h"
#include "LoadView.h"
#include "LoadingScreen.h"


class OmniTex : public QMainWindow
{
    Q_OBJECT

public:
    OmniTex(QWidget *parent = nullptr); 
protected:
    virtual void closeEvent(QCloseEvent* e) override; private:
    Ui::OmniTexClass ui;     
    EditorView* editorView;     
    LoadView* loadView;     
    LoadingScreen* loadingScreen; 
};