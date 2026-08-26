#include "ScoreboardWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QFontDatabase>
#include <cmath>
#include <algorithm>
#include <QImage>

ScoreboardWidget::ScoreboardWidget(ScoreManager* scoreMgr, QWidget* parent) 
    : QWidget(parent), m_scoreMgr(scoreMgr) 
{
    setMouseTracking(true);
    initSpace();
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ScoreboardWidget::updateUniverse);
    m_timer->start(16);
}

void ScoreboardWidget::refresh() {
    initSpace();
}

void ScoreboardWidget::initSpace() {
    auto rng = QRandomGenerator::global();

    // 1. Initialize Star Dust (Background)
    m_stardust.clear();
    for (int i = 0; i < 200; ++i) {
        StarDust sd;
        sd.pos = QPointF(rng->bounded(2000), rng->bounded(1500));
        sd.speed = rng->bounded(10, 30) / 100.0;
        sd.size = rng->bounded(1, 4);
        sd.brightness = rng->bounded(30, 100) / 100.0;
        m_stardust.append(sd);
    }

    // 2. Init Constellation Title "LEADERBOARD"
    initConstellationTitle();

    // 3. Initialize 4 Star Systems for 4 Modes
    m_systems.clear();
    QStringList modeIds = {"CLASSIC", "TIME_ATTACK", "CHAOS", "ENDLESS"};
    QStringList modeDisplayNames = {"CLASSIC", "TIME ATTACK", "CHAOS", "ENDLESS"};
    
    for (int sysIdx = 0; sysIdx < modeIds.size(); ++sysIdx) {
        StarSystem sys;
        sys.modeName = modeDisplayNames[sysIdx];
        sys.titleOpacity = 0.0;
        
        QVector<ScoreRecord> scores = m_scoreMgr->getTopScores(modeIds[sysIdx], 10);
        
        // اگر رکوردی نبود یک دامی بسازیم تا خورشید داشته باشیم
        if (scores.isEmpty()) {
            ScoreRecord dummy;
            dummy.username = "UNKNOWN";
            dummy.score = 0;
            dummy.mode = modeIds[sysIdx];
            scores.append(dummy);
        }

        for (int i = 0; i < scores.size(); ++i) {
            CelestialScore cs;
            cs.rank = i + 1;
            cs.name = scores[i].username;
            cs.score = scores[i].score;
            cs.mode = scores[i].mode;
            
            cs.angle = rng->bounded(360) * M_PI / 180.0;
            
            if (cs.rank == 1) {
                cs.orbitRadius = 0;
                cs.speed = 0;
                cs.baseSize = 45.0;
                // رنگ خورشید بر اساس مود
                if(sysIdx==0) cs.color = QColor(255, 210, 0);       // Classic: Gold
                else if(sysIdx==1) cs.color = QColor(0, 242, 254);  // Time Attack: Cyan
                else if(sysIdx==2) cs.color = QColor(255, 51, 102); // Survival: Pink/Red
                else cs.color = QColor(51, 255, 153);               // Zen: Emerald
            } else {
                cs.orbitRadius = 70 + (i * 40);
                cs.speed = (10.0 - i) / 1000.0;
                if (rng->bounded(2) == 0) cs.speed *= -1; 
                
                cs.baseSize = std::max(12.0, 30.0 - (i * 1.5));
                
                if (cs.rank == 2) cs.color = QColor(200, 220, 255); 
                else if (cs.rank == 3) cs.color = QColor(205, 127, 50); 
                else {
                    QVector<QColor> neonColors = {QColor(0, 242, 254), QColor(255, 0, 128), QColor(0, 255, 128), QColor(160, 32, 240)};
                    cs.color = neonColors[rng->bounded(neonColors.size())];
                }
            }
            sys.planets.append(cs);
        }
        m_systems.append(sys);
    }
    
    m_activeSystemIndex = 0;
    m_selectedIndex = -1;
    m_hoveredRank = -1;
    m_hoveredSystem = -1;
    
    // موقعیت دهی اولیه
    for (int i = 0; i < m_systems.size(); ++i) {
        if (i == m_activeSystemIndex) {
            m_systems[i].currentPos = QPointF(800 / 2.0, 600 / 2.0); // center
            m_systems[i].currentScale = 1.0;
        } else {
            m_systems[i].currentPos = QPointF(100, 150 + i * 150);
            m_systems[i].currentScale = 0.25;
        }
        m_systems[i].targetPos = m_systems[i].currentPos;
        m_systems[i].targetScale = m_systems[i].currentScale;
    }
}

