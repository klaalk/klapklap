#include "kk_modal.h"

#include <utility>

KKModal::KKModal(QWidget *parent) :
    QMessageBox(parent),
    primaryBtn(new QPushButton(this))
{
    connect(this, &QMessageBox::buttonClicked, this, &KKModal::on_Button_clicked);
    connect(this, &QMessageBox::rejected, this, &KKModal::on_Modal_rejected);

    primaryBtn->setProperty("class", "PrimaryBtn");
    addButton(primaryBtn, ButtonRole::NoRole);

    setWindowTitle("Notifica");
}

KKModal::~KKModal()
{
    delete primaryBtn;
}

void KKModal::setModal(const QString& text, const QString& btnText, QString modalType) {
    setText(text);
    setModalType(modalType);
    if (modalType.endsWith("Error")) {
        setIcon(QMessageBox::Icon::Critical);
    } else if (modalType.endsWith("Illegal")) {
        setIcon(QMessageBox::Icon::Warning);
    } else {
        setIcon(QMessageBox::Icon::Information);
    }
    primaryBtn->setText(btnText);
}

void KKModal::on_Modal_rejected()
{
    emit modalClosed(modalType);
}

void KKModal::on_Button_clicked(QAbstractButton *button)
{
    emit modalButtonClicked(button->text(), getModalType());
}

QString KKModal::getModalType() const
{
    return modalType;
}

void KKModal::setModalType(const QString &value)
{
    modalType = value;
}
