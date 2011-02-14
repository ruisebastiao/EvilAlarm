/*
    EvilAlarm
    Copyright (C) 2010-2011 Christian Pulvermacher

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "alarm_blubbels.h"
#include "backend.h"
#include "settings.h"

#include <QtDBus>
#include <QtGui>

AlarmBlubbels::AlarmBlubbels(QWidget *parent):
	Alarm(parent),
	label(new QLabel(this))
{
	setWindowTitle("EvilAlarm");

	//setup ui
	QHBoxLayout* mainlayout = new QHBoxLayout();
	QVBoxLayout* layout0 = new QVBoxLayout();
	QLabel *icon_label = new QLabel(this);
	icon_label->setPixmap(QPixmap("/usr/share/icons/hicolor/64x64/apps/evilalarm.png"));
	icon_label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);

	layout0->addWidget(icon_label);
	layout0->addWidget(label);
	snooze_button = new QPushButton(this);
	layout0->addWidget(snooze_button);

	mainlayout->addLayout(layout0);
	mainlayout->addWidget(&gamewidget);
	gamewidget.setFixedWidth(550); //we need lots of space
	setLayout(mainlayout);

	connect(snooze_button, SIGNAL(clicked()),
		this, SLOT(snooze()));

	connect(&gamewidget, SIGNAL(newScore(int)),
		this, SLOT(checkScore(int)));

	//actually start alarm
	restart();

	//refresh UI every second
	QTimer *ui_timer = new QTimer(this);
	connect(ui_timer, SIGNAL(timeout()),
		this, SLOT(updateScreen()));
	ui_timer->start(1000);
}

//starts/restarts the alarm
void AlarmBlubbels::restart()
{
	QSettings settings;
	inactivity_timeout = settings.value("inactivity_timeout", INACTIVITY_TIMEOUT).toInt();
	const int num_snooze_max = settings.value("num_snooze_max", NUM_SNOOZE_MAX).toInt();
	if(num_snooze_max == 0) {
		//snooze completely disabled
		snooze_button->setVisible(false);
	} else {
		snooze_button->setEnabled(num_snooze < num_snooze_max);
		snooze_button->setText(tr("Snooze (%1/%2)").arg(num_snooze).arg(num_snooze_max));
	}

	Alarm::restart();
}

AlarmBlubbels::~AlarmBlubbels()
{
}

/*
void AlarmBlubbels::accelUpdate(int x, int y, int z)
{
	//TODO: put this where?
	if(lastx == 0 and lasty == 0 and lastz == 0) {
		//initialize
		last_active.restart();
	} else if(max_diff > ACCELEROMETER_THRESHOLD and !backend->isVibrating()) {
		//device moved
		backend->volumeDown();
		last_active.restart();
	} else if(last_active.elapsed()/1000 > inactivity_timeout and !backend->isVibrating()) {
		//not moved for a while
		backend->volumeUp();
	}
*/

void AlarmBlubbels::updateScreen()
{
	//shutdown time reached?
	if(!snoozing and alarm_started.elapsed()/1000 > alarm_timeout*60) {
		close();
		return;
	}

	QString label_text = tr("<center><h1>%1</h1>").arg(QTime::currentTime().toString(Qt::SystemLocaleShortDate));

	//display remaining alarm or snooze time
	int secs_remaining;
	if(snoozing) {
		secs_remaining = QTime::currentTime().secsTo(snooze_till);
		label_text += tr("Snooze: ");
	} else {
		secs_remaining = alarm_timeout*60 - alarm_started.elapsed()/1000;
	}

	if(secs_remaining >= 60) {
		label_text += tr("%1&nbsp;min, ", "", secs_remaining/60).arg(secs_remaining/60);
	}
	label_text += tr("%1&nbsp;s remaining</center>", "", secs_remaining%60).arg(secs_remaining%60);

	label->setText(label_text);
}

void AlarmBlubbels::snooze()
{
	snooze_button->setEnabled(false);

	Alarm::snooze();
}

void AlarmBlubbels::checkScore(int score)
{
	//game finished, check if score is high enough
	QSettings settings;
	const int score_needed = settings.value("blubbels_score_needed", 500).toInt();

	if(score >= score_needed) {
		//yes, stop alarm
		QMessageBox::information(this, tr("Game Over"), tr("Your Score: %1<br>Congratulations!").arg(score));

		close();
	} else {
		QMessageBox::information(this, tr("Game Over"), tr("Your Score: %1<br>You need %2 to turn off the alarm, try again!").arg(score).arg(score_needed));
	}
}
