#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include <QVector>
#include "../core/GridManager.h"
#include "BallItem.h"
#include "CannonItem.h"
#include "AimLineItem.h"

// ساختار ذرات انفجاری نئونی
struct GameParticle {
    QPointF pos;
    QPointF vel;
    QColor color;
    qreal life = 1.0;
    qreal size = 6.0;
};

// ساختار متن‌های امتیاز شناور
struct FloatingScoreText {
    QPointF pos;
    QString text;
    QColor color;
    qreal life = 1.0;
};

// ساختار پرتو لیزری سطر

struct Shockwave {
    QPointF pos;
    qreal radius;
    qreal life;
    QColor color;
};
struct LaserRayEffect {
    qreal y;
    qreal life = 1.0;
};

struct SkillCard {
    BallType type;
    QString title;
    QString desc;
    QColor color;
    int count;
    QRectF rect;
};

class GameScene : public QGraphicsScene {
Q_OBJECT
public:
    static constexpr qreal SCENE_W = 800.0;
    static constexpr qreal SCENE_H = 600.0;
    static constexpr qreal PLAYFIELD_X = 224.0;
    static constexpr qreal PLAYFIELD_W = 352.0;

    explicit GameScene(const QString& username, const QString& mode = "Classic", int levelNumber = 1, QObject* parent = nullptr);
    ~GameScene() override;

    void pauseGame();
    void resumeGame();
    int getScore() const { return m_score; }
    qreal getAccuracyRatio() const { return m_shotsFired > 0 ? (static_cast<qreal>(m_shotsHit) / m_shotsFired) : 0.5; }
    int getComboStreak() const { return m_comboStreak; }

signals:
    void gameWon(int score);
    void gameOver(int score);
    void scoreChanged(int score);
    void pauseRequested();
    void shakeRequested(int intensity);

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
    void initSkills();
    void fireBall();
    void snapBallToGrid(const QPointF& hitPos, BallColor color, BallType type);
    void popMatches(int r, int c, BallColor color, BallType type);
    void checkFloatingBalls();
    void redrawGrid();
    void prepareNextCannonBall();
    bool findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC);
    void syncCannonColorsWithGrid();
    void popMatches(int r, int c, BallColor color, BallType type, BallColor secColor = BallColor::None);
    // سیستم ذرات و افکت‌های بصری
    void spawnPopParticles(const QPointF& pos, const QColor& color, int count = 25);
    void spawnFloatingText(const QPointF& pos, const QString& text, const QColor& color = QColor(0, 242, 254));
    void triggerLaserBeamEffect(int row);
    void activateSkill(int index);

    // داشبورد
    void drawLeftHUD(QPainter* painter);
    void drawRightSkillPanel(QPainter* painter);
    void drawPlayfieldFrame(QPainter* painter);

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
    BallItem* m_flyingBallItem = nullptr;

    int m_score = 0;
    int m_shotsFired = 0;
    int m_shotsHit = 0;
    int m_comboStreak = 0;
    
    // --- Live HUD Telemetry ---
    qreal m_displayedScore = 0.0;
    qreal m_ecgPhase = 0.0;
    qreal m_dangerLevel = 0.0;
    qreal m_overdrivePhase = 0.0;
    qreal m_aimAngleTelemetry = 0.0;
    int m_aimBouncesTelemetry = 0;
    qreal m_lowestGridY = 0.0;
    qreal m_gameplayTimeSeconds = 0.0;
    QPointF m_mouseHoverPos;
    // --------------------------

    bool m_isPaused = false;
    QVector<SkillCard> m_skills;

    // بافرهای مدیریت افکت‌ها
    QVector<GameParticle> m_particles;
    QVector<FloatingScoreText> m_floatingTexts;
    QVector<LaserRayEffect> m_laserBeams;
    QVector<Shockwave> m_shockwaves;
    QVector<QPointF> m_trail;
};