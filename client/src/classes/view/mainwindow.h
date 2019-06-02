#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "classes/client/kk_client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void tryLogin(QString username, QString password);

public slots:
    void openWindow();
    void openEditor();

private slots:
    void on_loginBtn_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
