#include "login.h"
#include "ui_loginwindow.h"

#include <QSizePolicy>

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::LoginWindow) {
    ui_->setupUi(this);

    gif = new QMovie(":/gif/animation.gif");
    ui_->gifViewer->setMovie(gif);
    ui_->gifViewer->setStyleSheet("background-color: white;");
    QSizePolicy p;
    setFixedSize(this->size());
    setStyleSheet("LoginWindow {background-color: white;}");
}

LoginWindow::~LoginWindow(){
    delete ui_;
}

void LoginWindow::enableLoginBtn() {
    ui_->loginBtn->setEnabled(true);
}

void LoginWindow::showLoader(bool show) {
    if(show) {
        ui_->gifViewer->show();
        gif->start();
    } else {
        ui_->gifViewer->hide();
        gif->stop();
    }
}

void LoginWindow::on_loginBtn_clicked() {
    QString username = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();
    emit loginBtnClicked(username, password);
}

