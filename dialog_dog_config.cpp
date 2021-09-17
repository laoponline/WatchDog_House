
#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码
#include "dialog_dog_config.h"
#include "ui_dialog_dog_config.h"

Dialog_Dog_Config::Dialog_Dog_Config(QWidget *parent, QString config_name , QString* for_new_name) :
    QDialog(parent),
    ui(new Ui::Dialog_Dog_Config)
{
    ui->setupUi(this);
    new_name = for_new_name;
    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");
    QStringList temp_dog_name_list = ini_setting.childGroups();   //获取小狗名字列表

    name = config_name;
    if (!name.isEmpty() && (temp_dog_name_list.contains(name)))   //如果小狗的名字不为空，且列表中含有这个名字
    {
        old_name = name;   //记录老名字
        setWindowTitle(QString::fromLocal8Bit("看门狗%1设置").arg(name));  //设计窗体名称


        ini_setting.beginGroup(name);   //开始读取各种配置
        if (ini_setting.value("Auto_Start","false").toString() == "true")             //显示心跳信号
             ui->checkBox_Auto_Start->setCheckState(Qt::Checked);
        else ui->checkBox_Auto_Start->setCheckState(Qt::Unchecked);
        ui->lineEdit_dog_name->setText(name); //小狗名称
        ui->lineEdit_countdown_PID->setText(ini_setting.value("PID_Count_Down",COUNT_DOWN_DEFAULT).toString());
        ui->lineEdit_countdown_Socket->setText(ini_setting.value("Socket_Count_Down",COUNT_DOWN_DEFAULT).toString());

        ui->lineEdit_target_position->setText(ini_setting.value("Target_Paht").toString());
        ui->lineEdit_target_arg->setText(ini_setting.value("Target_Arg").toString());

        if (ini_setting.value("Log_To_File_Enabled","false").toString() == "true")             //LOG保存地址
             ui->checkBox_Log_Enabled->setCheckState(Qt::Checked);
        else ui->checkBox_Log_Enabled->setCheckState(Qt::Unchecked);
        ui->lineEdit_Log_Addr->setText(ini_setting.value("Log_Path",LOG_PATH_DEFAULT).toString());

        if (ini_setting.value("PID_Enabled","false").toString() == "true")             //是否使用进程监控
             ui->checkBox_PID_Enabled->setCheckState(Qt::Checked);
        else ui->checkBox_PID_Enabled->setCheckState(Qt::Unchecked);
        ui->lineEdit_target_name->setText(ini_setting.value("PID_Name","").toString());

        if (ini_setting.value("Ram_Check_Enabled","false").toString() == "true")             //是否进行Ram检测
             ui->checkBox_Ram_Check_Enabled->setCheckState(Qt::Checked);
        else ui->checkBox_Ram_Check_Enabled->setCheckState(Qt::Unchecked);
        ui->lineEdit_ram_range->setText(ini_setting.value("Ram_Range","").toString());
        ui->lineEdit_ram_duration->setText(ini_setting.value("Ram_Duration","").toString());


        if (ini_setting.value("Socket_Enabled","false").toString() == "true")             //是否启用socket监控
             ui->checkBox_Socket_Enabled->setCheckState(Qt::Checked);
        else ui->checkBox_Socket_Enabled->setCheckState(Qt::Unchecked);
        ui->lineEdit_local_port->setText(ini_setting.value("Local_Port",LOCAL_PORT_DEFAULT).toString());
        if (ini_setting.value("Socket_External_Only","false").toString() == "true")             //是否启用socket监控
             ui->checkBox_socket_for_external->setCheckState(Qt::Checked);
        else ui->checkBox_socket_for_external->setCheckState(Qt::Unchecked);


        if (ini_setting.value("Show_Heartbeat","false").toString() == "true")             //显示心跳信号
             ui->checkBox_Show_Heartbeat->setCheckState(Qt::Checked);
        else ui->checkBox_Show_Heartbeat->setCheckState(Qt::Unchecked);        
        ui->lineEdit_Log_Clear_Duration->setText(ini_setting.value("Log_Clear_Duration",LOG_CLEAR_DURATION).toString());

        ui->lineEdit_reboot_delay->setText(ini_setting.value("Reboot_Delay_Ms",REBOOT_DELAY_DEFAULT).toString());


        ini_setting.endGroup();


    }
    else {
        is_registering = true;   //没有确定的名字的话，说明正在生成一个新的看门狗
        setWindowTitle(QString::fromLocal8Bit("新建看门狗"));     
        ui->lineEdit_Log_Addr->setText(LOG_PATH_DEFAULT);   //默认内容
    }

    ini_setting.endGroup();

}

