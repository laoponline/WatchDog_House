#ifndef DIALOG_SYSTEM_CONFIG_H
#define DIALOG_SYSTEM_CONFIG_H

#include <QDialog>
#include "globle_define.h"

namespace Ui {
class Dialog_System_Config;
}

class Dialog_System_Config : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_System_Config(QWidget *parent = nullptr);
    ~Dialog_System_Config();

private slots:
    void on_pushButton_save_reset_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_exit_clicked();

private:
    Ui::Dialog_System_Config *ui;
    void save_all();
};

#endif // DIALOG_SYSTEM_CONFIG_H
