#pragma execution_character_set("utf-8")   //告诉mscv 采用utf-8编码

#ifndef GLOBLE_DEFINE_H
#define GLOBLE_DEFINE_H


#include <QDebug>
#include <QColor>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QTimer>
#include <QProcess>



#define ORDER_RET_NO_ERROR 0 //无错误
#define ORDER_RET_INVALID_ORDER_CONTANT 1  //指令内容错误，未能生成完整指令
#define ORDER_RET_SUM_ERROR 2  //指令校验和错误
#define ORDER_RET_ADDR_ERROR 3  //指令地址错误
#define ORDER_RET_LAKE_OF_TARGET 4  //指令中不含有变量名
#define ORDER_RET_DATA_ERROR 5  //指令中不含有变量值
#define ORDER_RET_TARGET_ERROR 6 //未找到本地对应的变量名
#define ORDER_RET_DATA_TYPE_ERROR 7 //数据格式错误
#define ORDER_RET_TARGET_RO  8   //该指令是一个读取型指令，非错误
#define ORDER_RET_WRITE_NOT_ALLOWED 9 //目标是只读的
#define ORDER_RET_FFFF_ADDR 10  //该指令是一个读取型指令，非错误
#define ORDER_RET_SEND_STR_INVALID 11 //想要发射的目标结构体是个非活动的
#define ORDER_RET_ACK_TIMEOUT 12 //需要等待目标返回ACK，但是超时了

/*************
指令模块错误码

0： 检查通过
1： 指令是无效指令
2： SUM计算值错误
3： 指令地址错误
4： 指令中没有变量名
5： 指令中找不到变量
6： 现有数据结构体中找不到同名变量或者同名变量处于无效状态
7： 从数据结构体中得到的数据格式不对
8： 指令是一个读取型指令
9:  指令尝试写入一个只读的指令
10： 指令是群发指令


11：发射的结构体是无效结构体
12:  发射需要等待ACK，但是超时，没有等到ACK
*****************************************************/


#define ERROR_DETECTION_DURATION 20


#define STATE_IDLE 0
#define STATE_WORKING 1
#define COUNT_DOWN_DEFAULT 30         //默认喂狗时间30s
#define LOCAL_PORT_DEFAULT 8899  //默认本地监听端口
#define REBOOT_DELAY_DEFAULT 500  //默认重启延时

#define RETCODE_RESTART 5

#define DIALOG_NAME_MAIN "DH"
#define SYSTEM_VERSION "V0.3 R20210918"



#define INT_PATH "config.ini"
#define DATABASE_PATH_DEFAULT "Default.db"

#define LOG_PATH_DEFAULT "log/"
#define LOG_CLEAR_DURATION 3600 //串口LOG清除时间




#define LANGUAGE_DEFAULT "Simplified_Chinese"

#define LANGUAGE_ENGLISH 0
#define LANGUAGE_SIMPLIFIED_CHINESE 1
#define LANGUAGE_TRADITIONAL_CHINESE 2


#define CAPTURE_SENDER_IMAGE_PROCESS 0
#define CAPTURE_SENDER_ACK 1



#endif // GLOBLE_DEFINE_H
