#include "alarm_blubbels_settings.h"
#include <QtGui>

AlarmBlubbelsSettings::AlarmBlubbelsSettings(QWidget *parent):
	QWidget(parent)
{
	QGridLayout *layout = new QGridLayout(this);
	layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	layout->setColumnStretch(0, 1);

	QSettings settings;
	QLabel *threshold_label = new QLabel(tr("Score needed to turn off alarm"));
	threshold = new QSpinBox();
	threshold->setRange(10, 9999);
	threshold->setValue(settings.value("blubbels_threshold", 500).toInt());
	layout->addWidget(threshold_label, 0, 0);
	layout->addWidget(threshold, 0, 1);
}

//save any changes
AlarmBlubbelsSettings::~AlarmBlubbelsSettings()
{
	QSettings settings;
	settings.setValue("blubbels_threshold", threshold->value());
	settings.sync();
}
