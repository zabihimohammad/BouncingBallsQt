#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include "../core/GridManager.h"
#include "CannonItem.h"
#include "AimLineItem.h"

class GameScene : public QGraphicsScene {
Q_OBJECT
public:
    explicit GameScene(const QString& username, const QString& mode = "Classic", QObject* parent = nullptr);
    ~GameScene() override;

    void pauseGame();
    void resumeGame();
    int getScore() const { return m_score; }

signals:
    void gameWon(int score);
    void gameOver(int score);
    void scoreChanged(int score);
    void pauseRequested();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateGameLoop();

private:
    void initGame();
    void fireBall();
    void snapBallToGrid(const QPointF& hitPos, BallColor color, BallType type);
    void popMatches(int r, int c, BallColor color, BallType type);
    void checkFloatingBalls();
    void redrawGrid();
    void prepareNextCannonBall();
    bool findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC);

    QString m_username;
    QString m_mode;
    GridManager m_grid;

    CannonItem* m_cannon = nullptr;
    AimLineItem* m_aimLine = nullptr;
    QTimer* m_gameLoopTimer = nullptr;

    // متغیرهای فیزیک پرتابه فعال
    bool m_isFlying = false;
    QPointF m_flyingPos;
    QPointF m_flyingVel;
    BallColor m_flyingColor = BallColor::Red;
    BallType m_flyingType = BallType::Regular;
    QGraphicsEllipseItem* m_flyingBallItem = nullptr;

    int m_score = 0;
    int m_shotsFired = 0;
    bool m_isPaused = false;
};