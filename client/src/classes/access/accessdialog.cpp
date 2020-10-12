#include "accessdialog.h"
#include "ui_accessdialog.h"

#include <QFile>
#include <QSizePolicy>
#include <QScreen>
#include <QApplication>
#include <QRect>
#include <QDesktopWidget>
#include <QPaintDevice>



AccessDialog::AccessDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::AccessDialog),
    gif_(new QMovie(":/gif/animation.gif")),




    logo_(new QPixmap(":/images/logo.jp2"))
{
    QFile styleFile( ":/styles/access-dialog.qss");
    styleFile.open( QFile::ReadOnly );
    setStyleSheet(QString(styleFile.readAll()));

    ui_->setupUi(this);



#ifdef Q_OS_MACOS
    QSize layoutSize = logo_->size() / logo_->devicePixelRatio();
    logo_->setDevicePixelRatio(8.8);
     ui_->logo_view->setPixmap((*logo_).scaled(layoutSize,Qt::KeepAspectRatio,Qt::SmoothTransformation));
#endif

#ifndef Q_OS_MACOS
      ui_->logo_view->setPixmap((*logo_).scaled(217,217,Qt::KeepAspectRatio,Qt::SmoothTransformation));
#endif


   QSize *size = new QSize(300,300);


    gif_->setScaledSize(*size);




    ui_->gif_viewer->setMovie(gif_);



    ui_->signup_widget->hide();
}

AccessDialog::~AccessDialog(){
    delete ui_;
    delete gif_;
    delete logo_;
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

void AccessDialog::showLogin() {
    on_back_link_btn_clicked();
}
void AccessDialog::showSignup() {
    on_show_signup_btn_clicked();
}
void AccessDialog::on_login_btn_clicked() {
    QString email = ui_->login_username_input->text();
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

    showHintEmail = ui_->login_username_input->text().size() > 0;
    showHintPassword = ui_->login_password_input->text().size() > 0;
}

void AccessDialog::on_login_username_input_editingFinished()
{
    showHintEmail = ui_->login_username_input->text().size() > 0;
    bool isValid = checkLoginForm();
    ui_->login_btn->setEnabled(isValid);
}

void AccessDialog::on_login_username_input_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
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
    Q_UNUSED( arg1 )
    bool isValid = checkSingupForm();
    ui_->signup_btn->setEnabled(isValid);
}

bool AccessDialog::checkLoginForm() {
    QString username = ui_->login_username_input->text();
    QString password = ui_->login_password_input->text();

    bool isValidUsername = regexMatch(username, USERNAME_REGEX, showHintEmail, ui_->login_hint_label, "*inserisci un username valido");
    if (!isValidUsername) return false;

    bool isValidPassword = regexMatch(password, PASSWORD_REGEX, showHintPassword, ui_->login_hint_label_2, "*inserisci una password di minimo 8 caratteri e almeno 1 lettera maiuscola e 1 numero");
    if (!isValidPassword) return false;

    return isValidUsername && isValidPassword;
}

bool AccessDialog::checkSingupForm() {
    QString email = ui_->signup_email_input->text();
    QString password = ui_->signup_password_input->text();
    QString name = ui_->signup_name_input->text();
    QString surname = ui_->signup_surname_input->text();
    QString username = ui_->signup_username_input->text();

    bool isValidName = regexMatch(name, NAME_REGEX, showHintName, ui_->signup_hint_label, "*name must contain only alphabetic characters");
    if (!isValidName) return false;

    bool isValidSurname = regexMatch(surname, SURNAME_REGEX, showHintSurname, ui_->signup_hint_label, "*surname must contain only alphabetic characters");
    if (!isValidSurname) return false;

    bool isValidEmail = regexMatch(email, EMAIL_REGEX, showHintEmail, ui_->signup_hint_label, "*insert a valid email");
    if (!isValidEmail) return false;

    bool isValidUsername = regexMatch(username, USERNAME_REGEX, showHintUsername, ui_->signup_hint_label, "*insert a valid username");
    if (!isValidUsername) return false;

    bool isValidPassword = regexMatch(password, PASSWORD_REGEX, showHintPassword, ui_->signup_hint_label, "*insert password with minimum eight characters, at least one letter and one number");
    if (!isValidPassword) return false;

    return isValidEmail && isValidPassword && isValidName && isValidSurname && isValidUsername;
}

bool AccessDialog::regexMatch(const QString& value, QRegularExpression regex, bool canShowHint, QLabel* hintLabel, const QString& hintMessage) {
    if (!(regex).match(value).hasMatch()) {
        if (canShowHint)
            hintLabel->setText(hintMessage);
        return false;
    }
    hintLabel->setText("");
    return true;
    
}
