#include "HelpWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QImage>

HelpWidget::HelpWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    
    // Initial state of the menu
    // ID mapping: 100=Settings, 101=Pro Settings
    m_menuItems = {
        {"[01] GAMEPLAY", 1, 0, false, 0, QRectF()},
        {"[02] SETTINGS", 0, -1, true, 100, QRectF()},
        {"BASIC CONFIG", 1, 1, false, 1, QRectF()},
        {"PRO SETTINGS", 0, -1, true, 101, QRectF()},
        {"AUDIO LAUNCHER", 1, 2, false, 2, QRectF()},
        {"GRAPHICS REACTOR", 1, 3, false, 3, QRectF()},
        {"HAPTICS SEISMOGRAPH", 1, 4, false, 4, QRectF()},
        {"DISPLAY GEARBOX", 1, 5, false, 5, QRectF()},
        {"[03] TELEMETRY", 1, 6, false, 6, QRectF()}
    };
    
    QString p = QCoreApplication::applicationDirPath() + "/../assets/videos/";
    m_movies[0] = new QMovie(p + "gameplay_help.gif", QByteArray(), this);
    m_movies[1] = new QMovie(p + "settings_help.gif", QByteArray(), this);
    m_movies[2] = new QMovie(p + "adv_audio.gif", QByteArray(), this);
    m_movies[3] = new QMovie(p + "adv_graphics.gif", QByteArray(), this);
    m_movies[4] = new QMovie(p + "adv_haptics.gif", QByteArray(), this);
    m_movies[5] = new QMovie(p + "adv_display.gif", QByteArray(), this);
    m_movies[6] = new QMovie(p + "leaderboard_help.gif", QByteArray(), this);
    
    for (int i = 0; i < 7; ++i) {
        if (m_movies[i]->isValid()) {
            connect(m_movies[i], &QMovie::updated, this, [this](const QRect&){ update(); });
            m_movies[i]->start();
        }
    }
    m_activeVideoIndex = 2; // Default to Audio Launcher
    for (int i = 0; i < 7; ++i) {
        m_movies[i]->setPaused(i != m_activeVideoIndex);
    }
}

HelpWidget::~HelpWidget() {}

void HelpWidget::pauseAnimation() {
    for (int i=0; i<7; ++i) m_movies[i]->setPaused(true);
}
void HelpWidget::resumeAnimation() {
    for (int i=0; i<7; ++i) m_movies[i]->setPaused(false);
}

void HelpWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawBackground(painter);
    drawNeonBackButton(painter);
    drawMenu(painter);
    drawContent(painter);
}

void HelpWidget::drawBackground(QPainter& p) {
    int baseHue = ThemeManager::instance().getBaseHue();
    QLinearGradient bgGrad(0, 0, width(), height());
    bgGrad.setColorAt(0.0, QColor::fromHsv(baseHue, 220, 16));
    bgGrad.setColorAt(1.0, QColor::fromHsv(baseHue, 240, 6));
    p.fillRect(rect(), bgGrad);
    
    QColor gridCol = ThemeManager::instance().getPrimaryColor();
    gridCol.setAlpha(20);
    p.setPen(QPen(gridCol, 1));
    for (int x = 0; x < width(); x += 40) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) p.drawLine(0, y, width(), y);
}

