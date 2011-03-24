#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
class QSettings;
#include <QSpinBox>
#include <QTextEdit>

class NewDialog : public QDialog
{
	Q_OBJECT
public:
	NewDialog(QSettings &settings, QWidget *parent = NULL);
	void saveSettings(QSettings &settings);
	
	QSpinBox *_width;
	QSpinBox *_height;
	QTextEdit *_description;
};

#endif
