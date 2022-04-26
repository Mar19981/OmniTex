#include "OmniTex.h"
OmniTex::OmniTex(QWidget *parent)
    : QMainWindow(parent)
{
    editorView = new EditorView(this);
    loadView = new LoadView(this);
    loadingScreen = new LoadingScreen(this);
    ui.setupUi(this);
    ui.widgets->addWidget(loadView);
    ui.widgets->addWidget(loadingScreen);
    ui.widgets->addWidget(editorView);
   connect(loadView, &LoadView::imageLoaded, this, [=](QImage img) {         
       ui.widgets->setCurrentIndex(1);
        editorView->setImage(img);         
       });     
    connect(editorView, &EditorView::finishedComputing, this, [=]() {         
        loadingScreen->incrementProgress();
        });    
    connect(loadingScreen, &LoadingScreen::loadingFinished, this, [=]() {         
        ui.widgets->setCurrentIndex(2);
        });
    
}

void OmniTex::closeEvent(QCloseEvent* e)
{
    auto info = QMessageBox::question(this, "Close", "Close the application?", QMessageBox::Close | QMessageBox::Cancel);
    switch (info) {
        case QMessageBox::Close: e->accept(); break;     
        case QMessageBox::Cancel: e->ignore(); break;     
    }
}
