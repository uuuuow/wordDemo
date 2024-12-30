#pragma once
#include <QWizardPage>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include "Utils.h"
#include "ProjectWizard.h"
//委托受理情况
class CEntrustPage : public QWizardPage
{
	Q_OBJECT

public:
	CEntrustPage(ProjectWizardType type, QWidget *parent = 0);
	void setEnabled(bool enable);
	void getValue(VTProjectInfo & proInfo);
	void setValue(VTProjectInfo & proInfo);
	void setCommitments(QString licensePlateNumber, QString vehicleBrand, QString vehicleType);
protected:
	void initializePage() override;

private:
	//委托人
	QLabel *m_lbClient;
	QLineEdit *m_leClient;
	//委托事项
	QLabel *m_lbCommitments;
	QTextEdit *m_teCommitments;
	//委托日期
	QLabel *m_lbDateOfCommission;
	QLineEdit *m_leDateOfCommission;
	QPushButton *m_btnDateOfCommission;
	//鉴定日期
	QLabel *m_lbAppraisalDate;
	QLineEdit *m_leAppraisalDate;
	QPushButton *m_btnAppraisalDate;
	//鉴定地点
	QLabel *m_lbAppraisalLocation;
	QTextEdit *m_teAppraisalLocation;
	//鉴定材料
	QLabel *m_lbAppraisalMaterial;
	QTextEdit *m_teAppraisalMaterial;

	QVBoxLayout *m_vLayout;
	ProjectWizardType m_proWizardType;
};
