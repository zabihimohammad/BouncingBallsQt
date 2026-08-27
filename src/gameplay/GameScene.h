#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QVector>
#include <QRectF>
#include "../core/GridManager.h"
#include "BallItem.h"
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
    static constexpr qreal OVERDRIVE_DURATION = 6.0;

    explicit GameScene(const QString& username, const QString& mode = "Classic", int levelNumber = 1, int difficulty = 1, QObject* parent = nullptr);
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
    bool popMatches(int r, int c, BallColor color, BallType type, BallColor secColor = BallColor::None);
    void checkFloatingBalls();
    void redrawGrid();
    void prepareNextCannonBall();
    bool findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC);
    void syncCannonColorsWithGrid();

    void armSkill(int index);
    void disarmSkill();
    void addSkillAmmo(BallType skillType, int amount = 1);
    void grantRandomSupplyDrop();

    void advanceEndlessRow();
    void triggerWaveEscalation();
    void addTimeBonus(qreal seconds, const QString& reason);
    void checkOverdriveTrigger(const QPointF& center);
    void executeEmergencyPurge(const QPointF& center);

    void spawnPopParticles(const QPointF& pos, const QColor& color, int count = 25);
    void spawnFloatingText(const QPointF& pos, const QString& text, const QColor& color = QColor(0, 242, 254));
    void triggerLaserBeamEffect(int row);

    void drawLeftHUD(QPainter* painter);
    void drawRightSkillPanel(QPainter* painter);
    void drawPlayfieldFrame(QPainter* painter);

    QString m_username;
    QString m_mode;
    int m_levelNumber = 1;
    int m_difficulty = 1;
    GridManager m_grid;

    CannonItem* m_cannon = nullptr;
    AimLineItem* m_aimLine = nullptr;
    QTimer* m_gameLoopTimer = nullptr;

    bool m_isFlying = false;
    QPointF m_flyingPos;
    QPointF m_flyingVel;
    BallColor m_flyingColor = BallColor::Red;
    BallColor m_flyingSecondaryColor = BallColor::None;
    BallType m_flyingType = BallType::Regular;
    BallItem* m_flyingBallItem = nullptr;

    int m_score = 0;
    int m_shotsFired = 0;
    int m_shotsHit = 0;
    int m_comboStreak = 0;

    int m_armedSkillIndex = -1;
    BallColor m_savedBaseColor = BallColor::None;

    bool m_isTimeAttack = false;
    qreal m_timeRemaining = 75.0;

    // پارامترهای سطح سختی
    bool m_isEndless = false;
    int m_currentWave = 1;
    qreal m_waveTimer = 0.0;
    qreal m_currentDropInterval = 12.0;
    int m_missedShotsCount = 0;
    int m_maxMissedShots = 3;
    int m_activeColorsCount = 5;
    int m_clusterChance = 25;
    int m_maxAimBounces = 1;
    qreal m_baseScoreMultiplier = 1.0;
    qreal m_scoreMultiplier = 1.0;
    qreal m_multiplierDecayTimer = 0.0;
    qreal m_panicTimer = 0.0;
    qreal m_endlessRowTimer = 0.0;
    bool m_dangerTelegraph = false;

    bool m_isOverdrive = false;
    qreal m_overdriveTimer = 0.0;

    bool m_emergencyPurgeReady = false;
    qreal m_purgeCooldown = 0.0;
    qreal m_empEventTimer = 0.0;
    bool m_empSurgeActive = false;
    qreal m_empSurgeDuration = 0.0;

    qreal m_displayedScore = 0.0;
    qreal m_ecgPhase = 0.0;
    qreal m_dangerLevel = 0.0;
    qreal m_overdrivePhase = 0.0;
    qreal m_aimAngleTelemetry = 0.0;
    int m_aimBouncesTelemetry = 0;
    qreal m_lowestGridY = 0.0;
    qreal m_gameplayTimeSeconds = 0.0;
    QPointF m_mouseHoverPos;

    bool m_isPaused = false;
    QVector<SkillCard> m_skills;

    QVector<GameParticle> m_particles;
    QVector<FloatingScoreText> m_floatingTexts;
    QVector<LaserRayEffect> m_laserBeams;
    QVector<Shockwave> m_shockwaves;
    QVector<QPointF> m_trail;
};