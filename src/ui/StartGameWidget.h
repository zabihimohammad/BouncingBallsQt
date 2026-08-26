#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QColor>
#include <QLineEdit>
#include <QPushButton>

struct Vector3D {
    qreal x, y, z;
    Vector3D rotateX(qreal angle) const;
    Vector3D rotateY(qreal angle) const;
    Vector3D rotateZ(qreal angle) const;
    Vector3D normalized() const;
    Vector3D cross(const Vector3D& other) const;
    qreal dot(const Vector3D& other) const;
};

// چهره‌های سه‌بعدی برای قاره‌ها و سپر کره
struct Triangle3D {
    Vector3D p1, p2, p3;
    Vector3D center;
    bool isContinent;
    qreal energyPulse;
};

struct Particle3D {
    Vector3D pos;
    Vector3D velocity;
    qreal life;
    qreal maxLife;
    QColor color;
};

struct BgStar {
    QPointF pos;
    qreal size;
    qreal phase;
    qreal speed;
    QColor color;
};

struct OrbitalSector {
    QString id;
    QString name;
    QString description;
    QColor color;
    Vector3D localPos;
    Vector3D transformed;
    qreal hoverScale;
    bool isLocked;
};

class StartGameWidget : public QWidget {
    Q_OBJECT
public:
    explicit StartGameWidget(QWidget* parent = nullptr);

signals:
    void launchGame(const QString& username, const QString& mode);
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public:
    Q_INVOKABLE void pauseAnimation() { if(m_timer) m_timer->stop(); }
    Q_INVOKABLE void resumeAnimation() { if(m_timer && !m_timer->isActive()) m_timer->start(16); }

private slots:
    void updateEngine();
    void onPlayClicked();

private:
    void initEnvironment();
    void update3DTransforms();
    void emitThrusterSparks(const Vector3D& pos, const Vector3D& dir);
    
    // رندرینگ ماژولار و بسیار پیشرفته
    void drawCosmicNebula(QPainter& painter);
    void drawAtmosphere(QPainter& painter);
    void drawGeodesicGlobe(QPainter& painter);
    void drawSectorsAndBeacons(QPainter& painter);
    void drawAdvancedShip(QPainter& painter);
    void drawParticles(QPainter& painter);
    void drawCyberpunkHUD(QPainter& painter);
    void drawScanlines(QPainter& painter);
    void drawFloatingUI(QPainter& painter);

    QTimer* m_timer;
    
    QVector<Triangle3D> m_globeFaces;
    QVector<OrbitalSector> m_sectors;
    QVector<Particle3D> m_particles;
    QVector<BgStar> m_stars;
    
    qreal m_rotX = -20.0;
    qreal m_rotY = 40.0;
    qreal m_targetRotX = -20.0;
    qreal m_targetRotY = 40.0;
    
    bool m_isDragging = false;
    QPointF m_lastMousePos;
    QPointF m_currentMousePos;
    
    qreal m_globeRadius = 240.0;
    qreal m_time = 0.0;
    
    int m_hoveredSector = -1;
    int m_lockedSector = -1;
    qreal m_dockingProgress = 0.0;
    
    QLineEdit* m_nameInput;
    QPushButton* m_playBtn;
    QPushButton* m_backBtn;
};
