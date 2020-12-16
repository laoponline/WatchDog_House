#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码

#ifndef DIALOG_DOG_CONFIG_H
#define DIALOG_DOG_CONFIG_H

#include <QDialog>
#include <globle_define.h>

#define REC_CANCELED 0
#define RET_SAVED 1
#define RET_RENAMED 2
#define RET_DELETED 3


namespace Ui {
class Dialog_Dog_Config;
}

class Dialog_Dog_Config : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Dog_Config(QWidget *parent = nullptr , QString config_name = "" , QString* for_new_name = nullptr);
    ~Dialog_Dog_Config();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_exit_clicked();

    void on_checkBox_PID_Enabled_stateChanged(int arg1);

    void on_checkBox_Socket_Enabled_stateChanged(int arg1);

    void on_pushButton_Target_Dir_Set_clicked();

    void on_pushButton_Log_Dir_Set_clicked();

    void on_pushButton_delete_clicked();

private:
    QString name;
    QString old_name;
    QString* new_name;
    bool is_registering = false;   //标记当前是否正在生成一个新的看门狗
    QStringList exist_names;

    int ret;



    Ui::Dialog_Dog_Config *ui;


    void save_all();
};

#endif // DIALOG_DOG_CONFIG_H
