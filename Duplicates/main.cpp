#include "duplicates.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Duplicates w;
    w.show();

    return a.exec();
}