Dialog_Dog_Config::~Dialog_Dog_Config()
{
    delete ui;
}

void Dialog_Dog_Config::save_all()
{  
    name = ui->lineEdit_dog_name->text().toLocal8Bit();   //首先确定名字
    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //参数保存到设置文件
    ini_setting.beginGroup("Dogs");
    QStringList exist_dog_names = ini_setting.childGroups();   //获取小狗名字列表

    if(!is_registering && (old_name != name) && exist_dog_names.contains(old_name))   //如果不在注册中，且名字也重新改了
    {
        ini_setting.remove(old_name);  //先删除所有老的存储内容。这里暂时会让狗继续活着，等退出去以后再重新向主程序发送消息进行重启
        qDebug()<<"rename "<<old_name<<" to "<<&new_name;
        ret = RET_RENAMED;
    } else ret = RET_SAVED;

    if (new_name != nullptr)  //如果确实可以有新的名字
        *new_name = ui->lineEdit_dog_name->text();   //写入新的名字

    ini_setting.beginGroup(name);

    if (Qt::Checked == ui->checkBox_Auto_Start->checkState()) //是否自动开始工作
        ini_setting.setValue("Auto_Start","true");
    else ini_setting.setValue("Auto_Start","false");

    ini_setting.setValue("Name",ui->lineEdit_dog_name->text().toLocal8Bit());      //保存小狗名称



    ini_setting.setValue("Target_Paht",ui->lineEdit_target_position->text());  //目标位置
    ini_setting.setValue("Target_Arg",ui->lineEdit_target_arg->text());  //目标运行参数

    if (Qt::Checked == ui->checkBox_Log_Enabled->checkState())     //是否将Log保存到文件
        ini_setting.setValue("Log_To_File_Enabled","true");
    else ini_setting.setValue("Log_To_File_Enabled","false");
    ini_setting.setValue("Log_Path",ui->lineEdit_Log_Addr->text());  //Log保存位置

    if (Qt::Checked == ui->checkBox_PID_Enabled->checkState()) //pid检测
        ini_setting.setValue("PID_Enabled","true");
    else ini_setting.setValue("PID_Enabled","false");
    ini_setting.setValue("PID_Name",ui->lineEdit_target_name->text());  //目标运行参数
    ini_setting.setValue("PID_Count_Down",QString::number(ui->lineEdit_countdown_PID->text().toInt()));  //自动检查时间

    if (Qt::Checked == ui->checkBox_Ram_Check_Enabled->checkState()) //是否进行Ram检测
        ini_setting.setValue("Ram_Check_Enabled","true");
    else ini_setting.setValue("Ram_Check_Enabled","false");
    ini_setting.setValue("Ram_Range",ui->lineEdit_ram_range->text());
    ini_setting.setValue("Ram_Duration",QString::number(ui->lineEdit_ram_duration->text().toInt()));  //自动检查时间

    if (Qt::Checked == ui->checkBox_Socket_Enabled->checkState()) //Socket是否有效
        ini_setting.setValue("Socket_Enabled","true");
    else ini_setting.setValue("Socket_Enabled","false");
    if (Qt::Checked == ui->checkBox_socket_for_external->checkState()) //是否仅接受外部重启
        ini_setting.setValue("Socket_External_Only","true");
    else ini_setting.setValue("Socket_External_Only","false");
    ini_setting.setValue("Local_Port",ui->lineEdit_local_port->text());  //本地端口
    ini_setting.setValue("Socket_Count_Down",QString::number(ui->lineEdit_countdown_Socket->text().toInt()));  //自动复位时间

    if (Qt::Checked == ui->checkBox_Show_Heartbeat->checkState())  //是否显示心跳信号
        ini_setting.setValue("Show_Heartbeat","true");
    else ini_setting.setValue("Show_Heartbeat","false");
    ini_setting.setValue("Log_Clear_Duration",QString::number(ui->lineEdit_Log_Clear_Duration->text().toInt()));  //log清空时间

    ini_setting.setValue("Reboot_Delay_Ms",QString::number(ui->lineEdit_reboot_delay->text().toInt()));  //软件重启间隔


     ini_setting.endGroup();
     ini_setting.endGroup();
}