void HelpWidget::drawMenu(QPainter& p) {
    qreal startY = 100.0;
    qreal tabW = 320.0;
    qreal tabH = 45.0;
    qreal gap = 5.0;
    
    bool showBasic = m_menuItems[1].isExpanded;
    bool showPro = m_menuItems[3].isExpanded && showBasic;
    
    qreal y = startY;
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    
    for (int i = 0; i < m_menuItems.size(); ++i) {
        auto& item = m_menuItems[i];
        
        // Visibility logic
        if (item.id == 1 || item.id == 101) { if (!showBasic) continue; }
        if (item.id >= 2 && item.id <= 5) { if (!showPro) continue; }
        
        int indent = 0;
        if (item.id == 1 || item.id == 101) indent = 20;
        if (item.id >= 2 && item.id <= 5) indent = 40;
        
        item.rect = QRectF(40 + indent, y, tabW - indent, tabH);
        
        bool isHovered = (m_hoveredId == item.id);
        bool isActive = (item.type == 1 && item.videoIndex == m_activeVideoIndex);
        
        QColor bgActive = priCol;
        bgActive.setAlpha(80);
        QColor bgColor = isActive ? bgActive : (isHovered ? QColor(255, 255, 255, 20) : QColor(255, 255, 255, 5));
        p.setBrush(bgColor);
        p.setPen(QPen(isActive ? priCol : QColor(100, 100, 100), item.type == 0 ? 1 : 2));
        p.drawRoundedRect(item.rect, 5, 5);
        
        if (isActive) {
            p.setBrush(Qt::white);
            p.setPen(Qt::NoPen);
            p.drawRect(item.rect.left() + 5, item.rect.top() + 10, 4, tabH - 20);
        }
        
        QString prefix = "";
        if (item.type == 0) {
            prefix = item.isExpanded ? "▼ " : "▶ ";
        } else if (indent > 0) {
            prefix = "└ ";
        }
        
        p.setPen(isActive ? Qt::white : (isHovered ? priCol.lighter(130) : QColor(180, 180, 200)));
        QFont f("Consolas", item.type == 0 ? 15 : 13, isActive ? QFont::Bold : QFont::Normal);
        p.setFont(f);
        p.drawText(QRectF(item.rect.left() + 15, item.rect.top(), tabW - indent - 15, tabH), Qt::AlignLeft | Qt::AlignVCenter, prefix + item.text);
        
        y += tabH + gap;
    }
}

void HelpWidget::drawContent(QPainter& p) {
    QRectF contentRect(400, 100, width() - 440, height() - 140);
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    
    p.setBrush(QColor(10, 15, 30, 200));
    QColor borderCol = priCol;
    borderCol.setAlpha(150);
    p.setPen(QPen(borderCol, 2));
    p.drawRoundedRect(contentRect, 10, 10);
    
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 24, QFont::Bold));
    QStringList titles = {
        "TACTICAL ENGAGEMENT", 
        "SYSTEM OVERRIDE", 
        "AUDIO LAUNCHER (ADVANCED)",
        "GRAPHICS REACTOR (ADVANCED)",
        "HAPTICS SEISMOGRAPH (ADVANCED)",
        "DISPLAY GEARBOX (ADVANCED)",
        "MULTIVERSE ARCHIVE"
    };
    p.drawText(QRectF(contentRect.left() + 30, contentRect.top() + 20, contentRect.width() - 60, 40), Qt::AlignLeft | Qt::AlignVCenter, titles[m_activeVideoIndex]);
    
    p.setPen(QPen(QColor(255, 255, 255, 50), 1, Qt::DashLine));
    p.drawLine(contentRect.left() + 30, contentRect.top() + 70, contentRect.right() - 30, contentRect.top() + 70);

    QRectF animRect(contentRect.left() + 30, contentRect.top() + 90, contentRect.width() - 60, contentRect.height() - 280);
    
    if (m_movies[m_activeVideoIndex]->isValid()) {
        QImage currentFrame = m_movies[m_activeVideoIndex]->currentImage();
        if (!currentFrame.isNull()) {
            QImage scaled = currentFrame.scaled(animRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPointF pos = animRect.center() - QPointF(scaled.width()/2.0, scaled.height()/2.0);
            p.drawImage(pos, scaled);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(borderCol, 2));
            p.drawRect(QRectF(pos, scaled.size()));
        }
    } else {
        p.setPen(Qt::red);
        p.drawText(animRect, Qt::AlignCenter, "NO FEED AVAILABLE");
    }
    
    p.setPen(QColor(200, 220, 255));
    p.setFont(QFont("Consolas", 14));
    QStringList descs = {
        ">> ACTUAL COMBAT FOOTAGE:\n\n- Align cursor to set orbital trajectory.\n- [LEFT CLICK] to discharge energy core.\n- Pro Tip: Utilize containment walls to bank shots into blind spots.",
        ">> CONFIGURATION FOOTAGE:\n\n- The Settings module is a live-fire configuration zone.\n- Shoot floating UI nodes to toggle audio and visual parameters.\n- Engage 'PRO MODE' for advanced overrides.",
        ">> AUDIO LAUNCHER INTERACTION:\n\n- Select the AUDIO node in the 3D orbit.\n- Drag the audio orb from the cannon base backward and release it to launch.\n- The orb smashes crystals to change Tracks, or targets the Volume indicator.",
        ">> GRAPHICS REACTOR INTERACTION:\n\n- Select the GRAPHICS node in the 3D orbit.\n- Drag the plasma capsules (LOW / MEDIUM / ULTRA) into the Quantum Core.\n- The core absorbs the capsule and dynamically upgrades visual fidelity.",
        ">> HAPTICS SEISMOGRAPH INTERACTION:\n\n- Select the HAPTICS node in the 3D orbit.\n- Lift the heavy metallic weight using the mouse and drop it onto the anvil.\n- The kinetic impact sets the Screen Shake intensity.",
        ">> DISPLAY GEARBOX INTERACTION:\n\n- Select the DISPLAY node in the 3D orbit.\n- Rotate the massive mechanical lever around the gearbox core.\n- This physically shifts the FPS gear (60, 120, 144, 999 REDLINE).",
        ">> TELEMETRY FOOTAGE:\n\n- The Leaderboard visualizes top operatives as celestial bodies.\n- Interact with a planet to deploy the holographic Dossier Panel."
    };
    p.drawText(QRectF(animRect.left(), animRect.bottom() + 30, animRect.width(), 230), Qt::AlignLeft | Qt::TextWordWrap, descs[m_activeVideoIndex]);
}

