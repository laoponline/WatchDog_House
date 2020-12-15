
#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码
#include "dialog_system_config.h"
#include "ui_dialog_system_config.h"


Dialog_System_Config::Dialog_System_Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_System_Config)
{
    ui->setupUi(this);

    setWindowTitle(QString::fromLocal8Bit("狗屋设置"));   //窗口名称
    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("System");

    if (ini_setting.value("Startup_Minimun","false").toString() == "true")             //自动最小化
         ui->checkBox_Auto_Minimun->setCheckState(Qt::Checked);
    else ui->checkBox_Auto_Minimun->setCheckState(Qt::Unchecked);


    if (ini_setting.value("Auto_Start","false").toString() == "true")             //显示心跳信号
         ui->checkBox_Auto_Start->setCheckState(Qt::Checked);
    else ui->checkBox_Auto_Start->setCheckState(Qt::Unchecked);

    ui->lineEdit_window_name->setText(QString::fromLocal8Bit(ini_setting.value("Window_Name","").toByteArray())); //窗口名称

    ini_setting.endGroup();

}

Dialog_System_Config::~Dialog_System_Config()
{
    delete ui;
}


void Dialog_System_Config::save_all()
{
    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //参数保存到设置文件
    ini_setting.beginGroup("System");

    if (Qt::Checked == ui->checkBox_Auto_Minimun->checkState()) //是否自动最小化
        ini_setting.setValue("Startup_Minimun","true");
    else ini_setting.setValue("Startup_Minimun","false");

    if (Qt::Checked == ui->checkBox_Auto_Start->checkState()) //是否自动开始工作
        ini_setting.setValue("Auto_Start","true");
    else ini_setting.setValue("Auto_Start","false");


    ini_setting.setValue("Window_Name",ui->lineEdit_window_name->text().toLocal8Bit());      //保存软件名称
    ini_setting.endGroup();
}


void Dialog_System_Config::on_pushButton_save_reset_clicked()
{
    save_all();
    done(RETCODE_RESTART);
}

void Dialog_System_Config::on_pushButton_save_clicked()
{
    save_all();
    done(0);
}

void Dialog_System_Config::on_pushButton_exit_clicked()
{
     done(0);
}
