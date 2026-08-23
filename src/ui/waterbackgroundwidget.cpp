#include "waterbackgroundwidget.h"

#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QDebug>
#include <QtMath>

namespace {

constexpr int kTargetFPS = 60;

// Embedded so the widget has zero external resource dependencies.
// Identical copies live in shaders/water.vert and shaders/water.frag for
// easy editing in a real shader IDE -- keep both in sync if you change one.

const char *kVertexShaderSrc = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vUV;
void main()
{
    vUV = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)GLSL";

const char *kFragmentShaderSrc = R"GLSL(
#version 330 core

in vec2 vUV;
out vec4 fragColor;

uniform float uTime;
uniform vec2  uResolution;

uniform vec2  uMouse;
uniform vec2  uMouseVelocity;

#define MAX_TRAIL 24
uniform vec4  uTrail[MAX_TRAIL];
uniform int   uTrailCount;

const vec3 DEEP_COLOR    = vec3(0.02, 0.10, 0.20);
const vec3 SHALLOW_COLOR = vec3(0.05, 0.34, 0.55);
const vec3 CAUSTIC_COLOR = vec3(0.65, 0.95, 1.0);
const vec3 FOAM_COLOR    = vec3(0.85, 0.98, 1.0);

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

float voronoi(vec2 uv)
{
    vec2 ip = floor(uv);
    vec2 fp = fract(uv);
    float minDist = 8.0;

    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++)
    {
        vec2 neighbor = vec2(float(x), float(y));
        vec2 jitter = hash2(ip + neighbor);
        jitter = 0.5 + 0.45 * sin(uTime * 0.35 + 6.2831853 * jitter);
        vec2 diff = neighbor + jitter - fp;
        minDist = min(minDist, dot(diff, diff));
    }
    return sqrt(minDist);
}

float causticLayer(vec2 uv, float scale, float speed, float warp)
{
    vec2 p = uv * scale;
    p += vec2(uTime * speed, -uTime * speed * 0.6);
    p += warp * vec2(sin(p.y * 1.3 + uTime * 0.5), cos(p.x * 1.3 - uTime * 0.4));

    float d = voronoi(p);
    float cell = 1.0 - smoothstep(0.0, 0.35, d);
    return pow(cell, 3.0);
}

float segmentDist(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / max(dot(ba, ba), 1e-5), 0.0, 1.0);
    return length(pa - ba * h);
}

void main()
{
    vec2 uv = vUV;
    float aspect = uResolution.x / max(uResolution.y, 1.0);
    vec2 auv = vec2(uv.x * aspect, uv.y);

    float partDist = 1e6;
    float wakeAge   = 1e6;
    vec2 prevPoint = auv;

    for (int i = 0; i < uTrailCount; i++)
    {
        vec4 s = uTrail[i];
        vec2 p = vec2(s.x * aspect, s.y);
        if (i > 0)
        {
            float d = segmentDist(auv, prevPoint, p);
            if (d < partDist) { partDist = d; wakeAge = s.z; }
        }
        prevPoint = p;
    }

    vec2 mouseAUV = vec2(uMouse.x * aspect, uMouse.y);
    vec2 rel = auv - mouseAUV;
    float speed = length(uMouseVelocity);
    vec2 velDir = speed > 1e-4 ? normalize(uMouseVelocity) : vec2(1.0, 0.0);

    float along  = dot(rel, vec2(velDir.x * aspect, velDir.y));
    float across = dot(rel, vec2(-velDir.y * aspect, velDir.x));
    float stretch = 1.0 + clamp(speed * 6.0, 0.0, 2.5);
    float cursorDist = length(vec2(along / stretch, across));

    if (cursorDist < partDist) { partDist = cursorDist; wakeAge = 0.0; }

    float maxRadius = 0.045 + 0.05 * clamp(speed * 4.0, 0.0, 1.0);
    float closeTime = 0.9;
    float radius = (uTrailCount > 0)
        ? maxRadius * (1.0 - clamp(wakeAge / closeTime, 0.0, 1.0))
        : 0.0;

    float parting = (radius > 1e-4) ? smoothstep(radius * 0.35, radius, partDist) : 1.0;

    vec2 pushDir = (length(rel) > 1e-4) ? normalize(rel) : vec2(0.0);
    float pushAmount = (1.0 - parting) * 0.05;
    vec2 displacedUV = uv + vec2(pushDir.x / aspect, pushDir.y) * pushAmount;

    float c1 = causticLayer(displacedUV, 6.0, 0.06, 0.35);
    float c2 = causticLayer(displacedUV * 1.7 + 5.2, 10.0, -0.09, 0.5);
    float caustics = clamp(c1 * 0.7 + c2 * 0.5, 0.0, 1.0);

    float h = sin(displacedUV.x * 12.0 + uTime * 0.8) * 0.02
            + sin(displacedUV.y * 9.0  - uTime * 0.6) * 0.02;
    float depthMix = clamp(0.5 + h * 4.0, 0.0, 1.0);

    vec3 water = mix(DEEP_COLOR, SHALLOW_COLOR, depthMix);
    water += CAUSTIC_COLOR * caustics * 0.55;

    float rim = smoothstep(radius, radius * 0.6, partDist)
              - smoothstep(radius * 0.6, radius * 0.2, partDist);
    water += FOAM_COLOR * clamp(rim, 0.0, 1.0) * 0.8;

    float alpha = parting;
    fragColor = vec4(water * alpha, alpha);
}
)GLSL";

} // namespace

