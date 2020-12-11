#ifndef DOG_WIDGET_H
#define DOG_WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>


#include <globle_define.h>
#include "dialog_dog_config.h"



#define STATE_IDLE 0
#define STATE_WORKING 1
#define COUNT_DOWN_DEFAULT 30         //默认喂狗时间30s
#define RAM_SIZE_LOG_COUNT 5   //默认5个位置用于记录RAM消耗历史值
#define DOG_VERSION "0.1"

namespace Ui {
class Dog_Widget;
}

class Dog_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Dog_Widget(QWidget *parent = nullptr, QString config_name = "");
    ~Dog_Widget();
    QString name() {return my_name;}
    void stop();

public slots:
    void Stop_Working();
private slots:
    void Log_Add(QString index, QString target_name = QString::fromLocal8Bit("本地操作"));
    void on_pushButton_Start_clicked();

    void on_pushButton_Mannual_Reboot_clicked();

    void on_pushButton_System_Config_clicked();

    void on_pushButton_Log_Clear_clicked();

    void ui_refresh();
    void PID_Timeout();
    void Get_Connection();
    void Socket_Disconnected();
    void Socket_Read();
    void Socket_Timeout();
    void Socket_Feed(int ms);
signals:
    void Config_Return(QString target_name, int ret, QString new_name);

private:
    Ui::Dog_Widget *ui;
    QString my_name;

    QTimer pid_countdown_timer;   //PID检查倒计时
    QTimer socket_countdown_timer;   //Socket看门狗倒计时
    unsigned long long ram_size_log[RAM_SIZE_LOG_COUNT];   //用于记录前面5次内存消耗量。如果内存消耗都不变，有可能是发生了问题
    int ram_size_ptr = 0;
    int state;
    bool pid_enabled;
    bool socket_enabled;

    QString log_addr;
    QFile log_file;
    QTextStream log_write;

    QString socket_receive_buffer;   //接收到的Socket数据


    QTimer* ui_timer;
    QTimer* log_clear_timer;
    QTimer* pid_watch_timer;
    QTimer* socket_watch_timer;

    QTcpServer my_server;
    QTcpSocket* my_socket = nullptr;




    void reset_target(QString *return_log);

    void PID_Feed(QString target_name);

    void Log_Setup();


    void Delay_Ms_UnBlocked(unsigned int msec);

};

#endif // DOG_WIDGET_H
