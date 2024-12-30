#include "InputDialog.h"
#include <QVBoxLayout>
#include <InputWidget.h>
#include <QPushButton>
#include <QComboBox>
#include "Global.h"
#include <QLabel>
#include <QTimeEdit>
#include "OCRThread.h"
#include "Utils.h"
#include <QMessageBox>
#include <QCloseEvent>
#include "TipsDialog.h"

CInputDialog::CInputDialog(int type, VTTimeMode timeMode, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	if (0 == type)
		setWindowTitle(QString("参照物长度输入"));
	else if (1 == type)
		setWindowTitle(QString("参照物坐标输入"));
}

CInputDialog::~CInputDialog()
{
}

void CInputDialog::setWidget(QWidget * w)
{
	auto vLayout = new QVBoxLayout(this);
	vLayout->addWidget(w);
	
	auto hLayout = new QHBoxLayout(this);

	auto btnOK = new QPushButton(QString("确定"),this);
	auto btnCancel = new QPushButton(QString("取消"),this);

	hLayout->addSpacerItem(new QSpacerItem(0, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
	hLayout->addWidget(btnOK);
	hLayout->addWidget(btnCancel);

	vLayout->addSpacerItem(new QSpacerItem(1, 50, QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));

	auto splitLine = new QPushButton("", this);
	splitLine->setFixedHeight(1);
	vLayout->addWidget(splitLine);

	vLayout->addLayout(hLayout);
	setLayout(vLayout);
	
	resize(w->width(), w->height() + btnOK->height());
	connect(btnOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(btnCancel, &QPushButton::clicked,
		[=]() {
		QDialog::reject();
	});
}

void CInputDialog::onOK()
{
	QDialog::accept();
}