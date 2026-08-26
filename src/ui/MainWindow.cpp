#include "MainWindow.h"
#include "PauseDialog.h"
#include "GameOverDialog.h"
#include <QStatusBar>
#include <QKeyEvent>
#include <QTimer>
#include <QMessageBox>
#include "../core/SoundManager.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Bouncing Balls - Sharif CE Project");
    
    setMinimumSize(800, 600);
    setStyleSheet("background-color: #1e272e;");
    showFullScreen();

    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // ساخت تمام صفحات
    m_mainMenu = new MainMenuWidget(this);
    m_startMenu = new StartGameWidget(this);
    m_settingsWidget = new SettingsWidget(this);
    m_scoreboardWidget = new ScoreboardWidget(&m_scoreManager, this);
    m_advSettingsWidget = new AdvancedSettingsWidget(this); 

    // اضافه کردن به StackedWidget
    m_stackedWidget->addWidget(m_mainMenu);          // Index 0
    m_stackedWidget->addWidget(m_startMenu);         // Index 1
    m_stackedWidget->addWidget(m_settingsWidget);      // Index 2
    m_stackedWidget->addWidget(m_scoreboardWidget);    // Index 3
    m_stackedWidget->addWidget(m_advSettingsWidget);   // Index 4

    // -----------------------------------------------------------------
    // اتصالات منوی اصلی
    // -----------------------------------------------------------------
    connect(m_mainMenu, &MainMenuWidget::startGameClicked, this, [this]() { m_stackedWidget->setCurrentIndex(1); });
    connect(m_mainMenu, &MainMenuWidget::settingsClicked, this, [this]() { m_stackedWidget->setCurrentIndex(2); });
    connect(m_mainMenu, &MainMenuWidget::scoreboardClicked, this, [this]() {
        m_scoreboardWidget->refresh();
        m_stackedWidget->setCurrentIndex(3);
    });
    connect(m_mainMenu, &MainMenuWidget::helpClicked, this, [this]() {
        QMessageBox::information(this, "FIELD MANUAL", "Welcome to CYBER BOUNCE!\n\n1. Play Game to launch your mission.\n2. In-game, use left click to shoot/interact depending on the mode.\n3. Check Leaderboard to see your global standing.\n4. Use Settings to tune graphics and audio.\n\nGood luck, Operative!");
    });
    connect(m_mainMenu, &MainMenuWidget::exitClicked, this, &QMainWindow::close);

    // -----------------------------------------------------------------
    // اتصالات بازگشت و تنظیمات
    // -----------------------------------------------------------------
    connect(m_startMenu, &StartGameWidget::launchGame, this, &MainWindow::startNewGame);
    connect(m_startMenu, &StartGameWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
    connect(m_scoreboardWidget, &ScoreboardWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
    
    // بازگشت از مینی‌گیم تنظیمات به منوی اصلی
    connect(m_settingsWidget, &SettingsWidget::backClicked, this, [this]() { m_stackedWidget->setCurrentIndex(0); });
    
    // شلیک به حباب PRO MODE -> رفتن به تنظیمات پیشرفته (ایندکس ۴)
    connect(m_settingsWidget, &SettingsWidget::proModeClicked, this, [this]() {
        m_stackedWidget->setCurrentIndex(4);
    });

    // بازگشت از تنظیمات پیشرفته -> رفتن به مینی‌گیم (ایندکس ۲)
    connect(m_advSettingsWidget, &AdvancedSettingsWidget::backClicked, this, [this]() {
        m_stackedWidget->setCurrentIndex(2);
    });

    // اعمال تغییر تمام‌صفحه از مینی‌گیم
    connect(m_settingsWidget, &SettingsWidget::fullscreenToggled, this, [this](bool enabled) {
        if (enabled) {
            showFullScreen();
        } else {
            showNormal();
        }
    });

    // ===> سیستم هوشمند مدیریت منابع: توقف پردازش‌های پنهان <===
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, [this](int index) {
        for (int i = 0; i < m_stackedWidget->count(); ++i) {
            QWidget* w = m_stackedWidget->widget(i);
            if (!w) continue;
            
            if (i == index) {
                QMetaObject::invokeMethod(w, "resumeAnimation");
            } else {
                QMetaObject::invokeMethod(w, "pauseAnimation");
            }
        }
    });
    
    // فراخوانی دستی برای بار اول تا فقط تایمر منوی اصلی روشن بماند
    emit m_stackedWidget->currentChanged(0);

    // ===> این خط اضافه شد تا موتور صدا به محض باز شدن بازی بیدار شود
    SoundManager::instance(); 
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

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}