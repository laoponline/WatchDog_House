#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码

#include "system_resource_usage.h"

System_Resource_Usage::System_Resource_Usage(QWidget *parent)
{

    connect(&check_timer,SIGNAL(timeout()),this,SLOT(Check_All()));
    check_timer.setInterval(SYSTEM_USAGE_CHECK_TIME);
    check_timer.start();


}


System_Resource_Usage:: ~System_Resource_Usage()
{


}

//设置数据刷新率，最高10
void System_Resource_Usage::SetFrequency(int frequency)
{
    if (frequency > MAX_FREQUENCY)
        frequency = MAX_FREQUENCY;
    int ms = 1000/frequency;
    check_timer.stop();
    check_timer.setInterval(ms);
    check_timer.start();

}


void System_Resource_Usage::Ruquest_Usage(float* cpu_usage, float* ram_usage, float* disk_usage)
{
    if (cpu_usage != nullptr)
       *cpu_usage = current_CPU_Usage;

    if (ram_usage != nullptr)
       *cpu_usage = current_RAM_Usage;

    if (disk_usage != nullptr)
       *cpu_usage = current_Disk_Usage;

    emit System_Usage(current_CPU_Usage,current_RAM_Usage,current_Disk_Usage);  //发出信号

}

void System_Resource_Usage::Check_All()
{
     CPU_Usage_Check();
     RAM_Usage_Check();
     Disk_Usage_Check();

     emit System_Usage(current_CPU_Usage,current_RAM_Usage,current_Disk_Usage);  //发出信号

}


void System_Resource_Usage::CPU_Usage_Check()
{
    //原理是通过统计前面500ms的CPU的各种占用时间
    bool res;
    //总时间 = 内核占用时间 + 用户占用时间
    static FILETIME last_IdleTime;  //上次记录的时间CPU的空闲时间
    static FILETIME last_KernelTime; //上次记录的时间CPU的内核占用时间
    static FILETIME last_UserTime; //上次记录的CPU的用户程序占用时间
    FILETIME idleTime;  //本次采集的CPU的空闲时间
    FILETIME kernelTime;  //本次采集的CPU的内核占用时间
    FILETIME userTime;  //本次采集的CPU的用户程序占用时间
    res = GetSystemTimes(&idleTime,&kernelTime,&userTime);  //第一次采集

    long long idle = CompareFileTime(last_IdleTime,idleTime);  //处理差值
    long long kernel = CompareFileTime(last_KernelTime,kernelTime);
    long long user = CompareFileTime(last_UserTime,userTime);

    current_CPU_Usage =ceil( 100.0*( kernel + user - idle ) / ( kernel + user ) );  //计算占用率


    last_IdleTime = idleTime;  //将本次数据保存为上一次数据
    last_KernelTime = kernelTime;
    last_UserTime = userTime;
   // qDebug()<<"windows:CPU use rate:"<<current_CPU_Usage<<"%";

}


void System_Resource_Usage::RAM_Usage_Check()
{

   memsStat.dwLength = sizeof(memsStat);
   if(!GlobalMemoryStatusEx(&memsStat))//如果获取系统内存信息不成功，就直接返回
   {
       return;
   }
   memory_free = memsStat.ullAvailPhys;
   memory_total = memsStat.ullTotalPhys;
   memory_used = memory_total- memory_free;
   current_RAM_Usage = (memory_used*100.0) / memory_total;  //计算使用率
   //qDebug("windows:mem total: %.0lfMB ,used: %.0lfMB, usage = %2f",memory_total/( 1024.0*1024.0 ),memory_used/( 1024.0*1024.0 ),current_RAM_Usage);

}



void System_Resource_Usage::Disk_Usage_Check()
{

    //return;   //因为可能产生导致当前工作路径发生改变的问题，所以暂时不做处理

    static char path[_MAX_PATH];//存储当前系统存在的盘符
    int curdrive = _getdrive();
     //qDebug()<<"Get volume count = "<<curdrive;
    Disk_Total = 0UL;
    Disk_Total_Free = 0UL;
    for(int drive = 1; drive <= 26; drive++ )//遍历所有盘符
    {
        if( !_chdrive( drive ) )
        {
            sprintf(path, "%c:\\", drive + 'A' - 1 );
            ULARGE_INTEGER caller, total, free;
            WCHAR wszClassName[_MAX_PATH];
            memset(wszClassName,0,sizeof(wszClassName));
            MultiByteToWideChar(CP_ACP,0,path,strlen(path)+1,wszClassName,
                                sizeof(wszClassName)/sizeof(wszClassName[0]));

            if (GetDiskFreeSpaceEx(wszClassName, &caller, &total, &free) == 0)
            {
                //qDebug()<<"GetDiskFreeSpaceEx Filed!";
                return;
            }

            double dTepFree = free.QuadPart/( 1024.0*1024.0 );
            double dTepTotal = total.QuadPart/( 1024.0*1024.0 );
            //qDebug()<<"Get Windows Disk Information:"<<path<<"----free:"<<dTepFree<<"MB / "<<dTepTotal<<"MB";
            Disk_Total_Free += ceil(dTepFree);
            Disk_Total += ceil(dTepTotal);
        }
    }
    current_Disk_Usage = ((Disk_Total - Disk_Total_Free)*100.0) / Disk_Total;
    //_chdrive( curdrive );  //返回初始的磁盘，否则可能发生文件保存问题
    QDir::setCurrent(QCoreApplication::applicationDirPath());   //将默认路径设置到当前的exe目录文件下，否则Ini文件读取会出大问题
    //qDebug("Total disk capacity:%lu MB,Free disk capacity:%lu MB, usage = %02f",Disk_Total,Disk_Total_Free,current_Disk_Usage);


}



__int64 System_Resource_Usage::Filetime2Int64(const FILETIME* ftime)
{
    LARGE_INTEGER li;
    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart;
}

__int64 System_Resource_Usage::CompareFileTime(FILETIME preTime,FILETIME nowTime)
{
    return Filetime2Int64(&nowTime) - Filetime2Int64(&preTime);
}





