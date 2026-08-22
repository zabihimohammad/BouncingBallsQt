#pragma once
#include <QGraphicsScene>
#include <QTimer>
#include "../core/GridManager.h"
#include "CannonItem.h"
#include "AimLineItem.h"

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    GameScene(const QString& username, const QString& mode, QObject* parent = nullptr);

    void pauseGame();
    void resumeGame();
    int getScore() const { return m_score; }
    int getTimeLeft() const { return m_timeLeft; }

signals:
    void gameWon(int score);
    void gameOver(int score);
    void scoreChanged(int score);
    void timeChanged(int timeLeft);
    void pauseRequested();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateGameLoop();
    void onRowTimer();
    void onCountdownTimer();

private:
    void fireBall();
    void snapBallToGrid(const QPointF& pos, BallColor color);
    void popMatches(int r, int c, BallColor color);
    void checkFloatingBalls();
    void redrawGrid();

    QString m_username;
    QString m_mode;
    GridManager m_grid;
    CannonItem* m_cannon;
    AimLineItem* m_aimLine;

    QTimer* m_gameLoopTimer;
    QTimer* m_rowPushTimer;
    QTimer* m_countdownTimer;

    bool m_isFlying = false;
    QPointF m_flyingPos;
    QPointF m_flyingVel;
    BallColor m_flyingColor;

    int m_score = 0;
    int m_timeLeft = 120;
    int m_shotsFired = 0;
    bool m_isPaused = false;
};
