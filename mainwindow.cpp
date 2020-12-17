///看门狗屋
/// 郑乐行
/// 用来存放多个看门狗，对一台PC上的多个程序同时进行监控。
/// 版本0.0 20201129
/// 1. 可以选择socket和thread_name两种方式对目标进行监控
/// 2. PID_Name主要是通过监控进程名字是否存在来判断是否要对目标进行启动。如果目标PID消失，肯定是已经崩溃，会将其重启
/// 3. Socket方式主要是针对程序的运行逻辑进行监控。在程序内关键动作上加入Socket消息，可以记录LOG并且通过消息来延长定时器时间
/// 一旦定时器时间归零，则认为目标程序陷入了无法恢复的错误，且没有对自身进行重启。此时需要调入外部力量进行重启。
/// 4. 可以记录上次征程退出时主窗体显示的位置，并且在下次启动时重新显示在该位置上
///
/// 版本0.1 20201213
/// 1. 加入了系统资源监控功能，可以监控CPU 内存 硬盘的使用量
/// 2. 实装优化了看门狗的Log写入文件功能
/// 20201216
/// 3. 在看门狗内加入了RAM占用统计和表格
/// 4. 修复完成了Log保存到文件和手动另存为功能
///
///
///
///
///
///
#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

     ui->setupUi(this);
     //qDebug()<<"before current="<<QDir::current()<<"  app path="<<QCoreApplication::applicationDirPath();
     QDir::setCurrent(QCoreApplication::applicationDirPath());   //将默认路径设置到当前的exe目录文件下
    //qDebug()<<"after current="<<QDir::current();

     setAttribute(Qt::WA_DeleteOnClose);       //设定窗口关闭后就会析构，不加会引起程序UI关闭后留下后台线程
     setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);


     QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
     // qDebug()<<"ini path ="<<ini_setting.fileName();
     ini_setting.beginGroup("System");


    /*************窗口初始化*****************/

    QDesktopWidget * desktop = QApplication::desktop();   //获取当前屏幕的尺寸信息
    int screen_count = desktop->screenCount(); //获取所有屏幕的个数
    QRect screen_total_size = desktop->geometry(); //获取所有屏幕总大小
    int prim_screen = desktop->primaryScreen();//获取主屏幕是第几个
    QRect prim_screen_size = desktop->geometry(); //获取所有屏幕总大小

    //确认显示条件和上次一致，则将内容重新显示到上次的位置上
    if (screen_total_size.contains(ini_setting.value("pos", QPoint(200, 200)).toPoint())  //确认显示位置位于总屏幕范围内
        && (screen_count == ini_setting.value("screen_count", 1).toInt())     //确认屏幕个数也和上次保存的一样
        && (screen_total_size == ini_setting.value("screen_total_size", QRect(0,0,0,0)).toRect())) //确认屏幕的总范围也一样
    {
        qDebug()<<"display on last pos";
        resize(ini_setting.value("size", QSize(600, 500)).toSize());
        move(ini_setting.value("pos", QPoint(200, 200)).toPoint());
    } else {
      qDebug()<<"display on default pos";
      resize(this->minimumSize());   //重新显示到第一个屏幕上的某个位置，最小尺寸
      move(200,200);
    }

    int current_screen = desktop->screenNumber(this);//获取程序所在屏幕是第几个屏幕
    QRect current_screen_size = desktop->screenGeometry(current_screen);//获取程序所在屏幕的尺寸
    qDebug()<<"start current screen = "<<current_screen<<" size= "<<current_screen_size<<" total size= "<<screen_total_size;
    qDebug()<<"start  screen_count = "<<screen_count<<" prim_screen = "<<prim_screen;


    //setWindowFlags(Qt::FramelessWindowHint);//无边框
    //setAttribute(Qt::WA_TranslucentBackground);//背景透明

    if (ini_setting.value("Window_Name").toString() != "")    //如果配置文件另外设置了名称，则使用配置文件里的
        setWindowTitle(ini_setting.value("Window_Name").toString());
    else setWindowTitle(tr("看门狗屋"));

    /**************控件显示初始化*************************/
    ui->label_Version->setText(QString(DIALOG_NAME_MAIN) + ":" + SYSTEM_VERSION);       //显示版本信息
    setAttribute(Qt::WA_DeleteOnClose);       //设定窗口关闭后就会析构
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);    //不启用关闭按钮



    Tray_Icon_Setup();   //托盘功能初始化
    /**************系统定时器初始化********************/
    ui_timer = new QTimer(this);
    ui_timer->setInterval(200);   //100ms运行一次ui刷新,主要是进度条
    connect(ui_timer,SIGNAL(timeout()),this,SLOT(ui_refresh()));
    ui_timer->start();


    /**************LOG系统功能模块初始化********************/
    QTimer::singleShot(300,this,SLOT(delay_init()));



    ini_setting.endGroup();


}

