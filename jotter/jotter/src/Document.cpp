#include "Document.h"
#include <QImage>
#include <QUrl>
#include <QMimeData>
#include <QImageReader>
#include <QBuffer>

Document::Document(QObject *parent) : QTextDocument(parent) {
    setDefaultFont(QFont("Segoe UI", 12));
    setDocumentMargin(32);
}
QVariant Document::loadResource(int type, const QUrl &name) {
    const QString prefix = "data:image/png;base64,";
    const auto url = name.toString();
    if (type != QTextDocument::ImageResource || !url.startsWith(prefix)) return {};
    auto bytes = QByteArray::fromBase64(url.mid(prefix.size()).toLatin1());
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "PNG");
    auto size = reader.size();
    if (!size.isValid() || qint64(size.width()) * size.height() > 24000000) return {};
    return reader.read();
}
Editor::Editor(QWidget *parent) : QTextEdit(parent) {
    setDocument(new Document(this));
    setAcceptDrops(false);
    setPlaceholderText("A thought, a memory, a small moment worth keeping…");
}
void Editor::insertFromMimeData(const QMimeData *source) {
    // Paste plain text only: external HTML can reference private files or remote images.
    insertPlainText(source->text());
}
