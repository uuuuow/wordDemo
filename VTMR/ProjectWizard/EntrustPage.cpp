#include "EntrustPage.h"
#include <QPushButton>
#include "DateTimeDialog.h"
#include <QCompleter>
#include "SQLManager.h"

CEntrustPage::CEntrustPage(ProjectWizardType type, QWidget *parent)
{
	m_proWizardType = type;
	m_lbClient = new QLabel(QString("&委托人   "), this);
	m_leClient = new QLineEdit(this);
	auto completer = m_leClient->completer();
	if (!completer)
	{
		completer = new QCompleter(g_clientHistory, this);
		m_leClient->setCompleter(completer);
	}
	completer->setFilterMode(Qt::MatchStartsWith);

	m_lbCommitments = new QLabel(QString("&委托事项"), this);
	m_teCommitments = new QTextEdit(this);

	m_lbDateOfCommission = new QLabel(QString("&委托日期"), this);
	m_leDateOfCommission = new QLineEdit(this);
	m_btnDateOfCommission = new QPushButton("...", this);
	m_lbAppraisalDate = new QLabel(QString("&鉴定日期"), this);
	m_leAppraisalDate = new QLineEdit(this);
	m_btnAppraisalDate = new QPushButton("...", this);
	m_lbAppraisalLocation = new QLabel(QString("&鉴定地点"), this);
	m_teAppraisalLocation = new QTextEdit(this);
	m_lbAppraisalMaterial = new QLabel(QString("&鉴定材料"), this);
	m_teAppraisalMaterial = new QTextEdit(this);
	m_teAppraisalMaterial->setPlaceholderText(QString("事发时现场附近监控视频（复制件）"));
	m_vLayout = new QVBoxLayout;

	setTitle(QString("委托受理情况"));
	
	//委托人
	m_lbClient->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbClient->setBuddy(m_leClient);
	//委托事项
	m_lbCommitments->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbCommitments->setBuddy(m_teCommitments);
	m_teCommitments->setMaximumHeight(80);
	//委托日期
	m_lbDateOfCommission->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_leDateOfCommission->setEnabled(false);
	m_lbDateOfCommission->setBuddy(m_leDateOfCommission);
	auto datetime = QDateTime::currentDateTime();
	auto time = datetime.toString("yyyy-MM-dd");
	m_leDateOfCommission->setText(time);

	connect(m_btnDateOfCommission, &QPushButton::clicked,
		[=]() {
		auto dateTimeDialog = new CDateTimeDialog(datetime, false, m_leDateOfCommission);
		connect(dateTimeDialog, &CDateTimeDialog::sigSelectedDate,
			[=](QDateTime dateTime) {
			auto time = dateTime.toString("yyyy-MM-dd");
			m_leDateOfCommission->setText(time);
		});
		dateTimeDialog->exec();
	});
	//鉴定日期
	m_lbAppraisalDate->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_leAppraisalDate->setEnabled(false);
	m_lbAppraisalDate->setBuddy(m_leAppraisalDate);
	datetime = QDateTime::currentDateTime();
	time = datetime.toString("yyyy-MM-dd");
	m_leAppraisalDate->setText(time);

	connect(m_btnAppraisalDate, &QPushButton::clicked,
		[=]() {
		auto dateTimeDialog = new CDateTimeDialog(datetime, false, m_leAppraisalDate);
		connect(dateTimeDialog, &CDateTimeDialog::sigSelectedDate,
			[=](QDateTime dateTime) {
			auto time = dateTime.toString("yyyy-MM-dd");
			m_leAppraisalDate->setText(time);
		});
		dateTimeDialog->exec();
	});
	//鉴定地点
	m_lbAppraisalLocation->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbAppraisalLocation->setBuddy(m_teAppraisalLocation);
	m_teAppraisalLocation->setMaximumHeight(80);
	//鉴定材料
	m_lbAppraisalMaterial->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_lbAppraisalMaterial->setBuddy(m_teAppraisalMaterial);
	m_teAppraisalMaterial->setMaximumHeight(80);

	m_vLayout->setContentsMargins(10, 10, 10, 10);
	m_vLayout->setSpacing(10);

	QHBoxLayout *hLayout;
	//委托人
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbClient);
	hLayout->addWidget(m_leClient);
	m_vLayout->addItem(hLayout);
	//委托事项
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbCommitments);
	hLayout->addWidget(m_teCommitments);
	m_vLayout->addItem(hLayout);
	//委托日期
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbDateOfCommission);
	hLayout->addWidget(m_leDateOfCommission);
	hLayout->addWidget(m_btnDateOfCommission);
	//鉴定日期
	hLayout->addWidget(m_lbAppraisalDate);
	hLayout->addWidget(m_leAppraisalDate);
	hLayout->addWidget(m_btnAppraisalDate);
	m_vLayout->addItem(hLayout);
	//鉴定地点
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbAppraisalLocation);
	hLayout->addWidget(m_teAppraisalLocation);
	m_vLayout->addItem(hLayout);
	//鉴定材料
	hLayout = new QHBoxLayout;
	hLayout->setSpacing(10);
	hLayout->addWidget(m_lbAppraisalMaterial);
	hLayout->addWidget(m_teAppraisalMaterial);
	m_vLayout->addItem(hLayout);

	auto spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_vLayout->addItem(spacer);
	setLayout(m_vLayout);

	//registerField("client", m_leClient);//委托人
	//registerField("commitments*", m_teCommitments);//委托事项
	//registerField("date_of_commission", m_leDateOfCommission);//委托日期
	//registerField("appraisal_date", m_leAppraisalDate);//鉴定日期
	//registerField("appraisal_location", m_teAppraisalLocation);//鉴定地点
	//registerField("appraisal_material", m_teAppraisalMaterial);//鉴定材料
}

void CEntrustPage::initializePage()
{
}

void CEntrustPage::setEnabled(bool enable)
{
	m_leClient->setEnabled(enable);
	m_teCommitments->setEnabled(enable);
	m_btnDateOfCommission->setEnabled(enable);
	m_btnAppraisalDate->setEnabled(enable);
	m_teAppraisalLocation->setEnabled(enable);
	m_teAppraisalMaterial->setEnabled(enable);
}

void CEntrustPage::getValue(VTProjectInfo & proInfo)
{
	proInfo.client = m_leClient->text();
	proInfo.commitments = m_teCommitments->toPlainText();
	proInfo.dateOfCommission = m_leDateOfCommission->text();
	proInfo.appraisalDate = m_leAppraisalDate->text();
	proInfo.appraisalLocation = m_teAppraisalLocation->toPlainText();
	proInfo.appraisalMaterial = m_teAppraisalMaterial->toPlainText();

	ClinetEditHistoryInsertSql(proInfo.client);
}

void CEntrustPage::setValue(VTProjectInfo & proInfo)
{
	m_leClient->setText(proInfo.client);
	m_teCommitments->setPlainText(proInfo.commitments);
	m_leDateOfCommission->setText(proInfo.dateOfCommission);
	m_leAppraisalDate->setText(proInfo.appraisalDate);
	m_teAppraisalLocation->setPlainText(proInfo.appraisalLocation);
	m_teAppraisalMaterial->setPlainText(proInfo.appraisalMaterial);
}

void CEntrustPage::setCommitments(QString licensePlateNumber, QString vehicleBrand, QString vehicleType)
{
	auto str = QString("对%1%2%3事发时的速度进行鉴定").arg(licensePlateNumber).arg(vehicleBrand).arg(vehicleType);
	m_teCommitments->setPlaceholderText(str);
}