#include "CustomScrollArea.h"
#include <QWheelEvent>

CustomScrollArea::CustomScrollArea(QWidget *parent)
	: QScrollArea(parent)
{
	ui.setupUi(this);
}

CustomScrollArea::~CustomScrollArea()
{
}

void CustomScrollArea::wheelEvent(QWheelEvent *ev)
{
	ev->accept();
	QWidget::wheelEvent(ev);
}

void CustomScrollArea::keyPressEvent(QKeyEvent *ev)
{
	ev->accept();
	QWidget::keyPressEvent(ev);
}

void CustomScrollArea::keyReleaseEvent(QKeyEvent *ev)
{
	ev->accept();
	QWidget::keyReleaseEvent(ev);
}