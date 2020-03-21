#include "openfiledialog.h"
#include "ui_openfiledialog.h"

OpenFileDialog::OpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFileDialog)
{
    ui->setupUi(this);
    ui->verticalWidget_2->hide();
    ui->verticalWidget_4->show();
}

OpenFileDialog::~OpenFileDialog()
{
    delete ui;
}

void OpenFileDialog::initInfo(QStringList info) {
    ui->lineEdit_name->insert(info.value(0));
    ui->lineEdit_surname->insert(info.value(1));
    ui->lineEdit_email->insert(info.value(2));
    ui->lineEdit_pass->insert(info.value(3));
    ui->lineEdit_user->insert(info.value(4));

    QString registrationData = info.value(5);

    for(QString s : info.mid(6, info.size()-1)) {
        if(s!="")
            addFile(s);
    }
}

void OpenFileDialog::addFile(QString fileName) {
    QStringList splittedName = fileName.split("@");
    files_.insert(splittedName[2], fileName);
    ui->listWidget->addItem(splittedName[2]);

    if (ui->label_3->isVisible())
        ui->label_3->hide();
}

void OpenFileDialog::on_listWidget_itemClicked(QListWidgetItem *item)
{
    QString txt = item->text();
    selectedFile = txt;
    qDebug() << "item: " << txt;
    ui->lineEdit->setText(txt);
}

void OpenFileDialog::on_openBtn_clicked()
{
    QString completeFileName = files_.value(selectedFile);
    if(completeFileName == "" || completeFileName == nullptr) {
        completeFileName = ui->lineEdit->text();
    }
    emit openFileRequest(completeFileName);
}


void OpenFileDialog::on_pushButton_clicked()
{
    if(selectedFile != "") {
        ui->label_2->setText(files_.value(selectedFile));
    }
}

void OpenFileDialog::on_pushButton_2_clicked()
{
    ui->verticalWidget_4->show();
    ui->verticalWidget_2->hide();
}

void OpenFileDialog::on_pushButton_3_clicked()
{
    ui->verticalWidget_2->show();
    ui->verticalWidget_4->hide();
}

void OpenFileDialog::on_lineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    selectedFile = ui->lineEdit->text();
}