void ScoreboardWidget::initConstellationTitle() {
    m_titleNodes.clear();
    QImage img(800, 150, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    QFont font("Segoe UI", 70, QFont::Black);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 10.0);
    p.setFont(font);
    p.setPen(Qt::white);
    p.drawText(img.rect(), Qt::AlignCenter, "LEADERBOARD");
    
    for(int y = 0; y < img.height(); y += 8) {
        for(int x = 0; x < img.width(); x += 8) {
            if(qAlpha(img.pixel(x, y)) > 100) {
                // اضافه کردن کمی نویز برای طبیعی‌تر شدن
                int noiseX = QRandomGenerator::global()->bounded(4) - 2;
                int noiseY = QRandomGenerator::global()->bounded(4) - 2;
                m_titleNodes.append(QPointF(x + noiseX, y + noiseY));
            }
        }
    }
}

void ScoreboardWidget::calculate3DProjection() {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    
    for (int sysIdx = 0; sysIdx < m_systems.size(); ++sysIdx) {
        StarSystem& sys = m_systems[sysIdx];
        
        // آپدیت اهداف سیستم‌ها (آیا سیستم فعال است یا نه)
        if (sysIdx == m_activeSystemIndex) {
            if (m_selectedIndex != -1) {
                sys.targetPos = QPointF(cx - 200, cy); // اگر پنل باز است برود چپ
            } else {
                sys.targetPos = QPointF(cx, cy); // در غیر اینصورت مرکز
            }
            sys.targetScale = 1.0;
        } else {
            // چیدمان عمودی سیستم‌های غیرفعال در سمت چپ
            qreal yPos = cy - ((m_systems.size()-1) * 75.0) + (sysIdx * 150.0);
            sys.targetPos = QPointF(100, yPos);
            sys.targetScale = 0.25;
        }
        
        // حرکت نرم سیستم
        sys.currentPos += (sys.targetPos - sys.currentPos) * 0.08;
        sys.currentScale += (sys.targetScale - sys.currentScale) * 0.1;
        
        // محاسبه پروژکشن سیارات داخل سیستم
        for (auto& cs : sys.planets) {
            qreal dynamicOrbit = cs.orbitRadius * sys.currentScale * m_zoomFactor;
            qreal x = std::cos(cs.angle) * dynamicOrbit;
            qreal y = std::sin(cs.angle) * dynamicOrbit * m_orbitTilt;
            
            cs.projectedPos = sys.currentPos + QPointF(x, y);
            cs.zDepth = std::sin(cs.angle);
            cs.scale = sys.currentScale * m_zoomFactor * (0.6 + (cs.zDepth + 1.0) * 0.2);
        }
        
        // سورت کردن سیارات بر اساس عمق z برای کشیدن درست
        std::sort(sys.planets.begin(), sys.planets.end(), [](const CelestialScore& a, const CelestialScore& b) {
            return a.zDepth < b.zDepth;
        });
    }
}

void ScoreboardWidget::updateUniverse() {
    m_time += 0.05;
    
    for (auto& sys : m_systems) {
        for (auto& cs : sys.planets) {
            cs.angle += cs.speed;
            if (cs.angle > 2 * M_PI) cs.angle -= 2 * M_PI;
            if (cs.angle < 0) cs.angle += 2 * M_PI;
        }
    }

    for (auto& sd : m_stardust) {
        sd.pos.setX(sd.pos.x() - sd.speed);
        if (sd.pos.x() < 0) {
            sd.pos.setX(width() + 50);
            sd.pos.setY(QRandomGenerator::global()->bounded(height()));
        }
    }

    calculate3DProjection();
    update();
}

void ScoreboardWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawDeepSpace(painter);
    drawConstellationTitle(painter);
    
    // رسم سیستم‌ها (غیر فعال‌ها اول، فعال آخر تا روی همه‌چیز بیفتد)
    for (int i = 0; i < m_systems.size(); ++i) {
        if (i == m_activeSystemIndex) continue;
        drawOrbitRings(painter, m_systems[i]);
        for (const auto& cs : m_systems[i].planets) {
            drawCelestialBody(painter, cs, m_systems[i]);
        }
        drawSystemLabel(painter, m_systems[i]);
    }
    
    // سیستم فعال
    const auto& activeSys = m_systems[m_activeSystemIndex];
    drawOrbitRings(painter, activeSys);
    for (const auto& cs : activeSys.planets) {
        drawCelestialBody(painter, cs, activeSys);
    }
    drawSystemLabel(painter, activeSys);

    if (m_selectedIndex != -1) {
        for (const auto& cs : activeSys.planets) {
            if (cs.rank - 1 == m_selectedIndex) {
                painter.setPen(QPen(cs.color, 2, Qt::DashLine));
                painter.drawLine(cs.projectedPos, QPointF(width() - 500 - 40, height() / 2.0));
                drawDossierOverlay(painter, cs);
                break;
            }
        }
    }

    drawNeonBackButton(painter);
}

void ScoreboardWidget::drawDeepSpace(QPainter& painter) {
    int baseHue = ThemeManager::instance().getBaseHue();
    QLinearGradient bgGrad(0, 0, width(), height());
    bgGrad.setColorAt(0.0, QColor::fromHsv(baseHue, 220, 14));
    bgGrad.setColorAt(1.0, QColor::fromHsv(baseHue, 240, 6));
    painter.fillRect(rect(), bgGrad);

    // سحابی دور دست کهکشانی
    QRadialGradient neb(width()*0.5, height()*0.5, width()*0.6);
    QColor nebCol = ThemeManager::instance().getPrimaryColor();
    nebCol.setAlpha(25);
    neb.setColorAt(0.0, nebCol);
    neb.setColorAt(1.0, Qt::transparent);
    painter.fillRect(rect(), neb);

    painter.setPen(Qt::NoPen);
    for (int i = 0; i < m_stardust.size(); ++i) {
        const auto& sd = m_stardust[i];
        qreal size = sd.size * (1.0 + (m_zoomFactor - 1.0) * 0.2);
        QColor c;
        if (i % 3 == 0) c = ThemeManager::instance().getPrimaryColor();
        else if (i % 3 == 1) c = ThemeManager::instance().getSecondaryColor();
        else c = Qt::white;
        c.setAlpha(int(255 * sd.brightness));
        painter.setBrush(c);
        painter.drawEllipse(sd.pos, size, size);
    }
}

void ScoreboardWidget::drawConstellationTitle(QPainter& painter) {
    if (m_titleNodes.isEmpty()) return;
    
    painter.save();
    // محاسبه موقعیت در بالای صفحه
    qreal topOffset = 20.0;
    qreal leftOffset = (width() - 800) / 2.0;
    painter.translate(leftOffset, topOffset);
    
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    QColor lineCol = priCol;
    lineCol.setAlpha(80);

    // رسم خطوط بین نودهای نزدیک
    painter.setPen(QPen(lineCol, 1.0));
    for (int i = 0; i < m_titleNodes.size(); ++i) {
        for (int j = i + 1; j < m_titleNodes.size(); ++j) {
            qreal dist = std::hypot(m_titleNodes[i].x() - m_titleNodes[j].x(), m_titleNodes[i].y() - m_titleNodes[j].y());
            if (dist < 12.0) {
                painter.drawLine(m_titleNodes[i], m_titleNodes[j]);
            }
        }
    }
    
    // رسم نقطه‌ها
    qreal pulse = std::sin(m_time * 3.0) * 0.5 + 1.0;
    painter.setPen(Qt::NoPen);
    for (const auto& node : m_titleNodes) {
        painter.setBrush(QColor(255, 255, 255, 180));
        painter.drawEllipse(node, 1.5 * pulse, 1.5 * pulse);
    }
    
    // هاله کلی زیر متن
    QFont font("Segoe UI", 70, QFont::Black);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 10.0);
    painter.setFont(font);
    QColor glowText = priCol;
    glowText.setAlpha(20);
    painter.setPen(glowText);
    painter.drawText(QRectF(0, 0, 800, 150), Qt::AlignCenter, "LEADERBOARD");
    painter.restore();
}

