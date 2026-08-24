#include "Ball.h"
#include <QRandomGenerator>
#include <algorithm>

Ball::Ball(BallColor primaryColor, BallType type, BallColor secondaryColor, int row, int col, bool isLocked)
        : m_primaryColor(primaryColor),
          m_secondaryColor(secondaryColor),
          m_type(type),
          m_row(row),
          m_col(col),
          m_isLocked(isLocked),
          m_position(0.0, 0.0) {
}

bool Ball::matches(const Ball* other) const {
    if (!other) return false;
    return matches(other->getPrimaryColor(), other->getType(), other->getSecondaryColor());
}

bool Ball::matches(BallColor otherColor, BallType otherType, BallColor otherSecColor) const {
    if (isEmpty() || otherColor == BallColor::None) return false;
    if (m_isLocked) return false;

    // توپ سیاه با هیچ توپی همرنگ نمی‌شود
    if (isBlack() || otherColor == BallColor::Black) return false;

    // اگر هر کدام از دو توپ وایلدکارد (Rainbow) باشند
    if (m_type == BallType::Rainbow || otherType == BallType::Rainbow) return true;

    // تطبیق مستقیم رنگ اصلی
    if (m_primaryColor == otherColor) return true;

    // بررسی حالت‌های دو رنگ (DualColor)
    if (m_type == BallType::DualColor && m_secondaryColor == otherColor) return true;
    if (otherType == BallType::DualColor && (m_primaryColor == otherSecColor)) return true;
    if (m_type == BallType::DualColor && otherType == BallType::DualColor) {
        return (m_secondaryColor == otherSecColor || m_primaryColor == otherSecColor || m_secondaryColor == otherColor);
    }

    return false;
}

QColor Ball::toQColor(BallColor color) {
    switch (color) {
        case BallColor::Red:     return QColor(235, 77, 75);
        case BallColor::Green:   return QColor(106, 176, 76);
        case BallColor::Blue:    return QColor(72, 52, 212);
        case BallColor::Yellow:  return QColor(249, 202, 36);
        case BallColor::Purple:  return QColor(190, 46, 221);
        case BallColor::Black:   return QColor(44, 62, 80);
        default:                 return Qt::transparent;
    }
}

QColor Ball::getDisplayColor() const {
    if (m_isLocked) return QColor(149, 175, 192); // رنگ خاکستری قفل
    if (m_type == BallType::Rainbow) return QColor(255, 230, 0);
    if (m_type == BallType::Bomb) return QColor(231, 76, 60);
    if (m_type == BallType::Laser) return QColor(0, 210, 211);
    return toQColor(m_primaryColor);
}

BallColor Ball::getRandomColor(int count) {
    int maxColors = std::clamp(count, 1, 5);
    int r = QRandomGenerator::global()->bounded(maxColors);

    switch (r) {
        case 0:  return BallColor::Red;
        case 1:  return BallColor::Green;
        case 2:  return BallColor::Blue;
        case 3:  return BallColor::Yellow;
        case 4:  return BallColor::Purple;
        default: return BallColor::Red;
    }
}