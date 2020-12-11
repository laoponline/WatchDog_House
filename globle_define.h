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




#define ERROR_DETECTION_DURATION 20


#define STATE_IDLE 0
#define STATE_WORKING 1
#define COUNT_DOWN_DEFAULT 30         //默认喂狗时间30s
#define LOCAL_PORT_DEFAULT 8899  //默认本地监听端口

#define RETCODE_RESTART 5

#define DIALOG_NAME_MAIN "DH"
#define SYSTEM_VERSION "V0.1 R20201211"



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
