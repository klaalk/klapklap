//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include "kk_crypt.h"
#include <QByteArray>
#include <QtDebug>
#include <QtGlobal>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDataStream>
#include <utility> 
KKCrypt::KKCrypt():
    m_key(0),
    m_compressionMode(CompressionAuto),
    m_protectionMode(ProtectionChecksum),
    m_lastError(ErrorNoError)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
}

KKCrypt::KKCrypt(quint64 key):
    m_key(key),
    m_compressionMode(CompressionAuto),
    m_protectionMode(ProtectionChecksum),
    m_lastError(ErrorNoError)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
    splitKey();
}

void KKCrypt::setKey(quint64 key)
{
    m_key = key;
    splitKey();
}

void KKCrypt::splitKey()
{
    m_keyParts.clear();
    m_keyParts.resize(8);
    for (int i=0;i<8;i++) {
        quint64 part = m_key;
        for (int j=i; j>0; j--)
            part = part >> 8;
        part = part & 0xff;
        m_keyParts[i] = static_cast<char>(part);
    }
}

QByteArray KKCrypt::encryptToByteArray(const QString& plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();
    return encryptToByteArray(plaintextArray);
}

QByteArray KKCrypt::encryptToByteArray(QByteArray plaintext)
{
    if (m_keyParts.isEmpty()) {
        qWarning() << "No key set.";
        m_lastError = ErrorNoKeySet;
        return QByteArray();
    }


    QByteArray ba = std::move(plaintext);

    CryptoFlags flags = CryptoFlagNone;
    if (m_compressionMode == CompressionAlways) {
        ba = qCompress(ba, 9); //maximum compression
        flags |= CryptoFlagCompression;
    } else if (m_compressionMode == CompressionAuto) {
        QByteArray compressed = qCompress(ba, 9);
        if (compressed.count() < ba.count()) {
            ba = compressed;
            flags |= CryptoFlagCompression;
        }
    }

    QByteArray integrityProtection;
    if (m_protectionMode == ProtectionChecksum) {
        flags |= CryptoFlagChecksum;
        QDataStream s(&integrityProtection, QIODevice::WriteOnly);
        s << qChecksum(ba.constData(), ba.length());
    } else if (m_protectionMode == ProtectionHash) {
        flags |= CryptoFlagHash;
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);

        integrityProtection += hash.result();
    }

    //prepend a random char to the string
    char randomChar = char(qrand() & 0xFF);
    ba = randomChar + integrityProtection + ba;

    int pos(0);
    char lastChar(0);

    int cnt = ba.count();

    while (pos < cnt) {
        ba[pos] = ba.at(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
        lastChar = ba.at(pos);
        ++pos;
    }

    QByteArray resultArray;
    resultArray.append(char(0x03));  //version for future updates to algorithm
    resultArray.append(char(flags)); //encryption flags
    resultArray.append(ba);

    m_lastError = ErrorNoError;
    return resultArray;
}

QString KKCrypt::encryptToString(const QString& plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();
    QByteArray cypher = encryptToByteArray(plaintextArray);
    QString cypherString = QString::fromLatin1(cypher.toBase64());
    return cypherString;
}

QString KKCrypt::encryptToString(QByteArray plaintext)
{
    QByteArray cypher = encryptToByteArray(std::move(plaintext));
    QString cypherString = QString::fromLatin1(cypher.toBase64());
    return cypherString;
}

QString KKCrypt::decryptToString(const QString &cyphertext)
{
    QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    QByteArray plaintextArray = decryptToByteArray(cyphertextArray);
    QString plaintext = QString::fromUtf8(plaintextArray, plaintextArray.size());

    return plaintext;
}

QString KKCrypt::decryptToString(QByteArray cypher)
{
    QByteArray ba = decryptToByteArray(std::move(cypher));
    QString plaintext = QString::fromUtf8(ba, ba.size());

    return plaintext;
}

QByteArray KKCrypt::decryptToByteArray(const QString& cyphertext)
{
    QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    QByteArray ba = decryptToByteArray(cyphertextArray);

    return ba;
}

QByteArray KKCrypt::decryptToByteArray(const QByteArray& cypher)
{
    if (m_keyParts.isEmpty()) {
        qWarning() << "No key set.";
        m_lastError = ErrorNoKeySet;
        return QByteArray();
    }

    QByteArray ba = cypher;

    if( cypher.count() < 3 )
        return QByteArray();

    char version = ba.at(0);

    if (version != 3) {  //we only work with version 3
        m_lastError = ErrorUnknownVersion;
        qWarning() << "Invalid version or not a cyphertext.";
        return QByteArray();
    }

    CryptoFlags flags = CryptoFlags(ba.at(1));

    ba = ba.mid(2);
    int pos(0);
    int cnt(ba.count());
    char lastChar = 0;

    while (pos < cnt) {
        char currentChar = ba[pos];
        ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
        lastChar = currentChar;
        ++pos;
    }

    ba = ba.mid(1); //chop off the random number at the start

    bool integrityOk(true);
    if (flags.testFlag(CryptoFlagChecksum)) {
        if (ba.length() < 2) {
            m_lastError = ErrorIntegrityFailed;
            return QByteArray();
        }
        quint16 storedChecksum;
        {
            QDataStream s(&ba, QIODevice::ReadOnly);
            s >> storedChecksum;
        }
        ba = ba.mid(2);
        quint16 checksum = qChecksum(ba.constData(), ba.length());
        integrityOk = (checksum == storedChecksum);
    } else if (flags.testFlag(CryptoFlagHash)) {
        if (ba.length() < 20) {
            m_lastError = ErrorIntegrityFailed;
            return QByteArray();
        }
        QByteArray storedHash = ba.left(20);
        ba = ba.mid(20);
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);
        integrityOk = (hash.result() == storedHash);
    }

    if (!integrityOk) {
        m_lastError = ErrorIntegrityFailed;
        return QByteArray();
    }

    if (flags.testFlag(CryptoFlagCompression))
        ba = qUncompress(ba);

    m_lastError = ErrorNoError;
    return ba;
}

QString KKCrypt::random_psw(QString s){

    int len=20;
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[static_cast<unsigned long>(rand()) % (sizeof(alphanum) - 1)];
    }


    return s;
}

bool KKCrypt::containLetter(QChar letter, const QString& myString){
    foreach(QChar c, myString)
        if(letter==c)
            return true;
    return false;
}


bool KKCrypt::isEncryptedLink(const QString &cyphertext)
{
    QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());

    if (m_keyParts.isEmpty()) {
        return false;
    }

    QByteArray ba = cyphertextArray;

    if( cyphertextArray.count() < 3 )
        return false;

    char version = ba.at(0);

    if (version != 3) {  //we only work with version 3
        return false;
    }

    return true;
}
