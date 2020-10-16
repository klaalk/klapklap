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

#include "../../../../libs/src/classes/user/kk_user.h"
#include "../../libs/src/classes/crypt/kk_crypt.h"
#include "../../libs/src/classes/logger/kk_logger.h"
#include "../../libs/src/constants/kk_constants.h"

#include <classes/openfile/chooseavatardialog.h>

namespace Ui {
class OpenFileDialog;
}

class OpenFileDialog : public QDialog
{
    Q_OBJECT
signals:
    void openFile(QString link, QString fileName);
    void updateAccount(QString name, QString surname, QString alias, QString blobImage);
    void logout();
    void closed();

public:
    explicit OpenFileDialog(QWidget *parent = nullptr);
    ~OpenFileDialog() override;

    void setUser(KKUser* info);
    void setAvatar(const QString &avatar);
    void setUserFiles(const QStringList& files);    
    void setName(const QString &value);
    void setSurname(const QString &value);
    void setAlias(const QString &value);
    QString getAvatar() const;
    QString getName() const;
    QString getSurname() const;
    QString getAlias() const;
    void clear();

    QString getUsername() const;
    void setUsername(const QString &value);

private slots:
    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_openFileButton_clicked();
    void on_shareFileButton_clicked();
    void on_changeImageButton_clicked();
    void on_createFileNameLineEdit_textChanged(const QString &arg1);
    void on_saveChangesButton_clicked();
    void on_LogoutButton_clicked();

    void on_nameLineEdit_textChanged(const QString &arg1);

    void on_surnameLineEdit_textChanged(const QString &arg1);

    void on_aliasLineEdit_textChanged(const QString &arg1);

    void on_nameLineEdit_editingFinished();

    void on_surnameLineEdit_editingFinished();

    void on_aliasLineEdit_editingFinished();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void addFile(int fileIndex, const QString& fileName);
    void initializeFilesTableView();
    bool checkEditForm();
    bool chackEditChanges();
    bool regexMatch(const QString& value, QRegularExpression regex, bool canShowHint, QLabel* hintLabel, const QString& hintMessage);


    Ui::OpenFileDialog *ui;
    KKCrypt* crypt;
    QRegularExpression *fileNameRegexp;

    QString selectedFilename;
    QString selectedLink;

    QString pastedLink;
    QString pastedFilename;

    QString avatar;
    QString name;
    QString surname;
    QString alias;
    QString username;

    QMap<QString, QString> files;
    ChooseAvatarDialog chooseAvatarDialog;

    bool showHintName = false;
    bool showHintSurname = false;
    bool showHintAlias = false;
};

#endif // OPENFILEDIALOG_H
