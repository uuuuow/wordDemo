#pragma once

#include <QWizardPage>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include "Utils.h"
#include "ProjectWizard.h"
class CAppraisalStandardTable;

//鉴定人详情
class CAppraisalPage : public QWizardPage
{
	Q_OBJECT

public:
	CAppraisalPage(ProjectWizardType type, QWidget *parent = 0);
	void setEnabled(bool enable);
	void getValue(VTProjectInfo & proInfo);
	void setValue(VTProjectInfo & proInfo);
protected:
	void initializePage() override;
public slots:
	void onAdd();
	void onDelete(QString id);

private:
	//鉴定人
	QLabel *m_lbAppraiser1;
	QComboBox *m_cbAppraiser1;
	QLabel* m_lbAppraiserNum1;
	QLineEdit* m_leAppraiserNum1;
	QLabel* m_lbAppraiserPhotoText1;
	QLabel* m_lbAppraiserPhotoImage1;

	QLabel *m_lbAppraiser2;
	QComboBox *m_cbAppraiser2;
	QLabel* m_lbAppraiserNum2;
	QLineEdit* m_leAppraiserNum2;
	QLabel* m_lbAppraiserPhotoText2;
	QLabel* m_lbAppraiserPhotoImage2;
	//适应标准
	QLabel *m_lbStandard;
	CAppraisalStandardTable *m_appraisalStandardTable;

	QVBoxLayout *m_vLayout;
	ProjectWizardType m_proWizardType;
};