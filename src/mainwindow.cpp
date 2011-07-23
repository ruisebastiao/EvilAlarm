#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "selectalarmtype.h"
#include "alarmhistory.h"
#include "settings.h"
#include "about.h"
#include "daemon.h"
#include "alarm.h"
#include "module_list.h"

#include <QDeclarativeContext>
#include <QSettings>
#include <QGraphicsObject>
#include <QFile>

#include <iostream>

//#if defined(Q_WS_MAEMO)
//#include <alarmd/alarm_event.h>
//#endif


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_Maemo5StackedWindow);

    //load alarm
    QSettings settings;
    const QTime alarm_time = settings.value("wake_at", QTime::currentTime()).toTime();
    QDeclarativeContext *context = ui->view->rootContext();
    context->setContextProperty("evilalarm_hours", alarm_time.hour());
    context->setContextProperty("evilalarm_minutes", alarm_time.minute());
#ifdef EVILALARM
    context->setContextProperty("evilalarm_active", Daemon::isRunning());
#endif

    QString path;
    const QString realMaemo("/opt/evilalarm/qml/Wakedo/main.qml");
    if(QFile::exists(realMaemo)){
        path=realMaemo;
    }else{
        //simulator
        path=QApplication::applicationDirPath()+"/qml/Wakedo/main.qml";
    }

    //now load UI
    //bool r = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    ui->view->setSource(QUrl::fromLocalFile(path));


    QGraphicsObject *root_object = ui->view->rootObject();
    QObject::connect(root_object, SIGNAL(selectAlarmType()),
                     this, SLOT(showSelector()));
    QObject::connect(root_object, SIGNAL(alarmHistory()),
                     this, SLOT(showAlarmHistory()));
    QObject::connect(root_object, SIGNAL(unsetAlarm()),
                     this, SLOT(unsetEvilAlarm()));
    QObject::connect(root_object, SIGNAL(setAlarm(int, int)),
                     this, SLOT(setEvilAlarm(int, int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::showSelector() {
    static SelectAlarmType* selectAlarmType = new SelectAlarmType(this);
    selectAlarmType->show();
}
void MainWindow::showAlarmHistory() {
    static AlarmHistory* alarmHistory = new AlarmHistory(this);
    alarmHistory->show();
}

void MainWindow::setEvilAlarm(int hours, int minutes) {
#ifdef EVILALARM
    std::cout << "setEvilAlarm()\n";
    QSettings settings;

    const QTime wake_at(hours, minutes);

    if(Daemon::isRunning()) {
        std::cout << "reset needed\n";
        if(settings.value("wake_at").toTime() == wake_at)
            return; //time didn't change, nothing to do

        //new alarm time, discard old one
        unsetEvilAlarm();
    }

    settings.setValue("wake_at", wake_at);

    int msecs = QTime::currentTime().msecsTo(wake_at);
    if(msecs < 0) //alarm tomorrow?
        msecs += 24*60*60*1000; //+24h

    //save to history
    settings.beginGroup("history");
    const int num_used = settings.value(QString("%1/used").arg(wake_at.toString()), 0).toInt();
    settings.setValue(QString("%1/used").arg(wake_at.toString()), num_used+1);
    settings.endGroup();

    settings.sync();
    std::cout << "saved to history\n";

    Daemon::start();
#endif
}

void MainWindow::unsetEvilAlarm() {
#ifdef EVILALARM
    std::cout << "unsetEvilAlarm()\n";
    Daemon::stop();

    //since unsetEvilAlarm() is only called when the user deactivates a set alarm, remove the most recent alarm from history
    QSettings settings;
    settings.beginGroup("history");
    const QTime wake_at = settings.value("wake_at").toTime();
    const int num_used = settings.value(QString("%1/used").arg(wake_at.toString()), 0).toInt();
    if(num_used == 0) {
        std::cerr << "trying to remove nonexistant alarm from history\n";
        return;
    }
    settings.setValue(QString("%1/used").arg(wake_at.toString()), num_used-1);
    settings.endGroup();
    settings.sync();
#endif
}

/*void MainWindow::setAlarm(int hours,int minutes){
    // from http://www.tardigrada.hr/blog/2010/10/using-maemos-alarm-framework-with-qt-the-basics/


        alarm_event_t * event = 0;
        alarm_action_t * act = 0;

        // Setup
        event = alarm_event_create();
        // The application id field is needed for every alarm you set
        alarm_event_set_alarm_appid(event, "Alarm testing app");
        // Alarm description displayed when the alarm is triggered
        alarm_event_set_message(event, "The alarm message for the system UI");
        // Set the alarm time 30s form now
        event->alarm_time = time(0) + 30;

        // Acknowledge action
        act = alarm_event_add_actions(event, 1);
        alarm_action_set_label(act, "Stop");
        act->flags |= ALARM_ACTION_WHEN_RESPONDED;
        act->flags |= ALARM_ACTION_TYPE_NOP;

        // Snooze action
        act = alarm_event_add_actions(event, 1);
        alarm_action_set_label(act, "Snooze");
        act->flags |= ALARM_ACTION_WHEN_RESPONDED;
        act->flags |= ALARM_ACTION_TYPE_SNOOZE;

        // Pass the structure to the alarmd daemon
        alarmd_event_add(event);
        // Cleanup
        alarm_event_delete(event);
        act = 0;
        event = 0;


}
*/
void MainWindow::on_actionSettings_triggered()
{
    static Settings *settingsWindow = new Settings(this);
    settingsWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    settingsWindow->setWindowFlags(windowFlags() | Qt::Window);
    settingsWindow->show();

}

void MainWindow::on_actionAbout_triggered()
{
    static About *about = new About(this);
    about->setAttribute(Qt::WA_Maemo5StackedWindow);
    about->setWindowFlags(windowFlags() | Qt::Window);
    about->show();
}

void MainWindow::on_actionTest_Alarm_triggered()
{
    Alarm* test_alarm = ModuleList::getModuleInstance(this);
    test_alarm->exec();
    delete test_alarm;
}
