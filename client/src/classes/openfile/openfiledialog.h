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
    void addFile(QString fileName);
    ~OpenFileDialog();

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_openBtn_clicked();

private:
    Ui::OpenFileDialog *ui;
    QMap<QString, QString> files_;
    QString selectedFile;
};

#endif // OPENFILEDIALOG_H
