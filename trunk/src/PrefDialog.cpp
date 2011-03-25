#include "PrefDialog.h"
#include <QSettings>
#include <QDoubleSpinBox>
#include <QSlider>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>

PrefDialog::PrefDialog(QSettings &settings, QWidget *parent) : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
{
	QVBoxLayout * const vbox = new QVBoxLayout(this);
	QGroupBox * const joystickBox = new QGroupBox("Joystick Settings", this);
	vbox->addWidget(joystickBox);

	QGridLayout * const grid = new QGridLayout(joystickBox);

	grid->addWidget(new QLabel("Dead Zone:", this), 0, 0);
	grid->addWidget(deadZone = new QDoubleSpinBox(this), 0, 1);
	grid->addWidget(deadZoneSlider = new QSlider(Qt::Horizontal, this), 1, 0, 1, 2);

	grid->addWidget(new QLabel("Turning Speed:", this), 2, 0);
	grid->addWidget(turningSpeed = new QDoubleSpinBox(this), 2, 1);
	grid->addWidget(turningSpeedSlider = new QSlider(Qt::Horizontal, this), 3, 0, 1, 2);

	grid->addWidget(new QLabel("Walking Speed:", this), 4, 0);
	grid->addWidget(walkingSpeed = new QDoubleSpinBox(this), 4, 1);
	grid->addWidget(walkingSpeedSlider = new QSlider(Qt::Horizontal, this), 5, 0, 1, 2);

	deadZone->setSingleStep(0.01);
	turningSpeed->setSingleStep(0.1);
	walkingSpeed->setSingleStep(0.5);
	deadZone->setRange(0.0, 1.0);
	turningSpeed->setRange(0.0, 1.0);
	walkingSpeed->setRange(0.0, 10.0);
	deadZoneSlider->setRange(0, static_cast<int>(deadZone->maximum()*1000.0));
	turningSpeedSlider->setRange(0, static_cast<int>(turningSpeed->maximum()*1000.0));
	walkingSpeedSlider->setRange(0, static_cast<int>(walkingSpeed->maximum()*1000.0));

	connect(deadZone, SIGNAL(valueChanged(double)), this, SLOT(slot_ValueChanged(double)));
	connect(turningSpeed, SIGNAL(valueChanged(double)), this, SLOT(slot_ValueChanged(double)));
	connect(walkingSpeed, SIGNAL(valueChanged(double)), this, SLOT(slot_ValueChanged(double)));
	connect(deadZoneSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_SliderValueChanged(int)));
	connect(turningSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_SliderValueChanged(int)));
	connect(walkingSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_SliderValueChanged(int)));

	deadZone->setValue(settings.value("dead_zone", 0.2).toDouble());
	turningSpeed->setValue(settings.value("turning_speed", 1.0).toDouble());
	walkingSpeed->setValue(settings.value("walking_speed", 7.0).toDouble());

	connect(deadZone, SIGNAL(valueChanged(double)), this, SIGNAL(deadZoneChanged(double)));
	connect(turningSpeed, SIGNAL(valueChanged(double)), this, SIGNAL(turningSpeedChanged(double)));
	connect(walkingSpeed, SIGNAL(valueChanged(double)), this, SIGNAL(walkingSpeedChanged(double)));
}

PrefDialog::~PrefDialog()
{
}

void PrefDialog::saveSettings(QSettings &settings)
{
	settings.setValue("dead_zone", deadZone->value());
	settings.setValue("turning_speed", turningSpeed->value());
	settings.setValue("walking_speed", walkingSpeed->value());
}

void PrefDialog::slot_ValueChanged(double value)
{
	QSlider * const target = (sender() == deadZone) ? deadZoneSlider : (sender() == turningSpeed) ? turningSpeedSlider : walkingSpeedSlider;
	target->setValue(static_cast<int>(value*1000.0));
}

void PrefDialog::slot_SliderValueChanged(int value)
{
	QDoubleSpinBox * const target = (sender() == deadZoneSlider) ? deadZone : (sender() == turningSpeedSlider) ? turningSpeed : walkingSpeed;
	target->setValue(static_cast<double>(value)/1000.0);
}

void PrefDialog::emitChanged()
{
	emit(deadZoneChanged(deadZone->value()));
	emit(turningSpeedChanged(turningSpeed->value()));
	emit(walkingSpeedChanged(walkingSpeed->value()));
}
