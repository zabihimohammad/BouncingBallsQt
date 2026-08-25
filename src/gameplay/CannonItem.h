#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include "../core/Ball.h"

class CannonItem : public QGraphicsItem {
public:
    CannonItem(qreal width = 352.0, qreal height = 600.0);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void setAngle(qreal angleDeg);
    qreal getAngle() const { return m_angle; }

    // تنظیم گلوله آماده شلیک
    void setCurrentBall(BallColor color, BallType type = BallType::Regular);
    BallColor getCurrentColor() const { return m_currentColor; }
    BallType getCurrentType() const { return m_currentType; }

    // تنظیم گلوله بعدی در خشاب
    void setNextBall(BallColor color, BallType type = BallType::Regular);
    BallColor getNextColor() const { return m_nextColor; }
    BallType getNextType() const { return m_nextType; }

    // جابجایی سریع دو گلوله با کلید Space
    void swapBalls();

private:
    qreal m_sceneWidth;
    qreal m_sceneHeight;
    qreal m_angle = 90.0;

    BallColor m_currentColor = BallColor::Red;
    BallType m_currentType = BallType::Regular;

    BallColor m_nextColor = BallColor::Blue;
    BallType m_nextType = BallType::Regular;

    void drawBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type);
};