#include "Export.h"
#include "Document.h"
#include <QPdfWriter>
#include <QPageSize>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QFile>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QImage>
#include <QBuffer>
#include <QUrl>
#include <QDateTime>
#include <zip.h>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace {
void fail(const QString &message) { throw std::runtime_error(message.toStdString()); }
QString xml(const QString &text) {
    QString safe;
    for (auto c : text)
        if (c.unicode() >= 32 || c == '\t' || c == '\n' || c == '\r') safe += c;
    return safe.toHtmlEscaped();
}
QString date(const Entry &entry) {
    return QDateTime::fromString(entry.created, Qt::ISODate).toLocalTime().toString("MMMM d, yyyy · h:mm AP");
}
QString paragraph(const QString &text, bool heading = false) {
    return "<w:p><w:r><w:rPr>" + QString(heading ? "<w:b/><w:sz w:val=\"36\"/>" : "") +
        "</w:rPr><w:t xml:space=\"preserve\">" + xml(text) + "</w:t></w:r></w:p>";
}
void add(zip_t *archive, const QString &path, const QByteArray &bytes) {
    // zip_source_buffer does not own the source here; copy into a malloc-owned buffer.
    void *copy = malloc(size_t(bytes.size()));
    if (!copy) fail("Not enough memory to export.");
    memcpy(copy, bytes.constData(), size_t(bytes.size()));
    auto *source = zip_source_buffer(archive, copy, zip_uint64_t(bytes.size()), 1);
    if (!source) { free(copy); fail("Could not create Word document."); }
    if (zip_file_add(archive, path.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(source); fail("Could not add Word document content.");
    }
}
QString drawing(int id, qint64 width, qint64 height) {
    return QString(
        "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
        "<wp:extent cx=\"%2\" cy=\"%3\"/><wp:docPr id=\"%1\" name=\"Image %1\"/>"
        "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
        "<pic:pic><pic:nvPicPr><pic:cNvPr id=\"%1\" name=\"Image %1\"/><pic:cNvPicPr/></pic:nvPicPr>"
        "<pic:blipFill><a:blip r:embed=\"img%1\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>"
        "<pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%2\" cy=\"%3\"/></a:xfrm>"
        "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>"
        "</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>").arg(id).arg(width).arg(height);
}
}
namespace Export {
void pdf(const QString &path, const QString &name, const QVector<Entry> &entries) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) fail(file.errorString());
    {
        QPdfWriter writer(&file);
        writer.setTitle(name);
        writer.setCreator("Jotter");
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(18, 18, 18, 18));
        Document output;
        QTextCursor cursor(&output);
        QTextCharFormat heading;
        heading.setFontPointSize(22);
        heading.setFontWeight(QFont::Bold);
        for (int i = 0; i < entries.size(); ++i) {
            if (i) {
                QTextBlockFormat page;
                page.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
                cursor.insertBlock(page);
            }
            cursor.insertText(entries[i].title, heading);
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            cursor.insertText(date(entries[i]));
            cursor.insertBlock();
            cursor.insertHtml(entries[i].html);
        }
        output.print(&writer);
    }
    if (!file.commit()) fail("PDF could not be saved: " + file.errorString());
}
void docx(const QString &path, const QString &name, const QVector<Entry> &entries) {
    Q_UNUSED(name);
    QTemporaryDir temporary;
    if (!temporary.isValid()) fail("Could not create export temporary folder.");
    auto tempPath = temporary.filePath("export.docx");
    int error = 0;
    using Archive = std::unique_ptr<zip_t, decltype(&zip_discard)>;
    Archive archive(zip_open(tempPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error), zip_discard);
    if (!archive) fail("Could not create Word archive.");
    QString body, rels;
    int imageId = 0;
    for (int e = 0; e < entries.size(); ++e) {
        if (e) body += "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";
        body += paragraph(entries[e].title, true) + paragraph(date(entries[e]));
        Document document;
        document.setHtml(entries[e].html);
        for (auto block = document.begin(); block.isValid(); block = block.next()) {
            body += "<w:p>";
            for (auto it = block.begin(); !it.atEnd(); ++it) {
                auto fragment = it.fragment();
                if (!fragment.isValid()) continue;
                auto format = fragment.charFormat();
                if (format.isImageFormat()) {
                    const auto imageFormat = format.toImageFormat();
                    auto image = qvariant_cast<QImage>(document.resource(
                        QTextDocument::ImageResource, QUrl(imageFormat.name())));
                    if (image.isNull()) fail("An embedded image could not be read.");
                    QByteArray png;
                    QBuffer buffer(&png);
                    buffer.open(QIODevice::WriteOnly);
                    if (!image.save(&buffer, "PNG")) fail("Could not encode an image.");
                    ++imageId;
                    auto imagePath = QString("media/image%1.png").arg(imageId);
                    add(archive.get(), "word/" + imagePath, png);
                    rels += QString("<Relationship Id=\"img%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%2\"/>").arg(imageId).arg(imagePath);
                    double width = imageFormat.width() > 0 ? imageFormat.width() : image.width();
                    double height = imageFormat.height() > 0 ? imageFormat.height() : image.height();
                    double scale = qMin(1.0, qMin(600.0 / width, 850.0 / height));
                    body += drawing(imageId, qint64(width * scale * 9525), qint64(height * scale * 9525));
                } else {
                    body += "<w:r><w:rPr>";
                    if (format.fontWeight() >= QFont::Bold) body += "<w:b/>";
                    if (format.fontItalic()) body += "<w:i/>";
                    if (format.fontUnderline()) body += "<w:u w:val=\"single\"/>";
                    body += "</w:rPr>";
                    auto text = xml(fragment.text());
                    text.replace('\t', "</w:t><w:tab/><w:t xml:space=\"preserve\">");
                    text.replace(QChar::LineSeparator, "</w:t><w:br/><w:t xml:space=\"preserve\">");
                    text.replace('\n', "</w:t><w:br/><w:t xml:space=\"preserve\">");
                    body += "<w:t xml:space=\"preserve\">" + text + "</w:t>";
                    body += "</w:r>";
                }
            }
            body += "</w:p>";
        }
    }
    const QString header = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
    add(archive.get(), "[Content_Types].xml", (header +
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Default Extension=\"png\" ContentType=\"image/png\"/>"
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
        "</Types>").toUtf8());
    add(archive.get(), "_rels/.rels", (header +
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"doc\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>"
        "</Relationships>").toUtf8());
    add(archive.get(), "word/_rels/document.xml.rels", (header +
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + rels + "</Relationships>").toUtf8());
    add(archive.get(), "word/document.xml", (header +
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
        "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" "
        "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
        "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><w:body>" +
        body + "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/><w:pgMar w:top=\"1020\" w:right=\"1020\" w:bottom=\"1020\" w:left=\"1020\"/></w:sectPr></w:body></w:document>").toUtf8());
    if (zip_close(archive.get()) != 0) fail("Could not finish Word document.");
    archive.release();
    QFile source(tempPath);
    if (!source.open(QIODevice::ReadOnly)) fail(source.errorString());
    QSaveFile target(path);
    if (!target.open(QIODevice::WriteOnly)) fail(target.errorString());
    while (!source.atEnd()) {
        auto chunk = source.read(1024 * 1024);
        if (chunk.isEmpty() && source.error() != QFileDevice::NoError) fail(source.errorString());
        if (target.write(chunk) != chunk.size()) fail(target.errorString());
    }
    if (!target.commit()) fail(target.errorString());
}
}