WaterBackgroundWidget::WaterBackgroundWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // Alpha buffer is mandatory: this is what lets the widget report a
    // real alpha channel that Qt composites against whatever is stacked
    // underneath it (rather than always being opaque).
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setAlphaBufferSize(8);
    format.setSamples(4);       // MSAA smooths the caustic cell edges
    format.setSwapInterval(1);  // vsync -> effectively caps us at the display refresh
    setFormat(format);

    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true); // so we get move events without a button held

    m_clock.start();
    m_mouseDeltaTimer.start();

    connect(&m_frameTimer, &QTimer::timeout, this, [this] { update(); });
    m_frameTimer.start(1000 / kTargetFPS);
}

WaterBackgroundWidget::~WaterBackgroundWidget()
{
    makeCurrent();
    delete m_program;
    m_vbo.destroy();
    m_vao.destroy();
    doneCurrent();
}

void WaterBackgroundWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND);
    // The fragment shader outputs premultiplied color, so this is the
    // correct blend func to composite cleanly against the widget below us.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    setupShaders();
    setupGeometry();
}

void WaterBackgroundWidget::setupShaders()
{
    m_program = new QOpenGLShaderProgram(this);

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShaderSrc))
        qWarning() << "[Water] vertex shader compile error:" << m_program->log();

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShaderSrc))
        qWarning() << "[Water] fragment shader compile error:" << m_program->log();

    if (!m_program->link())
        qWarning() << "[Water] shader link error:" << m_program->log();

    m_uTimeLoc         = m_program->uniformLocation("uTime");
    m_uResolutionLoc    = m_program->uniformLocation("uResolution");
    m_uMouseLoc         = m_program->uniformLocation("uMouse");
    m_uMouseVelocityLoc = m_program->uniformLocation("uMouseVelocity");
    m_uTrailLoc         = m_program->uniformLocation("uTrail[0]");
    m_uTrailCountLoc    = m_program->uniformLocation("uTrailCount");
}

void WaterBackgroundWidget::setupGeometry()
{
    // Fullscreen quad as a triangle strip: (x, y, u, v) per vertex.
    static const GLfloat quad[] = {
        -1.f, -1.f,  0.f, 0.f,
         1.f, -1.f,  1.f, 0.f,
        -1.f,  1.f,  0.f, 1.f,
         1.f,  1.f,  1.f, 1.f,
    };

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(quad, sizeof(quad));

    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));
    m_program->release();

    m_vbo.release();
    m_vao.release();
}

void WaterBackgroundWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

QVector2D WaterBackgroundWidget::toUV(const QPointF &widgetPos) const
{
    float x = float(widgetPos.x() / qMax(1, width()));
    float y = 1.0f - float(widgetPos.y() / qMax(1, height())); // GL UV is y-up
    return QVector2D(x, y);
}

void WaterBackgroundWidget::pushTrailPoint(const QPointF &widgetPos)
{
    const float now = m_clock.elapsed() / 1000.0f;
    QVector2D uv = toUV(widgetPos);
    float speed = 0.0f;

    if (m_hasLastMouse)
    {
        QVector2D prevUV = toUV(m_lastMousePos);
        float dt = qMax(0.0001f, m_mouseDeltaTimer.restart() / 1000.0f);
        speed = (uv - prevUV).length() / dt;
    }
    else
    {
        m_mouseDeltaTimer.restart();
    }

    m_currentMouseUV = uv;
    // Smooth the velocity slightly so tiny per-event jitter doesn't make
    // the elongated cursor blob flicker in direction.
    QVector2D instVelocity = m_hasLastMouse
        ? (uv - toUV(m_lastMousePos)) * (1.0f / qMax(0.0001f, m_mouseDeltaTimer.elapsed() / 1000.0f))
        : QVector2D(0, 0);
    m_currentVelocity = m_currentVelocity * 0.6f + instVelocity * 0.4f;

    m_trail.push_back({uv, now, speed});
    while (m_trail.size() > kMaxTrail)
        m_trail.pop_front();

    m_lastMousePos = widgetPos;
    m_hasLastMouse = true;
}

void WaterBackgroundWidget::mouseMoveEvent(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    pushTrailPoint(event->position());
#else
    pushTrailPoint(event->localPos());
#endif
    QOpenGLWidget::mouseMoveEvent(event);
}

void WaterBackgroundWidget::leaveEvent(QEvent *event)
{
    m_hasLastMouse = false;
    m_currentVelocity = QVector2D(0.0f, 0.0f);
    QOpenGLWidget::leaveEvent(event);
}

void WaterBackgroundWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();
    m_vao.bind();

    const float t = m_clock.elapsed() / 1000.0f;
    m_program->setUniformValue(m_uTimeLoc, t);
    m_program->setUniformValue(m_uResolutionLoc, QVector2D(float(width()), float(height())));
    m_program->setUniformValue(m_uMouseLoc, m_currentMouseUV);
    m_program->setUniformValue(m_uMouseVelocityLoc, m_currentVelocity);

    // Decay velocity smoothly when the mouse has stopped moving so the
    // elongated "swipe" shape relaxes back into a circle instead of
    // freezing mid-stretch.
    m_currentVelocity *= 0.90f;

    // Drop trail samples older than the shader's close time so the wake
    // fully heals and doesn't leave the array full of stale points.
    while (!m_trail.empty() && (t - m_trail.front().capturedAt) > 1.2f)
        m_trail.pop_front();

    QVector<QVector4D> trailData;
    trailData.reserve(kMaxTrail);
    for (const auto &tp : m_trail)
        trailData.append(QVector4D(tp.uv.x(), tp.uv.y(), t - tp.capturedAt, tp.speed));
    while (trailData.size() < kMaxTrail)
        trailData.append(QVector4D(0.0f, 0.0f, 999.0f, 0.0f));

    m_program->setUniformValueArray(m_uTrailLoc, trailData.constData(), trailData.size());
    m_program->setUniformValue(m_uTrailCountLoc, int(m_trail.size()));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
    m_program->release();
}
