#pragma once
#include <QByteArray>

namespace Crypto {
constexpr int Iterations = 600000;
QByteArray random(int count);
QByteArray derive(const QByteArray &password, const QByteArray &salt);
QByteArray encrypt(const QByteArray &plain, const QByteArray &key,
                   const QByteArray &nonce, QByteArray &tag);
QByteArray decrypt(const QByteArray &cipher, const QByteArray &key,
                   const QByteArray &nonce, const QByteArray &tag);
void wipe(QByteArray &bytes);
}
