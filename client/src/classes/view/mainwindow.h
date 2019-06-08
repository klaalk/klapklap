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
    void signupBtnClicked(QString username, QString password,QString name, QString surname);

public slots:
    void openEditor();

private slots:
    void on_loginBtn_clicked();

    void on_signupBtn_clicked();

private:
    Ui::MainWindow *ui_ = nullptr;
    TextEdit *editor_ = nullptr;
};

#endif // MAINWINDOW_H
