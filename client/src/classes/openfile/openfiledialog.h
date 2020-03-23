#ifndef OPENFILEDIALOG_H
#define OPENFILEDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QAbstractButton>
#include <QTableWidgetItem>

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
    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_openFileButton_clicked();
    void on_createFileButton_clicked();
    void on_accountBtn_clicked();
    void on_documentiBtn_clicked();

    void on_shareFileButton_clicked();

private:
    void initializeFilesTableView();

    Ui::OpenFileDialog *ui;
    QMap<QString, QString> files_;
    QString selectedFile;
};

#endif // OPENFILEDIALOG_H
