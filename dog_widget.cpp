/// 对于PID，仅仅检查目标的存在性。（如果以后改进，可以针对目标的CPU占用率和内存占用进行检查，如果长时间不动则认为已经卡死）
/// 对于Socket，可以用来记录一些目标关键LOG，以备以后检查目标程序卡死位置。
/// 同时，一定时间没有接收到Socket心跳信号以后，则认为目标已经假死或者进入功能、逻辑的死循环
///
///
///
///

#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码
#include "dog_widget.h"
#include "ui_dog_widget.h"

Dog_Widget::Dog_Widget(QWidget *parent, QString config_name) :
    QWidget(parent),
    ui(new Ui::Dog_Widget)
{
    ui->setupUi(this);
    my_name = config_name ;

    Log_Setup();  //首先设定LOG

    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");
    ini_setting.beginGroup(my_name);


    /**************系统参数初始化********************/
    int log_clear_time = ini_setting.value("Log_Clear_Duration",3600).toInt();
    reboot_delay = ini_setting.value("Reboot_Delay_Ms",REBOOT_DELAY_DEFAULT).toInt();




    /**************控件显示初始化*************************/
    Plot_Setup(ui->widget_graphicsView);

    ui->label_Version->setText(tr("Ver:%1").arg(DOG_VERSION));

    ui->lineEdit_target_position->setText(ini_setting.value("Target_Paht").toString());
    ui->lineEdit_target_arg->setText(ini_setting.value("Target_Arg").toString());

    if (ini_setting.value("Show_Heartbeat","false").toString() == "true")             //显示心跳信号
         ui->checkBox_show_heartbeat->setCheckState(Qt::Checked);
    else ui->checkBox_show_heartbeat->setCheckState(Qt::Unchecked);


   // ui->label_image->setPixmap(QPixmap::fromImage(QImage(":/image/Dog_Sleep.png")).scaled(ui->label_image->size()));  //显示图片


    ui->progressBar_PID->setMaximum(100);  //进度条的最大最小值
    ui->progressBar_PID->setMinimum(0);
    ui->progressBar_socket->setMaximum(100);  //进度条的最大最小值
    ui->progressBar_socket->setMinimum(0);

    state = STATE_IDLE;

    /*************系统定时器初始化*****************/


    log_clear_timer = new QTimer(this);
    if (log_clear_time)
    {
        log_clear_timer->setInterval(log_clear_time*1000);   //隔一段时间清除一次串口数据记录
        connect(log_clear_timer,SIGNAL(timeout()),this,SLOT(on_pushButton_Log_Clear_clicked()));
        log_clear_timer->start();
    }



    //Log_Add(tr("已启用PID监控"));
    pid_watch_timer = new QTimer(this);
    socket_watch_timer = new QTimer(this);
//    pid_watch_timer->setInterval(count_down*1000);   //隔一段时间清除一次串口数据记录
//    connect(pid_watch_timer,SIGNAL(timeout()),this,SLOT(PID_Watch()));
//    pid_watch_timer->start();


     ui_timer = new QTimer(this);
     ui_timer->setInterval(100);   //100ms运行一次ui刷新,主要是进度条
     connect(ui_timer,SIGNAL(timeout()),this,SLOT(ui_refresh()));
     ui_timer->start();

     connect(&pid_countdown_timer,SIGNAL(timeout()),this,SLOT(PID_Timeout()));
     connect(&socket_countdown_timer,SIGNAL(timeout()),this,SLOT(Socket_Timeout()));

    qDebug()<<"pid enabled = "<<ini_setting.value("PID_Enabled","false").toString();
    qDebug()<<"socket enabled = "<<ini_setting.value("Socket_Enabled","false").toString();
    /*************开始前的功能启动*****************/

    if ((ini_setting.value("PID_Enabled","false").toString() == "false") && (ini_setting.value("Socket_Enabled","false").toString() == "false"))  //如果socket和pid都没有启动的话
    {
       Log_Add(tr("未启用任何监听，请重新配置"));
       ui->pushButton_Start->setEnabled(false);

    } else {

       if (ini_setting.value("PID_Enabled","false").toString() == "true")   //使用PID名称进行监控
       {
           ui->lineEdit_target_name->setText(ini_setting.value("PID_Name").toString());
           ui->lineEdit_countdown_PID->setText(ini_setting.value("PID_Count_Down",COUNT_DOWN_DEFAULT).toString());    //界面上的各种参数显示
           pid_enabled = true;
       } else {
           ui->lineEdit_target_name->setEnabled(false);
           ui->lineEdit_countdown_PID->setEnabled(false);
           ui->progressBar_PID->setValue(0);
           pid_enabled = false;
       }


       if (ini_setting.value("Socket_Enabled","false").toString() == "true") //使用Socket方式进行监控
       {
           ui->lineEdit_Dog_port->setText(ini_setting.value("Local_Port",LOCAL_PORT_DEFAULT).toString());
           ui->lineEdit_countdown_socket->setText(ini_setting.value("Socket_Count_Down",COUNT_DOWN_DEFAULT).toString());    //界面上的各种参数显示
           socket_enabled = true;
           if (ini_setting.value("Socket_External_Only","false").toString() == "true")
            socket_external_only = true;
           else socket_external_only = false;


       } else {
           ui->lineEdit_Dog_port->setEnabled(false);
           ui->lineEdit_countdown_socket->setEnabled(false);
           ui->progressBar_socket->setValue(0);
           socket_enabled = false;
       }


       if (ini_setting.value("Ram_Check_Enabled","false").toString() == "true")             //是否进行Ram检测
       {
           Ram_Check_Enabled = true;
           int pid_countdown = ini_setting.value("PID_Count_Down",COUNT_DOWN_DEFAULT).toInt();
           int ram_duration = ini_setting.value("Ram_Duration",COUNT_DOWN_DEFAULT).toInt();
           Ram_Check_Count = ram_duration / pid_countdown;  //计算需要统计的次数
           qDebug()<<"Ram check enabled, check count = "<<Ram_Check_Count;
           if (Ram_Check_Count > RAM_LOG_SIZE)  //不能超过一定范围
               Ram_Check_Count = RAM_LOG_SIZE;
       }

       if (ini_setting.value("Auto_Start","false").toString() == "true")             //如果需要自动启动，直接通过按钮实现
           on_pushButton_Start_clicked();

    }

    ini_setting.endGroup();
    ini_setting.endGroup();
}

