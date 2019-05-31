//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//


#ifndef SIMPLECRYPT_H
#define SIMPLECRYPT_H
#include <QString>
#include <QVector>
#include <QFlags>


class SimpleCrypt
{
public:

    enum CompressionMode {
        CompressionAuto,
        CompressionAlways,
        CompressionNever
    };

    enum IntegrityProtectionMode {
        ProtectionNone,
        ProtectionChecksum,
        ProtectionHash
    };

    enum Error {
        ErrorNoError,
        ErrorNoKeySet,
        ErrorUnknownVersion,
        ErrorIntegrityFailed,
    };


    SimpleCrypt();

    explicit SimpleCrypt(quint64 key);


    void setKey(quint64 key);

    bool hasKey() const {return !m_keyParts.isEmpty();}


    void setCompressionMode(CompressionMode mode) {m_compressionMode = mode;}

    void setIntegrityProtectionMode(IntegrityProtectionMode mode) {m_protectionMode = mode;}

    IntegrityProtectionMode integrityProtectionMode() const {return m_protectionMode;}


    Error lastError() const {return m_lastError;}


    QString encryptToString(const QString& plaintext) ;

    QString encryptToString(QByteArray plaintext) ;

    QByteArray encryptToByteArray(const QString& plaintext) ;

    QByteArray encryptToByteArray(QByteArray plaintext) ;


    QString decryptToString(const QString& cyphertext) ;

    QByteArray decryptToByteArray(const QString& cyphertext) ;

    QString decryptToString(QByteArray cypher) ;

    QByteArray decryptToByteArray(QByteArray cypher) ;

    QString random_psw(QString s);


    enum CryptoFlag{CryptoFlagNone = 0,
        CryptoFlagCompression = 0x01,
        CryptoFlagChecksum = 0x02,
        CryptoFlagHash = 0x04
    };
    Q_DECLARE_FLAGS(CryptoFlags, CryptoFlag);
private:

    void splitKey();

    quint64 m_key;
    QVector<char> m_keyParts;
    CompressionMode m_compressionMode;
    IntegrityProtectionMode m_protectionMode;
    Error m_lastError;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SimpleCrypt::CryptoFlags)

#endif // SimpleCrypt_H