void Dialog_Dog_Config::on_pushButton_save_clicked()
{


    QString name = ui->lineEdit_dog_name->text();

    if (name.isEmpty())
    {
        QMessageBox::critical(this,tr("名字错误"),tr("需要输入一个合适的名称！"));
        return;
    }

    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //参数保存到设置文件
    ini_setting.beginGroup("Dogs");
    QStringList exist_dog_names = ini_setting.childGroups();   //获取小狗名字列表
    ini_setting.endGroup();

    QStringList key_names = ini_setting.allKeys();   //读取所有Key的名字，用于确认重复
     QStringList keys;
    foreach(QString temp,key_names)
    {
        temp = temp.right(temp.size() - temp.lastIndexOf("/") -1);
        if (!keys.contains(temp) && (!exist_dog_names.contains(temp)))  //不重复的整理一个列表,且不包含小狗的名字
            keys.append(temp);
    }
    qDebug()<<"keys:"<<keys;


    if (is_registering && exist_dog_names.contains(name))   //注册过程中，如果名字重复了
    {
        QMessageBox::critical(this,tr("名字错误"),tr("名字重复，需要一个新的名字！"));
        return;
    }

   // if (keys.contains(name))   //如果名称和已有的Key名字重复
    if(keys.filter(name).size())
    {
        QMessageBox::critical(this,tr("名字错误"),tr("名字中含有关键字，需要一个新的名字！"));
        return;
    }

    if(name.contains('/') || name.contains('\\'))
    {
        QMessageBox::critical(this,tr("名字错误"),tr("名字包含非法字符，需要一个新的名字！"));
        return;
    }



    if(ui->lineEdit_target_position->text().isEmpty())
    {
        QMessageBox::critical(this,tr("目标错误"),tr("未填写正确的可执行文件位置！"));
        return;
    } else {
        QDir dir;

        if (!dir.exists(ui->lineEdit_target_position->text()))          //不存在指定文件
           {
                QMessageBox::critical(this,tr("目标错误"),tr("可执行文件不存在！"));
                return;
           }
    }

    if (!ui->checkBox_PID_Enabled->isChecked() && !ui->checkBox_Socket_Enabled->isChecked())
    {
        QMessageBox::critical(this,tr("选择方式"),tr("必须至少选择一种监控方式！"));
        return;
    }


    if (ui->checkBox_PID_Enabled->isChecked() && ui->lineEdit_target_name->text().isEmpty())
    {
        QMessageBox::critical(this,tr("信息不足"),tr("必须填写正确的目标PID名称！"));
        return;
    }

    if (ui->checkBox_Socket_Enabled->isChecked() && (ui->lineEdit_local_port->text().isEmpty() || ui->lineEdit_local_port->text().toInt() > 65535 || ui->lineEdit_local_port->text().toInt() < 0))
    {
        QMessageBox::critical(this,tr("信息错误"),tr("必须填写正确的端口！"));
        return;
    }

    if (ui->lineEdit_local_port->text().toInt() > 65535 || ui->lineEdit_local_port->text().toInt() < 0)
    {
        QMessageBox::critical(this,tr("信息错误"),tr("必须填写正确的端口！"));
        return;
    }


    save_all();  //确认都不重复了再保存
    done(ret);
}

