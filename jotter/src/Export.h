#pragma once
#include <QString>
#include <QVector>
#include "Journal.h"
namespace Export {
void pdf(const QString &path, const QString &name, const QVector<Entry> &entries);
void docx(const QString &path, const QString &name, const QVector<Entry> &entries);
}
