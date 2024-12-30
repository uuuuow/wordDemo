#include "PlayerSlider.h"
#include <QMouseEvent>
CPlayerSlider::CPlayerSlider(QWidget *parent)
	: QSlider(parent)
{
	
}

CPlayerSlider::~CPlayerSlider()
{
}

void CPlayerSlider::mousePressEvent(QMouseEvent *e)
{
	int value = ((float)e->pos().x() / (float)this->width())*(this->maximum() + 1);
	this->setValue(value);
	e->ignore();
}