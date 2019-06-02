#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    setStyleSheet("MainWindow {background-image:url(:/bg/MainPic.jpg)}");
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::openWindow() {
    show();
}

void MainWindow::openEditor() {
    hide();

//    editor = new TextEdit();

//    const QRect availableGeometry = QApplication::desktop()->availableGeometry(editor);
//    editor->resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
//    editor->move((availableGeometry.width() - editor->width()) / 2,
//            (availableGeometry.height() - editor->height()) / 2);

//    editor->show();
}

void MainWindow::on_loginBtn_clicked(){
    QString username = ui->login_email_input->text();
    QString password = ui->login_password_input->text();
    emit tryLogin(username, password);
}
