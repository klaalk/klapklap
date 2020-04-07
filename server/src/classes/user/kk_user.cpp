#include "kk_user.h"

KKUser::KKUser()
{

}

QString KKUser::getId() const
{
    return id;
}

void KKUser::setId(const QString &value)
{
    id = value;
}

QString KKUser::getName() const
{
    return name;
}

void KKUser::setName(const QString &value)
{
    name = value;
}

QString KKUser::getSurname() const
{
    return surname;
}

void KKUser::setSurname(const QString &value)
{
    surname = value;
}

QString KKUser::getEmail() const
{
    return email;
}

void KKUser::setEmail(const QString &value)
{
    email = value;
}

QString KKUser::getUsername() const
{
    return username;
}

void KKUser::setUsername(const QString &value)
{
    username = value;
}

QString KKUser::getImage() const
{
    return image;
}

void KKUser::setImage(const QString &value)
{
    image = value;
}

QString KKUser::getRegistrationDate() const
{
    return registrationDate;
}

void KKUser::setRegistrationDate(const QString &value)
{
    registrationDate = value;
}

QString KKUser::getPassword() const
{
    return password;
}

void KKUser::setPassword(const QString &value)
{
    password = value;
}
