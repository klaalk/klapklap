#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#include <QDateTime>
#include <QImageWriter>
#include <QMessageBox>
#include <QStandardPaths>

#define MAX_TABLE_SIZE 534

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

void OpenFileDialog::setUserInfo(const QStringList& info) {
    ui->nameLineEdit->insert(info.value(0));
    ui->surnameLineEdit->insert(info.value(1));
    ui->emailLineEdit->insert(info.value(2));
    ui->passwordLineEdit->insert(crypt->decryptToString(info.value(3)));
    ui->usernameLineEdit->insert(info.value(4));

    QString registrationDate = info.value(5);
    QDateTime registrationDateTime = QDateTime::fromString(registrationDate, Qt::ISODate);
    ui->registrationDateLabel->setText("Data di registrazione: " + registrationDateTime.toString(DATE_TIME_FORMAT));

    QStringList filesList = info.mid(6, info.size()-1);
    ui->filesTableWidget->setRowCount(filesList.size());

    int fileIndex = 0;
    for(const QString& fileName : filesList) {
        addFile(fileIndex, fileName);
        fileIndex++;
    }
}

void OpenFileDialog::addFile(int fileIndex, const QString& fileRow) {
    QStringList splittedFileRow = fileRow.split(FILENAME_SEPARATOR);
    QString fileName = crypt->decryptToString(splittedFileRow[0]);
    QStringList splittedFilename = fileName.split(FILENAME_SEPARATOR);

    files_.insert(splittedFilename[2], splittedFileRow[0]);
    ui->filesTableWidget->setItem(fileIndex, 0, new QTableWidgetItem(splittedFilename[2]));
    ui->filesTableWidget->setItem(fileIndex, 1, new QTableWidgetItem(splittedFilename[1]));

    QDateTime creationDateTime = QDateTime::fromString(splittedFileRow[1], Qt::ISODate);
    ui->filesTableWidget->setItem(fileIndex, 2, new QTableWidgetItem(creationDateTime.toString(DATE_TIME_FORMAT)));

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
        qDebug() << "APRO FILE CONDIVISO: " << pastedFilename;
    } else {
        selectedLink = (selectedLink != nullptr && !selectedLink.isEmpty()) ? selectedLink
                  : files_.value(selectedFilename);

        if (selectedLink != nullptr && !selectedLink.isEmpty()) {
            emit openFileRequest(selectedLink, selectedFilename);
            qDebug() << "APRO FILE ESISTENTE: " << selectedFilename;
        } else {
            QString newFileName = ui->createFileNameLineEdit->text();
             if (newFileName != "" && newFileName != nullptr) {
                emit openFileRequest(newFileName, newFileName);
                qDebug() << "APRO UN NUOVO FILE: " << newFileName;
            } else {
                qDebug() << "ERRORE NOME FILE DA CREARE VUOTO";
            }
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
    QString link = (pastedLink != nullptr && !pastedLink.isEmpty()) ? pastedLink : selectedLink;
    shareFileDialog.setShareFileLink(link);
    shareFileDialog.show();
}

void OpenFileDialog::on_changeImageButton_clicked()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
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

void OpenFileDialog::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
            ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

bool OpenFileDialog::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    ui->imageViewer->setScaledContents(false);

    QPixmap pixmap = QPixmap::fromImage(newImage);
    pixmap = pixmap.scaled(ui->imageViewer->width(), ui->imageViewer->height(), Qt::KeepAspectRatio);
    ui->imageViewer->setPixmap(pixmap);

    setWindowFilePath(fileName);
    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
            .arg(QDir::toNativeSeparators(fileName)).arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
    return true;
}
