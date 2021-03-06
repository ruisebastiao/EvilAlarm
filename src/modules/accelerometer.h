/*
    EvilAlarm
    Copyright (C) 2010 Christian Pulvermacher

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

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <QObject>
#include <QString>

class QTimer;

//emits orientationChanged() every interval miliseconds
class Accelerometer : public QObject
{
    Q_OBJECT
public:
    Accelerometer(QObject *parent, int interval);
    ~Accelerometer();
signals:
    void orientationChanged(int, int, int);
private slots:
    void orientationUpdate(const QString &, const QString &, const QString &, int, int, int);
    void tick();
private:
    QTimer *timer;
    int interval;
};
#endif