void ScoreboardWidget::drawSystemLabel(QPainter& painter, const StarSystem& sys) {
    // Label under the star system
    bool isActive = (&sys == &m_systems[m_activeSystemIndex]);
    
    painter.setPen(isActive ? Qt::white : QColor(150, 170, 190, 150));
    QFont font("Consolas", isActive ? 16 : 10, QFont::Bold);
    painter.setFont(font);
    
    qreal yOff = isActive ? 180.0 * sys.currentScale : 60.0;
    painter.drawText(QRectF(sys.currentPos.x() - 150, sys.currentPos.y() + yOff, 300, 30), 
                     Qt::AlignCenter, sys.modeName + " SYSTEM");
}

void ScoreboardWidget::drawOrbitRings(QPainter& painter, const StarSystem& sys) {
    QColor ringCol = ThemeManager::instance().getPrimaryColor();
    ringCol.setAlpha(40);
    painter.setPen(QPen(ringCol, 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    for (const auto& cs : sys.planets) {
        if (cs.orbitRadius > 0) {
            qreal rX = cs.orbitRadius * sys.currentScale * m_zoomFactor;
            qreal rY = rX * m_orbitTilt;
            painter.drawEllipse(sys.currentPos, rX, rY);
        }
    }
}

void ScoreboardWidget::drawCelestialBody(QPainter& painter, const CelestialScore& body, const StarSystem& sys) {
    qreal currentSize = body.baseSize * body.scale;
    bool isActiveSys = (&sys == &m_systems[m_activeSystemIndex]);
    bool isHovered = isActiveSys && (body.rank - 1 == m_hoveredRank || body.rank - 1 == m_selectedIndex);

    if (isHovered) currentSize *= 1.25;

    QRadialGradient glowGrad(body.projectedPos, currentSize * 2.8);
    QColor glowColor = body.color;
    glowColor.setAlpha(isHovered ? 180 : 80);
    glowGrad.setColorAt(0.0, glowColor);
    glowGrad.setColorAt(1.0, Qt::transparent);
    painter.setBrush(glowGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(body.projectedPos, currentSize * 2.8, currentSize * 2.8);

    QPointF focalPoint = body.projectedPos - QPointF(currentSize*0.3, currentSize*0.4);
    QRadialGradient bodyGrad(body.projectedPos, currentSize, focalPoint);
    bodyGrad.setColorAt(0.0, Qt::white);
    bodyGrad.setColorAt(0.15, body.color.lighter(150));
    bodyGrad.setColorAt(0.6, body.color);
    bodyGrad.setColorAt(0.9, body.color.darker(200));
    bodyGrad.setColorAt(1.0, QColor(0, 0, 0, 200));
    
    painter.setBrush(bodyGrad);
    painter.drawEllipse(body.projectedPos, currentSize, currentSize);

    if (!isHovered && currentSize > 8.0 && isActiveSys) {
        painter.setPen(QColor(255, 255, 255, 200));
        QFont f("Segoe UI", int(currentSize * 0.75), QFont::Black);
        painter.setFont(f);
        painter.drawText(QRectF(body.projectedPos.x() - currentSize, body.projectedPos.y() - currentSize, 
                                currentSize * 2, currentSize * 2), Qt::AlignCenter, QString::number(body.rank));
    }
}

void ScoreboardWidget::drawDossierOverlay(QPainter& painter, const CelestialScore& body) {
    qreal pw = 500;
    qreal ph = 580;
    QRectF panelRect(width() - pw - 40, height() / 2.0 - ph / 2.0, pw, ph);

    QLinearGradient glassGrad(panelRect.topLeft(), panelRect.bottomRight());
    glassGrad.setColorAt(0.0, QColor(255, 255, 255, 20));
    glassGrad.setColorAt(0.3, QColor(10, 15, 30, 200));
    glassGrad.setColorAt(0.8, QColor(5, 8, 20, 220));
    glassGrad.setColorAt(1.0, QColor(255, 255, 255, 10));
    painter.setBrush(glassGrad);
    
    QColor borderColor = body.color;
    painter.setPen(QPen(borderColor, 2));
    painter.drawRoundedRect(panelRect, 15, 15);

    painter.setPen(QPen(QColor(255, 255, 255, 10), 1));
    for (int i = 10; i < pw; i += 20) painter.drawLine(panelRect.left() + i, panelRect.top(), panelRect.left() + i, panelRect.bottom());
    for (int i = 10; i < ph; i += 20) painter.drawLine(panelRect.left(), panelRect.top() + i, panelRect.right(), panelRect.top() + i);

    painter.setPen(QPen(Qt::white, 3));
    painter.drawLine(panelRect.topLeft() + QPointF(0, 25), panelRect.topLeft());
    painter.drawLine(panelRect.topLeft(), panelRect.topLeft() + QPointF(25, 0));
    painter.drawLine(panelRect.bottomRight() - QPointF(0, 25), panelRect.bottomRight());
    painter.drawLine(panelRect.bottomRight(), panelRect.bottomRight() - QPointF(25, 0));

    QVector<ScoreRecord> allScores = m_scoreMgr->getScores();
    QVector<ScoreRecord> history;
    int maxPlayerScore = 0;
    for (const auto& r : allScores) {
        if (r.username == body.name) {
            history.append(r);
            if (r.score > maxPlayerScore) maxPlayerScore = r.score;
        }
    }
    std::reverse(history.begin(), history.end());

    qreal cx = panelRect.left() + 30;
    qreal cy = panelRect.top() + 30;

    QRadialGradient avatarGlow(cx + 40, cy + 40, 50);
    avatarGlow.setColorAt(0, QColor(borderColor.red(), borderColor.green(), borderColor.blue(), 100));
    avatarGlow.setColorAt(1, Qt::transparent);
    painter.setBrush(avatarGlow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 10, cy - 10, 100, 100);

    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx + 10, cy + 10, 60, 60);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 28, QFont::Black));
    painter.drawText(QRectF(cx + 10, cy + 10, 60, 60), Qt::AlignCenter, QString::number(body.rank));

    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 22, QFont::Bold));
    painter.drawText(QRectF(cx + 100, cy, pw - 150, 40), Qt::AlignLeft | Qt::AlignVCenter, body.name.toUpper());

    painter.setPen(borderColor.lighter(130));
    painter.setFont(QFont("Consolas", 12));
    painter.drawText(QRectF(cx + 100, cy + 40, pw - 150, 20), Qt::AlignLeft | Qt::AlignVCenter, "CLASS: ELITE OPERATIVE");
    
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1, Qt::DashLine));
    painter.drawLine(panelRect.left() + 20, cy + 90, panelRect.right() - 20, cy + 90);

    cy += 110;
    auto drawStatBox = [&](qreal x, qreal y, const QString& title, const QString& val, QColor c) {
        painter.setBrush(QColor(10, 20, 30, 150));
        painter.setPen(QPen(c, 1));
        painter.drawRoundedRect(x, y, 135, 60, 4, 4);
        painter.setPen(QColor(150, 170, 190));
        painter.setFont(QFont("Consolas", 9));
        painter.drawText(QRectF(x, y + 5, 135, 20), Qt::AlignCenter, title);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Consolas", 16, QFont::Bold));
        painter.drawText(QRectF(x, y + 25, 135, 30), Qt::AlignCenter, val);
    };

    drawStatBox(cx, cy, "MISSIONS", QString::number(history.size()), QColor(0, 242, 254));
    drawStatBox(cx + 150, cy, "TOP SCORE", QString::number(maxPlayerScore), QColor(255, 204, 0));
    drawStatBox(cx + 300, cy, "WIN RATE", history.size() > 0 ? "87%" : "0%", QColor(51, 255, 153));

    cy += 80;
    painter.setPen(QColor(255, 255, 255, 180));
    painter.setFont(QFont("Consolas", 12, QFont::Bold));
    painter.drawText(QRectF(cx, cy, pw, 20), Qt::AlignLeft, "COMBAT TELEMETRY");

    cy += 30;
    QRectF graphRect(cx, cy, pw - 60, 80);
    painter.setBrush(QColor(0, 0, 0, 100));
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter.drawRect(graphRect);
    
    for (int i = 1; i < 4; ++i) painter.drawLine(graphRect.left(), graphRect.top() + i * 20, graphRect.right(), graphRect.top() + i * 20);

    if (history.size() > 1 && maxPlayerScore > 0) {
        int pts = std::min(10, (int)history.size());
        QPolygonF graphPoly;
        qreal xStep = graphRect.width() / (pts - 1);
        for (int i = 0; i < pts; ++i) {
            int hIdx = history.size() - pts + i; 
            qreal norm = (qreal)history[hIdx].score / maxPlayerScore;
            graphPoly << QPointF(graphRect.left() + i * xStep, graphRect.bottom() - norm * graphRect.height());
        }
        painter.setPen(QPen(borderColor, 2));
        painter.drawPolyline(graphPoly);
        
        painter.setBrush(borderColor.lighter(150));
        painter.setPen(Qt::NoPen);
        for (const QPointF& pt : graphPoly) painter.drawEllipse(pt, 3, 3);
    } else {
        painter.setPen(QColor(150, 170, 190));
        painter.drawText(graphRect, Qt::AlignCenter, "INSUFFICIENT TELEMETRY DATA");
    }

    cy += 100;
    painter.setPen(QColor(255, 255, 255, 180));
    painter.setFont(QFont("Consolas", 12, QFont::Bold));
    painter.drawText(QRectF(cx, cy, pw, 20), Qt::AlignLeft, "RECENT MISSIONS");

    cy += 30;
    int itemsToShow = std::min(4, (int)history.size());
    for (int i = 0; i < itemsToShow; ++i) {
        const auto& rec = history[i];
        QRectF rowRect(cx, cy + i * 35, pw - 60, 30);
        
        painter.setBrush(QColor(255, 255, 255, 15));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rowRect, 3, 3);
        
        painter.setBrush(ThemeManager::instance().getPrimaryColor());
        painter.drawRect(rowRect.left() + 5, rowRect.top() + 8, 4, 14);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Consolas", 10, QFont::Bold));
        painter.drawText(QRectF(rowRect.left() + 15, rowRect.top(), 100, 30), Qt::AlignLeft | Qt::AlignVCenter, rec.mode);
        
        painter.setPen(QColor(255, 204, 0));
        painter.drawText(QRectF(rowRect.left() + 150, rowRect.top(), 100, 30), Qt::AlignLeft | Qt::AlignVCenter, QString::number(rec.score) + " PTS");
        
        painter.setPen(QColor(150, 170, 190));
        painter.setFont(QFont("Consolas", 9));
        painter.drawText(QRectF(rowRect.right() - 150, rowRect.top(), 140, 30), Qt::AlignRight | Qt::AlignVCenter, rec.timestamp);
    }

    painter.setPen(QColor(255, 255, 255, 100));
    painter.setFont(QFont("Consolas", 14));
    painter.drawText(QRectF(panelRect.right() - 35, panelRect.top() + 10, 30, 30), Qt::AlignCenter, "✖");
}

