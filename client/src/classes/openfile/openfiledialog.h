#ifndef OPENFILEDIALOG_H
#define OPENFILEDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QAbstractButton>
#include <QTableWidgetItem>
#include "../../libs/src/classes/crypt/kk_crypt.h"

namespace Ui {
class OpenFileDialog;
}

class OpenFileDialog : public QDialog
{
    Q_OBJECT
signals:
    void openFileRequest(QString fileName);

public:
    explicit OpenFileDialog(QWidget *parent = nullptr);
    ~OpenFileDialog();

    void setUserInfo(QStringList info);
    void addFile(int fileIndex, QString fileName);

private slots:
    void on_accountBtn_clicked();
    void on_documentiBtn_clicked();

    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_openFileButton_clicked();
    void on_shareFileButton_clicked();

    void on_createFileNameLineEdit_textChanged(const QString &arg1);

private:
    void initializeFilesTableView();

    Ui::OpenFileDialog *ui;
    QMap<QString, QString> files_;
};

#endif // OPENFILEDIALOG_H
