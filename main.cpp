#include "widget.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc < 2) {
        qWarning() << "pass file";
        return 1;
    }
    Widget w(argv[1]);
    w.show();

    return a.exec();
}
