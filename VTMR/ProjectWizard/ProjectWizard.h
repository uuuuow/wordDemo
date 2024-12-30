
#ifndef CLASSWIZARD_H
#define CLASSWIZARD_H

#include <QWizard>
#include "Global.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTextEdit;
class QComboBox;
class CAccidentPage;
class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;
class CProjectInfoPage;
class CMeasuringMethodPage;
class CEntrustPage;
class CAppraisalPage;
class CAppraiserWidget;

QT_END_NAMESPACE

enum ProjectWizardType
{
	PROJECT_WIZARD_CREATE,
	PROJECT_WIZARD_SHOW,
};
/*
*工程向导
*/
class CProjectWizard : public QWizard
{
    Q_OBJECT

public:
    CProjectWizard(ProjectWizardType type, VTProjectInfo *info, QString title, QWidget *parent = 0);

    void accept() override;
	CProjectInfoPage *m_proInfoPage;
	CMeasuringMethodPage *m_measuringMethod;
	CAccidentPage * m_accidentInfo;
	CEntrustPage *m_entrustInfo;
	CAppraisalPage *m_appraisalInfo;

	void initializePage(int id) override;
signals:
	void sigCreateNewProject(VTProjectInfo *proInfo);
	void sigUpdateProject(VTProjectInfo *proInfo);
public slots:
	void onCurrentIdChanged(int id);
private:
	ProjectWizardType m_proWizardType;
	VTProjectInfo *m_proInfo;
};

#endif
