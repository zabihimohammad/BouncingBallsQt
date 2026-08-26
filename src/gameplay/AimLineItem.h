#pragma once
#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QTimer>
#include <QTime>

class AimLineItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    AimLineItem(qreal minX, qreal maxX, qreal sceneHeight, qreal ballRadius);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateAim(const QPointF& startPos, qreal angleDeg);
    void clearAim();

private:
    qreal m_minX;
    qreal m_maxX;
    qreal m_sceneHeight;
    qreal m_ballRadius;
    QVector<QPointF> m_points;
    QTimer* m_animTimer;
};
