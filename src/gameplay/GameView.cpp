#include "GameView.h"
#include <QPainter>

GameView::GameView(QWidget* parent) : QGraphicsView(parent) {
    // حذف اسکرول‌بارهای افقی و عمودی
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // بهینه‌سازی رندرر با تکنیک‌های ضدپلگی و نرم‌سازی
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    // حذف کادر پیش‌فرض خاکستری ویجت برای یکپارچگی ظاهری با تم تاریک
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background: transparent; border: none;");

    // فعال‌سازی رهگیری بلادرنگ ماوس (بسیار مهم برای چرخش پیوسته توپ و رسم خط نشانه بدون نگه داشتن کلیک)
    setMouseTracking(true);

    // دریافت فوکوس کیبورد برای عملکرد دکمه Space (جابجایی گلوله) و Escape (منوی توقف)
    setFocusPolicy(Qt::StrongFocus);

    // حالت به‌روزرسانی هوشمند فریم‌ها برای جلوگیری از افت فریم
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void GameView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (scene()) {
        // فیت نگه‌داشتن همیشگی مختصات صحنه ۳۵۲x۶۰۰ با حفظ نسبت ابعاد در تغییر سایز یا تمام‌صفحه
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }
}