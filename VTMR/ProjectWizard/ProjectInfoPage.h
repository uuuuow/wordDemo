#pragma once

#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include  "Utils.h"
#include "ProjectWizard.h"

//填写工程详情页
class CProjectInfoPage : public QWizardPage
{
	Q_OBJECT

public:
	CProjectInfoPage(ProjectWizardType type, QWidget *parent = 0);
	void setEnabled(bool enable);
	void getValue(VTProjectInfo & proInfo);
	void setValue(VTProjectInfo & proInfo);
protected:
	void initializePage() override;
private:
	//工程名称
	QLabel *m_lbProjectName;
	QLineEdit *m_leProjectName;
	//视频路径
	QLabel *m_lbVideoPath;
	QLineEdit* m_leVideoPath;
	QPushButton *m_btnVideoPath;

	QVBoxLayout *m_vLayout;
	ProjectWizardType m_proWizardType;
};