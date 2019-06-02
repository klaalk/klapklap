#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "classes/textedit/textedit.h"

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
    void loginBtnClicked(QString username, QString password);

public slots:
    void openEditor();

private slots:
    void on_loginBtn_clicked();

private:
    Ui::MainWindow *ui_ = nullptr;
    TextEdit *editor_ = nullptr;
};

#endif // MAINWINDOW_H
