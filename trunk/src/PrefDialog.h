#ifndef PREFDIALOG_H
#define PREFDIALOG_H

#include <QDialog>
class QSettings;
class QDoubleSpinBox;
class QSlider;

class PrefDialog : public QDialog
{
	Q_OBJECT
public:
	PrefDialog(QSettings &settings, QWidget *parent = NULL);
	~PrefDialog();

	void saveSettings(QSettings &settings);
	void emitChanged();
signals:
	void deadZoneChanged(double deadZone);
	void turningSpeedChanged(double walkingSpeed);
	void walkingSpeedChanged(double walkingSpeed);
protected slots:
	void slot_ValueChanged(double value);
	void slot_SliderValueChanged(int value);
protected:
	QDoubleSpinBox *deadZone;
	QDoubleSpinBox *turningSpeed;
	QDoubleSpinBox *walkingSpeed;
	QSlider *deadZoneSlider;
	QSlider *turningSpeedSlider;
	QSlider *walkingSpeedSlider;
};

#endif
