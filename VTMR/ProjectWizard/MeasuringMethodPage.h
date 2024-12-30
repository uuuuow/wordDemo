#pragma once

#include <QWizardPage>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QGroupBox>
#include  "Utils.h"
#include "ProjectWizard.h"
//标定类型
class CMeasuringMethodPage : public QWizardPage
{
	Q_OBJECT

public:
	CMeasuringMethodPage(ProjectWizardType type, QWidget *parent = 0);
	void setEnabled(bool enable);
	void getValue(VTProjectInfo & proInfo);
	void setValue(VTProjectInfo & proInfo);
protected:
	void initializePage() override;
private:
	QLabel *m_lb1;
	QLabel *m_lb2;
	QRadioButton *m_rb1;
	QRadioButton *m_rb2;
	QRadioButton *m_rb3;
	QRadioButton *m_rb4;
	QRadioButton *m_rb5;
	QRadioButton *m_rb6;
	QRadioButton *m_rb7;
	QGridLayout *m_gl;
	QGroupBox *m_gb;
	QHBoxLayout *m_hLayout;
	ProjectWizardType m_proWizardType;
};