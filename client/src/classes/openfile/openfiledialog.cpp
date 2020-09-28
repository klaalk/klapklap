#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#include <QClipboard>
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
    setFixedSize(size());

    // Setting up table view
    initializeFilesTableView();
    connect(&chooseAvatarDialog, &ChooseAvatarDialog::updateAvatarRequest, this, &OpenFileDialog::setAvatar);

    ui->saveChangesButton->setEnabled(false);

    // Start showing layouts
    ui->accountLayout->show();
    ui->documentsLayout->show();
    ui->createFileNameLineEdit->setFocus();
}

OpenFileDialog::~OpenFileDialog()
{
    delete ui;
}


void OpenFileDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
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

void OpenFileDialog::setUser(KKUser* user) {
    ui->emailLabel->setText(user->getEmail());

    ui->usernameLabel->setText("Username: " + user->getUsername());

    QDateTime registrationDateTime = QDateTime::fromString(user->getRegistrationDate(), Qt::ISODate);
    ui->registrationDateLabel->setText("Data di registrazione: " + registrationDateTime.toString(DATE_TIME_FORMAT));

    avatar = user->getImage();
    setAvatar(user->getImage());
    setName(user->getName());
    setSurname(user->getSurname());
    setAlias(user->getAlias());

    ui->saveChangesButton->setEnabled(chackEditChanges());

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

void OpenFileDialog::setAvatar(const QString &avatar)
{
    QString path = ":images/avatars/"+avatar;
    bool exist = QFile::exists(path);
    if (exist) {
        QPixmap image = QPixmap(":images/avatars/"+avatar);
        QIcon ButtonIcon(image);
        ui->changeImageButton->setIcon(ButtonIcon);
        ui->changeImageButton->setIconSize(image.rect().size());
        ui->changeImageButton->setWhatsThis(avatar);

        ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());

    } else
       qDebug() << "[setUserAvatar] Avatar not exist: " + path;

}


void OpenFileDialog::setAlias(const QString &value)
{
    ui->aliasLineEdit->setText(value);
    alias = value;
}

void OpenFileDialog::setSurname(const QString &value)
{
    ui->surnameLineEdit->setText(value);
    surname = value;
}

void OpenFileDialog::setName(const QString &value)
{
    ui->nameLineEdit->setText(value);
    name = value;
}

QString OpenFileDialog::getAvatar() const
{
    return avatar;
}

QString OpenFileDialog::getName() const
{
    return name;
}

QString OpenFileDialog::getSurname() const
{
    return surname;
}

QString OpenFileDialog::getAlias() const
{
    return alias;
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
    avatar.clear();
    name.clear();
    surname.clear();
    alias.clear();
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
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(link);
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
    name = ui->nameLineEdit->text();
    surname = ui->surnameLineEdit->text();
    alias = ui->aliasLineEdit->text();
    avatar = ui->changeImageButton->whatsThis();
    emit updateAccountRequest(name, surname, alias, avatar);
}

void OpenFileDialog::on_LogoutButton_clicked()
{
    emit logoutRequest();
}

bool OpenFileDialog::checkEditForm() {

    bool isValidName = regexMatch(ui->nameLineEdit->text(), NAME_REGEX, showHintName, ui->editHintLabel, "*name must contain only alphabetic characters");
    if (!isValidName) return false;

    bool isValidSurname = regexMatch(ui->surnameLineEdit->text(), SURNAME_REGEX, showHintSurname, ui->editHintLabel, "*surname must contain only alphabetic characters");
    if (!isValidSurname) return false;

    bool isValidAlias = regexMatch(ui->aliasLineEdit->text(), USERNAME_REGEX, showHintAlias, ui->editHintLabel, "*insert a valid alias");
    if (!isValidAlias) return false;

    return isValidName && isValidSurname && isValidAlias;
}

bool OpenFileDialog::chackEditChanges()
{
    bool isChanged = name != ui->nameLineEdit->text()
            || surname != ui->surnameLineEdit->text()
            || alias != ui->aliasLineEdit->text()
            || avatar != ui->changeImageButton->whatsThis();
    return isChanged;
}

bool OpenFileDialog::regexMatch(const QString& value, QRegularExpression regex, bool canShowHint, QLabel* hintLabel, const QString& hintMessage) {
    if (!(regex).match(value).hasMatch()) {
        if (canShowHint)
            hintLabel->setText(hintMessage);
        return false;
    }
    hintLabel->setText("");
    return true;
}

void OpenFileDialog::on_nameLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )

    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());
}

void OpenFileDialog::on_surnameLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());
}

void OpenFileDialog::on_aliasLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());

}

void OpenFileDialog::on_nameLineEdit_editingFinished()
{
    showHintName = ui->nameLineEdit->text().size() > 0;
    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());
}

void OpenFileDialog::on_surnameLineEdit_editingFinished()
{
    showHintSurname = ui->surnameLineEdit->text().size() > 0;
    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());
}

void OpenFileDialog::on_aliasLineEdit_editingFinished()
{
    showHintAlias = ui->aliasLineEdit->text().size() > 0;
    ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());
}
