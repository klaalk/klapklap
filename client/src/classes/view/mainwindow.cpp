#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow), editor_(new TextEdit){

    ui_->setupUi(this);

    //Editor setup
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(editor_);
    editor_->resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    editor_->move((availableGeometry.width() - editor_->width()) / 2,
            (availableGeometry.height() - editor_->height()) / 2);
    // Main Window setup
    setStyleSheet("MainWindow {background-image:url(:/bg/MainPic.jpg)}");
}

MainWindow::~MainWindow(){
    delete editor_;
    delete ui_;
}

void MainWindow::openEditor() {
    editor_->show();
}

void MainWindow::on_loginBtn_clicked(){
    QString username = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();
    emit loginBtnClicked(username, password);
}

void MainWindow::on_signupBtn_clicked()
{    QString username = ui_->signup_email_input->text();
     QString password = ui_->signup_password_input->text();
     QString name = ui_->signup_name_input->text();
     QString surname = ui_->signup_surname_input->text();

     emit signupBtnClicked(username, password, name,surname);

}