void HelpWidget::drawNeonBackButton(QPainter& painter) {
    m_backBtnRect = QRectF(20, 20, 130, 45);
    QColor secCol = ThemeManager::instance().getSecondaryColor();
    if (m_backHovered) {
        QColor hoverBg = secCol;
        hoverBg.setAlpha(100);
        painter.setBrush(hoverBg); 
        painter.setPen(QPen(secCol, 2));
    } else {
        painter.setBrush(QColor(15, 23, 42, 200));
        painter.setPen(QPen(QColor(148, 163, 184), 1.5));
    }
    painter.drawRoundedRect(m_backBtnRect, 10, 10);
    painter.setPen(m_backHovered ? Qt::white : QColor(226, 232, 240));
    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
    painter.drawText(m_backBtnRect, Qt::AlignCenter, "◄ MAIN MENU");
}

void HelpWidget::mouseMoveEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    bool wasBackHovered = m_backHovered;
    m_backHovered = m_backBtnRect.contains(pos);
    
    int oldHover = m_hoveredId;
    m_hoveredId = -1;
    for (auto& item : m_menuItems) {
        if (!item.rect.isEmpty() && item.rect.contains(pos)) {
            m_hoveredId = item.id;
            break;
        }
    }
    
    if (wasBackHovered != m_backHovered || oldHover != m_hoveredId) {
        if (m_backHovered || m_hoveredId != -1) setCursor(Qt::PointingHandCursor);
        else setCursor(Qt::ArrowCursor);
        update();
    }
}
void HelpWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_backHovered) { emit backClicked(); return; }
        
        for (auto& item : m_menuItems) {
            if (item.id == m_hoveredId) {
                if (item.type == 0) {
                    item.isExpanded = !item.isExpanded;
                } else if (item.type == 1) {
                    m_activeVideoIndex = item.videoIndex;
                    for (int i = 0; i < 7; ++i) {
                        m_movies[i]->setPaused(i != m_activeVideoIndex);
                    }
                }
                update();
                break;
            }
        }
    }
}
