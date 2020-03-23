#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#define MAX_TABLE_SIZE 256

#define COLUMN_NAME_SIZE (double) 2/3 * MAX_TABLE_SIZE
#define COLUMN_CREATOR_SIZE (double) 1/3 * MAX_TABLE_SIZE

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
    ui->filesTableWidget->setColumnCount(2);
    ui->filesTableWidget->setHorizontalHeaderLabels({"Nome", "Creato da"});
    ui->filesTableWidget->verticalHeader()->setVisible(false);

    ui->filesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filesTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->filesTableWidget->setMinimumWidth(MAX_TABLE_SIZE);
    ui->filesTableWidget->setMaximumWidth(MAX_TABLE_SIZE);
    ui->filesTableWidget->setColumnWidth(0, COLUMN_NAME_SIZE - 1);
    ui->filesTableWidget->setColumnWidth(1, COLUMN_CREATOR_SIZE);
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
    QStringList splittedName = fileName.split("@");
    files_.insert(splittedName[2], fileName);
    ui->filesTableWidget->setItem(fileIndex, 0, new QTableWidgetItem(splittedName[2]));
    ui->filesTableWidget->setItem(fileIndex, 1, new QTableWidgetItem(splittedName[1]));
}

void OpenFileDialog::on_filesTableWidget_itemClicked(QTableWidgetItem *item)
{
    int rowIndex = item->row();
    selectedFile = ui->filesTableWidget->item(rowIndex, 0)->text();
    ui->shareFileTextEdit->setText(files_.value(selectedFile));
    ui->openFileButton->setEnabled(true);
    // TODO: abilitare modale condivisione file
    // TODO: ui->shareFileButton->setEnabled(true);
}

void OpenFileDialog::on_openFileButton_clicked()
{
    QString completeFileName = ui->shareFileTextEdit->toPlainText();
    if(completeFileName == "" || completeFileName == nullptr) {
        emit openFileRequest(completeFileName);
    } else {
        qDebug() << "ERRORE NESSUNO FILE TROVATO";
    }
}

void OpenFileDialog::on_createFileButton_clicked()
{
    QString newFileName = ui->createFileNameLineEdit->text();
    if (newFileName != "" && newFileName != nullptr) {
        emit openFileRequest(newFileName);
    } else {
        qDebug() << "ERRORE NOME FILE DA CREARE VUOTO";
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
