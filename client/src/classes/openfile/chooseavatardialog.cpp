#include "chooseavatardialog.h"
#include "ui_chooseavatardialog.h"

ChooseAvatarDialog::ChooseAvatarDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseAvatarDialog)
{
    ui->setupUi(this);
}

ChooseAvatarDialog::~ChooseAvatarDialog()
{
    delete ui;
}

void ChooseAvatarDialog::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel *label = static_cast<QLabel*>(childAt(event->pos()));
    if (label->objectName().startsWith("label")) {
        if (selectedAvatar != nullptr)
            selectedAvatar->setStyleSheet("");
        selectedAvatar = label;
        selectedAvatar->setStyleSheet("QLabel { border: 3px solid black; }");
    }
}

void ChooseAvatarDialog::showAvatars()
{
    QDirIterator it(":/images/avatars", QDirIterator::NoIteratorFlags);
    int i=0;
    int j=0;
    int counter=0;
    while (it.hasNext()) {
        qDebug() << it.next();
        QLabel *image = new QLabel();
        image->setPixmap(QPixmap(it.next()));
        image->setObjectName("label"+QVariant(counter).toString());
        ui->gridLayout->addWidget(image, i, j);
        counter++;
        j++;
        if (j >= 5) {
            j = 0;
            i++;
        }
    }
    show();
}
