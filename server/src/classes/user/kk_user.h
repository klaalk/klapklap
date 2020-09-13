#ifndef KKUSER_H
#define KKUSER_H

#include <QSharedPointer>
#include <QString>

class KKUser : public QObject
{
public:
    KKUser();

    QString getId() const;
    void setId(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    QString getSurname() const;
    void setSurname(const QString &value);

    QString getEmail() const;
    void setEmail(const QString &value);

    QString getUsername() const;
    void setUsername(const QString &value);

    QString getImage() const;
    void setImage(const QString &value);

    QString getRegistrationDate() const;
    void setRegistrationDate(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);

    QString getAlias() const;
    void setAlias(const QString &value);

private:
    QString id;
    QString name;
    QString surname;
    QString email;
    QString username;
    QString alias;
    QString image;
    QString registrationDate;
    QString password;
};

typedef QSharedPointer<KKUser> KKUserPtr;

#endif // KKUSER_H
