#include <QVulkanInstance>
#include <QtWidgets/QApplication>
#include <QLoggingCategory>
#include "OmniTex.h"
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "InvalidImageException.h"
#include "InvalidModelException.h"
#include "FatalException.h"

#ifdef NDEBUG
const bool DBG = false;
#else
#include <vld.h>
const bool DBG = true;
#endif 
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
        if (DBG)
        QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    OmniTex mainApp;
    auto sizeHint = mainApp.sizeHint();
        mainApp.setMinimumSize(sizeHint.width() * 1.5, sizeHint.height());
    try {
                mainApp.show();
    }
    catch (InvalidImageException& e) {
        QMessageBox::warning(&mainApp, "Error", e.what(), QMessageBox::Ok);
    }    
    catch (InvalidModelException& e) {
        QMessageBox::warning(&mainApp, "Error", e.what(), QMessageBox::Ok);
    }    
    catch (FatalException& e) {
        mainApp.hide();
        QMessageBox::critical(&mainApp, "Error", e.what(), QMessageBox::Close);
        return -1;
    }
    catch (std::exception& e) {
        QMessageBox::warning(&mainApp, "Error", e.what(), QMessageBox::Ok);
    }
    return app.exec();
}
