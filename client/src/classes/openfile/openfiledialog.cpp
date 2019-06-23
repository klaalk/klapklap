#include "openfiledialog.h"
#include "ui_openfiledialog.h"

OpenFileDialog::OpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFileDialog)
{
    ui->setupUi(this);
}

OpenFileDialog::~OpenFileDialog()
{
    delete ui;
}

void OpenFileDialog::addFile(QString fileName) {
    QStringList splittedName = fileName.split("&");
    files_.insert(splittedName[2], fileName);
    ui->listWidget->addItem(splittedName[2]);
}

void OpenFileDialog::on_listWidget_itemClicked(QListWidgetItem *item)
{
    QString txt = item->text();
    selectedFile = txt;
    qDebug() << "item: " << txt;
    ui->lineEdit->setText(txt);
}

void OpenFileDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    QString completeFileName = files_.value(selectedFile);
    if(selectedFile == "") {

    }
    emit openFileRequest(completeFileName);
}
