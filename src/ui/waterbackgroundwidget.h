#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QVector2D>
#include <QVector4D>
#include <deque>

/**
 * WaterBackgroundWidget
 * ---------------------
 * A fully GPU-driven, transparent-capable QOpenGLWidget that renders an
 * animated pool-water surface (procedural Voronoi caustics) and reacts to
 * the mouse by "parting" the water -- the area under the cursor's recent
 * path becomes transparent, revealing whatever is stacked underneath it
 * in the widget hierarchy (e.g. a PoolBallsWidget).
 *
 * Requires an alpha-enabled QSurfaceFormat (set in the constructor) and,
 * on the *parent* side, correct z-ordering -- see MainMenuWidget for the
 * Qt::WA_AlwaysStackOnTop note on the button overlay.
 */
class WaterBackgroundWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit WaterBackgroundWidget(QWidget *parent = nullptr);
    ~WaterBackgroundWidget() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct TrailPoint
    {
        QVector2D uv;      // cursor position in UV space (0..1, y-up) at capture time
        float capturedAt;  // seconds (from m_clock) when this sample was taken
        float speed;       // instantaneous speed at capture, UV units / second
    };

    void setupGeometry();
    void setupShaders();
    void pushTrailPoint(const QPointF &widgetPos);
    QVector2D toUV(const QPointF &widgetPos) const;

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;

    QElapsedTimer m_clock;          // master animation clock (uTime)
    QElapsedTimer m_mouseDeltaTimer; // used to compute instantaneous mouse speed
    QTimer m_frameTimer;             // drives ~60 FPS repaint

    QPointF m_lastMousePos;
    bool m_hasLastMouse = false;

    QVector2D m_currentMouseUV{0.5f, 0.5f};
    QVector2D m_currentVelocity{0.0f, 0.0f};

    static constexpr int kMaxTrail = 24;
    std::deque<TrailPoint> m_trail;

    int m_uTimeLoc = -1;
    int m_uResolutionLoc = -1;
    int m_uMouseLoc = -1;
    int m_uMouseVelocityLoc = -1;
    int m_uTrailLoc = -1;
    int m_uTrailCountLoc = -1;
};
