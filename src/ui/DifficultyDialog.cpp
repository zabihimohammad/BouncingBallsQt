#include "DifficultyDialog.h"
#include "../core/SoundManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>

DifficultyDialog::DifficultyDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(580, 520);
    setMouseTracking(true);

    m_options.append({0, "CADET PROTOCOL", "EASY // SAFE OPERATION", "4 Colors | 16s Descent | 5 Misses | 2-Bounce Aim | 3x Ammo", "x1.0 SCORE", QColor(51, 255, 153), QRectF()});
    m_options.append({1, "VETERAN PROTOCOL", "NORMAL // TACTICAL CHALLENGE", "5 Colors | 12s Descent | 3 Misses | 1-Bounce Aim | 2x Ammo", "x1.75 SCORE", QColor(0, 242, 254), QRectF()});
    m_options.append({2, "CYBER-GOD PROTOCOL", "EXTREME // APEX SURVIVAL", "Pre-seeded Hazards | 8s Descent | 2 Misses | 1-Bounce Aim | 6s Panic Clock", "x3.0 SCORE", QColor(255, 51, 102), QRectF()});
}

void DifficultyDialog::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    qreal startY = 115.0;
    qreal cardH = 95.0;
    qreal gap = 14.0;
    for (int i = 0; i < m_options.size(); ++i) {
        m_options[i].rect = QRectF(35, startY + i * (cardH + gap), width() - 70, cardH);
    }
    m_cancelBtnRect = QRectF(width() / 2.0 - 80, height() - 55, 160, 36);
}

void DifficultyDialog::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    m_hoveredIndex = -1;
    for (int i = 0; i < m_options.size(); ++i) {
        if (m_options[i].rect.contains(m_mousePos)) {
            m_hoveredIndex = i;
            break;
        }
    }
    m_cancelHovered = m_cancelBtnRect.contains(m_mousePos);
    update();
}

void DifficultyDialog::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_cancelHovered) {
            SoundManager::instance().playPop();
            reject();
            return;
        }
        for (int i = 0; i < m_options.size(); ++i) {
            if (m_options[i].rect.contains(m_mousePos)) {
                m_selectedDifficulty = m_options[i].id;
                SoundManager::instance().playShoot();
                emit difficultySelected(m_selectedDifficulty);
                accept();
                return;
            }
        }
    }
}

void DifficultyDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF dialogRect = rect().adjusted(4, 4, -4, -4);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(10, 16, 30, 245));
    bg.setColorAt(1.0, QColor(5, 8, 16, 250));
    painter.setBrush(bg);
    painter.setPen(QPen(QColor(0, 242, 254, 180), 2.0));
    painter.drawRoundedRect(dialogRect, 14, 14);

    painter.setPen(QColor(0, 242, 254));
    painter.setFont(QFont("Consolas", 18, QFont::Bold));
    painter.drawText(QRectF(0, 30, width(), 30), Qt::AlignCenter, "SELECT COMBAT SEVERITY");

    painter.setPen(QColor(148, 163, 184));
    painter.setFont(QFont("Consolas", 9));
    painter.drawText(QRectF(0, 65, width(), 20), Qt::AlignCenter, "Configure Endless Simulation parameters & reward multipliers");

    for (int i = 0; i < m_options.size(); ++i) {
        const auto& opt = m_options[i];
        bool isHovered = (m_hoveredIndex == i);

        painter.save();
        painter.setPen(QPen(isHovered ? opt.color : QColor(51, 65, 85), isHovered ? 2.0 : 1.2));
        painter.setBrush(isHovered ? QColor(opt.color.red(), opt.color.green(), opt.color.blue(), 45) : QColor(15, 23, 42, 220));
        painter.drawRoundedRect(opt.rect, 10, 10);

        painter.setPen(opt.color);
        painter.setFont(QFont("Consolas", 13, QFont::Bold));
        painter.drawText(QRectF(opt.rect.left() + 18, opt.rect.top() + 14, 300, 22), opt.title);

        QRectF multBadge(opt.rect.right() - 115, opt.rect.top() + 12, 100, 24);
        painter.setPen(QPen(opt.color, 1.2));
        painter.setBrush(QColor(opt.color.red(), opt.color.green(), opt.color.blue(), 30));
        painter.drawRoundedRect(multBadge, 4, 4);
        painter.setPen(opt.color);
        painter.setFont(QFont("Consolas", 9, QFont::Bold));
        painter.drawText(multBadge, Qt::AlignCenter, opt.multiplier);

        painter.setPen(isHovered ? Qt::white : QColor(148, 163, 184));
        painter.setFont(QFont("Consolas", 9, QFont::Bold));
        painter.drawText(QRectF(opt.rect.left() + 18, opt.rect.top() + 38, opt.rect.width() - 36, 18), opt.subtitle);

        painter.setPen(QColor(148, 163, 184));
        painter.setFont(QFont("Consolas", 8));
        painter.drawText(QRectF(opt.rect.left() + 18, opt.rect.top() + 60, opt.rect.width() - 36, 26), opt.details);
        painter.restore();
    }

    painter.setPen(QPen(m_cancelHovered ? QColor(255, 51, 102) : QColor(71, 85, 105), 1.5));
    painter.setBrush(m_cancelHovered ? QColor(255, 51, 102, 35) : QColor(15, 23, 42, 200));
    painter.drawRoundedRect(m_cancelBtnRect, 6, 6);
    painter.setPen(m_cancelHovered ? Qt::white : QColor(148, 163, 184));
    painter.setFont(QFont("Consolas", 10, QFont::Bold));
    painter.drawText(m_cancelBtnRect, Qt::AlignCenter, "ABORT");
}