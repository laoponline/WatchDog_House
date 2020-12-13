#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <QGuiApplication>

#include "globle_define.h"
#include "dialog_system_config.h"
#include "dialog_dog_config.h"
#include "dog_widget.h"
#include "system_resource_usage.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


/*****看门狗相关********/
    bool Dog_Exist(QString target_name);

protected slots:

    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);
private slots:

    void delay_init();
    void ui_refresh();

/*****看门狗相关********/
    int Dog_Add(QString target_name, int index = -1);
    int Dog_Kill(QString target_name);
    void Dog_Dealer(QString target_name, int index);
    void Dog_Config_Return(QString target_name, int ret, QString new_name = "");
    void on_pushButton_Add_Dog_clicked();

/******公共界面按键*********/
    void on_pushButton_exit_clicked();
    void on_pushButton_Minimun_clicked();
    void on_pushButton_System_Config_clicked();



private:
    System_Resource_Usage system_usage;
    float current_CPU_Usage,current_RAM_Usage,current_Disk_Usage;

    QSystemTrayIcon* tray_icon;
    QTimer* ui_timer;
    void Tray_Icon_Setup();

/*****看门狗相关********/
    QString alive_dog_name_list;   //目前已经在工作的小狗名单
    QList<void*> dog_ptr_list;
    QMap<QString,Dog_Widget*> dog_map;
    void Dog_Add_To_Tab(QString name);
    int Dog_Init();
    int Dog_Delete_From_Ini(QString target_name);


    Ui::MainWindow *ui;




};

#endif // MAINWINDOW_H
