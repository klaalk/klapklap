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

public:
    explicit ChooseAvatarDialog(QWidget *parent = nullptr);
    ~ChooseAvatarDialog();
    void showAvatars();
    void handleClick(QString string);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    Ui::ChooseAvatarDialog *ui;
    QLabel* selectedAvatar;
};

#endif // CHOOSEAVATARDIALOG_H
