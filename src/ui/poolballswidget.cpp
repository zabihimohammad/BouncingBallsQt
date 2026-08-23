#include "poolballswidget.h"

#include <QRandomGenerator>
#include <QSurfaceFormat>
#include <QDebug>

namespace {

constexpr int kBallCount = 6;
constexpr int kTargetFPS = 60;

const char *kVertexShaderSrc = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aUnit;
uniform vec2 uCenterNDC;
uniform vec2 uRadiusNDC;
out vec2 vUV;
void main()
{
    vUV = aUnit;
    vec2 pos = uCenterNDC + aUnit * uRadiusNDC;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)GLSL";

const char *kFragmentShaderSrc = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 fragColor;

uniform vec3 uBaseColor;
uniform vec3 uLightDir;

void main()
{
    float r2 = dot(vUV, vUV);
    if (r2 > 1.0) discard;

    vec3 normal   = normalize(vec3(vUV, sqrt(max(1.0 - r2, 0.0))));
    vec3 lightDir = normalize(uLightDir);
    vec3 viewDir  = vec3(0.0, 0.0, 1.0);
    vec3 halfDir  = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfDir), 0.0), 48.0);
    float fresnel = pow(1.0 - normal.z, 2.5);

    vec3 color = uBaseColor * (0.28 + 0.72 * diff);
    color += vec3(1.0) * spec * 0.9;
    color = mix(color, vec3(0.65, 0.88, 1.0), fresnel * 0.35);

    float edge = smoothstep(1.0, 0.9, r2);
    fragColor = vec4(color, edge);
}
)GLSL";

} // namespace

PoolBallsWidget::PoolBallsWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setAlphaBufferSize(8);
    format.setSamples(4);
    format.setSwapInterval(1);
    setFormat(format);

    setAttribute(Qt::WA_NoSystemBackground);

    auto *rng = QRandomGenerator::global();
    static const QVector3D palette[] = {
        {0.95f, 0.25f, 0.25f}, {0.25f, 0.55f, 0.95f}, {0.98f, 0.82f, 0.20f},
        {0.30f, 0.85f, 0.45f}, {0.85f, 0.35f, 0.85f}, {1.00f, 0.55f, 0.15f},
    };

    m_balls.reserve(kBallCount);
    for (int i = 0; i < kBallCount; ++i)
    {
        Ball b;
        b.pos = QVector2D(float(rng->generateDouble()), float(rng->generateDouble()));
        b.velocity = QVector2D(float(rng->generateDouble()) - 0.5f,
                                float(rng->generateDouble()) - 0.5f) * 0.25f;
        b.radiusPx = 26.0f + float(rng->bounded(22));
        b.color = palette[i % 6];
        m_balls.append(b);
    }

    m_clock.start();
    connect(&m_frameTimer, &QTimer::timeout, this, [this] { update(); });
    m_frameTimer.start(1000 / kTargetFPS);
}

void PoolBallsWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // Solid base color -- this is the "pool floor" visible through parted water.
    glClearColor(0.02f, 0.09f, 0.15f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupShaders();
    setupGeometry();
    m_lastFrameMs = m_clock.elapsed();
}

void PoolBallsWidget::setupShaders()
{
    m_program = new QOpenGLShaderProgram(this);

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShaderSrc))
        qWarning() << "[Balls] vertex shader compile error:" << m_program->log();
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShaderSrc))
        qWarning() << "[Balls] fragment shader compile error:" << m_program->log();
    if (!m_program->link())
        qWarning() << "[Balls] shader link error:" << m_program->log();

    m_uCenterLoc   = m_program->uniformLocation("uCenterNDC");
    m_uRadiusLoc   = m_program->uniformLocation("uRadiusNDC");
    m_uColorLoc    = m_program->uniformLocation("uBaseColor");
    m_uLightDirLoc = m_program->uniformLocation("uLightDir");
}

void PoolBallsWidget::setupGeometry()
{
    static const GLfloat unitQuad[] = {
        -1.f, -1.f,
         1.f, -1.f,
        -1.f,  1.f,
         1.f,  1.f,
    };

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(unitQuad, sizeof(unitQuad));

    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    m_program->release();

    m_vbo.release();
    m_vao.release();
}

void PoolBallsWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void PoolBallsWidget::stepSimulation(float dt)
{
    for (auto &b : m_balls)
    {
        b.pos += b.velocity * dt;

        if (b.pos.x() < 0.0f || b.pos.x() > 1.0f) b.velocity.setX(-b.velocity.x());
        if (b.pos.y() < 0.0f || b.pos.y() > 1.0f) b.velocity.setY(-b.velocity.y());

        b.pos.setX(qBound(0.0f, b.pos.x(), 1.0f));
        b.pos.setY(qBound(0.0f, b.pos.y(), 1.0f));
    }
}

void PoolBallsWidget::paintGL()
{
    const qint64 now = m_clock.elapsed();
    const float dt = qMax(0.0f, float(now - m_lastFrameMs) / 1000.0f);
    m_lastFrameMs = now;

    stepSimulation(dt);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();
    m_vao.bind();
    m_program->setUniformValue(m_uLightDirLoc, QVector3D(-0.4f, 0.6f, 0.7f));

    for (const auto &b : m_balls)
    {
        QVector2D centerNDC(b.pos.x() * 2.0f - 1.0f, b.pos.y() * 2.0f - 1.0f);
        float rx = (b.radiusPx / float(qMax(1, width())))  * 2.0f;
        float ry = (b.radiusPx / float(qMax(1, height()))) * 2.0f;

        m_program->setUniformValue(m_uCenterLoc, centerNDC);
        m_program->setUniformValue(m_uRadiusLoc, QVector2D(rx, ry));
        m_program->setUniformValue(m_uColorLoc, b.color);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    m_vao.release();
    m_program->release();
}
