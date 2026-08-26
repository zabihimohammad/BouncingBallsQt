#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include <QVector>
#include <QRectF>
#include "../core/GridManager.h"
#include "CannonItem.h"
#include "AimLineItem.h"

struct GameParticle {
    QPointF pos;
    QPointF vel;
    QColor color;
    qreal life = 1.0;
    qreal size = 6.0;
};

struct FloatingScoreText {
    QPointF pos;
    QString text;
    QColor color;
    qreal life = 1.0;
};

struct LaserRayEffect {
    qreal y;
    qreal life = 1.0;
};

struct SkillPod {
    BallType type;
    QString name;
    QString hotkey;
    QColor color;
    int count;
    QRectF rect;
};

struct SatelliteNode {
    QString id;
    QString label;
    QPointF center;
    qreal radius = 22.0;
    bool isHovered = false;
};

class GameScene : public QGraphicsScene {
Q_OBJECT
public:
    static constexpr qreal SCENE_W = 1000.0;
    static constexpr qreal SCENE_H = 700.0;
    static constexpr qreal PLAYFIELD_W = 528.0; // 12 ستون x 44px
    static constexpr qreal PLAYFIELD_X = (SCENE_W - PLAYFIELD_W) / 2.0; // 236.0 px (مرکز تقارن)

    explicit GameScene(const QString& username, const QString& mode = "Classic", int levelNumber = 1, QObject* parent = nullptr);
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
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private slots:
    void updateGameLoop();

private:
    void initGame();
    void initSkillPods();
    void fireBall();
    void snapBallToGrid(const QPointF& hitPos, BallColor color, BallType type);
    void popMatches(int r, int c, BallColor color, BallType type, BallColor secColor = BallColor::None);
    void checkFloatingBalls();
    void redrawGrid();
    void prepareNextCannonBall();
    bool findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC);
    void syncCannonColorsWithGrid();

    // سیستم جلوه‌های ویژه
    void spawnPopParticles(const QPointF& pos, const QColor& color, int count = 25);
    void spawnFloatingText(const QPointF& pos, const QString& text, const QColor& color = QColor(0, 242, 254));
    void triggerLaserBeamEffect(int row);

    // ترسیم ماژولار کنسول هولوگرافیک
    void drawDeepSpaceNebula(QPainter* painter);
    void drawForceFieldPlayfield(QPainter* painter);
    void drawTelemetryGlassHUD(QPainter* painter);
    void drawSatelliteControls(QPainter* painter);
    void drawOrbitalSkillPods(QPainter* painter);

    QString m_username;
    QString m_mode;
    int m_levelNumber = 1;
    GridManager m_grid;

    CannonItem* m_cannon = nullptr;
    AimLineItem* m_aimLine = nullptr;
    QTimer* m_gameLoopTimer = nullptr;

    bool m_isFlying = false;
    QPointF m_flyingPos;
    QPointF m_flyingVel;
    BallColor m_flyingColor = BallColor::Red;
    BallColor m_flyingSecondaryColor = BallColor::None;
    BallColor m_loadedSecondaryColor = BallColor::None;
    BallType m_flyingType = BallType::Regular;
    QGraphicsEllipseItem* m_flyingBallItem = nullptr;

    int m_score = 0;
    int m_shotsFired = 0;
    int m_shotsHit = 0;
    int m_comboStreak = 0;
    qreal m_overdriveCharge = 0.0;
    bool m_isPaused = false;
    qreal m_time = 0.0;

    QVector<SkillPod> m_skillPods;
    QVector<SatelliteNode> m_satelliteNodes;
    QVector<GameParticle> m_particles;
    QVector<FloatingScoreText> m_floatingTexts;
    QVector<LaserRayEffect> m_laserBeams;
};