#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class MainMenuWidget : public QWidget {
    Q_OBJECT
public:
    MainMenuWidget(QWidget* parent = nullptr);

signals:
    void startGameClicked();
    void scoreboardClicked();
    void settingsClicked();
    void exitClicked();
};
