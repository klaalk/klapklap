#include "login.h"
#include "ui_accessdialog.h"

#include <QSizePolicy>

AccessDialog::AccessDialog(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::AccessDialog),
    gif_(new QMovie(":/gif/animation.gif")),
    logo_(new QPixmap(":/images/logo.jpg")),
    emailRegexp_(new QRegularExpression("^\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b")){

    ui_->setupUi(this);
    setFixedSize(this->size());
    setStyleSheet("AccessDialog {background-color: white;}");

    ui_->gif_viewer->setStyleSheet("background-color: white;");
    ui_->gif_viewer->setMovie(gif_);
    ui_->logo_view->setPixmap((*logo_).scaled(200,190,Qt::KeepAspectRatio));
    ui_->signup_widget->hide();
}

AccessDialog::~AccessDialog(){
    delete ui_;
}

void AccessDialog::enableLoginBtn() {
    ui_->login_btn->setEnabled(true);
}

void AccessDialog::showLoader(bool show) {
    if(show) {
        ui_->login_widget->hide();
        ui_->gif_viewer->show();
        gif_->start();
    } else {
        ui_->gif_viewer->hide();
        gif_->stop();
        ui_->login_widget->show();
    }
}

void AccessDialog::on_login_btn_clicked() {
    QString email = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();
    emit loginBtnClicked(email, password);
}

void AccessDialog::on_show_signup_btn_clicked()
{
    ui_->login_widget->hide();
    ui_->signup_widget->show();
}

void AccessDialog::on_signup_btn_clicked()
{

}

void AccessDialog::on_back_link_btn_clicked()
{
    ui_->login_widget->show();
    ui_->signup_widget->hide();
}

void AccessDialog::on_login_email_input_editingFinished()
{
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_email_input_textChanged(const QString &arg1)
{
    if (!canCheckEmail) return;
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_password_input_editingFinished()
{
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);

    canCheckEmail = !isValid;
}

bool AccessDialog::checkLoginForm() {
    QString email = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();

    bool isValidEmail = false;

    if (!(*emailRegexp_).match(email).hasMatch()) {
       ui_->login_hint_label->setText("Email inserita non valida");
       isValidEmail = false;
    } else {
       ui_->login_hint_label->setText("");
       isValidEmail = true;
    }

    return isValidEmail;
}



