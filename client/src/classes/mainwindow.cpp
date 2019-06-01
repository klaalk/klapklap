#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginBtn_clicked()
{
    QString username = ui->login_email_input->text();
    QString password = ui->login_password_input->text();

    if(username == "test" && password == "test") {
        QMessageBox::information(this, "Login", "Username and password is correct");
    }
}
