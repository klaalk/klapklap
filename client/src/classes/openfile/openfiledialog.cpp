#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#include <QDateTime>
#include <QImageWriter>
#include <QMessageBox>
#include <QStandardPaths>

#define MAX_TABLE_SIZE 16777215

#define COLUMN_NAME_SIZE 250
#define COLUMN_CREATOR_SIZE 150


OpenFileDialog::OpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFileDialog),
    crypt(new KKCrypt(Q_UINT64_C(0x0c2ad4a4acb9f023))),
    fileNameRegexp(new QRegularExpression("[\\/:*?<>|\".]"))
{
    // Setting up window
    ui->setupUi(this);
    setFixedSize(this->size());
    setStyleSheet("OpenFileDialog {background-color: white;}");

    // Setting up table view
    initializeFilesTableView();
    connect(&chooseAvatarDialog, &ChooseAvatarDialog::updateAvatarRequest, this, &OpenFileDialog::setUserAvatar);
    // Start showing layouts
    ui->accountLayout->show();
    ui->documentsLayout->show();
}

OpenFileDialog::~OpenFileDialog()
{
    delete ui;
}


void OpenFileDialog::closeEvent(QCloseEvent *e)
{
    clear();
}

void OpenFileDialog::initializeFilesTableView() {
    ui->filesTableWidget->clear();
    ui->filesTableWidget->setColumnCount(3);
    ui->filesTableWidget->setHorizontalHeaderLabels({"Nome file", "Creato da", "Creato il"});
    ui->filesTableWidget->verticalHeader()->setVisible(false);

    ui->filesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filesTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->filesTableWidget->setColumnWidth(0, COLUMN_NAME_SIZE);
    ui->filesTableWidget->setColumnWidth(1, COLUMN_CREATOR_SIZE);
    ui->filesTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void OpenFileDialog::setUserInfo(const QStringList& info) {
    ui->nameLineEdit->setText(info.value(0));
    ui->surnameLineEdit->setText(info.value(1));
    ui->aliasLineEdit->setText(info.value(5));
    ui->emailLabel->setText("Email: " + info.value(2));
    ui->usernameLabel->setText("Username: " + info.value(4));
    QString registrationDate = info.value(6);
    QDateTime registrationDateTime = QDateTime::fromString(registrationDate, Qt::ISODate);
    ui->registrationDateLabel->setText("Data di registrazione: " + registrationDateTime.toString(DATE_TIME_FORMAT));
}

void OpenFileDialog::setUserFiles(const QStringList &files)
{
    files_.clear();
    ui->filesTableWidget->setRowCount(files.size());

    int fileIndex = 0;
    for(const QString& fileName : files) {
        addFile(fileIndex, fileName);
        fileIndex++;
    }
}

void OpenFileDialog::setUserAvatar(const QString &avatar)
{
    QString path = ":images/avatars/"+avatar;
    bool exist = QFile::exists(path);
    if (exist) {
        QPixmap image = QPixmap(":images/avatars/"+avatar);
        QIcon ButtonIcon(image);
        ui->changeImageButton->setIcon(ButtonIcon);
        ui->changeImageButton->setIconSize(image.rect().size());
        this->avatar = avatar;
    } else
       qDebug() << "[setUserAvatar] Avatar not exist: " + path;
}

void OpenFileDialog::addFile(int fileIndex, const QString& fileRow) {
    QStringList splittedFileRow = fileRow.split(FILENAME_SEPARATOR);
    QString fileName = crypt->decryptToString(splittedFileRow[0]);
    QStringList splittedFilename = fileName.split(FILENAME_SEPARATOR);
    QDateTime creationDateTime = QDateTime::fromString(splittedFileRow[1], Qt::ISODate);

    files_.insert(splittedFilename[2], splittedFileRow[0]);
    QTableWidgetItem* name =  new QTableWidgetItem(splittedFilename[2]);
    QTableWidgetItem* owner =  new QTableWidgetItem(splittedFilename[1]);
    QTableWidgetItem* date = new QTableWidgetItem(creationDateTime.toString(DATE_TIME_FORMAT));
    owner->setTextAlignment(Qt::AlignCenter);
    date->setTextAlignment(Qt::AlignCenter);

    ui->filesTableWidget->setItem(fileIndex, 0, name);
    ui->filesTableWidget->setItem(fileIndex, 1, owner);
    ui->filesTableWidget->setItem(fileIndex, 2, date);
}

void OpenFileDialog::clear()
{
    ui->createFileNameLineEdit->clear();
    selectedFilename.clear();
    selectedLink.clear();
    pastedFilename.clear();
    pastedLink.clear();
}


void OpenFileDialog::on_filesTableWidget_itemClicked(QTableWidgetItem *item)
{
    selectedFilename = ui->filesTableWidget->item(item->row(), 0)->text();
    ui->createFileNameLineEdit->setText(selectedFilename);

    ui->openFileButton->setEnabled(true);
    ui->shareFileButton->setEnabled(true);
}

void OpenFileDialog::on_openFileButton_clicked()
{
    if (pastedLink != nullptr && !pastedLink.isEmpty()) {
        emit openFileRequest(pastedLink, pastedFilename);
    } else {
        selectedLink = (selectedLink != nullptr && !selectedLink.isEmpty()) ? selectedLink
                  : files_.value(selectedFilename);

        if (selectedLink != nullptr && !selectedLink.isEmpty()) {
            emit openFileRequest(selectedLink, selectedFilename);
        } else {
            QString newFileName = ui->createFileNameLineEdit->text();
             if (newFileName != "" && newFileName != nullptr) {
                emit openFileRequest(newFileName, newFileName);
            } else {
                qDebug() << "[on_openFileButton_clicked] Errore nell'apertura file: nome file vuoto!";
            }
        }
    }
}

void OpenFileDialog::on_shareFileButton_clicked()
{
    QString link = (pastedLink != nullptr && !pastedLink.isEmpty()) ? pastedLink : selectedLink;
    shareFileDialog.setShareFileLink(link);
    shareFileDialog.show();
}

void OpenFileDialog::on_changeImageButton_clicked()
{
    chooseAvatarDialog.showAvatars();
}

void OpenFileDialog::on_createFileNameLineEdit_textChanged(const QString &lineEditText)
{

    QString decryptedLink = crypt->isEncryptedLink(lineEditText) ? crypt->decryptToString(lineEditText) : nullptr;

    if (decryptedLink != nullptr && !decryptedLink.isEmpty()) {
        QStringList splittedDecryptedLink = decryptedLink.split(FILENAME_SEPARATOR);
        if (splittedDecryptedLink.size() == 3) {
            pastedLink = lineEditText;
            pastedFilename = splittedDecryptedLink[2];
            ui->createFileNameLineEdit->setText(pastedFilename);
        } else {
            ui->createFileNameLineEdit->setText("");
        }
    } else {
        bool isPastedFileName = lineEditText == pastedFilename && pastedFilename != nullptr && !pastedFilename.isEmpty();
        bool isFileNameValid = lineEditText.size() > 0 && lineEditText.length() <= FILENAME_MAX_LENGTH && !fileNameRegexp->match(lineEditText).hasMatch();
        bool isSharedFileName = false;
        bool isLink = false;
        if (!isPastedFileName) {
            pastedLink = "";
            pastedFilename = "";

            if (isFileNameValid) {
                selectedLink = files_.value(lineEditText);
                if (selectedLink != nullptr && !selectedLink.isEmpty()) {
                    selectedFilename = lineEditText;
                    isSharedFileName = true;
                } else {
                    selectedFilename = "";
                     isSharedFileName = false;
                }
            }
        }
        isLink = isFileNameValid && (isPastedFileName || isSharedFileName);
        ui->openFileButton->setEnabled(isFileNameValid);
        ui->shareFileButton->setEnabled(isLink);

        if (isLink) ui->createFileNameLineEdit->setStyleSheet("font-weight: bold;");
        else ui->createFileNameLineEdit->setStyleSheet("");
    }
}

void OpenFileDialog::on_saveChangesButton_clicked()
{
    QString name = ui->nameLineEdit->text();
    QString surname = ui->surnameLineEdit->text();
    QString alias = ui->aliasLineEdit->text();
    emit updateAccountRequest(name, surname, alias, avatar);
}

void OpenFileDialog::on_LogoutButton_clicked()
{
    emit logoutRequest();
}