Dog_Widget::~Dog_Widget()
{
    closing_window = true;
    if (STATE_WORKING == state)  //如果还在工作，则要先关闭断开
        on_pushButton_Start_clicked();

    delete ui;
}

//外部调用的停止指令
void Dog_Widget::stop()
{
    if (STATE_WORKING == state)  //如果还在工作，则要先关闭断开
        on_pushButton_Start_clicked();
}


//停止当前的工作，准备被删除
void Dog_Widget::Stop_Working()
{

    state = STATE_IDLE;

}



//对目标进行重启
void Dog_Widget::reset_target(QString* return_log)
{
    QString log;
    QString target_name = ui->lineEdit_target_name->text();
    QProcess *p = new QProcess(this);

    {
        p->start(QString("tasklist"));
        p->waitForFinished();
        QString temp =  p->readAllStandardOutput();
        if (temp.contains(target_name))       //寻找目前是否存在目标名称的进程，有的话才结束
          {
            Socket_Feed();  //喂狗，防止几个看门狗连续到期引发多次重启
            PID_Feed();

            log +=  target_name + tr("已找到，正在结束...");
            p->start(QString("taskkill /im %1 /f").arg(target_name));   //根据目标进程名称关闭程序
            p->waitForFinished();
            QString temp =  QString(p->readAllStandardOutput());
            log += temp;
          }
        else log += target_name + tr("不存在；");
    }


    QString path = ui->lineEdit_target_position->text(); //获取目标程序的位置
    QString arg = ui->lineEdit_target_arg->text();   //运行参数
    QString dir = QDir(path).absolutePath();
    path += " " + arg;
    qDebug()<<"full path with arg = "<<path;
    qDebug()<<"full dir  = "<<dir;

    Delay_Ms_UnBlocked(reboot_delay);

    log += tr("启动程序...");

    if (p->startDetached(path))    //启动目标程序。如果不用detached，会导致看门狗阻塞，等到目标程序退出后才会继续。
    {
        log += tr("成功！");

        p->start(QString("tasklist"));
        p->waitForFinished();
        QString temp =  p->readAllStandardOutput();
        QStringList all_process = temp.split("\r");      //将查找进程是否存在的列表进行分割，进一步寻找进程名称
        //qDebug()<<"line_count="<<all_process.size();

        foreach(QString line,all_process)
        {
            if (line.contains(target_name))
            {
                qDebug()<<line;
                QStringList line_index = line.split(' ',QString::SkipEmptyParts); //空白部分去除的字符串分割
                  qDebug()<<line_index.at(1);
                 ui->lineEdit_target_pid->setText(line_index.at(1));
                break;
            }

        }

        log += "pid="+ui->lineEdit_target_pid->text();
    }
    else {

         log += tr("失败！error=") + p->errorString();
    }

    *return_log += log;

    delete p;
}



void Dog_Widget::ui_refresh()
{

    if (pid_enabled)    //如果需要Pid监控,则刷新进度条为剩余存活时间
    {
        int left_ms = pid_countdown_timer.remainingTime();
        ui->progressBar_PID->setValue(left_ms/(ui->lineEdit_countdown_PID->text().toInt()*10));
    }

    if (socket_enabled && !socket_external_only)    //如果需要Pid监控,则刷新进度条为剩余存活时间
    {
        int left_ms = socket_countdown_timer.remainingTime();
        ui->progressBar_socket->setValue(left_ms/(ui->lineEdit_countdown_socket->text().toInt()*10));
    }

    if (my_socket != nullptr)
    {
        if (my_socket->isOpen())  //如果socket已经连接了
        {
            ui->lineEdit_target_IP->setText(my_socket->peerAddress().toString());  //显示数据
            ui->lineEdit_target_port->setText(QString::number( my_socket->peerPort()));
        } else {
            ui->lineEdit_target_IP->setText("");
            ui->lineEdit_target_port->setText("");
        }
    } else {
        ui->lineEdit_target_IP->setText("");
        ui->lineEdit_target_port->setText("");
    }

}




