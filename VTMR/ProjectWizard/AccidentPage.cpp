#include "AccidentPage.h"
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include "DateTimeDialog.h"
#include <QCompleter>

CAccidentPage::CAccidentPage(ProjectWizardType type, QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(QString("事故基本信息录入"));
	m_proWizardType = type;

	m_lbAccidentTime = new QLabel(QString("&事故时间"), this);
	m_leAccidentTime = new QLineEdit(this);
	m_btnAccidentTime = new QPushButton("...", this);

	m_lbAccidentLocation = new QLabel(QString("&事故地点"), this);
	m_teAccidentLocation = new QTextEdit(this);
	m_teAccidentLocation->setMaximumHeight(60);

	m_lbVehiclePlateNumber = new QLabel(QString("&车辆号牌"), this);
	m_leVehiclePlateNumber = new QLineEdit(this);

	m_lbVehicleBrand = new QLabel(QString("&车辆品牌"), this);
	m_leVehicleBrand = new QLineEdit(this);

	m_lbVehicleType = new QLabel(QString("&车辆类型"), this);
	m_cbVehicleType = new QComboBox(this);

	m_lbVehicleVIN = new QLabel(QString("&车辆识别代码"), this);
	m_leVehicleVIN = new QLineEdit(this);

	m_vLayout = new QVBoxLayout;


	//事故时间
	m_lbAccidentTime->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_leAccidentTime->setEnabled(false);
	m_lbAccidentTime->setBuddy(m_leAccidentTime);
	auto datetime = QDateTime::currentDateTime();
	auto time = datetime.toString("yyyy-MM-dd hh:mm:ss");
	m_leAccidentTime->setText(time);

	connect(m_btnAccidentTime, &QPushButton::clicked,
		[=]() {
		CDateTimeDialog *pDateTimeDialog = new CDateTimeDialog(datetime, true, m_leAccidentTime);
		connect(pDateTimeDialog, &CDateTimeDialog::sigSelectedDate,
			[=](QDateTime dateTime) {
			QString time = dateTime.toString("yyyy-MM-dd hh:mm:ss");
			m_leAccidentTime->setText(time);
		});
		pDateTimeDialog->exec();
	});
	//事故地点
	m_lbAccidentLocation->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbAccidentLocation->setBuddy(m_teAccidentLocation);
	//车辆号牌
	m_leVehiclePlateNumber->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbVehiclePlateNumber->setBuddy(m_leVehiclePlateNumber);
	//车辆品牌
	m_lbVehicleBrand->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbVehicleBrand->setBuddy(m_leVehicleBrand);
	//车辆类型
	m_lbVehicleType->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_cbVehicleType->setEditable(true);
	QStringList strings;
	strings << QString("小型轿车") 
		<< QString("小型普通客车") 
		<< QString("电驱动两轮车") 
		<< QString("自行车") 
		<< QString("重型自卸货车");
	auto completer = new QCompleter(strings, this);
	completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	m_cbVehicleType->setCompleter(completer);

	m_cbVehicleType->setInsertPolicy(QComboBox::InsertAtTop);
	m_cbVehicleType->addItem(QString("小型轿车"));
	m_cbVehicleType->addItem(QString("小型普通客车"));
	m_cbVehicleType->addItem(QString("电驱动两轮车"));
	m_cbVehicleType->addItem(QString("自行车"));
	m_cbVehicleType->addItem(QString("重型自卸货车"));
	m_lbVehicleType->setBuddy(m_cbVehicleType);

	//车辆识别代码VIN
	m_lbVehicleVIN->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbVehicleVIN->setBuddy(m_leVehicleVIN);

	QRegularExpression regex("[A-Za-z0-9]*");
	QRegularExpressionValidator* validator = new QRegularExpressionValidator(regex, this);
	m_leVehicleVIN->setValidator(validator);

	m_vLayout->setContentsMargins(10, 10, 10, 10);
	m_vLayout->setSpacing(10);

	QHBoxLayout *hLayout;
	//事故时间
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbAccidentTime);
	hLayout->addWidget(m_leAccidentTime);
	hLayout->addWidget(m_btnAccidentTime);
	m_vLayout->addItem(hLayout);
	//事故地点
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbAccidentLocation);
	hLayout->addWidget(m_teAccidentLocation);
	m_vLayout->addItem(hLayout);
	
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	//车辆类型
	hLayout->addWidget(m_lbVehicleType);
	hLayout->addWidget(m_cbVehicleType);
	//车辆品牌
	hLayout->addWidget(m_lbVehicleBrand);
	hLayout->addWidget(m_leVehicleBrand);
	//车辆号牌
	hLayout->addWidget(m_lbVehiclePlateNumber);
	hLayout->addWidget(m_leVehiclePlateNumber);
	m_vLayout->addItem(hLayout);
	//车辆识别代码VIN
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbVehicleVIN);
	hLayout->addWidget(m_leVehicleVIN);
	m_vLayout->addItem(hLayout);

	auto spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_vLayout->addItem(spacer);
	setLayout(m_vLayout);
}

void CAccidentPage::initializePage()
{

}

void CAccidentPage::setEnabled(bool enable)
{
	m_btnAccidentTime->setEnabled(enable);
	m_teAccidentLocation->setEnabled(enable);
	m_leVehiclePlateNumber->setEnabled(enable);
	m_leVehicleBrand->setEnabled(enable);
	m_cbVehicleType->setEnabled(enable);
	m_leVehicleVIN->setEnabled(enable);
	m_cbVehicleType->setEnabled(enable);
}


void CAccidentPage::getValue(VTProjectInfo & proInfo)
{
	proInfo.accidentTime = m_leAccidentTime->text();
	proInfo.accidentLocation = m_teAccidentLocation->toPlainText();
	proInfo.licensePlateNumber = m_leVehiclePlateNumber->text();
	proInfo.vehicleBrand = m_leVehicleBrand->text();
	proInfo.vehicleType = m_cbVehicleType->currentText();
	proInfo.vehicleVIN = m_leVehicleVIN->text();
}

void CAccidentPage::setValue(VTProjectInfo & proInfo)
{
	m_leAccidentTime->setText(proInfo.accidentTime);
	m_teAccidentLocation->setPlainText(proInfo.accidentLocation);
	m_leVehiclePlateNumber->setText(proInfo.licensePlateNumber);
	m_leVehicleBrand->setText(proInfo.vehicleBrand);
	m_cbVehicleType->setCurrentText(proInfo.vehicleType);
	m_leVehicleVIN->setText(proInfo.vehicleVIN);
}

void CAccidentPage::getValue(QString &licensePlateNumber, QString &vehicleBrand, QString &vehicleType)
{
	licensePlateNumber = m_leVehiclePlateNumber->text();
	vehicleBrand = m_leVehicleBrand->text();
	vehicleType = m_cbVehicleType->currentText();
}

bool CAccidentPage::validatePage()
{
	int vinSize = 17;
	QString vehicleVIN = m_leVehicleVIN->text();
	QString vehicleType = m_cbVehicleType->currentText();
	if (vehicleVIN.length() <= 0)
	{
		VTMessageBoxOk(QString("车辆识别代码长度不能为0"), windowFlags(), QMessageBox::Information, this);
		return false;
	}
	//if (vehicleType == QString("电驱动两轮车"))
	//	vinSize = 15;
	//if (vehicleVIN.length() > 0 && vehicleVIN.length() != vinSize) {
	//	VTMessageBoxOk(QString("车辆识别代码长度不是%1位").arg(vinSize), windowFlags(), QMessageBox::Information, this);
	//	return false;
	//}
	return true;
}