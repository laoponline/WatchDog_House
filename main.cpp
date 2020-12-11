
#if _MSC_VER >= 1600	// MSVC2015 > 1899,	MSVC_VER = 14.0
#pragma execution_character_set("utf-8")
#endif


#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>



int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));  //使用mingw编译器的话就要使用UTF-8,VS15版本貌似也可以
   // QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));  //使用VS编译器的话就要使用GBK

    QApplication a(argc, argv);

    int ret = -1;
    do {
        MainWindow w;
        w.show();
        ret = a.exec();
    } while (RETCODE_RESTART == ret);

    return a.exec();
}
