#include <QtWidgets>
#include "ProjectWizard.h"
#include <QUuid>
#include "AppraiserDialog.h"
#include "ProjectInfoPage.h"
#include "MeasuringMethodPage.h"
#include "AccidentPage.h"
#include "DateTimeDialog.h"
#include "EntrustPage.h"
#include "AppraisalPage.h"

CProjectWizard::CProjectWizard(ProjectWizardType type, VTProjectInfo *info, QString title, QWidget *parent)
    : QWizard(parent)
{
	setDefaultProperty("QTextEdit", "PlainText", "textChanged");
	m_proWizardType = type;
	if (info)
		m_proInfo = info;
	else
		m_proInfo = new VTProjectInfo;

	setWindowTitle(title);
	setFixedSize(QSize(780, 260));
	setWizardStyle(QWizard::ClassicStyle);
	setButtonText(QWizard::CustomButton1, QString("修 改"));
	setButtonText(QWizard::NextButton, QString("下一步"));
	setButtonText(QWizard::BackButton, QString("上一步"));
	setButtonText(QWizard::CancelButton, QString("取  消"));
	setButtonText(QWizard::FinishButton, QString("完  成"));

	connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(onCurrentIdChanged(int)));
	connect(button(QWizard::CustomButton1), &QPushButton::clicked,
		[=]() {
		if (currentPage() == m_accidentInfo)
			m_accidentInfo->setEnabled(true);
		else if (currentPage() == m_entrustInfo)
			m_entrustInfo->setEnabled(true);
		else if (currentPage() == m_appraisalInfo)
			m_appraisalInfo->setEnabled(true);
		
	});

	connect(button(QWizard::NextButton), &QPushButton::clicked,
		[=]() {
		QString licensePlateNumber, vehicleBrand, vehicleType;
		if (currentPage() == m_entrustInfo) {
			m_accidentInfo->getValue(licensePlateNumber, vehicleBrand, vehicleType);
			m_entrustInfo->setCommitments(licensePlateNumber, vehicleBrand, vehicleType);
		}
	});

	m_proInfoPage = new CProjectInfoPage(type, this);
	//m_measuringMethod = new CMeasuringMethodPage(type, this);
	m_accidentInfo = new CAccidentPage(type, this);
	m_entrustInfo = new CEntrustPage(type, this);
	m_appraisalInfo = new CAppraisalPage(type, this);
	addPage(m_proInfoPage);
	//addPage(m_measuringMethod);
	addPage(m_accidentInfo);
	addPage(m_entrustInfo);
	addPage(m_appraisalInfo);


	if (type == PROJECT_WIZARD_SHOW) {
		connect(this, SIGNAL(sigUpdateProject(VTProjectInfo*)), (QWidget*)g_vtmr, SLOT(onUpdateProject(VTProjectInfo*)));
		setOption(QWizard::HaveCustomButton1);
		m_proInfoPage->setValue(*m_proInfo);
		//m_measuringMethod->setValue(*m_proInfo);
		m_accidentInfo->setValue(*m_proInfo);
		m_entrustInfo->setValue(*m_proInfo);
		m_appraisalInfo->setValue(*m_proInfo);

		m_proInfoPage->setEnabled(false);
		//m_measuringMethod->setEnabled(false);
		m_accidentInfo->setEnabled(false);
		m_entrustInfo->setEnabled(false);
		m_appraisalInfo->setEnabled(false);
	}
	else if (type == PROJECT_WIZARD_CREATE) {
		connect(this, SIGNAL(sigCreateNewProject(VTProjectInfo*)), (QWidget*)g_vtmr, SLOT(onCreateNewProject(VTProjectInfo*)));
		m_proInfoPage->setEnabled(true);
		//m_measuringMethod->setEnabled(true);
		m_accidentInfo->setEnabled(true);
		m_entrustInfo->setEnabled(true);
		m_appraisalInfo->setEnabled(true);
	}
}

void CProjectWizard::initializePage(int id)
{
}

void CProjectWizard::onCurrentIdChanged(int id)
{
	if (m_proWizardType == PROJECT_WIZARD_SHOW) {
		if (currentPage() == m_proInfoPage)
			button(QWizard::CustomButton1)->setVisible(false);
		else
			button(QWizard::CustomButton1)->setVisible(true);
	}

	if (currentPage() == m_proInfoPage) {
		setFixedSize(QSize(780, 260));
	}
	else if (currentPage() == m_accidentInfo) {
		setFixedSize(QSize(780, 380));
	}
	else if (currentPage() == m_entrustInfo) {
		setFixedSize(QSize(780, 480));
	}
	else if (currentPage() == m_appraisalInfo) {
		setFixedSize(QSize(780, 720));
	}
}

void CProjectWizard::accept()
{
	auto datetime = QDateTime::currentDateTime();
	auto timestamp = datetime.toSecsSinceEpoch();

	auto id = QUuid::createUuid();
	auto uuid = id.toString();
	uuid.remove("{").remove("}").remove("-");

	if (m_proWizardType == PROJECT_WIZARD_CREATE) {
		m_proInfo->uuid = uuid;
		m_proInfo->createTime = timestamp;
	}

	m_proInfoPage->getValue(*m_proInfo);
	//m_measuringMethod->getValue(*m_proInfo);
	m_accidentInfo->getValue(*m_proInfo);
	m_entrustInfo->getValue(*m_proInfo);
	m_appraisalInfo->getValue(*m_proInfo);

	if (m_proWizardType == PROJECT_WIZARD_CREATE) {
		emit sigCreateNewProject(m_proInfo);
	}
	else if (m_proWizardType == PROJECT_WIZARD_SHOW) {
		emit sigUpdateProject(m_proInfo);
	}
	QDialog::accept();
}
