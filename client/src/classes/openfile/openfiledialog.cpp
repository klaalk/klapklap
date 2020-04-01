#include "openfiledialog.h"
#include "ui_openfiledialog.h"

#include <QDateTime>
#include <QImageWriter>
#include <QMessageBox>
#include <QStandardPaths>

#define MAX_TABLE_SIZE 534

#define COLUMN_NAME_SIZE 250
#define COLUMN_CREATOR_SIZE 150
#define DATE_TIME_FORMAT "dd.MM.yyyy hh:mm"

OpenFileDialog::OpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFileDialog),
    crypt(new SimpleCrypt(Q_UINT64_C(0x0c2ad4a4acb9f023)))
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
    ui->passwordLineEdit->insert(crypt->decryptToString(info.value(3)));
    ui->usernameLineEdit->insert(info.value(4));

    QString registrationDate = info.value(5);
    QDateTime registrationDateTime = QDateTime::fromString(registrationDate, Qt::ISODate);
    ui->registrationDateLabel->setText("Data di registrazione: " + registrationDateTime.toString(DATE_TIME_FORMAT));

    QStringList filesList = info.mid(6, info.size()-1);
    ui->filesTableWidget->setRowCount(filesList.size());

    int fileIndex = 0;
    for(QString fileName : filesList) {
        addFile(fileIndex, fileName);
        fileIndex++;
    }
}

void OpenFileDialog::addFile(int fileIndex, QString fileName) {
    QStringList splittedName = fileName.split("#");
    QStringList splittedLink = splittedName[0].split("@");

    files_.insert(splittedLink[2], splittedName[0]);
    ui->filesTableWidget->setItem(fileIndex, 0, new QTableWidgetItem(splittedLink[2]));
    ui->filesTableWidget->setItem(fileIndex, 1, new QTableWidgetItem(crypt->decryptToString(splittedLink[1])));

    QDateTime creationDateTime = QDateTime::fromString(splittedLink[3], Qt::ISODate);
    ui->filesTableWidget->setItem(fileIndex, 2, new QTableWidgetItem(creationDateTime.toString(DATE_TIME_FORMAT)));

}

void OpenFileDialog::on_filesTableWidget_itemClicked(QTableWidgetItem *item)
{
    selectedFileName = ui->filesTableWidget->item(item->row(), 0)->text();
    ui->createFileNameLineEdit->setText(selectedFileName);

    ui->openFileButton->setEnabled(true);
    ui->shareFileButton->setEnabled(true);
}

void OpenFileDialog::on_openFileButton_clicked()
{
    QString newFileName = ui->createFileNameLineEdit->text();
    if(newFileName == selectedFileName) {
        emit openFileRequest(selectedFileName);
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
    QString newFileName = ui->createFileNameLineEdit->text();
    QString completeFileName = files_.value(newFileName);
    ui->createFileNameLineEdit->setText(completeFileName);
}

void OpenFileDialog::on_changeImageButton_clicked()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void OpenFileDialog::on_createFileNameLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    QString newFileName = ui->createFileNameLineEdit->text();

    ui->openFileButton->setEnabled(newFileName.size() > 0);
    ui->shareFileButton->setEnabled(selectedFileName == newFileName);
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
