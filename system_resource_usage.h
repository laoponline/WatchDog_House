#ifndef SYSTEM_RESOURCE_USAGE_H
#define SYSTEM_RESOURCE_USAGE_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Psapi.lib")
#include <windows.h>
#include <tlhelp32.h>
#include<direct.h>
#include<winternl.h>
#include <psapi.h>
//#include <atlconv.h>
#include <cmath>
#include <string.h>


#define SYSTEM_USAGE_CHECK_TIME  200   //刷新时间
#define MAX_FREQUENCY 50   //最高刷新率
//参考 https://www.cnblogs.com/ybqjymy/p/13862489.html

namespace Ui {
class System_Resource_Usage;
}

class System_Resource_Usage : public QObject
{
    Q_OBJECT
public:
    explicit System_Resource_Usage(QWidget *parent = nullptr);
    virtual ~System_Resource_Usage();
    void SetFrequency(int frequency);  //设置数据刷新频率

signals:
    void System_Usage(float cpu_usage, float ram_usage, float disk_usage);

public slots:
    void Ruquest_Usage(float* cpu_usage = nullptr, float* ram_usage = nullptr, float* disk_usage = nullptr);
    float CPU_Usage() {return current_CPU_Usage;}
    float RAM_Usage() {return current_RAM_Usage;}
    float DISK_Usage() {return current_Disk_Usage;}
private slots:

    void Check_All();

    void CPU_Usage_Check();
    void RAM_Usage_Check();
    void Disk_Usage_Check();
private:

    unsigned long long memory_total,memory_free,memory_used;   //计算RAM用
    MEMORYSTATUSEX memsStat;
    unsigned long Disk_Total_Free,Disk_Total;   //硬盘使用
    float current_CPU_Usage,current_RAM_Usage,current_Disk_Usage;

    QTimer check_timer;

    long long Filetime2Int64(const FILETIME *ftime);
    long long CompareFileTime(FILETIME preTime, FILETIME nowTime);

};

#endif // SYSTEM_RESOURCE_USAGE_H
