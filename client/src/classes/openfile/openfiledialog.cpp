#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#include <QClipboard>
#include <QDateTime>
#include <QImageWriter>
#include <QMessageBox>
#include <QStandardPaths>
#include <QPainter>
#include <QPainterPath>

#define COLUMN_NAME_SIZE 200
#define COLUMN_DATE_SIZE 150
#define COLUMN_ACCESS_SIZE 150
#define COLUMN_DELETE_SIZE 20


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
    delete crypt;
    delete fileNameRegexp;
}


void OpenFileDialog::closeEvent(QCloseEvent *e)
{
    e->ignore();
    clear();
    emit closed();
}

void OpenFileDialog::initializeFilesTableView() {
    ui->filesTableWidget->clear();
    ui->filesTableWidget->setColumnCount(4);
    ui->filesTableWidget->setHorizontalHeaderLabels({"Nome file", "Ultimo accesso", "Data partecipazione", ""});
    ui->filesTableWidget->verticalHeader()->setVisible(false);

    ui->filesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filesTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->filesTableWidget->setColumnWidth(0, COLUMN_NAME_SIZE);
    ui->filesTableWidget->setColumnWidth(1, COLUMN_ACCESS_SIZE);
    ui->filesTableWidget->setColumnWidth(2, COLUMN_DATE_SIZE);
    ui->filesTableWidget->setColumnWidth(3, COLUMN_DELETE_SIZE);
    ui->filesTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void OpenFileDialog::setUser(KKUser* user) {

    ui->usernameLabel->setText("Utente: " + user->getUsername());
    ui->emailLabel->setText("Email: " + user->getEmail());

    QDateTime registrationDateTime = QDateTime::fromString(user->getRegistrationDate(), Qt::ISODate);
    ui->registrationDateLabel->setText("Registrato il " + registrationDateTime.toString(DATE_TIME_FORMAT));

    avatar = user->getImage();
    setAvatar(user->getImage());
    setName(user->getName());
    setSurname(user->getSurname());
    setAlias(user->getAlias());
    setUsername(user->getUsername());
    ui->saveChangesButton->setEnabled(chackEditChanges());

}

void OpenFileDialog::setUserFiles(const QStringList &files)
{
    this->files.clear();
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

        const QPixmap orig = QPixmap(":/images/avatars/"+avatar);

        int size = qMax(orig.width(), orig.height());

        QPixmap rounded = QPixmap(size, size);
        rounded.fill(Qt::transparent);

        QPainterPath path;
        path.addEllipse(rounded.rect());

        QPainter painter(&rounded);

        painter.setClipPath(path);

        painter.fillRect(rounded.rect(), Qt::black);
        painter.setRenderHint(QPainter::Antialiasing);

        int x = qAbs(orig.width() - size) / 2;
        int y = qAbs(orig.height() - size) / 2;
        painter.drawPixmap(x, y, orig.width(), orig.height(), orig);

        rounded.setDevicePixelRatio(10);


        ui->changeImageButton->setIcon(QIcon(rounded.scaled(1110,800,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        ui->changeImageButton->setWhatsThis(avatar);
        ui->saveChangesButton->setEnabled(checkEditForm() && chackEditChanges());

    } else
       KKLogger::log(QString("[setUserAvatar] Avatar not exist: %1").arg(path), username);

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

    QString filename = splittedFileRow[0];
    QString hashname = splittedFileRow[1];
    QDateTime creationDateTime = QDateTime::fromString(splittedFileRow[2], Qt::ISODate);
    QDateTime lastAccessDateTime = QDateTime::fromString(splittedFileRow[3], Qt::ISODate);

    files.insert(filename, hashname);
    QTableWidgetItem* name =  new QTableWidgetItem(filename);
    QTableWidgetItem* access = new QTableWidgetItem(lastAccessDateTime.toString(DATE_TIME_FORMAT));
    QTableWidgetItem* date = new QTableWidgetItem(creationDateTime.toString(DATE_TIME_FORMAT));
    QTableWidgetItem* delet = new QTableWidgetItem(QIcon(":/icons/delete-icon.png"), "");

    date->setTextAlignment(Qt::AlignCenter);
    access->setTextAlignment(Qt::AlignCenter);
    delet->setTextAlignment(Qt::AlignCenter);

    ui->filesTableWidget->setItem(fileIndex, 0, name);
    ui->filesTableWidget->setItem(fileIndex, 1, access);
    ui->filesTableWidget->setItem(fileIndex, 2, date);
    ui->filesTableWidget->setItem(fileIndex, 3, delet);

}

QString OpenFileDialog::createInnerFilename(QString filename)
{
    QString innerFilename = filename;
    int identifier = 0;
    for(QString key : files.keys()) {
        if (key.startsWith(innerFilename))
            identifier++;
    }
    if (identifier > 0)
        innerFilename = QString("%1 (%2)").arg(innerFilename, QString::number(identifier+1));

    return innerFilename;
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
    if (item->column() < 3) {
        ui->createFileNameLineEdit->setText(selectedFilename);

        ui->openFileButton->setEnabled(true);
        ui->shareFileButton->setEnabled(true);
    } else if (item->column() == 3) {
        emit deleteFile(files.value(selectedFilename));
    }
}

void OpenFileDialog::on_openFileButton_clicked()
{
    if (pastedLink != nullptr && !pastedLink.isEmpty()) {
        emit openFile(pastedLink, pastedFilename);
    } else {
        selectedLink = (selectedLink != nullptr && !selectedLink.isEmpty()) ? selectedLink
                  : files.value(selectedFilename);

        if (selectedLink != nullptr && !selectedLink.isEmpty()) {
            emit openFile(selectedLink, selectedFilename);
        } else {
            QString newFileName = ui->createFileNameLineEdit->text();
             if (newFileName != "" && newFileName != nullptr) {
                emit openFile(newFileName, newFileName);
            } else {
                 KKLogger::log("[on_openFileButton_clicked] Errore nell'apertura file: nome file vuoto!", username);
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
            QString innerFilename;
            for (auto entry : files.toStdMap()) {
                if (pastedLink == entry.second) {
                    innerFilename = entry.first;
                    break;
                }
            }
            if (innerFilename.isEmpty()) {
                innerFilename = createInnerFilename(splittedDecryptedLink[2]);
            }
            pastedFilename = innerFilename;
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
                selectedLink = files.value(lineEditText);
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
    emit updateAccount(name, surname, alias, avatar);
}

void OpenFileDialog::on_LogoutButton_clicked()
{
    emit logout();
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

QString OpenFileDialog::getUsername() const
{
    return username;
}

void OpenFileDialog::setUsername(const QString &value)
{
    username = value;
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
