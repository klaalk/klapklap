#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginBtnClicked(QString username, QString password);
    void signupBtnClicked(QString username, QString password,QString name, QString surname);

private slots:
    void on_loginBtn_clicked();
    void on_signupBtn_clicked();
private:
    Ui::LoginWindow *ui_ = nullptr;
};

#endif // LOGINWINDOW_H