void ScoreboardWidget::drawNeonBackButton(QPainter& painter) {
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
    painter.drawText(m_backBtnRect, Qt::AlignCenter, "◄ HYPERJUMP");
    
    painter.setPen(QColor(148, 163, 184, 150));
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(QRectF(20, 75, 200, 20), Qt::AlignLeft, "Scroll mouse to zoom");
}

void ScoreboardWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    
    bool wasBackHovered = m_backHovered;
    m_backHovered = m_backBtnRect.contains(m_mousePos);
    if (m_backHovered != wasBackHovered) update();

    int oldHoverRank = m_hoveredRank;
    int oldHoverSys = m_hoveredSystem;
    m_hoveredRank = -1;
    m_hoveredSystem = -1;
    
    if (m_selectedIndex != -1) {
        qreal pw = 500;
        qreal ph = 580;
        QRectF panelRect(width() - pw - 40, height() / 2.0 - ph / 2.0, pw, ph);
        QRectF closeRect(panelRect.right() - 35, panelRect.top() + 10, 30, 30);
        if (closeRect.contains(m_mousePos)) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }

    // Check collision with inactive systems
    for (int i = 0; i < m_systems.size(); ++i) {
        if (i == m_activeSystemIndex) continue;
        qreal dist = std::hypot(m_mousePos.x() - m_systems[i].currentPos.x(), m_mousePos.y() - m_systems[i].currentPos.y());
        if (dist <= 100 * m_systems[i].currentScale) {
            m_hoveredSystem = i;
            break;
        }
    }

    // Check collision with planets in active system
    if (m_hoveredSystem == -1) {
        const auto& activeSys = m_systems[m_activeSystemIndex];
        for (int i = activeSys.planets.size() - 1; i >= 0; --i) {
            const auto& cs = activeSys.planets[i];
            qreal r = cs.baseSize * cs.scale;
            qreal dist = std::hypot(m_mousePos.x() - cs.projectedPos.x(), m_mousePos.y() - cs.projectedPos.y());
            if (dist <= r * 1.5) { 
                m_hoveredRank = cs.rank - 1; 
                break;
            }
        }
    }
    
    if (m_hoveredRank != oldHoverRank || m_hoveredSystem != oldHoverSys) {
        if (m_hoveredSystem != -1) setCursor(Qt::PointingHandCursor);
        update(); 
    }
}

void ScoreboardWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_backHovered) {
            emit backClicked();
            return;
        }

        // Clicked close button on dossier?
        if (m_selectedIndex != -1) {
            qreal pw = 500;
            qreal ph = 580;
            QRectF panelRect(width() - pw - 40, height() / 2.0 - ph / 2.0, pw, ph);
            QRectF closeRect(panelRect.right() - 35, panelRect.top() + 10, 30, 30);
            if (closeRect.contains(m_mousePos)) {
                m_selectedIndex = -1;
                update();
                return;
            }
        }

        // Clicked inactive system to activate it?
        if (m_hoveredSystem != -1 && m_hoveredSystem != m_activeSystemIndex) {
            m_activeSystemIndex = m_hoveredSystem;
            m_selectedIndex = -1;
            m_hoveredSystem = -1;
            update();
            return;
        }

        // Clicked planet in active system?
        if (m_hoveredRank != -1) {
            if (m_selectedIndex == m_hoveredRank) {
                m_selectedIndex = -1;
            } else {
                m_selectedIndex = m_hoveredRank;
            }
        } else {
            m_selectedIndex = -1;
        }
        update();
    }
}

void ScoreboardWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    calculate3DProjection();
}

void ScoreboardWidget::wheelEvent(QWheelEvent* event) {
    m_targetZoom += event->angleDelta().y() / 1200.0;
    m_targetZoom = std::clamp(m_targetZoom, 0.4, 4.0);
}
