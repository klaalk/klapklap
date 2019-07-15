#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QMovie>
#include <QLabel>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void enableLoginBtn();
    void showLoader(bool show);
signals:
    void loginBtnClicked(QString username, QString password);
    void signupBtnClicked(QString username, QString password,QString name, QString surname);

private slots:
    void on_loginBtn_clicked();
private:
    Ui::LoginWindow *ui_ = nullptr;
    QMovie *gif;
};

#endif // LOGINWINDOW_H
