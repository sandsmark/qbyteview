#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(const QString &path);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent*) override;

    int m_buckets[256] = {0};
    QByteArray m_data;
    int m_highest;
    int m_entropy;
    QImage m_image;
    QString m_path;
    QString m_overlayText;
    QVector<int> m_xPos;
    QVector<int> m_yPos;
};

#endif // WIDGET_H
