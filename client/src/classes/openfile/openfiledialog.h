#ifndef OPENFILEDIALOG_H
#define OPENFILEDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QListWidgetItem>
#include <QAbstractButton>

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
    void initInfo(QStringList info);
    void addFile(QString fileName);
    ~OpenFileDialog();

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_openBtn_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_lineEdit_textChanged(const QString &arg1);
private:
    Ui::OpenFileDialog *ui;
    QMap<QString, QString> files_;
    QString selectedFile;
};

#endif // OPENFILEDIALOG_H
