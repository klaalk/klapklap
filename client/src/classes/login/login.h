#ifndef ACCESSDIALOG_H
#define ACCESSDIALOG_H

#include <QMainWindow>
#include <QMovie>
#include <QLabel>
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>

namespace Ui {
class AccessDialog;
}


class AccessDialog : public QMainWindow
{
    Q_OBJECT
public:
    explicit AccessDialog(QWidget *parent = nullptr);
    ~AccessDialog();
    void enableLoginBtn();
    void showLoader(bool show);

signals:
    void loginBtnClicked(QString email, QString password);
    void signupBtnClicked(QString email, QString password, QString name, QString surname, QString username);

private slots:
    void on_login_btn_clicked();
    void on_show_signup_btn_clicked();
    void on_signup_btn_clicked();

    void on_back_link_btn_clicked();

    void on_login_email_input_editingFinished();

    void on_login_password_input_editingFinished();

    void on_login_email_input_textChanged(const QString &arg1);

private:
    Ui::AccessDialog *ui_ = nullptr;
    QMovie *gif_;
    QPixmap *logo_;
    QRegularExpression *emailRegexp_;

    bool canCheckEmail = false;

    bool checkLoginForm();
};

#endif // ACCESSDIALOG_H
