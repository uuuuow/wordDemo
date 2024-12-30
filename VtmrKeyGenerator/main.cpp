#include "VtmrKeyGenerator.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VtmrKeyGenerator w;
    
    w.show();
    return a.exec();
}
