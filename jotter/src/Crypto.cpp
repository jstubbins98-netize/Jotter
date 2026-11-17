#include "Crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <memory>
#include <stdexcept>

namespace {
using Context = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;
constexpr char Aad[] = "Jotter encrypted journal v1";
void require(bool ok) {
    if (!ok) throw std::runtime_error("Encryption operation failed.");
}
}
namespace Crypto {
void wipe(QByteArray &bytes) {
    if (!bytes.isEmpty()) OPENSSL_cleanse(bytes.data(), size_t(bytes.size()));
    bytes.clear();
}
QByteArray random(int count) {
    QByteArray result(count, '\0');
    require(RAND_bytes(reinterpret_cast<unsigned char *>(result.data()), count) == 1);
    return result;
}
QByteArray derive(const QByteArray &password, const QByteArray &salt) {
    require(salt.size() == 16);
    QByteArray key(32, '\0');
    require(PKCS5_PBKDF2_HMAC(password.constData(), int(password.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()), int(salt.size()),
        Iterations, EVP_sha256(), int(key.size()),
        reinterpret_cast<unsigned char *>(key.data())) == 1);
    return key;
}
QByteArray encrypt(const QByteArray &plain, const QByteArray &key,
                   const QByteArray &nonce, QByteArray &tag) {
    require(key.size() == 32 && nonce.size() == 12);
    Context ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    require(bool(ctx));
    require(EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr,
        reinterpret_cast<const unsigned char *>(key.constData()),
        reinterpret_cast<const unsigned char *>(nonce.constData())) == 1);
    int length = 0, tail = 0;
    require(EVP_EncryptUpdate(ctx.get(), nullptr, &length,
        reinterpret_cast<const unsigned char *>(Aad), sizeof(Aad) - 1) == 1);
    QByteArray cipher(plain.size() + 16, '\0');
    require(EVP_EncryptUpdate(ctx.get(), reinterpret_cast<unsigned char *>(cipher.data()),
        &length, reinterpret_cast<const unsigned char *>(plain.constData()), int(plain.size())) == 1);
    require(EVP_EncryptFinal_ex(ctx.get(),
        reinterpret_cast<unsigned char *>(cipher.data()) + length, &tail) == 1);
    cipher.resize(length + tail);
    tag.resize(16);
    require(EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, 16, tag.data()) == 1);
    return cipher;
}
QByteArray decrypt(const QByteArray &cipher, const QByteArray &key,
                   const QByteArray &nonce, const QByteArray &tag) {
    require(key.size() == 32 && nonce.size() == 12 && tag.size() == 16);
    Context ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    require(bool(ctx));
    require(EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr,
        reinterpret_cast<const unsigned char *>(key.constData()),
        reinterpret_cast<const unsigned char *>(nonce.constData())) == 1);
    int length = 0, tail = 0;
    require(EVP_DecryptUpdate(ctx.get(), nullptr, &length,
        reinterpret_cast<const unsigned char *>(Aad), sizeof(Aad) - 1) == 1);
    QByteArray plain(cipher.size() + 16, '\0');
    require(EVP_DecryptUpdate(ctx.get(), reinterpret_cast<unsigned char *>(plain.data()),
        &length, reinterpret_cast<const unsigned char *>(cipher.constData()), int(cipher.size())) == 1);
    require(EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, 16,
        const_cast<char *>(tag.constData())) == 1);
    if (EVP_DecryptFinal_ex(ctx.get(),
        reinterpret_cast<unsigned char *>(plain.data()) + length, &tail) != 1) {
        wipe(plain);
        throw std::runtime_error("Incorrect password, or this journal is damaged.");
    }
    plain.resize(length + tail);
    return plain;
}
}
