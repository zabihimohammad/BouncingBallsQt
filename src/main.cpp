#include <QApplication>
#include "ui/MainWindow.h"
#include "ui/ThemeManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(ThemeManager::getMasterStyleSheet());

    MainWindow window;
    window.show();

    return app.exec();
}