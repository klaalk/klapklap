#include "sharefiledialog.h"
#include "ui_sharefiledialog.h"

#include <QClipboard>

ShareFileDialog::ShareFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShareFileDialog)
{
    setStyleSheet("ShareFileDialog {background-color: white;}");
    ui->setupUi(this);
}

ShareFileDialog::~ShareFileDialog()
{
    delete ui;
}

void ShareFileDialog::setShareFileLink(const QString &link)
{
    sharedLink = link;
    QFontMetrics metrics(ui->linkLabel->font());
    QString elidedText = metrics.elidedText(link, Qt::ElideRight, ui->linkLabel->width());
    ui->linkLabel->setText(elidedText);
}

void ShareFileDialog::on_pushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(sharedLink);
}
