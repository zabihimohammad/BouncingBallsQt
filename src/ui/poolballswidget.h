#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QVector2D>
#include <QVector3D>
#include <QVector>

/**
 * PoolBallsWidget
 * ---------------
 * Renders a handful of "3D" balls using the classic sphere-impostor trick:
 * each ball is a single flat quad, and the fragment shader reconstructs a
 * sphere normal analytically from the quad's local UV. No 3D geometry, no
 * mesh, fully lit and antialiased, and cheap enough for dozens of balls.
 *
 * This widget sits *underneath* WaterBackgroundWidget in the z-order, so
 * it becomes visible through the water wherever the water is "parted".
 */
class PoolBallsWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit PoolBallsWidget(QWidget *parent = nullptr);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    struct Ball
    {
        QVector2D pos;      // normalized position, 0..1
        QVector2D velocity; // normalized units / second
        float radiusPx = 30.0f;
        QVector3D color;
    };

    void setupShaders();
    void setupGeometry();
    void stepSimulation(float dt);

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;

    QElapsedTimer m_clock;
    QTimer m_frameTimer;
    qint64 m_lastFrameMs = 0;

    QVector<Ball> m_balls;

    int m_uCenterLoc = -1;
    int m_uRadiusLoc = -1;
    int m_uColorLoc = -1;
    int m_uLightDirLoc = -1;
};
