#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QVector>
#include <QString>
#include <QColor>
#include <QPolygonF>
#include <QStringList>

struct OrbitNode {
    qreal angle;          
    QString label;        
    QColor color;         
    QPointF projectedPos; 
    qreal scale;          
    qreal zDepth;         
};

struct Star {
    QPointF pos;
    qreal size;
    qreal phase; 
    QColor color;
};

struct AdvParticle {
    QPointF pos;
    QPointF velocity;
    qreal life; 
    QColor color;
};

struct ReactorCapsule {
    QPointF homePos;
    QPointF currentPos;
    QString label;
    QColor color;
    int qualityLevel;
};

class AdvancedSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdvancedSettingsWidget(QWidget* parent = nullptr);

signals:
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateFrame();

private:
    void initOrbit();
    void calculate3D();
    void generateStars(int w, int h);
    void spawnParticles(const QPointF& pos, const QColor& color, int count, qreal speedMult = 1.0);

    // توابع مینی‌گیم‌ها
    void updateAudioLauncher();
    void renderAudioLauncher(QPainter& painter);
    
    void updateGraphicsReactor();
    void renderGraphicsReactor(QPainter& painter);
    
    void updateHapticsSeismograph();
    void renderHapticsSeismograph(QPainter& painter);
    
    void updateDisplayGearbox();
    void renderDisplayGearbox(QPainter& painter);

    QTimer* m_timer;
    QVector<OrbitNode> m_nodes;
    QVector<Star> m_bgStars;
    QVector<Star> m_constellationStars; 
    QList<QPolygonF> m_constellationPolys; 
    QVector<AdvParticle> m_particles; 

    qreal m_time = 0.0;
    qreal m_orbitSpeed = 0.3;
    QPointF m_mousePos;
    
    bool m_inSubMenu = false;
    qreal m_planetOffsetY = 0.0;
    qreal m_currentScale = 1.0; 
    int m_activeNodeIndex = -1;

    qreal m_orbitRadiusX = 400.0; 
    qreal m_orbitTilt = 20.0;     

    bool m_hasInteracted[4] = {false, false, false, false};

    // ================== AUDIO ==================
    bool m_isDraggingCannon = false;
    QPointF m_cannonBase;      
    QPointF m_dragPos;         
    QPointF m_audioBallPos;    
    QPointF m_audioBallVel;    
    bool m_audioBallFlying = false; 
    int m_volume = 80;   
    QStringList m_tracks;
    int m_currentTrackIndex = 0;
    QPointF m_trackCrystalPos;

    // ================== GRAPHICS ==================
    QPointF m_reactorCorePos;
    QVector<ReactorCapsule> m_capsules;
    bool m_isDraggingCapsule = false;
    int m_draggedCapsuleIndex = -1;
    int m_graphicsQuality = 2; 

    // ================== HAPTICS ==================
    QPointF m_anvilPos;
    QPointF m_weightPos;
    bool m_isDraggingWeight = false;
    bool m_weightFalling = false;
    qreal m_weightVelY = 0.0;
    int m_shakeIntensity = 50; 
    qreal m_currentScreenShake = 0.0; 

    // ================== DISPLAY ==================
    QPointF m_gearCenter;
    qreal m_leverAngle = -110.0; // شروع از حالت عمودی (متمرکز به بالا)
    bool m_isDraggingLever = false;
    int m_fpsOptions[4] = {60, 120, 144, 999}; 
    int m_currentFpsIndex = 1;
};