#include "login.h"
#include "ui_accessdialog.h"

#include <QSizePolicy>

AccessDialog::AccessDialog(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::AccessDialog),
    gif_(new QMovie(":/gif/animation.gif")),
    logo_(new QPixmap(":/images/logo.jpg")),
    emailRegexp_(new QRegularExpression("^\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,4}\\b")),
    passwordRegexp_(new QRegularExpression("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$")),
    usernameRegexp_(new QRegularExpression("^[A-Za-z0-9]+(?:[ _-][A-Za-z0-9]+)*$")),
    nameRegexp_(new QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")),
    surnameRegexp_(new QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")) {

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
    delete gif_;
    delete logo_;
    delete emailRegexp_;
    delete passwordRegexp_;
    delete usernameRegexp_;
    delete nameRegexp_;
    delete surnameRegexp_;
}

void AccessDialog::enableLoginBtn() {
    ui_->login_btn->setEnabled(true);
}

void AccessDialog::showLoader(bool show) {
    if(show) {
        ui_->access_frame->hide();
        ui_->gif_viewer->show();
        gif_->start();
    } else {
        ui_->gif_viewer->hide();
        gif_->stop();
        ui_->access_frame->show();
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

    showHintEmail = ui_->signup_email_input->text().size() > 0;
    showHintPassword = ui_->signup_password_input->text().size() > 0;
}

void AccessDialog::on_signup_btn_clicked()
{
    QString email = ui_->signup_email_input->text();
    QString password = ui_->signup_password_input->text();
    QString username = ui_->signup_username_input->text();
    QString surname = ui_->signup_surname_input->text();
    QString name = ui_->signup_name_input->text();
    emit signupBtnClicked(email, password, name, surname, username);
}

void AccessDialog::on_back_link_btn_clicked()
{
    ui_->login_widget->show();
    ui_->signup_widget->hide();

    showHintEmail = ui_->login_email_input->text().size() > 0;
    showHintPassword = ui_->login_password_input->text().size() > 0;
}

void AccessDialog::on_login_email_input_editingFinished()
{
    showHintEmail = ui_->login_email_input->text().size() > 0;
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_email_input_textChanged(const QString &arg1)
{
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_password_input_editingFinished()
{
    showHintPassword = ui_->login_password_input->text().size() > 0;
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_password_input_textChanged(const QString &arg1)
{
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_email_input_editingFinished()
{
    showHintEmail = ui_->signup_email_input->text().size() > 0;
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_email_input_textChanged(const QString &arg1)
{
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_password_input_editingFinished()
{
    showHintPassword = ui_->signup_password_input->text().size() > 0;
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_password_input_textChanged(const QString &arg1)
{
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_name_input_editingFinished()
{
    showHintName = ui_->signup_name_input->text().size() > 0;
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_name_input_textChanged(const QString &arg1)
{
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_surname_input_editingFinished()
{
    showHintSurname = ui_->signup_surname_input->text().size() > 0;
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_surname_input_textChanged(const QString &arg1)
{
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_username_input_editingFinished()
{
    showHintUsername = ui_->signup_username_input->text().size() > 0;
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

void AccessDialog::on_signup_username_input_textChanged(const QString &arg1)
{
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

bool AccessDialog::checkLoginForm() {
    QString email = ui_->login_email_input->text();
    QString password = ui_->login_password_input->text();

    bool isValidEmail = regexMatch(email, emailRegexp_, showHintEmail, ui_->login_hint_label, "*insert a valid email");
    if (!isValidEmail) return false;

    bool isValidPassword = regexMatch(password, passwordRegexp_, showHintPassword, ui_->login_hint_label_2, "*insert password with minimum eight characters, at least one letter and one number");
    if (!isValidPassword) return false;

    return isValidEmail && isValidPassword;
}

bool AccessDialog::checkSingupForm() {
    QString email = ui_->signup_email_input->text();
    QString password = ui_->signup_password_input->text();
    QString name = ui_->signup_name_input->text();
    QString surname = ui_->signup_surname_input->text();
    QString username = ui_->signup_username_input->text();

    bool isValidName = regexMatch(name, nameRegexp_, showHintName, ui_->signup_hint_label, "*name must contain only alphabetic characters");
    if (!isValidName) return false;

    bool isValidSurname = regexMatch(surname, surnameRegexp_, showHintSurname,ui_->signup_hint_label, "*surname must contain only alphabetic characters");
    if (!isValidSurname) return false;

    bool isValidEmail = regexMatch(email, emailRegexp_, showHintEmail, ui_->signup_hint_label, "*insert a valid email");
    if (!isValidEmail) return false;

    bool isValidUsername = regexMatch(username, usernameRegexp_, showHintUsername, ui_->signup_hint_label, "*insert a valid username");
    if (!isValidUsername) return false;

    bool isValidPassword = regexMatch(password, passwordRegexp_, showHintPassword, ui_->signup_hint_label, "*insert password with minimum eight characters, at least one letter and one number");
    if (!isValidPassword) return false;

    return isValidEmail && isValidPassword && isValidName && isValidSurname && isValidUsername;
}

bool AccessDialog::regexMatch(QString value, QRegularExpression *regex, bool canShowHint, QLabel* hintLabel, QString hintMessage) {
    if (!(*regex).match(value).hasMatch()) {
        if (canShowHint)
            hintLabel->setText(hintMessage);
        return false;
    } else {
        hintLabel->setText("");
        return true;
    }
}
