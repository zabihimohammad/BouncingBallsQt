#include "MainWindow.h"
#include "PauseDialog.h"
#include "GameOverDialog.h"
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Bouncing Balls - Sharif CE Project");
    setFixedSize(380, 660);
    setStyleSheet("background-color: #1e272e;");

    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    m_mainMenu = new MainMenuWidget(this);
    m_startMenu = new StartGameWidget(this);
    m_settingsWidget = new SettingsWidget(this);
    m_scoreboardWidget = new ScoreboardWidget(&m_scoreManager, this);

    m_stackedWidget->addWidget(m_mainMenu);         // Index 0
    m_stackedWidget->addWidget(m_startMenu);        // Index 1
    m_stackedWidget->addWidget(m_settingsWidget);     // Index 2
    m_stackedWidget->addWidget(m_scoreboardWidget);   // Index 3

    connect(m_mainMenu, &MainMenuWidget::startGameClicked, this, [this]() { m_stackedWidget->setCurrentIndex(1); });
    connect(m_mainMenu, &MainMenuWidget::settingsClicked, this, [this]() { m_stackedWidget->setCurrentIndex(2); });
    connect(m_mainMenu, &MainMenuWidget::scoreboardClicked, this, [this]() {
        m_scoreboardWidget->refresh();
        m_stackedWidget->setCurrentIndex(3);
    });
    connect(m_mainMenu, &MainMenuWidget::exitClicked, this, &QMainWindow::close);

    connect(m_startMenu, &StartGameWidget::launchGame, this, &MainWindow::startNewGame);
    connect(m_startMenu, &StartGameWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
    connect(m_settingsWidget, &SettingsWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
    connect(m_scoreboardWidget, &ScoreboardWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
}

void MainWindow::startNewGame(const QString& username, const QString& mode) {
    m_currentUser = username;
    m_currentMode = mode;

    m_gameScene = new GameScene(username, mode, this);
    m_gameView = new GameView(this);
    m_gameView->setScene(m_gameScene);

    connect(m_gameScene, &GameScene::gameOver, this, &MainWindow::handleGameOver);
    connect(m_gameScene, &GameScene::gameWon, this, &MainWindow::handleGameWon);
    connect(m_gameScene, &GameScene::pauseRequested, this, &MainWindow::showPauseMenu);

    m_stackedWidget->addWidget(m_gameView);
    m_stackedWidget->setCurrentWidget(m_gameView);
}

void MainWindow::handleGameOver(int score) {
    m_scoreManager.saveScore(m_currentUser, m_currentMode, score);
    auto dlg = new GameOverDialog(false, m_currentUser, score, this);
    connect(dlg, &GameOverDialog::returnToMenu, this, &MainWindow::returnToMainMenu);
    dlg->exec();
}

void MainWindow::handleGameWon(int score) {
    m_scoreManager.saveScore(m_currentUser, m_currentMode, score);
    auto dlg = new GameOverDialog(true, m_currentUser, score, this);
    connect(dlg, &GameOverDialog::returnToMenu, this, &MainWindow::returnToMainMenu);
    dlg->exec();
}

void MainWindow::showPauseMenu() {
    m_gameScene->pauseGame();
    auto dlg = new PauseDialog(this);
    connect(dlg, &PauseDialog::resumeGame, this, [this]() { m_gameScene->resumeGame(); });
    connect(dlg, &PauseDialog::exitToMenu, this, &MainWindow::returnToMainMenu);
    dlg->exec();
}

void MainWindow::returnToMainMenu() {
    m_stackedWidget->setCurrentIndex(0);
    if (m_gameView) {
        m_stackedWidget->removeWidget(m_gameView);
        delete m_gameView;
        m_gameView = nullptr;
        m_gameScene = nullptr;
    }
}