/**********绘图相关的函数************/
void Dog_Widget::Plot_Setup(QCustomPlot *customPlot)
{
    qDebug("sensor Setup Plot: start...");

//    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
//    if (ini_setting.value("Plot/AutoScale","false").toString() == "true")    //图标高度自动调整
//    Plot_AutoScale = true;
//    else Plot_AutoScale = false;

    //移除默认坐标轴
   // customPlot->xAxis->setVisible(false);
    customPlot->yAxis->setVisible(false);

    //添加坐标轴
   // xAxis = customPlot->axisRect()->addAxis(QCPAxis::atBottom);
    yAxis1 = customPlot->axisRect()->addAxis(QCPAxis::atLeft);   //显示RAM的坐标
    yAxis1->setVisible(true);
    yAxis1->setLabel("RAM(kb)");
    yAxis1->setTickLabelSide(QCPAxis::lsInside);    //数字显示在内侧

    yAxis2 = customPlot->axisRect()->addAxis(QCPAxis::atRight);   //显示CPU的坐标
    yAxis2->setVisible(true);
    yAxis2->setLabel("CPU");
    yAxis2->setTickLabelSide(QCPAxis::lsInside);    //数字显示在内侧



    //更改横轴为时间轴
    //customPlot->xAxis->setLabel("Time"); //设置X轴的标签
    //customPlot->xAxis->setTickLabelSide(QCPAxis::lsInside);    //数字显示在内侧
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);  //设计一个指向新的QCPAxisTickerTime的指针
    timeTicker->setTimeFormat("%d-%h:%m:%s");                       //时间显示的格式
    timeTicker->setTickCount(10);
    customPlot->xAxis->setTickLabelRotation(20);                                           //调整显示角度
    customPlot->xAxis->setTicker(timeTicker);



    //添加图形
     QPalette pa;

    customPlot->addGraph(customPlot->xAxis,yAxis1);
    customPlot->graph(0)->setPen(QPen(QColor(255, 0, 0))); // 红色显示RAM占用
    customPlot->graph(0)->setAdaptiveSampling(true);
    customPlot->graph(0)->setName("Ram");
    yAxis1->setRange(0,100000);



    customPlot->addGraph(customPlot->xAxis,yAxis2);
    customPlot->graph(1)->setPen(QPen(QColor(0, 0, 255))); // 蓝色显示RCUP占用
    customPlot->graph(1)->setAdaptiveSampling(true);
    customPlot->graph(1)->setName("Cpu");
    yAxis2->setRange(0,100);

    customPlot->setBackground(QColor(255,255,255));             //设置背景颜色
    customPlot->legend->setBrush(QColor(255,255,255,0));//legend背景色设为白色但背景透明，允许图像在legend区域可见
    customPlot->legend->setVisible(true);   //显示图例


    customPlot->replot(QCustomPlot::rpQueuedReplot);

    tracer = new QCPItemTracer(customPlot);
    tracer->setGraph(customPlot->graph(0)); //游标设置为ram，保证每个数据都会有
    tracer->setStyle(QCPItemTracer::tsCrosshair);  //游标形状
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(Qt::red);
    tracer->setSize(10);

    label = new QCPItemText(customPlot);
    label->setLayer("overlay");
    label->setLayer(customPlot->graph(0)->layer());
    label->setClipToAxisRect(false);
    label->setPadding(QMargins(5, 5, 5, 5));
    label->setBrush(QBrush(QColor(244, 244, 244, 100)));
    label->setPen(QPen(Qt::blue));
    label->position->setParentAnchor(tracer->position);
    label->setFont(QFont("宋体", 8));

    arrow = new QCPItemLine(customPlot);
    arrow->setLayer("overlay");
    arrow->setClipToAxisRect(false);
    arrow->setHead(QCPLineEnding::esSpikeArrow);


    tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
    tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

    label->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    arrow->end->setParentAnchor(tracer->position);
    arrow->start->setParentAnchor(label->position);



    tracer->setVisible(true);
    label->setVisible(true);
    arrow->setVisible(true);


    connect(customPlot, SIGNAL(mouseMove(QMouseEvent*)), this,SLOT(SLOT_Show_Tracer(QMouseEvent*)));
    //connect(customPlot, SIGNAL(mousePress(QMouseEvent*)), this,SLOT(SLOT_Show_Tracer(QMouseEvent*)));

     qDebug("sensor Setup Plot: end...");
}



