#include "widget.h"
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <qmath.h>
#include <QElapsedTimer>

Widget::Widget(const QString &path) :
    m_path(path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open file";
        return;
    }
    m_data = file.readAll();
    for (unsigned char byte : m_data) {
        m_buckets[byte]++;
    }
    float entropy = 0;

    m_highest = 0;
    for (int b=0; b < 256; b++) {
        if (!m_buckets[b]) {
            continue;
        }
        if (m_buckets[b] > m_highest) {
            m_highest = m_buckets[b];
        }

        float p = float(m_buckets[b]) / m_data.length();
        entropy -= p * log(p) / log(256);
    }
    m_entropy = entropy * 255;

    for (int b=0; b < 256; b++) {
        if (!m_buckets[b]) {
            continue;
        }
        m_buckets[b] = m_buckets[b] * 127 / m_highest + 128;
    }
    qDebug() << entropy << m_entropy;


}

Widget::~Widget()
{

}


//rotate/flip a quadrant appropriately
static void rot(const unsigned n, unsigned *x, unsigned *y, const unsigned rx, const unsigned ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }

        std::swap(*x, *y);
    }
}

static void d2xy(unsigned count, unsigned index, unsigned *x, unsigned *y) {
    unsigned rx, ry, s;
    *x = *y = 0;
    for (s=1; s<count; s*=2) {
        rx = 1 & (index/2);
        ry = 1 & (index ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        index /= 4;
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    const qreal n = qSqrt(qCeil(qSqrt(qreal(m_data.length()))) * qCeil(qreal(qSqrt(qreal(m_data.length())))));
    const qreal blockwidth = qreal(width()) / qCeil(n);
    const qreal blockheight = qreal(height()) / qCeil(n);
    qDebug() << blockwidth << blockheight << m_data.length() << width() << height() << n;
    QElapsedTimer timer; timer.start();
    for (int index=0; index<m_data.length(); index++) {
        const unsigned char byte = m_data[index];
        unsigned tx, ty;
        d2xy(ceil(n), index, &tx, &ty);
        painter.fillRect(QRectF(tx * blockwidth, ty * blockheight, blockwidth + 1, blockheight + 1), QColor::fromHsv(byte, m_entropy, m_buckets[byte]));
    }
    qDebug() << timer.elapsed() << "ms";
    painter.setPen(Qt::black);
    QFont fnt = font();
    fnt.setPixelSize(30);
    painter.setFont(fnt);
    QRect bounding;
    painter.drawText(rect(),Qt::AlignLeft, m_path, &bounding);
    painter.fillRect(bounding, QColor(255, 255, 255, 128));
    painter.drawText(rect(),Qt::AlignLeft, m_path, &bounding);
}
