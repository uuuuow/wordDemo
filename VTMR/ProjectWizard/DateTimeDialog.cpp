#include "DateTimeDialog.h"
#include <QDebug>
#include <QTextCharFormat>
CDateTimeDialog::CDateTimeDialog(QDateTime datetime, bool show, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.calendarWidget->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
	ui.calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
	ui.calendarWidget->setSelectionMode(QCalendarWidget::SingleSelection);

	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	ui.timeEdit->setTime(datetime.time());
	if (!show)
	{
		ui.timeEdit->hide();
	}
}

CDateTimeDialog::~CDateTimeDialog()
{
	disconnect(ui.btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	disconnect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


void CDateTimeDialog::accept()
{
	auto date= ui.calendarWidget->selectedDate();
	auto datestr = date.toString("yyyy-MM-dd");
	auto time = ui.timeEdit->time();
	auto timestr = time.toString("hh:mm:ss");
	auto datetimestr = QString("%1 %2").arg(datestr).arg(timestr);
	auto dateTime = QDateTime::fromString(datetimestr, "yyyy-MM-dd hh:mm:ss");
	emit sigSelectedDate(dateTime);
	QDialog::accept();
}