void Dog_Widget::Plot_Add_Data(int type, QVariant data)
{
     double second_start_from_month;


        QDateTime current_time = QDateTime::currentDateTime(); //现在时间
        //current_data.DateTime = current_time.toString("yyyy-MM-dd hh:mm:ss");
        QDateTime month_start_time = QDateTime::currentDateTime();   //本月开始时间
        QDate month_start_date = current_time.date();
        month_start_date.setDate(month_start_date.year(),month_start_date.month(),1);  //设置到本月一号
        month_start_time.setDate(month_start_date);     //设置到本月一号的0点0分
        QTime time_0(0,0);
        month_start_time.setTime(time_0);

        second_start_from_month = current_time.toSecsSinceEpoch()-month_start_time.toSecsSinceEpoch(); //从本月初到现在的秒数
        second_start_from_month += 60*60*24;     //为了不显示0号，直接显示1号
         qDebug()<<"Plot_Add_Data: second start from month = "<<second_start_from_month;



    switch (type)
    {
     case DATA_TYPE_CPU_USAGE :
        ui->widget_graphicsView->graph(1)->addData(second_start_from_month,data.toDouble());
        qDebug()<<"add cpu usage data:"<<data.toDouble();
        yAxis2->rescale();  //气压
        yAxis2->setRangeLower(0);
        yAxis2->setRangeUpper(yAxis2->range().upper+5);
          break;

     case DATA_TYPE_RAM_USAGE :
        ui->widget_graphicsView->graph(0)->addData(second_start_from_month,data.toDouble());
        qDebug()<<"add ram usage data:"<<data.toDouble();
        yAxis1->rescale();     //Ram
        yAxis1->setRangeLower(yAxis1->range().lower-10);
        yAxis1->setRangeUpper(yAxis1->range().upper+10);
        break;

    default:

        break;

    }

    int time = QDateTime::currentDateTime().toSecsSinceEpoch() - month_start_time.toSecsSinceEpoch(); //从本月初到现在的秒数
    time += 60*60*24;     //为了不显示0号，直接显示1号



    Plot_X_AtuoRange(ui->widget_graphicsView,second_start_from_month);  //重新绘图并调整X轴
    ui->widget_graphicsView->replot(QCustomPlot::rpQueuedReplot);  //重新绘制
}


void Dog_Widget::Plot_X_AtuoRange(QCustomPlot *customPlot,double time)   //用于重新计算X轴的range,让X轴永远保持指定长度，在长度不足的情况下则自动拉伸。time是最新一个数据的时间参数。
{

    if ((time - customPlot->graph(0)->dataMainKey(0)) < TABLE_TIME_PERIOD_SEC)
    {
        customPlot->xAxis->setRange(customPlot->graph(0)->dataMainKey(0), time - customPlot->graph(0)->dataMainKey(0), Qt::AlignLeft);  //如果目前距离最早一个数据不超过xxx秒，则显示范围设置为全部范围
    }
    else {


        for (int i = 0; i < customPlot->graphCount(); ++i)  //移除过老的数据
        {

            if (customPlot->graph(i)->dataMainKey(0) > 0.1 && (time - customPlot->graph(i)->dataMainKey(0)) > TABLE_TIME_PERIOD_SEC)        //如果其中一个数据的长度超标了
            {
                int count = customPlot->graph(i)->dataCount();
                customPlot->graph(i)->data()->removeBefore(customPlot->graph(i)->dataMainKey(count-1) - TABLE_TIME_PERIOD_SEC);  // 移除最前面的数据
            }

        }

        customPlot->xAxis->setRange(customPlot->graph(0)->dataMainKey(0), TABLE_TIME_PERIOD_SEC, Qt::AlignLeft);  //如果超过xxx秒,则显示固定范围
        //customPlot->graph(0)->data()->removeBefore(customPlot->graph(0)->dataMainKey(0)+1);  // 移除最前面的数据
        }


}


void Dog_Widget::SLOT_Show_Tracer(QMouseEvent* event)
{

    QCustomPlot *customPlot = ui->widget_graphicsView;
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double x_pos = x;

    //for(int i=0;i<1;i++)//ui->widget9->graph(0)->dataCount()
    //{
    double cpu_usage = 0;
    double ram_usage = 0;

    QSharedPointer<QCPGraphDataContainer> tmpContainer;
    tmpContainer = tracer->graph()->data();
    //使用二分法快速查找所在点数据！！！敲黑板，下边这段是重点
    int low = 0, high = tmpContainer->size();
    while(high > low)
    {
        int middle = (low + high) / 2;
        if(x < tmpContainer->constBegin()->mainKey() ||         //如果在范围之外
                x > (tmpContainer->constEnd()-1)->mainKey())
            break;

        if(x == (tmpContainer->constBegin() + middle)->mainKey())      //此时正好落在某一个数据点上
        {
            ram_usage = (customPlot->graph(0)->data()->constBegin() + middle)->mainValue();
            cpu_usage = (customPlot->graph(1)->data()->constBegin() + middle)->mainValue();
            break;
        }
        if(x > (tmpContainer->constBegin() + middle)->mainKey())
        {
            low = middle;
        }
        else if(x < (tmpContainer->constBegin() + middle)->mainKey())
        {
            high = middle;
        }


        if(high - low <= 1)     //如果落在某两个点之间，需要插值计算所在位置数据
        {
            x = (tmpContainer->constBegin() + low)->mainKey();//选取较小值 不会做插值计算
            ram_usage = (customPlot->graph(0)->data()->constBegin() + low)->mainValue();
            cpu_usage = (customPlot->graph(1)->data()->constBegin() + low)->mainValue();
        }

    }

    QString x_datetime = QDateTime::fromTime_t(x).toString("hh:mm:ss");   //将X轴的数字转换成可读的时间
    //tracer->position->setCoords(x, temp);
    tracer->setGraphKey(x);    //设置游标的X轴位置
    tracer->updatePosition();
    //label->position->setCoords(-10, customPlot->yAxis->range().center());  //
                   if (x < customPlot->xAxis->range().center())   //根据X的位置，对标签进行左右偏移
                   {
                       label->position->setCoords(20, customPlot->yAxis->range().center());
                      // arrow->start->setCoords(0,0);
                   }//
                   else
                   {
                       label->position->setCoords(-130, customPlot->yAxis->range().center());  //
                      // arrow->start->setCoords(customPlot->xAxis->range().size()/6,0);
                   }

    label->setText(QString("%1\r\n ram_usage:%2kb\r\n cpu_usage:%3\r\n").arg(tr("详细数据")).arg(ram_usage,2,'g').arg((int)cpu_usage));//显示游标内容
    //}
    customPlot->replot();
}



