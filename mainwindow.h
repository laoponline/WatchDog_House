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



    bool Dog_Exist(QString target_name);
protected slots:
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);
private slots:






    void on_pushButton_exit_clicked();

    void on_pushButton_Minimun_clicked();

    void on_pushButton_System_Config_clicked();

    void on_pushButton_Add_Dog_clicked();

    void delay_init();

    int Dog_Add(QString target_name, int index = -1);
    int Dog_Kill(QString target_name);


    void Dog_Dealer(QString target_name, int index);
    int Dog_Config_Return(QString target_name, int ret, QString new_name = "");
    void ui_refresh();
private:

    QSystemTrayIcon* tray_icon;
    QTimer* ui_timer;

    QString alive_dog_name_list;   //目前已经在工作的小狗名单
    QList<void*> dog_ptr_list;
    QMap<QString,Dog_Widget*> dog_map;

    System_Resource_Usage system_usage;


    float current_CPU_Usage,current_RAM_Usage,current_Disk_Usage;

    Ui::MainWindow *ui;


    void Tray_Icon_Setup();
    void Dog_Add_To_Tab(QString name);
    //int Dog_Add();

    int Dog_Init();
    int Dog_Delete_From_Ini(QString target_name);
};

#endif // MAINWINDOW_H
