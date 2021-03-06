#include "chooseavatardialog.h"
#include "ui_chooseavatardialog.h"

ChooseAvatarDialog::ChooseAvatarDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseAvatarDialog)
{
    loadAvatars();
    selectedAvatar = nullptr;
    ui->setupUi(this);
}

ChooseAvatarDialog::~ChooseAvatarDialog()
{
    for(QLabel *avatar : avatars)
        delete avatar;

    delete ui;
}

void ChooseAvatarDialog::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget *widget = childAt(event->pos());
    if (widget != nullptr && widget->objectName().startsWith("avatar")) {
        if (selectedAvatar != nullptr)
            selectedAvatar->setStyleSheet("");

        selectedAvatar = static_cast<QLabel*>(widget);
        selectedAvatar->setStyleSheet("QLabel { border: 3px solid black; }");
    }
}

void ChooseAvatarDialog::showAvatars()
{
    int i=0;
    int j=0;
    for(QLabel* avatar : avatars) {
        ui->gridLayout->addWidget(avatar, i, j++);
        if (j >= 5) {
            j = 0;
            i++;
        }
    }
    show();
}

void ChooseAvatarDialog::loadAvatars()
{
    QDirIterator it(":/images/avatars", QDirIterator::NoIteratorFlags);
    QStringList avatarsImages;
    while (it.hasNext()) {
        avatarsImages.push_back(it.next());
    }

    int counter=0;

    for(QString avatarImage : avatarsImages) {
        QLabel *avatar = new QLabel();
        QPixmap avpix(avatarImage);

        #ifdef Q_OS_MACOS

        avpix.setDevicePixelRatio(10);

        avatar->setPixmap(avpix.scaled(1110,800,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
         #endif

         #ifndef Q_OS_MACOS
        avatar->setPixmap(avpix);
         #endif


        avatar->setObjectName("avatar"+avatarImage.split(":/images/avatars/")[1]);
        avatars.push_back(avatar);
        counter++;
    }
}

void ChooseAvatarDialog::on_applyBtn_clicked()
{
    if (selectedAvatar != nullptr) {
        QString avatar = selectedAvatar->objectName();
        emit updateAvatarRequest(avatar.split("avatar")[1]);
    }
    hide();
}

void ChooseAvatarDialog::on_cancelBtn_clicked()
{
    hide();
}