/**********LOG相关的函数************************/
void Dog_Widget::Log_Setup()       //设置系统log
{
    qDebug("Setup Log: start...");

    QSettings ini_setting(INT_PATH,QSettings::IniFormat);   //设置文件读取
    ini_setting.beginGroup("Dogs");
    ini_setting.beginGroup(my_name);

     qDebug()<<"Setup Log: log to file enable = "<<ini_setting.value("Log_To_File_Enabled","false").toString();
    if (ini_setting.value("Log_To_File_Enabled","false").toString() == "true")   //如果确认要进行LOG文件记录
    {
        QString log_path = ini_setting.value("Log_Path",LOG_PATH_DEFAULT).toString();
        if (log_path == LOG_PATH_DEFAULT)
         ini_setting.setValue("Log_Path",LOG_PATH_DEFAULT);

        log_addr = log_path + my_name + QDate::currentDate().toString("yyyy-M");   //LOG的路径组成为 log/名称/月份
        qDebug()<<"Setup Log: log file addr = "<<log_addr;
        QDir log_dir;

        if (!log_dir.exists(log_addr))          //不存在目录则创建目录
           {
                log_dir.mkpath(log_addr);
                qDebug()<<"Setup Log: mkpath ";
           }
        else qDebug()<<"Setup Log: path exists ";

        if (log_file.isOpen())
        {
            log_file.flush();
            log_file.close();
        }
        log_file.setFileName(log_addr + QDate::currentDate().toString("/yyyy-MM-dd-")+"log.txt");
        bool open_success = log_file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);   //跳到最后，并且使用Text,这样endl会自动换行
        qDebug()<<"Setup Log: file open = "<<open_success;
        if (open_success)
        {
            log_write.setDevice(&log_file);
        } else log_file.close();
    }

    ini_setting.endGroup();
    ini_setting.endGroup();
    qDebug("Setup Log: end...");
}




void Dog_Widget::Log_Add(QString index, QString target_name)  //写入LOG用
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString write(target_name + datetime.toString(":MM-dd hh:mm:ss ") + index);
    ui->textBrowser_Log->append(write);

    if (log_file.isOpen())
    {
        log_write<<write<<endl;
    }

}



/**********PID检查相关的内容*************/

//PID的超时，仅仅检查PID是否存在作为是否要重新启动目标的依据
void Dog_Widget::PID_Timeout()
{
    qDebug("Countdown triggered");

    pid_countdown_timer.stop();     //重新设置时间
    pid_countdown_timer.setInterval(ui->lineEdit_countdown_PID->text().toInt()*1000); //读取时间
    pid_countdown_timer.start();



        QString target_name = ui->lineEdit_target_name->text();
        QProcess *p = new QProcess(this);


            p->start(QString("tasklist"));
            p->waitForFinished();
            QString temp =  p->readAllStandardOutput();
            if (!temp.contains(target_name))       //寻找目前是否存在目标名称的进程，没有的话要启动
            {
                reseting = true;
                Socket_Feed();  //在重启的时候
                QString log = tr("PID看门狗复位：");
                reset_target(&log);
                Log_Add(log);
                reseting = false;
            }
            else
            {

                QStringList all_process = temp.split("\r");      //将查找进程是否存在的列表进行分割，进一步寻找进程名称
                foreach(QString line,all_process)
                {
                    if (line.contains(target_name))
                    {
                        qDebug()<<line;
                        QStringList line_index = line.split(' ',QString::SkipEmptyParts); //空白部分去除的字符串分割
                          qDebug()<<"pid = "<<line_index.at(1)<<" Ram = "<<line_index.at(4);
                         ui->lineEdit_target_pid->setText(line_index.at(1));  //显示PID

                         QString ram_data = line_index.at(4);
                         ram_data.remove(',');  //去掉数字里的逗号
                         unsigned long long  ram_usage = ram_data.toInt(); //计算Ram使用

                         Ram_Usage_Check(ram_usage);  //Ram占用检测
                        break;
                    }

                }

            }
            delete p;

}


