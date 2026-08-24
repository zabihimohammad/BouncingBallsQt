#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QVector>
#include <QPointF>

class AimLineItem : public QGraphicsItem {
public:
    AimLineItem(qreal sceneWidth, qreal sceneHeight, qreal ballRadius = 22.0);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // محاسبه مسیر پرتو با قابلیت بازتاب از دیواره‌های چپ و راست
    void updateAim(const QPointF& startPos, qreal angleDeg);
    void clearAim();

private:
    qreal m_sceneWidth;
    qreal m_sceneHeight;
    qreal m_ballRadius;
    QVector<QPointF> m_points;
};