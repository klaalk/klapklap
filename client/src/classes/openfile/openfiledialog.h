#ifndef OPENFILEDIALOG_H
#define OPENFILEDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QAbstractButton>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>
#include <QImageReader>

#include <classes/openfile/sharefiledialog.h>
#include <../../libs/src/classes/crypt/kk_crypt.h>
#include <../../libs/src/constants/kk_constants.h>

namespace Ui {
class OpenFileDialog;
}

class OpenFileDialog : public QDialog
{
    Q_OBJECT
signals:
    void openFileRequest(QString link, QString fileName);

public:
    explicit OpenFileDialog(QWidget *parent = nullptr);
    ~OpenFileDialog();

    void setUserInfo(const QStringList& info);
    void setUserFiles(const QStringList& files);
    void addFile(int fileIndex, const QString& fileName);

private slots:
    void on_accountBtn_clicked();
    void on_documentiBtn_clicked();

    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_openFileButton_clicked();
    void on_shareFileButton_clicked();
    void on_changeImageButton_clicked();

    void on_createFileNameLineEdit_textChanged(const QString &arg1);


private:
    void initializeFilesTableView();
    void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

    void setImage(const QImage &newImage);
    bool loadFile(const QString &fileName);

    Ui::OpenFileDialog *ui;
    KKCrypt* crypt;
    QRegularExpression *fileNameRegexp;

    QString selectedFilename;
    QString selectedLink;

    QString pastedLink;
    QString pastedFilename;

    QMap<QString, QString> files_;

    ShareFileDialog shareFileDialog;
};

#endif // OPENFILEDIALOG_H
