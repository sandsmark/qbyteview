#include "widget.h"
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <qmath.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    //QFile file("/home/sandsmark/xo/crashfuck/patrick/09e96bb1-182d-4dce-944e-dcbdc3fd36dc.lines");
    //QFile file("/home/sandsmark/Downloads/eames-20121011205606-base-base.dfu");
    QFile file("/home/sandsmark/Downloads/u-boot.bin");
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
    for (unsigned char b=0; b < 255; b++) {
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
    qDebug() << entropy << m_entropy;


}

Widget::~Widget()
{

}


//rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int rx, int ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }

        //Swap x and y
        int t  = *x;
        *x = *y;
        *y = t;
    }
}

void d2xy(int n, int d, int *x, int *y) {
    int rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    const qreal n = qSqrt(qCeil(qSqrt(qreal(m_data.length()))) * qCeil(qreal(qSqrt(qreal(m_data.length())))));
    const qreal blockwidth = qreal(width()) / qCeil(n);
    const qreal blockheight = qreal(height()) / qCeil(n);
    qDebug() << blockwidth << blockheight << m_data.length() << width() << height() << n;
    for (int bc=0; bc<m_data.length(); bc++) {
            const unsigned char c = m_data[bc];
            int tx, ty;
            d2xy(ceil(n), bc, &tx, &ty);
            painter.fillRect(tx * blockwidth, ty * blockheight, blockwidth, blockheight, QColor::fromHsv(c, m_entropy, m_buckets[c] * 127 / m_highest + 128));
    }
    QPen pen;
    pen.setWidth(5);
    pen.setColor(Qt::blue);
    painter.setPen(pen);
    painter.drawRect(QRect(0, 0, n * blockwidth, n * blockheight));
}
