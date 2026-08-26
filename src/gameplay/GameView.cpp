#include "../ui/ThemeManager.h"
#include "GameView.h"
#include "GameScene.h"
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

GameView::GameView(QWidget* parent) : QGraphicsView(parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background: transparent; border: none;");
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void GameView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (scene()) fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void GameView::triggerShake(int intensity) {
    m_currentShake = qMax(m_currentShake, (qreal)intensity);
    if (!m_shakeTimer) {
        m_shakeTimer = new QTimer(this);
        connect(m_shakeTimer, &QTimer::timeout, this, &GameView::updateShake);
    }
    if (!m_shakeTimer->isActive()) m_shakeTimer->start(16);
}

void GameView::updateShake() {
    if (m_currentShake < 0.5) {
        m_currentShake = 0;
        m_shakeTimer->stop();
        resetTransform();
        if (scene()) fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        return;
    }
    m_currentShake *= 0.85;
    qreal dx = ((QRandomGenerator::global()->bounded(200) - 100) / 100.0) * m_currentShake;
    qreal dy = ((QRandomGenerator::global()->bounded(200) - 100) / 100.0) * m_currentShake;
    resetTransform();
    if (scene()) fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    translate(dx, dy);
}

void GameView::drawBackground(QPainter* painter, const QRectF&) {
    painter->save();
    painter->resetTransform();

    int W = viewport()->width();
    int H = viewport()->height();
    QRectF vpRect(0, 0, W, H);
    QPointF C(W * 0.5, H * 0.5);

    qreal accuracy = 0.5;
    int combo = 0;
    if (auto* gs = qobject_cast<GameScene*>(scene())) {
        accuracy = gs->getAccuracyRatio();
        combo    = gs->getComboStreak();
    }
    // Dynamic Speed: 0.4x to 3.5x
    qreal warpSpeed = 0.4 + accuracy * 2.6 + (combo > 2 ? 0.6 : 0.0);

    qreal t = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
    qreal breath = 0.88 + 0.12 * std::sin(t * M_PI * 1.05);
    qreal hueDrift = std::fmod(t / 60.0, 1.0);
    int themeBaseHue = ThemeManager::instance().getBaseHue();

    qreal diagR = std::sqrt((qreal)W*W + (qreal)H*H) * 0.60;
    auto dToR = [&](qreal d) -> qreal {
        return diagR * std::pow(1.0 - d, 1.20);
    };

    // ── 1. DEEP SPACE COSMIC BASE (DARK INDIGO/BLACK) ────────────────────────
    QColor bgCol = QColor::fromHsv(themeBaseHue, 200, 15);
    painter->fillRect(vpRect, bgCol);

    // ── 2. VIVID, DENSE INWARD-SPIRALING COSMIC NEBULAE ───────────────────────
    {
        painter->setPen(Qt::NoPen);
        const int NUM_ARMS = 4;
        const int PUFFS_PER_ARM = 10;

        for (int a = 0; a < NUM_ARMS; ++a) {
            qreal armBaseAngle = a * (M_PI / 2.0) + (M_PI / 4.0); // 4 corners
            int baseHue = (themeBaseHue + a * 20) % 360;

            for (int p = 0; p < PUFFS_PER_ARM; ++p) {
                qreal phase = std::fmod((p / (qreal)PUFFS_PER_ARM) - (t * warpSpeed * 0.08), 1.0);
                if (phase < 0.0) phase += 1.0;

                qreal r = std::pow(phase, 1.35) * diagR * 0.98;
                if (r < 20.0) continue;

                qreal spiralAngle = armBaseAngle + (1.0 - phase) * 2.2 + (t * warpSpeed * 0.08);
                qreal cloudX = C.x() + std::cos(spiralAngle) * r;
                qreal cloudY = C.y() + std::sin(spiralAngle) * r;

                qreal cloudRadius = (70.0 + phase * 260.0) * (std::min(W, H) / 800.0);

                int alpha = int((0.35 + 0.65 * phase) * 160.0 * breath);
                int hue = (baseHue + int((1.0 - phase) * 40.0) + int(hueDrift * 30)) % 360;
                int sat = int(190 + (1.0 - phase) * 65);
                int val = int(160 + (1.0 - phase) * 95);

                QRadialGradient cloudGrad(cloudX, cloudY, cloudRadius);
                QColor coreCol = QColor::fromHsv(hue, sat, val, std::clamp(alpha, 0, 255));
                QColor midCol  = QColor::fromHsv((hue + 15) % 360, sat - 30, val - 40, std::clamp(int(alpha * 0.65), 0, 255));
                
                cloudGrad.setColorAt(0.0, coreCol);
                cloudGrad.setColorAt(0.5, midCol);
                cloudGrad.setColorAt(1.0, QColor(0, 0, 0, 0));

                painter->setBrush(cloudGrad);
                painter->save();
                painter->translate(cloudX, cloudY);
                painter->rotate((spiralAngle + M_PI/2.0) * 180.0 / M_PI);
                painter->drawEllipse(QPointF(0, 0), cloudRadius * 1.5, cloudRadius * 0.85);
                painter->restore();
            }
        }
    }

    // ── 3. DISTANT TWINKLING STARFIELD (180+ Stars across the deep cosmos) ────
    {
        static struct StarInfo {
            float x, y, speed, freq, baseAlpha, size;
            int type;
        } stars[180];
        static bool starsInitialized = false;

        if (!starsInitialized) {
            starsInitialized = true;
            auto* rng = QRandomGenerator::global();
            for (int i = 0; i < 180; ++i) {
                stars[i].x = rng->bounded(10000) / 10000.0f;
                stars[i].y = rng->bounded(10000) / 10000.0f;
                stars[i].speed = 0.5f + (rng->bounded(100) / 100.0f) * 1.5f;
                stars[i].freq = 1.0f + (rng->bounded(100) / 100.0f) * 3.5f;
                stars[i].baseAlpha = 100.0f + rng->bounded(155);
                stars[i].size = 0.8f + (rng->bounded(100) / 100.0f) * 2.0f;
                stars[i].type = rng->bounded(6);
            }
        }

        for (int i = 0; i < 180; ++i) {
            qreal px = stars[i].x * W;
            qreal py = stars[i].y * H;

            // Twinkle oscillation
            qreal twinkle = 0.4 + 0.6 * std::sin(t * stars[i].freq + i * 1.7);
            int starAlpha = int(stars[i].baseAlpha * twinkle * breath);
            if (starAlpha < 15) continue;

            // Multi-colored stars
            QColor starCol;
            if (stars[i].type == 0) starCol = QColor(255, 255, 255, starAlpha);        // Brilliant Diamond White
            else if (stars[i].type == 1) starCol = ThemeManager::instance().getPrimaryColor();
            else if (stars[i].type == 2) starCol = ThemeManager::instance().getSecondaryColor();
            else if (stars[i].type == 3) starCol = QColor::fromHsv(themeBaseHue, 180, 255, starAlpha);
            else if (stars[i].type == 4) starCol = QColor::fromHsv((themeBaseHue+40)%360, 180, 255, starAlpha);
            else starCol = QColor::fromHsv((themeBaseHue+320)%360, 180, 255, starAlpha);                         // Emerald Nebula Star

            qreal sz = stars[i].size;

            // Star Core
            painter->setBrush(starCol);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(px, py), sz, sz);

            // 4-Point Diffraction Spike Glint for bright stars
            if (sz > 1.8f && twinkle > 0.75) {
                painter->setPen(QPen(starCol, 0.8));
                qreal spikeLen = sz * 3.5;
                painter->drawLine(QPointF(px - spikeLen, py), QPointF(px + spikeLen, py));
                painter->drawLine(QPointF(px, py - spikeLen), QPointF(px, py + spikeLen));
            }
        }
    }

    // ── 4. HYPNOTIC TUNNEL WALLS (Counter-Rotating Rings) ─────────────────────
    {
        const int NR = 28;
        painter->setBrush(Qt::NoBrush);

        for (int i = 0; i < NR; i++) {
            qreal baseD = (qreal)i / NR;
            qreal animPhase = std::fmod(t * warpSpeed * 0.22, 1.0);
            qreal d = std::fmod(baseD + animPhase, 1.0);
            qreal r = dToR(d);
            if (r < 2.0) continue;

            qreal rotRate = 6.0 + d * 14.0;
            qreal twist = (i % 2 == 0) ? (t * rotRate) : (-t * rotRate * 0.65);

            int alpha = int(std::pow(d, 0.45) * 190 * breath);
            if (alpha < 8) continue;

            QColor col;
            if (d > 0.80) {
                col = QColor(255, int(210 + (d-0.8)*225), int(110 + (d-0.8)*725), alpha);
            } else if (d > 0.55) {
                int hue = int(themeBaseHue + 25 + hueDrift * 30) % 360;
                col = QColor::fromHsv(hue, 200, 220, alpha);
            } else if (d > 0.25) {
                int hue = int(themeBaseHue + hueDrift * 25) % 360;
                col = QColor::fromHsv(hue, 180, 170, alpha);
            } else {
                int nearAlpha = int(alpha * (0.4 + d * 2.4));
                int hue = int(themeBaseHue - 10 + hueDrift * 20) % 360;
                col = QColor::fromHsv(hue, 160, 150, std::min(255, nearAlpha));
            }

            qreal baseW = 0.6 + d * 2.8;
            if (d < 0.30) baseW = 1.0 + (0.3 - d) * 5.0;

            painter->save();
            painter->translate(C);
            painter->rotate(twist);

            // Glow pass
            QColor glow = col; glow.setAlpha(col.alpha() / 6);
            painter->setPen(QPen(glow, baseW + 5.0));
            painter->drawEllipse(QPointF(0,0), r, r * 0.88);

            // Bright core ring
            painter->setPen(QPen(col, baseW));
            painter->drawEllipse(QPointF(0,0), r, r * 0.88);

            painter->restore();
        }
    }

    // ── 5. SPIRAL ENERGY STREAKS ─────────────────────────────────────────────
    {
        const int NS = 140;
        qreal spiralRot = t * warpSpeed * 0.15;

        for (int i = 0; i < NS; i++) {
            qreal angle = i * (2.0 * M_PI / NS) + spiralRot;
            qreal pSpeed = 0.9 + (i % 11) * 0.08;
            qreal d0 = std::fmod((i * 0.23) + t * pSpeed * 0.30 * warpSpeed, 1.0);
            qreal sLen = 0.038 + warpSpeed * 0.040;
            qreal d1 = std::max(0.0, d0 - sLen);

            qreal r0 = dToR(d0);
            qreal r1 = dToR(d1);
            if (r1 - r0 < 1.5) continue;

            int alpha = int(std::pow(d0, 0.5) * 180 * breath);
            if (d1 < 0.05) alpha = int(alpha * (d1 / 0.05));
            if (alpha < 10) continue;

            int hue = int(themeBaseHue + hueDrift*72 + (i % 5)*20) % 360;
            QColor col = QColor::fromHsv(hue, 210, 240, alpha);
            if (i % 7 == 0) col = ThemeManager::instance().getSecondaryColor(); col.setAlpha(alpha);

            qreal w = 0.5 + (1.0 - d0) * 2.2;
            QPointF p0(C.x() + std::cos(angle)*r0, C.y() + std::sin(angle)*r0);
            QPointF p1(C.x() + std::cos(angle)*r1, C.y() + std::sin(angle)*r1);

            QColor glow = col; glow.setAlpha(alpha / 6);
            painter->setPen(QPen(glow, w + 3.5, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(p0, p1);
            painter->setPen(QPen(col, w, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(p0, p1);
        }
    }

    // ── 6. PULSING SINGULARITY CORE ──────────────────────────────────────────
    {
        struct { qreal r; int hOffset, s, v, a; } layers[] = {
            { 0.011, 0,  25, 255, 255 },
            { 0.026, -10, 200, 255, 210 },
            { 0.058, -25, 220, 210, 140 },
            { 0.110, 10, 200, 180, 65 },
        };
        painter->setPen(Qt::NoPen);
        for (auto& l : layers) {
            qreal lr = diagR * l.r * breath;
            QColor lc = QColor::fromHsv((themeBaseHue + l.hOffset + 360) % 360, l.s, l.v, int(l.a * breath));
            QRadialGradient g(C, lr);
            g.setColorAt(0.0, lc);
            g.setColorAt(1.0, Qt::transparent);
            painter->setBrush(g);
            painter->drawEllipse(C, lr, lr * 0.91);
        }
    }

    // ── 7. WORMHOLE IRIS RING ─────────────────────────────────────────────────
    {
        qreal irisR = diagR * 0.055 * breath + 1.5 * std::sin(t * 2.0);
        painter->setBrush(Qt::NoBrush);
        for (int g = 3; g >= 1; g--) {
            painter->setPen(QPen(QColor(ThemeManager::instance().getSecondaryColor().red(), ThemeManager::instance().getSecondaryColor().green(), ThemeManager::instance().getSecondaryColor().blue(), int(55 * breath / g)), 1.5 + g * 2.5));
            painter->drawEllipse(C, irisR, irisR * 0.90);
        }
        painter->setPen(QPen(QColor(ThemeManager::instance().getSecondaryColor().red(), ThemeManager::instance().getSecondaryColor().green(), ThemeManager::instance().getSecondaryColor().blue(), int(210 * breath)), 1.8));
        painter->drawEllipse(C, irisR, irisR * 0.90);
    }

    // ── 8. SOFT VIGNETTE (Gentle Edge Shading) ────────────────────────────────
    {
        QRadialGradient vig(C, std::max(W, H) * 0.70);
        vig.setColorAt(0.45, QColor(0, 0, 0, 0));
        vig.setColorAt(1.00, QColor(0, 0, 0, 120));
        painter->fillRect(vpRect, vig);
    }

    painter->restore();
    QGraphicsView::drawBackground(painter, QRectF());
}
