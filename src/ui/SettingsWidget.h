#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QString>
#include <QKeyEvent>

enum class TargetType { 
    VolUp, VolDown, Fullscreen, ThemeNeon, ThemeArcade, EMP, BackToMenu, ProMode
};

struct SettingTarget {
    QPointF pos;
    QPointF velocity;
    qreal radius;
    QColor color;
    QString label;
    TargetType type;
    
    // فیزیک ژله‌ای و چرخش حلقه‌ها
    qreal hitScale = 1.0; 
    qreal squash = 1.0;
    qreal stretch = 1.0;
    qreal rotX = 0;
    qreal rotY = 0;
    qreal rotZ = 0;
    qreal rotSpeedX = 0;
    qreal rotSpeedY = 0;
    qreal rotSpeedZ = 0;
};

struct SettingsProjectile {
    QPointF pos;
    QPointF velocity;
    bool active = true;
    QColor color;
};

struct SettingsParticle {
    QPointF pos;
    QPointF velocity;
    qreal life;
    qreal maxLife;
    QColor color;
};

struct SettingsBgStar {
    QPointF pos;
    qreal size;
    qreal phase;
    qreal speed;
    QColor color;
};

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);

signals:
    void backClicked();
    void fullscreenToggled(bool enabled);
    void proModeClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public:
    Q_INVOKABLE void pauseAnimation() { if(m_timer) m_timer->stop(); }
    Q_INVOKABLE void resumeAnimation() { if(m_timer && !m_timer->isActive()) m_timer->start(16); }

private slots:
    void gameLoop();

private:
    void initEnvironment();
    void initTargets();
    void scatterTargets();
    
    // هندلر برخورد و افکت‌ها
    void triggerHitEffect(int targetIndex, const QPointF& hitPos);
    void handleTargetAction(TargetType type, const QPointF& hitPos);
    void triggerEMP(const QPointF& center);
    void spawnParticles(const QPointF& pos, const QColor& color, int count);
    void fireProjectile();
    
    // فیزیک
    void keepTargetsInBounds();
    void applyTargetRepulsion();

    // رندرینگ
    void drawNebulaBackground(QPainter& painter);
    void drawTargetDrone(QPainter& painter, const SettingTarget& t);
    void drawDroneVisuals(QPainter& painter, const SettingTarget& t, qreal r);
    void drawLaserProjectiles(QPainter& painter);
    void drawParticles(QPainter& painter);
    void drawAdvancedCannon(QPainter& painter);
    void drawHUD(QPainter& painter);
    void drawGlitchSweep(QPainter& painter);

    QTimer* m_timer;
    QVector<SettingTarget> m_targets;
    QVector<SettingsProjectile> m_projectiles;
    QVector<SettingsParticle> m_particles;
    QVector<SettingsBgStar> m_stars;

    QPointF m_mousePos;
    qreal m_cannonAngle = -M_PI/2;
    bool m_firstShow = true;
    qreal m_time = 0.0;
    
    // سیستم شلیک ممتد و توپخانه
    bool m_isFiring = false;
    int m_framesSinceLastFire = 0;
    qreal m_cannonRecoil = 0.0;
    qreal m_cannonHeat = 0.0;
    
    // سیستم گلیچ و تغییر تم
    bool m_isGlitching = false;
    qreal m_glitchWave = 0.0;
    
    // تنظیمات
    int m_volume = 80;
    bool m_fullscreen = false;
    QString m_currentTheme = "Cyber Neon";
    
    qreal m_empRadius = 0.0;
    bool m_empActive = false;
};