//Ram占用检测，如果超过一定时间内存占用未变化，则说明程序假死了，需要重启
void Dog_Widget::Ram_Usage_Check(unsigned long long  ram_usage)
{

    Plot_Add_Data(DATA_TYPE_RAM_USAGE, ram_usage / 1000.0); //绘图

    if (Ram_Check_Enabled)  //如果要Check的话，有问题就要重启
    {
        ram_size_log[ram_size_ptr] = ram_usage;   //记录最近几个RAM消耗
        qDebug("ram_size_log[%d] = %llu",ram_size_ptr,ram_size_log[ram_size_ptr]);
        ram_size_ptr = (ram_size_ptr+1) % Ram_Check_Count;  //回环

        unsigned long long  temp_ram_usage = ram_size_log[0];  //选取第一个进行对比
        int i = 1;
        for (;i<Ram_Check_Count;i++)  //遍历，如果有不一样的内存占用，就跳出
        {
            if (temp_ram_usage != ram_size_log[i])
                break;
        }

        if (i == Ram_Check_Count)  //如果等于，则说明没有找到不一样的RAM占用值，说明RAM占用一直没变
        {
            Log_Add(tr("内存占用%1连续%2次未变化").arg(temp_ram_usage).arg(Ram_Check_Count));
            QString log = tr("Ram统计看门狗复位：");
            reset_target(&log);
            Log_Add(log);
        }
    }

}

void Dog_Widget::PID_Feed(QString target_name)
{
    if ((STATE_WORKING == state) && pid_countdown_timer.isActive())   //如果狗正在工作，重置喂狗倒计时器
        pid_countdown_timer.start();

    if (ui->checkBox_show_heartbeat->isChecked())    //如果需要显示心跳信号
        Log_Add(tr("收到PID心跳信号"),target_name);
}




/************Socket相关******************/
///新Socket连接进入的处理函数。如果当前没有已经建立的Socket连接，则放进新连接
///如果当前已经存在有效的连接，则放弃之前的，重新建立（之前的可能已经被重启了）
void Dog_Widget::Get_Connection()
{
    if (!my_server.hasPendingConnections())
    return;

    Socket_Disconnected();   //先尝试消除旧的连接

    my_socket = my_server.nextPendingConnection();   //获取新连接
    connect(my_socket, SIGNAL(readyRead()), this, SLOT(Socket_Read())); // 会移进线程里
    connect(my_socket, SIGNAL(disconnected()), this, SLOT(Socket_Disconnected()));     //子线程断开的时候会进行处理，删除对应的列表项
    Log_Add(tr("Socket接收到新链接：%1:%2").arg(my_socket->peerAddress().toString()).arg(my_socket->peerPort()));
    ui->lineEdit_Dog_IP->setText(my_socket->localAddress().toString());  //显示本地的IP
    ui->lineEdit_Dog_port->setText(QString::number(my_socket->localPort()));   //显示本地实际侦听的端口

    ui->lineEdit_target_IP->setText(my_socket->peerAddress().toString());  //显示数据
    ui->lineEdit_target_port->setText(QString::number( my_socket->peerPort()));
    Socket_Feed();   //主动喂狗，防止上来就发生重启
}

//Socket超时，不论PID是否存在，都要重新启动目标
void Dog_Widget::Socket_Timeout()
{

    socket_countdown_timer.setInterval(ui->lineEdit_countdown_socket->text().toInt()*1000);  //必须重新设置，不然会变成left_over + ms
    PID_Feed(tr("本地"));  //防止发生PID重启
    QString log = tr("Socket看门狗复位：");
    reset_target(&log);
    Log_Add(log);

}


//Socket喂狗函数
void Dog_Widget::Socket_Feed(int ms)
{

    if (socket_external_only)  //socket仅用于外部重启的时候，则不需要Socket喂狗了
        return;

    if ((STATE_WORKING == state) && socket_countdown_timer.isActive())   //如果狗正在工作，重置喂狗倒计时器
    {
        if (ms)   //如果有指定时间，则认为要延长指定的ms数
        {
            int left_over = socket_countdown_timer.remainingTime();
            qDebug()<<"left over = "<<left_over<<" add "<<ms<<", interval = "<<socket_countdown_timer.interval();
            if ((left_over + ms) > ui->lineEdit_countdown_socket->text().toInt()*1000)  //如果合计时长大于总时长，不得超过总时长
                socket_countdown_timer.start();
            else {
                socket_countdown_timer.start(left_over + ms);   //设定为合计时长
            }
        }
        else socket_countdown_timer.start(ui->lineEdit_countdown_socket->text().toInt()*1000);

    }

}



void Dog_Widget::Socket_Disconnected()
{
 static bool socket_disconnecting = false;   //防止重入，会调用野指针


    if ((my_socket != nullptr) && (!socket_disconnecting))
    {
        socket_disconnecting = true;
        qDebug()<<"socket disconnected";
        Log_Add(tr("Socket连接断开"));
        my_socket->disconnectFromHost();
        my_socket->close();

        if (my_server.isListening())
        {
            ui->lineEdit_Dog_IP->setText(my_server.serverAddress().toString());  //显示本地的IP
            ui->lineEdit_Dog_port->setText(QString::number( my_server.serverPort()));   //显示本地实际侦听的端口
        } else {
            ui->lineEdit_Dog_IP->setText("");  //清空显示
            //ui->lineEdit_Dog_port->setText("");
        }

        disconnect(my_socket, SIGNAL(readyRead()), this, SLOT(Socket_Read())); // 会移进线程里
        disconnect(my_socket, SIGNAL(disconnected()), this, SLOT(Socket_Disconnected()));     //子线程断开的时候会进行处理，删除对应的列表项

        if (!closing_window)
        {
            int wait_time_out = 0;     //开始等待连接关闭
            while (wait_time_out < 10)  //最多等待100ms
            {
                Delay_Ms_UnBlocked(10);   //等待10个MS,非阻塞
                if (my_socket->state() == QAbstractSocket::UnconnectedState)
                    break;
                wait_time_out ++;
            }
        }


        delete my_socket; //彻底释放
        my_socket = nullptr;
        socket_disconnecting = false;


    }

}