void MainWindow::delay_init()
{
    qDebug()<<"Begin Delay Init...";
    Dog_Init();



    qDebug()<<"End Delay Init...";

}



MainWindow::~MainWindow()
{
    QDesktopWidget * desktop = QApplication::desktop();
    int current_screen = desktop->screenNumber(this);//获取程序所在屏幕是第几个屏幕
    QRect current_screen_size = desktop->screenGeometry(current_screen);//获取程序所在屏幕的尺寸
    QRect screen_total_size = desktop->geometry(); //获取所有屏幕总大小
    int screen_count = desktop->screenCount(); //获取所有屏幕的个数
    int prim_screen = desktop->primaryScreen();//获取主屏幕是第几个



    qDebug()<<"close current screen = "<<current_screen<<" size= "<<current_screen_size<<" total size= "<<screen_total_size;
    qDebug()<<"close screen_count = "<<screen_count<<" prim_screen = "<<prim_screen;


    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.setValue("System/size", size());  //记录显示的位置信息
    ini_setting.setValue("System/pos", pos());
    ini_setting.setValue("System/screen_count", screen_count);   //记录显示在第几个屏幕
    ini_setting.setValue("System/screen_total_size", screen_total_size);  //记录总的屏幕大小
    ini_setting.setValue("System/dispaly_screen", current_screen);  //记录显示在第几个屏幕
    ini_setting.sync();
    qDebug()<<"close size = "<<size()<<" pos = "<<pos();



    delete ui;
}



int MainWindow::Dog_Init()
{
    qDebug()<<"Begin Dog Init...";
    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");
    QStringList temp_dog_name_list = ini_setting.childGroups();   //获取小狗名字列表
    qDebug()<<"Dog list:"<<temp_dog_name_list;

    foreach (QString dog_name,temp_dog_name_list)
    {
       int ret = Dog_Add(dog_name);
       if (ret < 0)
       {         
           continue;
       }

       alive_dog_name_list.append(dog_name);

    }


    qDebug()<<"End Dog Init...";
    return alive_dog_name_list.size();
}

//为主界面增加一个看门狗
int MainWindow::Dog_Add(QString target_name, int index)
{
    qDebug()<<"Begin Dog Add:"<<target_name;
    if (dog_map.contains(target_name))   //如果已经存在同名的看门狗，则不能新建
    {
        qDebug()<<"Dog "<<target_name<<" add failed, already existed";
        return -1;
    }

    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");
    QStringList temp_dog_name_list = ini_setting.childGroups();   //获取小狗名字列表

    if (!temp_dog_name_list.contains(target_name))
    {
       qDebug()<<"Dog "<<target_name<<" add failed, not exist in inifile";
        return -2;
    }

    ini_setting.beginGroup(target_name);  //设定好读取各种信息的位置
    Dog_Widget* dog = new Dog_Widget(this,target_name);
    connect(dog,SIGNAL(Config_Return(QString, int, QString)),this,SLOT(Dog_Config_Return(QString , int , QString)));  //链接看门狗穿回来的信号，如果配置过可能就需要将其重启
    if (index >= 0)  //如果原来有固定位置，则插入到原来的位置里，否则放到最后面去
    {
        ui->tabWidget_Dog_House->insertTab(index,dog,target_name);
    }
    else ui->tabWidget_Dog_House->addTab(dog,target_name);    //将sensor显示到界面上去

    ui->tabWidget_Dog_House->setCurrentWidget(dog);

    dog_map.insert(target_name,dog);   //加入列表
    qDebug()<<"Dog "<<target_name<<" added success";
    return 0;
}

void MainWindow::Dog_Config_Return(QString target_name, int ret, QString new_name)
{
    qDebug()<<"Dog house get "<<target_name<<"ret ="<<ret;
    int pos = -1;
    switch (ret)
    {

     case RET_DELETED:
        Dog_Delete_From_Ini(target_name); //杀掉并完全删除一个看门狗你
        break;
    case RET_SAVED:  //原样重启一个看门狗
        pos = Dog_Kill(target_name);
        if (pos >= 0)
            Dog_Add(target_name,pos);
        else Dog_Add(target_name);
        break;
    case RET_RENAMED:
        pos = Dog_Kill(target_name);  //在看门狗配置窗口中，实际已经将新的名字写入过了，这里只需要将旧的干掉重新启动新的就可以了
        if (pos >= 0)
            Dog_Add(new_name,pos);
        else Dog_Add(new_name);
        break;
    default:
        break;

    }

}

