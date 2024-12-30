#include "MeasuringMethodWnd.h"
#include"Utils.h"
#include "Global.h"

CMeasuringMethodWnd::CMeasuringMethodWnd(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString("请选择标定方式"));

	if (g_proInfo.mode == FIXED_CAM_ONESELF) {
		//固定摄像头,车身自标定
		ui.RBFixCamOneself->setChecked(true);
	}
	else if (g_proInfo.mode == FIXED_CAM_GROUND_REFERENCE)
	{
		//固定摄像头，地面参照物
		ui.RBFixCamGroundRef->setChecked(true);
	}
	else if (g_proInfo.mode == FIXED_CAM_1D) {
		//固定摄像头,一维线性
		ui.RBFixCam1D->setChecked(true);
	}
	else if (g_proInfo.mode == FIXED_CAM_2D) {
		//固定摄像头,二维线性
		ui.RBFixCam2D->setChecked(true);
	}
	else if (g_proInfo.mode == MOVING_CAM_SPACE) {
		//车载摄像头,空间标定
		ui.RBMovingCamSpace->setChecked(true);
	}
	else if (g_proInfo.mode == MOVING_CAM_2D_ONESELF) {
		//车载摄像头,二维本车
		ui.RBMovingCam2DOneself->setChecked(true);
	}
	else if (g_proInfo.mode == MOVING_CAM_2D_TARGET_VEHICLE) {
		//车载摄像头，二维目标车
		ui.RBMovingCam2DTargetVehicle->setChecked(true);
	}

	connect(ui.RBFixCamOneself, &QRadioButton::clicked, 
		[=](bool checked) {
		if (checked)g_proInfo.mode = FIXED_CAM_ONESELF; 
		}
	);

	connect(ui.RBFixCamGroundRef, &QRadioButton::clicked, 
		[=](bool checked) {
		if (checked)g_proInfo.mode = FIXED_CAM_GROUND_REFERENCE; 
		}
	);

	connect(ui.RBFixCam1D, &QRadioButton::clicked, 
		[=](bool checked) {
			if (checked)g_proInfo.mode = FIXED_CAM_1D; 
		}
	);

	connect(ui.RBFixCam2D, &QRadioButton::clicked, 
		[=](bool checked) {
			if (checked)g_proInfo.mode = FIXED_CAM_2D; 
		}
	);

	connect(ui.RBMovingCamSpace, &QRadioButton::clicked, 
		[=](bool checked) {
			if (checked)g_proInfo.mode = MOVING_CAM_SPACE; 
		}
	);

	connect(ui.RBMovingCam2DOneself, &QRadioButton::clicked, 
		[=](bool checked) {
			if (checked)g_proInfo.mode = MOVING_CAM_2D_ONESELF; 
		}
	);

	connect(ui.RBMovingCam2DTargetVehicle, &QRadioButton::clicked, 
		[=](bool checked) {
			if(checked)
				g_proInfo.mode = MOVING_CAM_2D_TARGET_VEHICLE; 
		}
	);
}

CMeasuringMethodWnd::~CMeasuringMethodWnd()
{
}