void Dog_Widget::Socket_Read()
{
    QString recvStr = my_socket->readAll();
    socket_receive_buffer += recvStr;
    qDebug()<<"socke get message="<<recvStr;


    //    "name::abc;time::2018-09-19 13.52.55;object::alive=15;";
    //    "name::abc;time::2018-09-19 13:52:55;object::setname=abc;";
    //    "name::abc;time::2018-09-19 13.52.55;object::log=restart;";
        //    "name::abc;time::2018-09-19 13.52.55;object::reset=1;";

    QString target_name;
    QString message_time;

    if (!recvStr.isEmpty())
    {
        QList<QString> content = recvStr.split(";");         //开始分离解码，取出每种属性
        QString bad_message;
        foreach (QString current_content, content)
        {

            if (current_content.contains("::"))          //每种属性必须含有冒号
            {
                QString property_name = current_content.left(current_content.indexOf("::"));      //提取object
                QString property_content = current_content.right(current_content.size() - current_content.indexOf("::") - 2);     //提取内容
                //qDebug()<<"find '::' in"<<current_content<<" left="<<property_name<<" right="<<property_content;
                if (property_name == "name")        //名字的处理
                {
                    target_name = property_content;
                }

                if (property_name == "time")          //消息发送的对方时间
                {
                    qDebug()<<"message time="<<property_content;
                    message_time = property_content;
                }

                if (property_name == "object")          //各种操作
                {
                    if (1 != property_content.count('='))        //少于或者多于1个等号
                        bad_message.append(",lack of object or mutiple '='");
                    else
                    {
                        QString object = property_content.left(property_content.indexOf('='));      //指令类型，或者属性名称，只能含有1个等号
                        QString object_content = property_content.right(property_content.size() - property_content.indexOf('=') - 1);     //指令内容
                        qDebug()<<"find '=' in"<<property_content<<" left="<<object<<" right="<<object_content;

                        if (object == "alive")   //心跳包
                        {
                            qDebug()<<"heartbeat, add alive time = "<<object_content.toFloat()<<" s";
                            //countdown.start(object_content.toFloat()*1000);      //重启计时器，并设定时间
                            Socket_Feed(object_content.toFloat()*1000); //喂狗
                            if (ui->checkBox_show_heartbeat->isChecked())    //如果需要显示心跳信号
                                Log_Add(tr("收到socket心跳信号,对方时间：%1,增加寿命%2s").arg(message_time).arg(object_content.toFloat()),target_name);
                        }
                        else  if (object == "setname")         //命名
                        {
                            qDebug()<<"set_name!";
                            //                               emit Send_Log( QString("name changed form %1 to %2").arg(this->feeder_name).arg(object_content),this->feeder_name);
                            //                               this->feeder_name = object_content;

                        }
                        else if (object == "log")        //log请求指令
                        {
                            qDebug()<<"get log:"<<object<<":"<<object_content;
                            QString index = object_content;
                            Log_Add(index,target_name);
                        }
                        else if (object == "reset")        //log请求指令
                        {
                            qDebug()<<"get reset:"<<object<<":"<<object_content;

                            int delay = object_content.toInt();
                            Log_Add(tr("收到socket重启请求,对方时间：%1,延迟=%2ms").arg(message_time).arg(delay),target_name);
                            if (delay > 0)
                            {
                                Delay_Ms_UnBlocked(delay);   //等待对应的时间
                            }
                            Socket_Timeout();
                            if (my_socket->state() == QAbstractSocket::ConnectedState)  //如果连接正常
                            {
                                my_socket->write("reset=done;");
                                my_socket->flush();
                            }
                        }

                    }

                }


            }else {
                bad_message.append(",has no spliter : ");
            }

            if (!bad_message.isEmpty())
                break;
        }

    }

}

