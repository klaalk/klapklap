#ifndef SHAREFILEDIALOG_H
#define SHAREFILEDIALOG_H

#include <QDialog>

namespace Ui {
class ShareFileDialog;
}

class ShareFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShareFileDialog(QWidget *parent = nullptr);
    ~ShareFileDialog();

    void setShareFileLink(const QString& link);

private slots:


    void on_buttons_accepted();

private:
    Ui::ShareFileDialog *ui;
};

#endif // SHAREFILEDIALOG_H
