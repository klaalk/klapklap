#ifndef ACCESSDIALOG_H
#define ACCESSDIALOG_H

#include <QDialog>
#include <QMovie>
#include <QLabel>
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>

#include "../../../../libs/src/constants/kk_constants.h"

namespace Ui {
class AccessDialog;
}


class AccessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AccessDialog(QWidget *parent = nullptr);
    ~AccessDialog();
    void showLoader(bool show);
    void showLogin();
    void showSignup();
signals:
    void loginBtnClicked(QString email, QString password);
    void signupBtnClicked(QString email, QString password, QString name, QString surname, QString username);

private slots:
    void on_login_btn_clicked();
    void on_show_signup_btn_clicked();
    void on_signup_btn_clicked();

    void on_back_link_btn_clicked();

    void on_login_username_input_editingFinished();
    void on_login_username_input_textChanged(const QString &arg1);
    void on_login_password_input_editingFinished();
    void on_login_password_input_textChanged(const QString &arg1);

    void on_signup_email_input_editingFinished();
    void on_signup_email_input_textChanged(const QString &arg1);
    void on_signup_password_input_editingFinished();
    void on_signup_password_input_textChanged(const QString &arg1);
    void on_signup_name_input_editingFinished();
    void on_signup_name_input_textChanged(const QString &arg1);
    void on_signup_surname_input_editingFinished();
    void on_signup_surname_input_textChanged(const QString &arg1);
    void on_signup_username_input_editingFinished();
    void on_signup_username_input_textChanged(const QString &arg1);

private:
    Ui::AccessDialog *ui_ = nullptr;
    QMovie *gif_;
    QPixmap *logo_;

    bool showHintEmail = false;
    bool showHintPassword = false;
    bool showHintName = false;
    bool showHintSurname = false;
    bool showHintUsername = false;

    bool checkLoginForm();
    bool checkSingupForm();
    bool regexMatch(const QString& value, QRegularExpression regex, bool canShowHint, QLabel* hintLabel, const QString& hintMessage);
};

#endif // ACCESSDIALOG_H
