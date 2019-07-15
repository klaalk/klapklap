#ifndef MODALDIALOG_H
#define MODALDIALOG_H

#include <QDialog>

namespace Ui {
class ModalDialog;
}

class ModalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModalDialog(QWidget *parent = nullptr);
    ~ModalDialog();
    void setModal(QString text, QString btnText, QString modalType);

signals:
    void modalButtonClicked(QString btnName, QString modalType);
    void modalClosed(QString modalType);

private slots:
    void on_confirmBtn_clicked();

    void on_ModalDialog_rejected();

private:
    Ui::ModalDialog *ui;
    QString modalType;
};

#endif // MODALDIALOG_H
