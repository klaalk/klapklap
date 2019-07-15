#include "modaldialog.h"
#include "ui_modaldialog.h"

ModalDialog::ModalDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModalDialog)
{
    ui->setupUi(this);
    setWindowTitle("Errore");
    setStyleSheet("ModalDialog {background-color: white;}");
}

ModalDialog::~ModalDialog()
{
    delete ui;
}

void ModalDialog::setModal(QString text, QString btnText, QString modalType) {
    ui->content->setText(text);
    ui->confirmBtn->setText(btnText);
    this->modalType = modalType;
}

void ModalDialog::on_confirmBtn_clicked()
{
    emit modalButtonClicked(ui->confirmBtn->objectName(), modalType);
}

void ModalDialog::on_ModalDialog_rejected()
{
    emit modalClosed(modalType);
}
