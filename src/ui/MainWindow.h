#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QKeyEvent>
#include "../core/ScoreManager.h"
#include "../gameplay/GameScene.h"
#include "../gameplay/GameView.h"
#include "MainMenuWidget.h"
#include "StartGameWidget.h"
#include "SettingsWidget.h"
#include "ScoreboardWidget.h"
#include "AdvancedSettingsWidget.h" // <--- اضافه شد

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void startNewGame(const QString& username, const QString& mode);
    void handleGameOver(int score);
    void handleGameWon(int score);
    void showPauseMenu();
    void returnToMainMenu();

private:
    QStackedWidget* m_stackedWidget;
    MainMenuWidget* m_mainMenu;
    StartGameWidget* m_startMenu;
    SettingsWidget* m_settingsWidget;
    ScoreboardWidget* m_scoreboardWidget;
    AdvancedSettingsWidget* m_advSettingsWidget; // <--- اضافه شد

    GameView* m_gameView = nullptr;
    GameScene* m_gameScene = nullptr;
    ScoreManager m_scoreManager;

    QString m_currentUser;
    QString m_currentMode;
};