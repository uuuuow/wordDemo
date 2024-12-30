#pragma once

#include <QDialog>
#include "ui_PhotoDialog.h"

class CPhotoDialog : public QDialog
{
	Q_OBJECT

public:
	CPhotoDialog(QString title, QString path, QWidget *parent = Q_NULLPTR);
	~CPhotoDialog();

	QImage m_photo;

private:
	Ui::CPhotoDialog ui;
};
