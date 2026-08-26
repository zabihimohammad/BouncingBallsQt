#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPolygonF>

struct MenuBall {
    QPointF pos;
    QPointF velocity;
    qreal radius;
    qreal mass; 
    QColor color;
    bool isBackground;
    QVector<QPointF> trail; 
};

struct MenuParticle {
    QPointF pos;
    QPointF velocity;
    qreal life;
    qreal maxLife;
    QColor color;
    qreal size;
};

struct MenuShockwave {
    QPointF pos;
    qreal radius;
    qreal maxRadius;
    qreal intensity;
    QColor color;
};

struct GasBubble {
    QPointF pos;
    qreal speed;
    qreal size;
    qreal wobblePhase;
    qreal zDepth; // For parallax layering
};

struct MenuReactorNode {
    int id; // 0: Start, 1: Settings, 2: Database, 3: Exit, 4: Help
    QString label;
    QString subLabel;
    QString tag;
    QColor primaryColor;
    QColor accentColor;
    QPointF basePos;
    QPointF currentPos;
    qreal radius;
    qreal hover = 0.0;
    qreal spinAngle = 0.0;
    qreal pulsePhase = 0.0;
};

class MainMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget* parent = nullptr);

signals:
    void startGameClicked();
    void scoreboardClicked();
    void settingsClicked();
    void helpClicked();
    void exitClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public slots:
    Q_INVOKABLE void pauseAnimation() { if(m_animTimer) m_animTimer->stop(); }
    Q_INVOKABLE void resumeAnimation() { if(m_animTimer && !m_animTimer->isActive()) m_animTimer->start(16); }

private slots:
    void updateAnimation();

private:
    void setupUI();
    void initBalls();
    void initBubbles();
    void initReactorNodes();
    void updateReactorLayout();
    void resetBallsToCorners(int w, int h);
    void triggerGravityExplosion(const QPointF& center);
    void spawnCollisionEffects(const QPointF& pos, const QColor& color, qreal force);

    // رندرینگ ماژولار
    void drawNebulaBackground(QPainter& painter);
    void drawConstellationWeb(QPainter& painter);
    void drawPlasmaOrbsAndTrails(QPainter& painter);
    void drawEffects(QPainter& painter);
    void drawHolographicTitle(QPainter& painter);
    void drawReactorNodes(QPainter& painter); // سیستم راکتورهای مداری دکمه‌ها
    void drawHyperdriveTransition(QPainter& painter); // انیمیشن جهش وارپ

    QTimer* m_animTimer;
    QVector<MenuBall> m_balls;
    QVector<GasBubble> m_bubbles;
    QVector<MenuParticle> m_particles;
    QVector<MenuShockwave> m_shockwaves;
    QVector<MenuReactorNode> m_reactors;
    
    QPointF m_mousePos;
    bool m_isVortexActive = false; 
    int m_hoveredReactorIdx = -1;

    // ترنزیشن هایپردِرایو (Hyperdrive Warp)
    bool m_isTransitioning = false;
    qreal m_transitionProgress = 0.0;
    int m_transitionTarget = -1; // 0:Play, 1:Settings, 2:Score, 3:Exit

    qreal m_time = 0.0;
};