#include "login.h"
#include "ui_loginwindow.h"

#include <QMessageBox>
#include <QDesktopWidget>

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::LoginWindow) {
    ui_->setupUi(this);


    // Main Window setup
    setStyleSheet("LoginWindow {background-image:url(:/bg/MainPic.jpg)}");
}

LoginWindow::~LoginWindow(){
    delete ui_;
}

void LoginWindow::on_loginBtn_clicked() {
    QString username = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();
    emit loginBtnClicked(username, password);
}

void LoginWindow::on_signupBtn_clicked() {
    QString username = ui_->signup_email_input->text();
    QString password = ui_->signup_password_input->text();
    QString name = ui_->signup_name_input->text();
    QString surname = ui_->signup_surname_input->text();
    emit signupBtnClicked(username, password, name, surname);
}
