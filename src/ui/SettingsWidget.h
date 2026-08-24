#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QString>

enum class TargetType { 
    VolUp, VolDown, Fullscreen, ThemeNeon, ThemeArcade, EMP, BackToMenu 
};

struct SettingTarget {
    QPointF pos;
    QPointF velocity;
    qreal radius;
    QColor color;
    QString label;
    TargetType type;
    qreal hitScale = 1.0; 
};

struct Projectile {
    QPointF pos;
    QPointF velocity;
    bool active = true;
};

struct Particle {
    QPointF pos;
    QPointF velocity;
    qreal life; 
    QColor color;
};

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);

    signals:
        void backClicked();
    void fullscreenToggled(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void gameLoop();

private:
    void initTargets();
    void scatterTargets(); // برای پخش کردن حباب‌ها در شروع
    void triggerHitEffect(int targetIndex, const QPointF& hitPos);
    void triggerEMP(const QPointF& center);
    void spawnParticles(const QPointF& pos, const QColor& color, int count);
    void keepTargetsInBounds();
    void applyTargetRepulsion(); // جلوگیری از روی هم افتادن

    QTimer* m_timer;
    QVector<SettingTarget> m_targets;
    QVector<Projectile> m_projectiles;
    QVector<Particle> m_particles;

    QPointF m_mousePos;
    qreal m_cannonAngle = 0.0;
    bool m_firstShow = true;
    
    int m_volume = 80;
    bool m_fullscreen = false;
    QString m_currentTheme = "Cyber Neon";
    
    qreal m_empRadius = 0.0;
    bool m_empActive = false;
};