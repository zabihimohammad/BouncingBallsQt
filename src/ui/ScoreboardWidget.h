#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QString>
#include <QRectF>
#include <QWheelEvent>
#include <QPainterPath>
#include <QPolygonF>
#include "../core/ScoreManager.h"

struct CelestialScore {
    int rank;
    QString name;
    int score;
    QString mode;

    qreal angle;
    qreal orbitRadius;
    qreal speed;
    QColor color;
    qreal baseSize;

    QPointF projectedPos;
    qreal zDepth;
    qreal scale;
};

struct StarSystem {
    QString modeName;
    QVector<CelestialScore> planets;
    
    QPointF currentPos;
    QPointF targetPos;
    qreal currentScale;
    qreal targetScale;
    qreal titleOpacity;
};

struct StarDust {
    QPointF pos;
    qreal speed;
    qreal size;
    qreal brightness;
};

class ScoreboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScoreboardWidget(ScoreManager* scoreMgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public:
    Q_INVOKABLE void pauseAnimation() { if(m_timer) m_timer->stop(); }
    Q_INVOKABLE void resumeAnimation() { if(m_timer && !m_timer->isActive()) m_timer->start(16); }

private slots:
    void updateUniverse();

private:
    void initSpace();
    void initConstellationTitle();
    void calculate3DProjection();
    void drawDeepSpace(QPainter& painter);
    void drawConstellationTitle(QPainter& painter);
    void drawOrbitRings(QPainter& painter, const StarSystem& sys);
    void drawCelestialBody(QPainter& painter, const CelestialScore& body, const StarSystem& sys);
    void drawDossierOverlay(QPainter& painter, const CelestialScore& body);
    void drawSystemLabel(QPainter& painter, const StarSystem& sys);
    void drawNeonBackButton(QPainter& painter);

    ScoreManager* m_scoreMgr;
    QTimer* m_timer;
    
    QVector<StarSystem> m_systems;
    int m_activeSystemIndex = 0;
    
    QVector<QPointF> m_titleNodes; // گره‌های عنوان صوزت فلکی
    QVector<StarDust> m_stardust;
    
    QPointF m_mousePos;
    int m_hoveredRank = -1; // رنک سیاره هاور شده در سیستم فعال
    int m_hoveredSystem = -1; // ایندکس سیستمی که موس رویشه
    int m_selectedIndex = -1; // رنک سیاره کلیک شده در سیستم فعال
    qreal m_time = 0.0;
    
    QRectF m_backBtnRect;
    bool m_backHovered = false;

    qreal m_orbitTilt = 0.35; 
    qreal m_zoomFactor = 1.0;
    qreal m_targetZoom = 1.0;
};
