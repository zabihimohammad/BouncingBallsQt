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
#include "AdvancedSettingsWidget.h"
#include "HelpWidget.h"
#include "LevelSelectWidget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // ۱. متد بررسی درخواست شروع بازی و تفکیک انتخاب مرحله
    void handleLaunchRequest(const QString& username, const QString& mode);

    // ۲. متد شروع بازی با پشتیبانی از شماره مرحله (پیش‌فرض = ۱)
    void startNewGame(const QString& username, const QString& mode, int levelNumber = 1);

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
    AdvancedSettingsWidget* m_advSettingsWidget;
    HelpWidget* m_helpWidget;
    LevelSelectWidget* m_levelSelectWidget;

    GameView* m_gameView = nullptr;
    GameScene* m_gameScene = nullptr;
    ScoreManager m_scoreManager;

    QString m_currentUser;
    QString m_currentMode;
    int m_selectedLevel = 1;
};