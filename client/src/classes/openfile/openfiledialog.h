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

#include <classes/openfile/chooseavatardialog.h>
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
    void updateAccountRequest(QString name, QString surname, QString alias, QString blobImage);
    void logoutRequest();
public:
    explicit OpenFileDialog(QWidget *parent = nullptr);
    ~OpenFileDialog();

    void setUserInfo(const QStringList& info);
    void setUserFiles(const QStringList& files);
    void setUserAvatar(const QString &avatar);
    void addFile(int fileIndex, const QString& fileName);

private slots:
    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_openFileButton_clicked();
    void on_shareFileButton_clicked();
    void on_changeImageButton_clicked();
    void on_createFileNameLineEdit_textChanged(const QString &arg1);
    void on_saveChangesButton_clicked();
    void on_LogoutButton_clicked();

private:
    void initializeFilesTableView();

    Ui::OpenFileDialog *ui;
    KKCrypt* crypt;
    QRegularExpression *fileNameRegexp;

    QString selectedFilename;
    QString selectedLink;

    QString pastedLink;
    QString pastedFilename;

    QString avatar;

    QMap<QString, QString> files_;

    ShareFileDialog shareFileDialog;
    ChooseAvatarDialog chooseAvatarDialog;
};

#endif // OPENFILEDIALOG_H
