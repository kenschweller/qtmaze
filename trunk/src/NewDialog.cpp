#include "NewDialog.h"
#include <QSettings>

#include <QFormLayout>
#include <QPushButton>

#include <QCloseEvent>

NewDialog::NewDialog(QSettings &settings, QWidget *parent) : QDialog(parent)
{
	QFormLayout * const form = new QFormLayout(this);
	form->addRow("Width", _width = new QSpinBox(this));
	form->addRow("Height", _height = new QSpinBox(this));
	// form->addRow("Description", _description = new QTextEdit(this));

	QPushButton * const newButton = new QPushButton("New", this);
	QPushButton * const cancelButton = new QPushButton("Cancel", this);
	connect(newButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	form->addRow(newButton);
	form->addRow(cancelButton);

	_width->setMinimum(1);
	_width->setMaximum(300);
	_height->setMinimum(1);
	_height->setMaximum(300);
	// _description->setReadOnly(false);
	// _description->setAcceptRichText(false);

	setWindowTitle("New maze dimensions");
	// setWindowTitle("New maze dimensions and description");
	setModal(true);

	_width->setValue(settings.value("maze_width", 32).toInt());
	_height->setValue(settings.value("maze_height", 32).toInt());
}

void NewDialog::saveSettings(QSettings &settings)
{
	settings.setValue("maze_width", _width->value());
	settings.setValue("maze_height", _height->value());
}
