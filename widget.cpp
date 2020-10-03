#include "widget.h"
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <qmath.h>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QFileInfo>

Widget::Widget(const QString &path) :
    m_path(QFileInfo(path).fileName())
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


    m_overlayText = m_path;
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

static int xy2d (unsigned count, unsigned x, unsigned y) {
    unsigned index=0;
    for (unsigned s=count/2; s>0; s/=2) {
        unsigned rx = (x & s) > 0;
        unsigned ry = (y & s) > 0;
        index += s * s * ((3 * rx) ^ ry);
        rot(count, &x, &y, rx, ry);
    }
    return index;
}

static void d2xy(unsigned count, unsigned index, unsigned *x, unsigned *y) {
    *x = *y = 0;
    for (unsigned s=1; s<count; s*=2) {
        unsigned rx = 1 & (index/2);
        unsigned ry = 1 & (index ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        index /= 4;
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    if (m_image.size() != size()) {
        m_image = QImage(size(), QImage::Format_ARGB32);
        QPainter painter(&m_image);
        // Size of each side
        const qreal side = sqrt(m_data.size());
        // Up to the next power of two
        const unsigned n = pow(2, ceil(log2(side)));
        const qreal blockwidth = qreal(width()) / qCeil(n);
        const qreal blockheight = qreal(height()) / qCeil(n);
        QElapsedTimer timer; timer.start();
        if (blockwidth < 1 || blockheight < 1) {
            painter.setOpacity(qMin(blockwidth, blockheight));
        }
        for (int index=0; index<m_data.length(); index++) {
            const unsigned char byte = m_data[index];
            unsigned tx, ty;
            d2xy(n, index, &tx, &ty);
            painter.fillRect(QRectF(tx * blockwidth, ty * blockheight, blockwidth + 1, blockheight + 1), QColor::fromHsv(byte, m_entropy, m_buckets[byte]));
        }
        qDebug() << "repaint in" << timer.elapsed() << "ms";
    }

    QPainter painter(this);
    painter.drawImage(0, 0, m_image);

    painter.setPen(QPen(QColor(0, 0, 0, 128)));
    QFont fnt = font();
    fnt.setPixelSize(15);
    painter.setFont(fnt);

    QRect bounding = painter.fontMetrics().boundingRect(rect(), Qt::AlignRight, m_overlayText);
    bounding.moveRight(width() - 10);
    bounding.moveTop(0);
    painter.fillRect(bounding.marginsAdded(QMargins(10, 10, 10, 10)), QColor(255, 255, 255, 128));
    painter.drawText(bounding, Qt::AlignRight, m_overlayText);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    // Size of each side
    const qreal side = sqrt(m_data.size());
    // Up to the next power of two
    const unsigned n = pow(2, ceil(log2(side)));
    const qreal blockwidth = qreal(width()) / n;
    const qreal blockheight = qreal(height()) / n;
    const unsigned d = xy2d(n, qRound(event->x() / blockwidth), qRound(event->y() / blockheight));
    m_overlayText = m_path + '\n' +
        QString::number(event->x()) + ',' + QString::number(event->y()) + '\n' +
        QString::number(d) + + " / " + QString::number(m_data.size());;

    if (d < m_data.size()) {
        uint8_t val = uint8_t(m_data[d]);
        m_overlayText += "\n0x" + QString::number(val, 16) + " (" + QString::number(m_buckets[val]) + ')';
    }
    update();

    // For debugging:
    //unsigned tx, ty;
    //d2xy(n, d, &tx, &ty);
    //qDebug() << "back x\t" << qRound((qreal)tx) * (blockwidth);
    //qDebug() <<"event x\t" << blockwidth * int(event->x()/blockwidth);
}
