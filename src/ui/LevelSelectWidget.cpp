#include "LevelSelectWidget.h"
#include "../core/SoundManager.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QMouseEvent>

LevelSelectWidget::LevelSelectWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    initCards();
}

void LevelSelectWidget::initCards() {
    m_cards.clear();
    m_cards.append({1, "STAGE 01: ORBITAL ENTRY", "Standard Match-3 Protocol", "EASY", QColor(0, 242, 254), QRectF()});
    m_cards.append({2, "STAGE 02: GLACIAL SHIELD", "Dual-Layer Ice Shields", "MEDIUM", QColor(116, 185, 255), QRectF()});
    m_cards.append({3, "STAGE 03: CYBER VAULT", "Locked Vault & Golden Key", "HARD", QColor(255, 204, 0), QRectF()});
    m_cards.append({4, "STAGE 04: FOG OF WAR", "Mystery Orbs & Black Obstacles", "EXPERT", QColor(165, 94, 234), QRectF()});
    m_cards.append({5, "STAGE 05: REACTOR OVERLOAD", "Apex Challenge: All Hazards Active", "EXTREME", QColor(255, 51, 102), QRectF()});
}

void LevelSelectWidget::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int cardW = qMin(540, w - 80);
    int cardH = 80;
    int startY = 130;
    int spacing = 18;

    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i].rect = QRectF((w - cardW) / 2.0, startY + i * (cardH + spacing), cardW, cardH);
    }

    m_backBtnRect = QRectF(30, 30, 140, 42);
    QWidget::resizeEvent(event);
}

void LevelSelectWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    m_hoveredIndex = -1;

    for (int i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i].rect.contains(m_mousePos)) {
            m_hoveredIndex = i;
            break;
        }
    }

    m_backHovered = m_backBtnRect.contains(m_mousePos);
    update();
    QWidget::mouseMoveEvent(event);
}

void LevelSelectWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_backHovered) {
            SoundManager::instance().playPop();
            emit backClicked();
            return;
        }

        for (const auto& card : m_cards) {
            if (card.rect.contains(event->position())) {
                SoundManager::instance().playShoot();
                emit levelSelected(card.levelNumber);
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void LevelSelectWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor priCol = ThemeManager::instance().getPrimaryColor();
    int baseHue = ThemeManager::instance().getBaseHue();

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor::fromHsv(baseHue, 220, 16));
    bg.setColorAt(1.0, QColor::fromHsv(baseHue, 240, 6));
    painter.fillRect(rect(), bg);

    painter.setPen(priCol);
    painter.setFont(QFont("Consolas", 26, QFont::Black));
    painter.drawText(QRectF(0, 40, width(), 40), Qt::AlignCenter, "SELECT MISSION SECTOR");

    painter.setPen(QColor(148, 163, 184));
    painter.setFont(QFont("Consolas", 11));
    painter.drawText(QRectF(0, 80, width(), 30), Qt::AlignCenter, "Choose campaign deployment level");

    for (int i = 0; i < m_cards.size(); ++i) {
        const auto& card = m_cards[i];
        bool isHovered = (m_hoveredIndex == i);

        painter.save();
        if (isHovered) {
            painter.setPen(QPen(card.themeColor, 2));
            painter.setBrush(QColor(card.themeColor.red(), card.themeColor.green(), card.themeColor.blue(), 40));
        } else {
            painter.setPen(QPen(QColor(51, 65, 85), 1.5));
            painter.setBrush(QColor(15, 23, 42, 220));
        }
        painter.drawRoundedRect(card.rect, 12, 12);

        QRectF numBadge(card.rect.left() + 16, card.rect.top() + 16, 48, 48);
        painter.setPen(Qt::NoPen);
        painter.setBrush(isHovered ? card.themeColor : QColor(30, 41, 59));
        painter.drawRoundedRect(numBadge, 8, 8);

        painter.setPen(isHovered ? Qt::black : card.themeColor);
        painter.setFont(QFont("Consolas", 15, QFont::Black));
        painter.drawText(numBadge, Qt::AlignCenter, QString("0%1").arg(card.levelNumber));

        painter.setPen(Qt::white);
        painter.setFont(QFont("Consolas", 13, QFont::Bold));
        painter.drawText(QRectF(card.rect.left() + 80, card.rect.top() + 16, card.rect.width() - 190, 24), card.title);

        painter.setPen(QColor(148, 163, 184));
        painter.setFont(QFont("Consolas", 10));
        painter.drawText(QRectF(card.rect.left() + 80, card.rect.top() + 42, card.rect.width() - 190, 20), card.subtitle);

        QRectF diffBadge(card.rect.right() - 105, card.rect.top() + 26, 90, 28);
        painter.setPen(QPen(card.themeColor, 1));
        painter.setBrush(QColor(card.themeColor.red(), card.themeColor.green(), card.themeColor.blue(), 25));
        painter.drawRoundedRect(diffBadge, 6, 6);

        painter.setPen(card.themeColor);
        painter.setFont(QFont("Consolas", 9, QFont::Bold));
        painter.drawText(diffBadge, Qt::AlignCenter, card.difficulty);
        painter.restore();
    }

    QColor bCol = m_backHovered ? QColor(255, 51, 102) : priCol;
    painter.setPen(QPen(bCol, 2));
    painter.setBrush(m_backHovered ? QColor(255, 51, 102, 40) : QColor(priCol.red(), priCol.green(), priCol.blue(), 15));
    painter.drawRoundedRect(m_backBtnRect, 8, 8);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 10, QFont::Bold));
    painter.drawText(m_backBtnRect, Qt::AlignCenter, "◄ ABORT");
}