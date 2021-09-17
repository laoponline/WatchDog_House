#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码

#ifndef DOG_WIDGET_H
#define DOG_WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

#include <qcustomplot.h>
#include <globle_define.h>
#include "dialog_dog_config.h"


#define TABLE_TIME_PERIOD_SEC 60*60   //显示的图标，从左至右的总时间 60秒*分钟数
#define DATA_TYPE_CPU_USAGE 0
#define DATA_TYPE_RAM_USAGE 1

#define STATE_IDLE 0
#define STATE_WORKING 1
#define COUNT_DOWN_DEFAULT 30         //默认喂狗时间30s
#define RAM_LOG_SIZE 50   //默认5个位置用于记录RAM消耗历史值
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
    void Socket_Feed(int ms  = 0);
    void Plot_X_AtuoRange(QCustomPlot *customPlot, double time);
    void Plot_Add_Data(int type, QVariant data);
    void SLOT_Show_Tracer(QMouseEvent *event);
    void on_pushButton_Log_Saveto_clicked();

signals:
    void Config_Return(QString target_name, int ret, QString new_name);

private:
    /**********基础项目***********/
    Ui::Dog_Widget *ui;
    QString my_name;
    bool reseting = false;  //正在进行重启的标记位。如果为true，则说明其中某一项看门狗正在进行目标重启，不能再用其它方式同时再进行操作，否则可能多重启动

    QTimer pid_countdown_timer;   //PID检查倒计时
    QTimer socket_countdown_timer;   //Socket看门狗倒计时

    int state;
    bool pid_enabled;
    bool socket_enabled;
    bool socket_external_only = false;
    bool closing_window = false;
    int reboot_delay = REBOOT_DELAY_DEFAULT;

    QString socket_receive_buffer;   //接收到的Socket数据
    QTimer* ui_timer;

    QTimer* pid_watch_timer;
    QTimer* socket_watch_timer;

    QTcpServer my_server;
    QTcpSocket* my_socket = nullptr;


    void PID_Feed(QString target_name = "");
    void reset_target(QString *return_log);

    void Delay_Ms_UnBlocked(unsigned int msec);
    /*********LOG相关********/
private:
    void Log_Setup();
    bool Log_to_file;
    QString log_addr;
    QTimer* log_clear_timer;
    QFile log_file;
    QTextStream log_write;
private slots:
    void Log_Add(QString index, QString target_name = QString::fromLocal8Bit("本地操作"));

    /********内存监控相关********/
private:
    bool Ram_Check_Enabled = false;
    int Ram_Check_Count = 0;
    void Plot_Setup(QCustomPlot *customPlot);
    unsigned long long ram_size_log[RAM_LOG_SIZE];   //用于记录前面5次内存消耗量。如果内存消耗都不变，有可能是发生了问题
    int ram_size_ptr = 0;
    QCPAxis *yAxis1;
    QCPAxis *yAxis2;
    QCPItemTracer *tracer = nullptr;// 跟踪的点
    QCPItemText *label = nullptr;   // 显示的数值
    QCPItemLine *arrow = nullptr;   // 箭头


    void Ram_Usage_Check(unsigned long long  ram_usage);
};

#endif // DOG_WIDGET_H