/*****************按键响应***************/
void Dog_Widget::on_pushButton_Start_clicked()
{
    if (STATE_IDLE == state)
    {
        if (pid_enabled)
        {
            if (ui->lineEdit_target_name->text().isEmpty() || ui->lineEdit_target_position->text().isEmpty())
            {
                Log_Add(tr("没有监控目标，监控失败"));
                return;
            }

            pid_countdown_timer.setInterval(ui->lineEdit_countdown_PID->text().toInt()*1000); //读取时间            
            pid_countdown_timer.start();
            Log_Add(tr("启动PID监控"));
        }

        if (socket_enabled)
        {
            if (my_server.isListening())    //先关闭
                my_server.close();

             bool ret = my_server.listen(QHostAddress::AnyIPv4, ui->lineEdit_Dog_port->text().toUShort());
             if (ret )
             {
                 qDebug()<<"start listen to any ip";
                 ui->lineEdit_Dog_IP->setText(my_server.serverAddress().toString());  //显示本地的IP
                 ui->lineEdit_Dog_port->setText(QString::number( my_server.serverPort()));   //显示本地实际侦听的端口
                 connect(&my_server,SIGNAL(newConnection()),this,SLOT(Get_Connection()));  //处理新进连接的方式
             }
             else  qDebug()<<"listen error = "<<my_server.errorString();

             if (socket_external_only)
             {
                 Log_Add(tr("启动Socket外部监控，不计时，仅能通过Socket指令重启目标"));
             } else {
                 socket_countdown_timer.setInterval(ui->lineEdit_countdown_socket->text().toInt()*1000); //读取时间
                 socket_countdown_timer.start();
                 Log_Add(tr("启动Socket监控"));
             }




        }

        if (Ram_Check_Enabled)
           Log_Add(tr("Ram占用检测启动，次数=%1").arg(Ram_Check_Count));
        else Log_Add(tr("Ram占用检测未启动"));


        ui->lineEdit_target_name->setEnabled(false);       //开始监控后自动关掉这两个可修改的部分
        ui->lineEdit_target_position->setEnabled(false);

        //ui->label_image->setPixmap(QPixmap::fromImage(QImage(":/image/Dog_awake.jpg")).scaled(ui->label_image->size()));
        QIcon icon = QIcon("./image/Dog_awake.jpg");      //设置图标等内容

        ui->pushButton_Start->setText(tr("停止监控"));
        state = STATE_WORKING;

    }
    else if (STATE_WORKING == state)
    {
        if (pid_enabled)
        {
            pid_countdown_timer.stop(); //关闭定时器
            //disconnect(&pid_countdown_timer,SIGNAL(timeout()),this,SLOT(PID_Timeout()));

            ui->lineEdit_target_name->setEnabled(true);       //开始监控后自动关掉这两个可修改的部分
            ui->lineEdit_target_position->setEnabled(true);
            ui->lineEdit_target_pid->setText("");
            ui->progressBar_PID->setValue(0);


            Log_Add(tr("停止PID监控"));
        }

        if (socket_enabled)
        {
            socket_countdown_timer.stop();

            Socket_Disconnected();   //断开socket
            ui->lineEdit_Dog_IP->setText("");  //清空显示
            //ui->lineEdit_Dog_port->setText("");
            ui->progressBar_socket->setValue(0);


            if (my_server.isListening())    //先关闭server
                my_server.close();


            Log_Add(tr("停止Socket监控"));
        }

        qDebug()<<"stop working ";
        //ui->label_image->setPixmap(QPixmap::fromImage(QImage("/image/Dog_Sleep.png")).scaled(ui->label_image->size()));
        QIcon icon = QIcon("/image/Dog_Sleep.jpg");      //设置图标等内容
        ui->pushButton_Start->setText(tr("开始监控"));
        state = STATE_IDLE;
    }

}



void Dog_Widget::on_pushButton_Mannual_Reboot_clicked()
{
    QString log = tr("手动重启：");
//    Socket_Feed();
//    PID_Feed();

    reset_target(&log);

    if ((STATE_WORKING == state) && pid_countdown_timer.isActive())   //如果狗正在工作，重置喂狗倒计时器
        pid_countdown_timer.start();

    Log_Add(log);
}







void Dog_Widget::on_pushButton_System_Config_clicked()
{
    Log_Add(tr("进行系统设置"));
    QString new_name;
    Dialog_Dog_Config *system_config_dlg = new Dialog_Dog_Config(this,my_name,&new_name);
    int ret = system_config_dlg->exec();       //展示设置界面，模态
    qDebug()<<"Config dialog ret = "<<ret;
    switch (ret)
    {
     case RET_SAVED:
     case RET_RENAMED:
     case RET_DELETED:
        emit Config_Return(my_name,ret,new_name);   //发送配置结果给主进程，让主进程对看门狗进行处置
        break;

     case REC_CANCELED:
    default:
        break;

    }
}

void Dog_Widget::on_pushButton_Log_Clear_clicked()
{
    ui->textBrowser_Log->clear();
    Log_Add(tr("Log清空"));
}



void Dog_Widget::Delay_Ms_UnBlocked(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}



void Dog_Widget::on_pushButton_Log_Saveto_clicked()
{
    QString save_path = QFileDialog::getSaveFileName(this,tr("另存为"),QCoreApplication::applicationDirPath(),tr("文本文件(*.txt)")); //选择路径
    qDebug()<<"save file: save_path = "<<save_path;

    if (save_path.isEmpty())
        return;

    QFile save_file;

    save_file.setFileName(save_path);
    bool open_success = save_file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);   //跳到最后，并且使用Text,这样endl会自动换行
    qDebug()<<"save file : file open = "<<open_success;
    if (open_success)
    {
        QTextStream file_write;
        file_write.setDevice(&save_file);
        file_write<<ui->textBrowser_Log->toPlainText()<<endl;
        save_file.flush();   //保存数据
        save_file.close();
        QMessageBox::information(this,tr("保存成功"),tr("保存成功"));
    } else {
      QMessageBox::critical(this,tr("保存失败"),tr("保存失败：%1").arg(save_file.errorString()));
      save_file.close();
    }



}
























