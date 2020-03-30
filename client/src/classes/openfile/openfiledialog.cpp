#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#define MAX_TABLE_SIZE 534

#define COLUMN_NAME_SIZE 250
#define COLUMN_CREATOR_SIZE 150

OpenFileDialog::OpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFileDialog)
{
    // Setting up window
    ui->setupUi(this);
    setFixedSize(this->size());
    setStyleSheet("OpenFileDialog {background-color: white;}");

    // Setting up table view
    initializeFilesTableView();
    // Start showing layouts
    ui->accountLayout->show();
    ui->documentsLayout->hide();
}

OpenFileDialog::~OpenFileDialog()
{
    delete ui;
}

void OpenFileDialog::initializeFilesTableView() {
    ui->filesTableWidget->setColumnCount(3);
    ui->filesTableWidget->setHorizontalHeaderLabels({"Nome file", "Creato da", "Creato il"});
    ui->filesTableWidget->verticalHeader()->setVisible(false);

    ui->filesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filesTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->filesTableWidget->setMinimumWidth(MAX_TABLE_SIZE);
    ui->filesTableWidget->setMaximumWidth(MAX_TABLE_SIZE);
    ui->filesTableWidget->setColumnWidth(0, COLUMN_NAME_SIZE);
    ui->filesTableWidget->setColumnWidth(1, COLUMN_CREATOR_SIZE);
    ui->filesTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void OpenFileDialog::setUserInfo(QStringList info) {
    ui->nameLineEdit->insert(info.value(0));
    ui->surnameLineEdit->insert(info.value(1));
    ui->emailLineEdit->insert(info.value(2));
    ui->passwordLineEdit->insert(info.value(3));
    ui->usernameLineEdit->insert(info.value(4));

    QString registrationDate = info.value(5);
    ui->registrationDateLabel->setText("Data di registrazione: " + registrationDate);

    QStringList filesList = info.mid(6, info.size()-1);
    ui->filesTableWidget->setRowCount(filesList.size());

    int fileIndex = 0;
    for(QString fileName : filesList) {
        addFile(fileIndex, fileName);
        fileIndex++;
    }
}

void OpenFileDialog::addFile(int fileIndex, QString fileName) {
    SimpleCrypt crypt(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QStringList splittedName = fileName.split("@");
    files_.insert(splittedName[2], fileName);
    ui->filesTableWidget->setItem(fileIndex, 0, new QTableWidgetItem(splittedName[2]));
    ui->filesTableWidget->setItem(fileIndex, 1, new QTableWidgetItem(crypt.decryptToString(splittedName[1])));
}

void OpenFileDialog::on_filesTableWidget_itemClicked(QTableWidgetItem *item)
{
    int rowIndex = item->row();
    QString fileName = ui->filesTableWidget->item(rowIndex, 0)->text();
    ui->createFileNameLineEdit->setText(fileName);

    ui->openFileButton->setEnabled(true);
    ui->shareFileButton->setEnabled(true);
}

void OpenFileDialog::on_openFileButton_clicked()
{
    QString newFileName = ui->createFileNameLineEdit->text();
    QString completeFileName = files_.value(newFileName);

    if(completeFileName != "" || completeFileName != nullptr) {
        emit openFileRequest(completeFileName);
        qDebug() << "APRO FILE ESISTENTE";
    } else {
        qDebug() << "CREO UN NUOVO FILE";

        if (newFileName != "" && newFileName != nullptr) {
            emit openFileRequest(newFileName);
        } else {
            qDebug() << "ERRORE NOME FILE DA CREARE VUOTO";
        }
    }
}

void OpenFileDialog::on_accountBtn_clicked()
{
    ui->accountLayout->show();
    ui->documentsLayout->hide();
}

void OpenFileDialog::on_documentiBtn_clicked()
{
    ui->documentsLayout->show();
    ui->accountLayout->hide();
}

void OpenFileDialog::on_shareFileButton_clicked()
{
    // TODO: aprire modale per inserire email destinatari
}

void OpenFileDialog::on_createFileNameLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ui->openFileButton->setEnabled(ui->createFileNameLineEdit->text().size() > 0);
}
