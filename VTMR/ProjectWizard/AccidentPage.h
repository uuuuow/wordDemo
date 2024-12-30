#pragma once

#include <QWizardPage>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include "Utils.h"
#include "ProjectWizard.h"
//事故基本信息页
class CAccidentPage : public QWizardPage
{
	Q_OBJECT

public:
	CAccidentPage(ProjectWizardType type, QWidget *parent = 0);
	void setEnabled(bool enable);
	void getValue(VTProjectInfo &proInfo);
	void setValue(VTProjectInfo &proInfo);
	void getValue(QString &licensePlateNumber, QString &vehicleBrand, QString &vehicleType);

protected:
	void initializePage() override;
	bool validatePage() override;

private:
	//事故时间
	QLabel *m_lbAccidentTime;
	QLineEdit *m_leAccidentTime;
	QPushButton *m_btnAccidentTime;
	//事故地点
	QLabel *m_lbAccidentLocation;
	QTextEdit *m_teAccidentLocation;
	//车辆号牌
	QLabel *m_lbVehiclePlateNumber;
	QLineEdit *m_leVehiclePlateNumber;
	//车辆品牌
	QLabel *m_lbVehicleBrand;
	QLineEdit *m_leVehicleBrand;
	//车辆类型
	QLabel *m_lbVehicleType;
	QComboBox *m_cbVehicleType;
	//车辆识别代码VIN
	QLabel *m_lbVehicleVIN;
	QLineEdit *m_leVehicleVIN;

	QVBoxLayout *m_vLayout;
	ProjectWizardType m_proWizardType;
};
