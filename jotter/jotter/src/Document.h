#pragma once
#include <QTextDocument>
#include <QTextEdit>

// Local files and network resources are deliberately never loaded from journal HTML.
class Document : public QTextDocument {
public:
    explicit Document(QObject *parent = nullptr);
protected:
    QVariant loadResource(int type, const QUrl &name) override;
};
class Editor : public QTextEdit {
public:
    explicit Editor(QWidget *parent = nullptr);
protected:
    void insertFromMimeData(const QMimeData *source) override;
};
