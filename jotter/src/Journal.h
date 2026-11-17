#pragma once
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QByteArray>

struct Entry {
    QString id;
    QString title;
    QString created;
    QString html;
};
struct Journal {
    QString name;
    QVector<Entry> entries;
};
namespace JournalFile {
constexpr qint64 MaximumSize = 128 * 1024 * 1024;
QJsonObject read(const QString &path);
bool isSecret(const QJsonObject &envelope);
Journal decode(const QJsonObject &envelope, const QByteArray &key);
void save(const QString &path, const Journal &journal,
          const QByteArray &key, const QByteArray &salt);
QByteArray salt(const QJsonObject &envelope);
Entry newEntry();
}
