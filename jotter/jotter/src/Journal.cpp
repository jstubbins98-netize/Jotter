#include "Journal.h"
#include "Crypto.h"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QUuid>
#include <QSet>
#include <stdexcept>

namespace {
void fail(const QString &message) { throw std::runtime_error(message.toStdString()); }
QJsonObject object(const QByteArray &data) {
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        fail("This is not a valid Jotter journal.");
    return document.object();
}
QByteArray binary(const QJsonObject &o, const char *key) {
    auto result = QByteArray::fromBase64Encoding(o[key].toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    if (!result) fail("Invalid journal encoding.");
    return result.decoded;
}
}
namespace JournalFile {
Entry newEntry() {
    return {QUuid::createUuid().toString(QUuid::WithoutBraces), "Untitled entry",
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate), "<p></p>"};
}
QJsonObject read(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) fail(file.errorString());
    if (file.size() > MaximumSize) fail("Journal exceeds the 128 MB file limit.");
    auto envelope = object(file.readAll());
    if (envelope["format"] != "jotter" || envelope["version"].toInt() != 1 ||
        !envelope["secret"].isBool()) fail("Unsupported journal format or version.");
    return envelope;
}
bool isSecret(const QJsonObject &envelope) { return envelope["secret"].toBool(); }
QByteArray salt(const QJsonObject &envelope) {
    if (envelope["kdf"] != "pbkdf2-sha256" ||
        envelope["iterations"].toInt() != Crypto::Iterations ||
        envelope["cipher"] != "aes-256-gcm") fail("Unsupported encryption settings.");
    auto result = binary(envelope, "salt");
    if (result.size() != 16) fail("Invalid encryption salt.");
    return result;
}
Journal decode(const QJsonObject &envelope, const QByteArray &key) {
    QJsonObject data;
    if (isSecret(envelope)) {
        salt(envelope);
        QByteArray plain = Crypto::decrypt(binary(envelope, "data"), key,
            binary(envelope, "nonce"), binary(envelope, "tag"));
        try { data = object(plain); } catch (...) { Crypto::wipe(plain); throw; }
        Crypto::wipe(plain);
    } else data = envelope["journal"].toObject();
    if (!data["name"].isString() || !data["entries"].isArray())
        fail("Journal content is invalid.");
    Journal journal{data["name"].toString(), {}};
    QSet<QString> ids;
    for (const auto &value : data["entries"].toArray()) {
        auto o = value.toObject();
        if (!o["id"].isString() || !o["title"].isString() ||
            !o["created"].isString() || !o["html"].isString())
            fail("An entry is invalid.");
        auto id = o["id"].toString();
        if (id.isEmpty() || ids.contains(id)) fail("Invalid entry identifier.");
        ids.insert(id);
        journal.entries.push_back({id, o["title"].toString(),
            o["created"].toString(), o["html"].toString()});
    }
    return journal;
}
void save(const QString &path, const Journal &journal,
          const QByteArray &key, const QByteArray &saltValue) {
    QJsonArray entries;
    for (const auto &e : journal.entries)
        entries.append(QJsonObject{{"id", e.id}, {"title", e.title},
            {"created", e.created}, {"html", e.html}});
    QJsonObject data{{"name", journal.name}, {"entries", entries}};
    QJsonObject envelope{{"format", "jotter"}, {"version", 1}, {"secret", !key.isEmpty()}};
    if (!key.isEmpty()) {
        if (saltValue.size() != 16) fail("Invalid encryption salt.");
        auto nonce = Crypto::random(12);
        QByteArray tag;
        auto plain = QJsonDocument(data).toJson(QJsonDocument::Compact);
        QByteArray cipher;
        try { cipher = Crypto::encrypt(plain, key, nonce, tag); }
        catch (...) { Crypto::wipe(plain); throw; }
        Crypto::wipe(plain);
        envelope["kdf"] = "pbkdf2-sha256";
        envelope["iterations"] = Crypto::Iterations;
        envelope["cipher"] = "aes-256-gcm";
        envelope["salt"] = QString::fromLatin1(saltValue.toBase64());
        envelope["nonce"] = QString::fromLatin1(nonce.toBase64());
        envelope["tag"] = QString::fromLatin1(tag.toBase64());
        envelope["data"] = QString::fromLatin1(cipher.toBase64());
    } else envelope["journal"] = data;
    auto bytes = QJsonDocument(envelope).toJson(QJsonDocument::Compact);
    if (bytes.size() > MaximumSize) fail("Journal exceeds the 128 MB limit. Remove images or split it into multiple journals.");
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        fail("Could not save journal: " + file.errorString());
}
}
