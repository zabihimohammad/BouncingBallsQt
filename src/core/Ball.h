#pragma once

#include <QColor>
#include <QPointF>

// رنگ‌های پایه استاندارد
enum class BallColor {
    None,
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Black       // توپ مشکی خنثی / ضد انفجار
};

// رفتار و نقش عملیاتی توپ
enum class BallType {
    Regular,      // توپ عادی
    DualColor,    // توپ دو رنگ
    Rainbow,      // وایلدکارد (تطبیق با همه)
    Bomb,         // انفجار شعاعی ۳x۳
    Laser         // پاکسازی خطی افقی
};

class Ball {
public:
    Ball(BallColor primaryColor = BallColor::None,
         BallType type = BallType::Regular,
         BallColor secondaryColor = BallColor::None,
         int row = -1,
         int col = -1,
         bool isLocked = false);

    // Getters & Setters
    BallColor getPrimaryColor() const { return m_primaryColor; }
    void setPrimaryColor(BallColor color) { m_primaryColor = color; }

    BallColor getSecondaryColor() const { return m_secondaryColor; }
    void setSecondaryColor(BallColor color) { m_secondaryColor = color; }

    BallType getType() const { return m_type; }
    void setType(BallType type) { m_type = type; }

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setGridPos(int r, int c) { m_row = r; m_col = c; }

    bool isLocked() const { return m_isLocked; }
    void setLocked(bool locked) { m_isLocked = locked; }
    void unlock() { m_isLocked = false; }

    QPointF getPosition() const { return m_position; }
    void setPosition(const QPointF &pos) { m_position = pos; }

    // متدهای وضعیت و منطق
    bool isEmpty() const { return m_primaryColor == BallColor::None; }
    bool isBlack() const { return m_primaryColor == BallColor::Black; }

    // تطبیق هوشمند دوطرفه بین دو توپ یا با یک رنگ و نوع مشخص
    bool matches(const Ball* other) const;
    bool matches(BallColor color, BallType type = BallType::Regular, BallColor secColor = BallColor::None) const;

    // توابع کمکی گرافیک و شانس
    QColor getDisplayColor() const;
    static QColor toQColor(BallColor color);
    static BallColor getRandomColor(int count = 5);

private:
    BallColor m_primaryColor;
    BallColor m_secondaryColor;
    BallType m_type;
    int m_row;
    int m_col;
    bool m_isLocked;
    QPointF m_position;
};