#include "sharefiledialog.h"
#include "ui_sharefiledialog.h"

ShareFileDialog::ShareFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShareFileDialog)
{
    ui->setupUi(this);
}

ShareFileDialog::~ShareFileDialog()
{
    delete ui;
}

void ShareFileDialog::setShareFileLink(const QString &link)
{
    ui->linkLabel->setText(link);
}

void ShareFileDialog::on_buttons_accepted()
{
    QString text = ui->emailsPlainTextEdit->toPlainText();
    if (text.size()>0) {
        // Mandare evento per spedire email
    }
}
