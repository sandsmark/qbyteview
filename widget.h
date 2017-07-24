#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *);

    int m_buckets[256] = {0};
    QByteArray m_data;
    int m_highest;
    int m_entropy;
    QImage m_image;
};

#endif // WIDGET_H
