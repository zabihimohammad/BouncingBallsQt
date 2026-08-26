#include "HelpWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QImage>

HelpWidget::HelpWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    m_tabRects.resize(7);
    
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
    switchTab(2); // Start at Advanced Audio as requested
}

HelpWidget::~HelpWidget() {}

void HelpWidget::pauseAnimation() {
    for (int i=0; i<7; ++i) m_movies[i]->setPaused(true);
}
void HelpWidget::resumeAnimation() {
    for (int i=0; i<7; ++i) m_movies[i]->setPaused(false);
}

void HelpWidget::switchTab(int index) {
    m_activeTab = index;
    for (int i = 0; i < 7; ++i) {
        m_movies[i]->setPaused(i != index);
    }
    update();
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
    QLinearGradient bgGrad(0, 0, width(), height());
    bgGrad.setColorAt(0.0, QColor(4, 6, 12));
    bgGrad.setColorAt(1.0, QColor(10, 15, 30));
    p.fillRect(rect(), bgGrad);
    p.setPen(QPen(QColor(0, 242, 254, 15), 1));
    for (int x = 0; x < width(); x += 40) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) p.drawLine(0, y, width(), y);
}

void HelpWidget::drawMenu(QPainter& p) {
    qreal startY = 100.0;
    qreal tabW = 320.0;
    qreal tabH = 50.0;
    qreal gap = 15.0;
    QStringList tabTitles = {
        "[01] GAMEPLAY (TACTICAL)",
        "[02] BASIC SETTINGS", 
        "[03] ADVANCED: AUDIO",
        "[04] ADVANCED: GRAPHICS",
        "[05] ADVANCED: HAPTICS",
        "[06] ADVANCED: DISPLAY",
        "[07] TELEMETRY (LEADERBOARD)"
    };
    for (int i = 0; i < 7; ++i) {
        m_tabRects[i] = QRectF(40, startY + i * (tabH + gap), tabW, tabH);
        bool isActive = (m_activeTab == i);
        bool isHovered = (m_hoveredTab == i);
        QColor bgColor = isActive ? QColor(0, 242, 254, 80) : (isHovered ? QColor(255, 255, 255, 20) : QColor(255, 255, 255, 5));
        p.setBrush(bgColor);
        p.setPen(QPen(isActive ? QColor(0, 242, 254) : QColor(100, 100, 100), 2));
        p.drawRoundedRect(m_tabRects[i], 5, 5);
        if (isActive) {
            p.setBrush(Qt::white);
            p.setPen(Qt::NoPen);
            p.drawRect(m_tabRects[i].left() + 5, m_tabRects[i].top() + 10, 4, tabH - 20);
        }
        p.setPen(isActive ? Qt::white : (isHovered ? QColor(200, 220, 255) : QColor(180, 180, 200)));
        QFont f("Consolas", 14, isActive ? QFont::Bold : QFont::Normal);
        p.setFont(f);
        p.drawText(QRectF(m_tabRects[i].left() + 20, m_tabRects[i].top(), tabW - 20, tabH), Qt::AlignLeft | Qt::AlignVCenter, tabTitles[i]);
    }
}

void HelpWidget::drawContent(QPainter& p) {
    QRectF contentRect(400, 100, width() - 440, height() - 140);
    p.setBrush(QColor(10, 15, 30, 200));
    p.setPen(QPen(QColor(0, 242, 254, 150), 2));
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
    p.drawText(QRectF(contentRect.left() + 30, contentRect.top() + 20, contentRect.width() - 60, 40), Qt::AlignLeft | Qt::AlignVCenter, titles[m_activeTab]);
    
    p.setPen(QPen(QColor(255, 255, 255, 50), 1, Qt::DashLine));
    p.drawLine(contentRect.left() + 30, contentRect.top() + 70, contentRect.right() - 30, contentRect.top() + 70);

    QRectF animRect(contentRect.left() + 30, contentRect.top() + 90, contentRect.width() - 60, contentRect.height() - 280);
    
    if (m_movies[m_activeTab]->isValid()) {
        QImage currentFrame = m_movies[m_activeTab]->currentImage();
        if (!currentFrame.isNull()) {
            QImage scaled = currentFrame.scaled(animRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPointF pos = animRect.center() - QPointF(scaled.width()/2.0, scaled.height()/2.0);
            p.drawImage(pos, scaled);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(0, 242, 254, 150), 2));
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
        ">> AUDIO LAUNCHER INTERACTION:\n\n- Enter the Advanced Settings 3D orbit and select the AUDIO node.\n- Drag the audio orb from the cannon base backward and release it to launch.\n- The orb will smash the floating crystals, cycling through the soundtrack playlist.",
        ">> GRAPHICS REACTOR INTERACTION:\n\n- Enter the Advanced Settings 3D orbit and select the GRAPHICS node.\n- Drag the plasma capsules (LOW / MEDIUM / ULTRA) into the Quantum Core.\n- The core will absorb the capsule and dynamically upgrade visual fidelity.",
        ">> HAPTICS SEISMOGRAPH INTERACTION:\n\n- Enter the Advanced Settings 3D orbit and select the HAPTICS node.\n- Lift the heavy metallic weight using the mouse and drop it onto the anvil.\n- The kinetic impact determines the Screen Shake intensity in-game.",
        ">> DISPLAY GEARBOX INTERACTION:\n\n- Enter the Advanced Settings 3D orbit and select the DISPLAY node.\n- Rotate the massive mechanical lever around the gearbox core.\n- This physically shifts the FPS gear (60, 120, 144, 999 REDLINE).",
        ">> TELEMETRY FOOTAGE:\n\n- The Leaderboard visualizes top operatives as celestial bodies.\n- Interact with a planet to deploy the holographic Dossier Panel."
    };
    p.drawText(QRectF(animRect.left(), animRect.bottom() + 30, animRect.width(), 230), Qt::AlignLeft | Qt::TextWordWrap, descs[m_activeTab]);
}

void HelpWidget::drawNeonBackButton(QPainter& painter) {
    m_backBtnRect = QRectF(20, 20, 130, 45);
    if (m_backHovered) {
        painter.setBrush(QColor(255, 51, 102, 100)); 
        painter.setPen(QPen(QColor(255, 51, 102), 2));
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
    int oldHover = m_hoveredTab;
    m_hoveredTab = -1;
    for (int i = 0; i < 7; ++i) {
        if (m_tabRects[i].contains(pos)) {
            m_hoveredTab = i;
            break;
        }
    }
    if (wasBackHovered != m_backHovered || oldHover != m_hoveredTab) {
        if (m_backHovered || m_hoveredTab != -1) setCursor(Qt::PointingHandCursor);
        else setCursor(Qt::ArrowCursor);
        update();
    }
}
void HelpWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_backHovered) { emit backClicked(); return; }
        if (m_hoveredTab != -1 && m_hoveredTab != m_activeTab) switchTab(m_hoveredTab);
    }
}
