#pragma once

#include <QPushButton>
#include "ui_CloseBtn.h"

class CCloseBtn : public QPushButton
{
	Q_OBJECT

public:
	CCloseBtn(QWidget *parent = Q_NULLPTR);
	void setColor(QColor color);
	~CCloseBtn();

protected:
	void paintEvent(QPaintEvent *ev);

private:
	Ui::CCloseBtn ui;
	QColor m_color;
};
