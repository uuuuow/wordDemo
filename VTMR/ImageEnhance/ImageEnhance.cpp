#include "ImageEnhance.h"

CImageEnhance::CImageEnhance(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.lbBrightness->setFocusPolicy(Qt::NoFocus);
	ui.lbContrast->setFocusPolicy(Qt::NoFocus);
	ui.lbSaturation->setFocusPolicy(Qt::NoFocus);
	ui.leBrightness->setFocusPolicy(Qt::ClickFocus);
	ui.leContrast->setFocusPolicy(Qt::ClickFocus);
	ui.leSaturation->setFocusPolicy(Qt::ClickFocus);
	ui.hsBrightness->setFocusPolicy(Qt::ClickFocus);
	ui.hsContrast->setFocusPolicy(Qt::ClickFocus);
	ui.hsSaturation->setFocusPolicy(Qt::ClickFocus);

	connect(ui.hsBrightness, &QAbstractSlider::valueChanged, [=](int value) {
		emit sigBrightnessValueChanged(value);
		auto text = ui.leBrightness->text();
		auto brightness = text.toInt();
		if (brightness != value)
		{
			ui.leBrightness->setText(QString("%1").arg(value));
		}
	});

	connect(ui.hsContrast, &QAbstractSlider::valueChanged, [=](int value) {
		emit sigContrastValueChanged(value);
		auto text = ui.leContrast->text();
		auto contrast = text.toInt();
		if (contrast != value)
		{
			ui.leContrast->setText(QString("%1").arg(value));
		}
	});

	connect(ui.hsSaturation, &QAbstractSlider::valueChanged, [=](int value) {
		emit sigSaturationValueChanged(value);
		auto text = ui.leSaturation->text();
		auto saturation = text.toInt();
		if (saturation != value)
		{
			ui.leSaturation->setText(QString("%1").arg(value));
		}
	});

	connect(ui.leBrightness, &QLineEdit::textChanged, [=](const QString &text) {
		auto value = text.toInt();
		if (ui.hsBrightness->sliderPosition() != value)
		{
			ui.hsBrightness->setSliderPosition(value);
		}
	});

	connect(ui.leContrast, &QLineEdit::textChanged, [=](const QString &text) {
		auto value = text.toInt();
		if (ui.hsContrast->sliderPosition() != value)
		{
			ui.hsContrast->setSliderPosition(value);
		}
	});

	connect(ui.leSaturation, &QLineEdit::textChanged, [=](const QString &text) {
		int value = text.toInt();
		if (ui.hsSaturation->sliderPosition() != value)
		{
			ui.hsSaturation->setSliderPosition(value);
		}
	});
}

CImageEnhance::~CImageEnhance()
{
}

void CImageEnhance::setValue(int brightness, int contrast, int saturation)
{
	if (ui.hsBrightness->sliderPosition() != brightness)
		ui.hsBrightness->setSliderPosition(brightness);
	if (ui.hsContrast->sliderPosition() != contrast)
		ui.hsContrast->setSliderPosition(contrast);
	if (ui.hsSaturation->sliderPosition() != saturation)
		ui.hsSaturation->setSliderPosition(saturation);
}
