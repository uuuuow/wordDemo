#include "LogBrowser.h"
#include <QKeyEvent>

CLogBrowser::CLogBrowser(QWidget *parent)
	: QTextBrowser(parent)
{}

CLogBrowser::~CLogBrowser()
{}

void CLogBrowser::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Up || 
        event->key() == Qt::Key_Down || 
        event->key() == Qt::Key_Right || 
        event->key() == Qt::Key_Left)
    {
        event->ignore();
        return;
    }

    QWidget::keyPressEvent(event);
}