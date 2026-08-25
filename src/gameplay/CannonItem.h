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

    void setCurrentBall(BallColor color, BallType type = BallType::Regular, BallColor secColor = BallColor::None);
    BallColor getCurrentColor() const { return m_currentColor; }
    BallType getCurrentType() const { return m_currentType; }
    BallColor getCurrentSecondaryColor() const { return m_currentSecColor; }

    void setNextBall(BallColor color, BallType type = BallType::Regular, BallColor secColor = BallColor::None);
    BallColor getNextColor() const { return m_nextColor; }
    BallType getNextType() const { return m_nextType; }
    BallColor getNextSecondaryColor() const { return m_nextSecColor; }

    void swapBalls();
    bool isNextBallClicked(const QPointF& localPos) const;
    void triggerFireRecoil();

private:
    qreal m_sceneWidth;
    qreal m_sceneHeight;
    qreal m_angle = 90.0;

    BallColor m_currentColor = BallColor::Red;
    BallColor m_currentSecColor = BallColor::None;
    BallType m_currentType = BallType::Regular;

    BallColor m_nextColor = BallColor::Blue;
    BallColor m_nextSecColor = BallColor::None;
    BallType m_nextType = BallType::Regular;

    qreal m_recoilOffset = 0.0;
    qreal m_muzzleFlashAlpha = 0.0;

    void drawBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor = BallColor::None);
};