void Dialog_Dog_Config::on_pushButton_exit_clicked()
{
    done(REC_CANCELED);
}



void Dialog_Dog_Config::on_checkBox_PID_Enabled_stateChanged(int arg1)
{
//    if (ui->checkBox_PID_Enabled->isChecked())
//    {
//        ui->checkBox_Socket_Enabled->setCheckState(Qt::Unchecked);
//        ui->checkBox_Socket_Enabled->setEnabled(false);
//        ui->lineEdit_local_port->setEnabled(false);
//    } else {
//        ui->checkBox_Socket_Enabled->setEnabled(true);
//        ui->lineEdit_local_port->setEnabled(true);
//    }
}

void Dialog_Dog_Config::on_checkBox_Socket_Enabled_stateChanged(int arg1)
{
//    if (ui->checkBox_Socket_Enabled->isChecked())
//    {
//        ui->checkBox_PID_Enabled->setCheckState(Qt::Unchecked);
//        ui->checkBox_PID_Enabled->setEnabled(false);
//        ui->lineEdit_target_name->setEnabled(false);
//    } else {
//        ui->checkBox_PID_Enabled->setEnabled(true);
//        ui->lineEdit_target_name->setEnabled(true);
//    }
}

void Dialog_Dog_Config::on_pushButton_Target_Dir_Set_clicked()
{
    QFileDialog *fileDialog = new QFileDialog;

    fileDialog->setWindowTitle(tr("选择目标文件"));//设置文件保存对话框的标题
    fileDialog->setNameFilter("*.exe");   //只能选择exe文件
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);//设置文件对话框为保存模式
    fileDialog->setFileMode(QFileDialog::ExistingFile);//设置为选择已经存在的文件
    fileDialog->setViewMode(QFileDialog::Detail);//文件以详细的形式显示，显示文件名，大小，创建日期等信息；

    fileDialog->setDirectory(ui->lineEdit_target_position->text());//设置文件对话框打开时初始打开的位置

    if(fileDialog->exec() == QDialog::Accepted) //注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
    {
        QString path = fileDialog->selectedFiles()[0];//得到用户选择的文件名
        qDebug()<<"select path = "<<path;
        ui->lineEdit_target_position->setText(path);     //显示图片和路径
        QString file_name = QFileInfo(path).fileName();
        ui->lineEdit_target_name->setText(file_name);  // 截取文件名作为进程名
        if (ui->lineEdit_dog_name->text().isEmpty())
        ui->lineEdit_dog_name->setText("Dog for "+file_name);
    }
}







void Dialog_Dog_Config::on_pushButton_Log_Dir_Set_clicked()
{
    QFileDialog *fileDialog = new QFileDialog;

    fileDialog->setWindowTitle(tr("Select log dir"));//设置文件保存对话框的标题
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);//设置文件对话框为保存模式
    fileDialog->setFileMode(QFileDialog::Directory);//设置为选择目录
    //fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    fileDialog->setViewMode(QFileDialog::Detail);//文件以详细的形式显示，显示文件名，大小，创建日期等信息；

    fileDialog->setDirectory(ui->lineEdit_Log_Addr->text());//设置文件对话框打开时初始打开的位置

    if(fileDialog->exec() == QDialog::Accepted) //注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
       {
            QString path = fileDialog->selectedFiles()[0];//得到用户选择的文件名
             qDebug()<<"select path = "<<path;
                ui->lineEdit_Log_Addr->setText(path);     //显示图片和路径
       }
}

void Dialog_Dog_Config::on_pushButton_delete_clicked()
{
    done(RET_DELETED);   //删除当前的这个看门狗
}
