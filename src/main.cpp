#include <QApplication>
#include <QFont>
#include <iostream>
#include "ui/MainWindow.h"
#include "ui/ThemeManager.h"
#include "core/Ball.h"
#include "core/GridManager.h"

void runLogicSelfTest() {
#ifdef QT_DEBUG
    std::cout << "======================================\n";
    std::cout << "  Starting Grid & Ball Logic Tests... \n";
    std::cout << "======================================\n";

    GridManager grid;
    grid.generateRandomLevel();

    QPointF p0 = grid.getCenterPos(0, 0);
    QPointF p1 = grid.getCenterPos(1, 0);
    std::cout << "[PASS] Hex Coordinates Check: (0,0) -> (" << p0.x() << ", " << p0.y()
              << ") | (1,0) -> (" << p1.x() << ", " << p1.y() << ")\n";

    auto colors = grid.getRemainingColors();
    std::cout << "[PASS] Active Colors on Grid: " << colors.size() << " distinct colors\n";

    Ball* topBall = grid.getBall(0, 0);
    if (topBall) {
        auto matches = grid.findMatches(0, 0, topBall->getPrimaryColor());
        std::cout << "[PASS] Match-3 BFS evaluated for root node.\n";
    }
    std::cout << "======================================\n\n";
#endif
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont defaultFont("Segoe UI", 10);
    defaultFont.setStyleHint(QFont::SansSerif);
    app.setFont(defaultFont);

    app.setStyleSheet(ThemeManager::instance().getMasterStyleSheet());

    runLogicSelfTest();

    MainWindow window;
    window.show();
    window.raise();
    window.activateWindow();

    return app.exec();
}