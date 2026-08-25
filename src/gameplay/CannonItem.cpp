#include "CannonItem.h"
#include <QRadialGradient>
#include <cmath>

CannonItem::CannonItem(qreal width, qreal height)
        : m_sceneWidth(width), m_sceneHeight(height) {
    setZValue(20);
}

QRectF CannonItem::boundingRect() const {
    return QRectF(-100, -70, 200, 140);
}

void CannonItem::setAngle(qreal angleDeg) {
    m_angle = angleDeg;
    update();
}

void CannonItem::setCurrentBall(BallColor color, BallType type, BallColor secColor) {
    m_currentColor = color;
    m_currentType = type;
    m_currentSecColor = secColor;
    update();
}

void CannonItem::setNextBall(BallColor color, BallType type, BallColor secColor) {
    m_nextColor = color;
    m_nextType = type;
    m_nextSecColor = secColor;
    update();
}

void CannonItem::swapBalls() {
    std::swap(m_currentColor, m_nextColor);
    std::swap(m_currentType, m_nextType);
    std::swap(m_currentSecColor, m_nextSecColor);
    update();
}

void CannonItem::drawBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor) {
    painter->save();

    // رندر دوتکه برای توپ دو رنگ
    if (type == BallType::DualColor) {
        QLinearGradient dualGrad(center.x() - radius, center.y(), center.x() + radius, center.y());
        QColor c1 = Ball::toQColor(color);
        QColor c2 = Ball::toQColor(secColor != BallColor::None ? secColor : BallColor::Yellow);
        dualGrad.setColorAt(0.0, c1);
        dualGrad.setColorAt(0.48, c1);
        dualGrad.setColorAt(0.52, c2);
        dualGrad.setColorAt(1.0, c2);

        painter->setPen(Qt::NoPen);
        painter->setBrush(dualGrad);
        painter->drawEllipse(center, radius, radius);
        painter->restore();
        return;
    }

    QColor baseCol = Ball::toQColor(color);
    if (type == BallType::Rainbow) baseCol = QColor(255, 230, 0);
    else if (type == BallType::Bomb) baseCol = QColor(231, 76, 60);
    else if (type == BallType::Laser) baseCol = QColor(0, 210, 211);

    QRadialGradient grad(center.x() - radius * 0.3, center.y() - radius * 0.3, radius * 1.3);
    grad.setColorAt(0.0, QColor(255, 255, 255, 230));
    grad.setColorAt(0.35, baseCol);
    grad.setColorAt(1.0, baseCol.darker(200));

    painter->setPen(Qt::NoPen);
    painter->setBrush(grad);
    painter->drawEllipse(center, radius, radius);

    if (type == BallType::Bomb) {
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(Qt::black);
        painter->drawEllipse(center, radius * 0.35, radius * 0.35);
    } else if (type == BallType::Rainbow) {
        painter->setPen(QPen(Qt::white, 2, Qt::DotLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(center, radius * 0.6, radius * 0.6);
    }

    painter->restore();
}

bool CannonItem::isNextBallClicked(const QPointF& localPos) const {
    qreal dist = std::hypot(localPos.x() - (-65.0), localPos.y() - 0.0);
    return dist <= 28.0;
}

void CannonItem::triggerFireRecoil() {
    m_recoilOffset = 14.0;       // پرتاب ۱۴ پیکسلی به عقب
    m_muzzleFlashAlpha = 1.0;    // اوج روشنایی جرقه دهانه
    update();
}
void CannonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ۱. استهلاک نرم لگد و جرقه در هر چرخه ترسیم (Easing)
    m_recoilOffset *= 0.82;
    if (m_recoilOffset < 0.2) m_recoilOffset = 0.0;

    m_muzzleFlashAlpha *= 0.78;
    if (m_muzzleFlashAlpha < 0.03) m_muzzleFlashAlpha = 0.0;

    // ۲. پایه شلیک نئونی
    QRadialGradient baseGlow(0, 0, 48);
    baseGlow.setColorAt(0.0, QColor(30, 41, 59));
    baseGlow.setColorAt(0.7, QColor(15, 23, 42));
    baseGlow.setColorAt(1.0, QColor(56, 189, 248, 180));

    painter->setBrush(baseGlow);
    painter->setPen(QPen(QColor(56, 189, 248), 2));
    painter->drawEllipse(QPointF(0, 0), 40, 40);

    // ۳. لوله توپ همراه با انیمیشن لگد (Recoil)
    painter->save();
    painter->rotate(-m_angle + 90.0);
    painter->translate(0, m_recoilOffset); // حرکت به عقب در راستای لوله

    painter->setBrush(QBrush(QColor(51, 65, 85)));
    painter->setPen(QPen(QColor(148, 163, 184), 1.5));
    painter->drawRoundedRect(-13, -52, 26, 52, 6, 6);

    // ۴. جرقه دهانه لوله شلیک (Muzzle Flash)
    if (m_muzzleFlashAlpha > 0.01) {
        QRadialGradient flashGrad(0, -60, 26);
        flashGrad.setColorAt(0.0, QColor(255, 255, 255, int(m_muzzleFlashAlpha * 255)));
        flashGrad.setColorAt(0.5, QColor(0, 242, 254, int(m_muzzleFlashAlpha * 200)));
        flashGrad.setColorAt(1.0, Qt::transparent);

        painter->setBrush(flashGrad);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, -60), 26, 26);
    }
    painter->restore();

    // گلوله آماده شلیک در مرکز
    drawBall(painter, QPointF(0, 0), 18.0, m_currentColor, m_currentType, m_currentSecColor);

    // حلقه و گلوله بعدی در خشاب
    painter->setPen(QPen(QColor(148, 163, 184, 120), 1.5, Qt::DashLine));
    painter->setBrush(QColor(15, 23, 42, 160));
    painter->drawEllipse(QPointF(-65, 0), 18, 18);

    drawBall(painter, QPointF(-65, 0), 13.0, m_nextColor, m_nextType, m_nextSecColor);
    }