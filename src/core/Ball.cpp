#include "Ball.h"
#include <QRandomGenerator>

Ball::Ball(BallColor color, int row, int col)
    : m_color(color), m_row(row), m_col(col), m_locked(color == BallColor::LOCKED), m_effect(TimeEffect::NONE) {}

QColor Ball::toQColor(BallColor c) {
    switch (c) {
        case BallColor::RED: return QColor(235, 77, 75);
        case BallColor::BLUE: return QColor(72, 52, 212);
        case BallColor::GREEN: return QColor(106, 176, 76);
        case BallColor::YELLOW: return QColor(249, 202, 36);
        case BallColor::PURPLE: return QColor(190, 46, 221);
        case BallColor::BLACK: return QColor(44, 62, 80);
        case BallColor::DUAL: return QColor(224, 86, 253);
        case BallColor::LOCKED: return QColor(149, 175, 192);
        case BallColor::RAINBOW: return QColor(255, 255, 255);
        case BallColor::BOMB: return QColor(231, 76, 60);
        case BallColor::LASER: return QColor(0, 210, 211);
    }
    return Qt::white;
}

BallColor Ball::getRandomColor(int colorCount) {
    int r = QRandomGenerator::global()->bounded(colorCount);
    switch (r) {
        case 0: return BallColor::RED;
        case 1: return BallColor::BLUE;
        case 2: return BallColor::GREEN;
        case 3: return BallColor::YELLOW;
        case 4: return BallColor::PURPLE;
        default: return BallColor::RED;
    }
}