//干掉一个看门狗，但是不从文件里删除。返回狗原本所在的位置
int MainWindow::Dog_Kill(QString target_name)
{
    qDebug()<<"Try killing Dog "<<target_name;
    if (!dog_map.contains(target_name))   //如果没有同名的看门狗，则返回
    {
        qDebug()<<"Failed, "<<target_name<<"is not alive or not exist!!!";
        return -2;
    }

    Dog_Widget* dog = dog_map.value(target_name);     //获取指针
    int ret = ui->tabWidget_Dog_House->indexOf(dog);
    if (ret >= 0)
    {
        qDebug()<<"Dog exist in tabWidget, index = "<<ret;
        ui->tabWidget_Dog_House->removeTab(ui->tabWidget_Dog_House->indexOf(dog));
        dog->stop();  //关停所有工作
        dog_map.remove(target_name);  //从活动列表里删除
        delete dog; //彻底清空
        qDebug()<<"Done";
    } else  qDebug()<<"Failed, "<<target_name<<" do not exist in tabWidget!!!";
    return ret;
}

//从配置文件里彻底删除一个看门狗的信息
int MainWindow::Dog_Delete_From_Ini(QString target_name)
{
    qDebug()<<"Try deleteing Dog "<<target_name<<" from ini file";
    if (dog_map.contains(target_name))   //如果看门狗还在活动列表里，先杀掉再删除
        Dog_Kill(target_name);

    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");

    QStringList temp_dog_name_list = ini_setting.childGroups();   //获取小狗名字列表
    if (!temp_dog_name_list.contains(target_name))
    {
        qDebug()<<"Failed, dog "<<target_name<<" not found in ini file!!!";
        return -1;
    }

    ini_setting.remove(target_name);  //删掉所有有关联的Key
    qDebug()<<"Done";
    return 0;

}


bool MainWindow::Dog_Exist(QString target_name)
{
    if (dog_map.contains(target_name))   //如果已经存在同名的看门狗，则不能新建
        return true;
    else return false;

}


//根据看门狗配置窗口的返回值对看门狗进行处理
void MainWindow::Dog_Dealer(QString target_name, int index)
{
    if (!dog_map.contains(target_name))   //如果已经存在同名的看门狗，则不能新建
        return;
    Dog_Widget* dog = dog_map.value(target_name);     //获取指针
    switch (index) {
    case RET_SAVED:
        break;
    case RET_RENAMED:
        break;
    case RET_DELETED:


       break;
    case REC_CANCELED:
   default:
       break;


    }


}


void MainWindow::Tray_Icon_Setup()
{
    tray_icon = new QSystemTrayIcon;
    QIcon icon = QIcon("./image/Dog_Sleep.jpg");      //设置图标等内容
    tray_icon->setIcon(icon);
    tray_icon->setToolTip(QString::fromLocal8Bit("看门狗已经最小化"));
    connect(tray_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));   //添加响应的槽函数
    tray_icon->hide();
    tray_icon->show();
}



void MainWindow::on_pushButton_exit_clicked()
{

    qApp->exit(1);

}

void MainWindow::on_pushButton_System_Config_clicked()
{

    Dialog_System_Config *system_config_dlg = new Dialog_System_Config(this);
    int ret = system_config_dlg->exec();       //展示设置界面，模态
    //system_config_dlg->show();   //非模态

    if (RETCODE_RESTART == ret)    //如果点了确认的话，要重启程序
    {
        qDebug()<<"try to reboot program";
        qApp->exit(RETCODE_RESTART);
    } else {


    }
}

void MainWindow::on_pushButton_Minimun_clicked()
{
    this->hide();   //系统最小化
}


void MainWindow::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason){
    case QSystemTrayIcon::Trigger: //单击托盘图标

        break;
    case QSystemTrayIcon::DoubleClick://双击托盘图标 双击后显示/隐藏主程序窗口
        if (this->isHidden())
        this->show();
        else this->hide();
        break;
    default:
        break;
    }

}

void MainWindow::on_pushButton_Add_Dog_clicked()
{
    qDebug()<<"Add new dog... ";
    QString dog_name;
    Dialog_Dog_Config *dog_config_dlg = new Dialog_Dog_Config(this,"",&dog_name);
    int ret = dog_config_dlg->exec();       //展示设置界面，模态
    qDebug()<<"add ret = "<<ret<<" name = "<<dog_name;
    if (RET_SAVED == ret)   //确认保存了某个名字
        Dog_Add(dog_name);  //添加狗
   // dog_config_dlg->show();   //非模态

}


void MainWindow::ui_refresh()
{
    //system_usage.Ruquest_Usage(&current_CPU_Usage,&current_RAM_Usage,&current_Disk_Usage);  //更新显示占用率
    //qDebug()<<" cpu = "<<current_CPU_Usage<<" ram = "<<current_RAM_Usage<<" disk = "<<current_Disk_Usage;
    ui->progressBar_CPU->setValue(system_usage.CPU_Usage());
    ui->progressBar_RAM->setValue(system_usage.RAM_Usage());
    ui->progressBar_DISK->setValue(system_usage.DISK_Usage());

}



