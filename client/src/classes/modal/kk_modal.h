#ifndef MODALDIALOG_H
#define MODALDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QDebug>

class KKModal : public QMessageBox
{
    Q_OBJECT

public:
    explicit KKModal(QWidget *parent = nullptr);
    ~KKModal();
    void setModal(const QString& text, const QString& btnText, QString modalType);

    QString getModalType() const;
    void setModalType(const QString &value);

signals:
    void modalButtonClicked(QString btnName, QString modalType);
    void modalClosed(QString modalType);

private slots:
    void on_Modal_rejected();
    void on_Button_clicked(QAbstractButton *button);

private:
    QString modalType;
    QPushButton* primaryBtn;
};

#endif // MODALDIALOG_H
