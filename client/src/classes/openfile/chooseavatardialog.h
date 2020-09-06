#ifndef CHOOSEAVATARDIALOG_H
#define CHOOSEAVATARDIALOG_H

#include <QDialog>
#include <QDirIterator>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class ChooseAvatarDialog;
}

class ChooseAvatarDialog : public QDialog
{
    Q_OBJECT
signals:
    void updateAvatarRequest(QString avatar);

public:
    explicit ChooseAvatarDialog(QWidget *parent = nullptr);
    ~ChooseAvatarDialog();
    void showAvatars();
    void loadAvatars();

    void handleClick(QString string);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::ChooseAvatarDialog *ui;
    QLabel* selectedAvatar;
    QVector<QLabel *> avatars;
};

#endif // CHOOSEAVATARDIALOG_H
