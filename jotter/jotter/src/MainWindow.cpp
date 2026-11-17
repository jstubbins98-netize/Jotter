#include "MainWindow.h"
#include "Document.h"
#include "Crypto.h"
#include "Export.h"
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLockFile>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QTextImageFormat>
#include <QToolBar>
#include <QBuffer>
#include <QRegularExpression>
#include <stdexcept>

MainWindow::MainWindow() {
    resize(1160, 800);
    setMinimumSize(800, 560);
    auto *toolbar = addToolBar("Journal");
    toolbar->setMovable(false);
    auto *newAction = toolbar->addAction("New journal");
    newAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    auto *openAction = toolbar->addAction("Open journal");
    openAction->setShortcut(QKeySequence::Open);
    saveAction = toolbar->addAction("Save");
    saveAction->setShortcut(QKeySequence::Save);
    toolbar->addSeparator();
    addAction = toolbar->addAction("+ New entry");
    addAction->setShortcut(QKeySequence::New);
    imageAction = toolbar->addAction("Import image");
    toolbar->addSeparator();
    exportEntryAction = toolbar->addAction("Export entry");
    lockAction = toolbar->addAction("Lock journal");
    lockAction->setShortcut(QKeySequence("Ctrl+L"));

    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addActions({newAction, openAction, saveAction});
    exportAllAction = fileMenu->addAction("Export entire journal…");
    fileMenu->addAction(exportEntryAction);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);
    auto *journalMenu = menuBar()->addMenu("&Journal");
    journalMenu->addAction(addAction);
    deleteAction = journalMenu->addAction("Delete entry…");
    renameAction = journalMenu->addAction("Rename journal…");
    journalMenu->addAction(lockAction);
    auto *help = menuBar()->addMenu("&Help");
    help->addAction("About Jotter", this, [this] {
        QMessageBox::about(this, "Jotter", "Jotter 1.0\nA local journal for your everyday thoughts.\n\n"
            "Secret journals use AES-256-GCM and PBKDF2-SHA256 (600,000 iterations).\n"
            "Passwords cannot be recovered. Exported documents are not encrypted.\n\n"
            "Save with Ctrl+S. Lock a secret journal with Ctrl+L.\n"
            "Images are stored inside the journal. Pasted text is plain text.");
    });

    auto *splitter = new QSplitter;
    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(24, 28, 24, 24);
    auto *brand = new QLabel("jotter");
    brand->setObjectName("brand");
    sideLayout->addWidget(brand);
    auto *tagline = new QLabel("A little space for your thoughts.");
    tagline->setWordWrap(true);
    sideLayout->addWidget(tagline);
    sideLayout->addSpacing(28);
    journalLabel = new QLabel("Your next chapter");
    journalLabel->setWordWrap(true);
    journalLabel->setObjectName("journalName");
    sideLayout->addWidget(journalLabel);
    search = new QLineEdit;
    search->setPlaceholderText("Find an entry by title…");
    sideLayout->addWidget(search);
    list = new QListWidget;
    list->setSpacing(5);
    sideLayout->addWidget(list, 1);
    secret = new QCheckBox("Keep this journal secret");
    secret->setToolTip("Encrypt every entry and image using a password you choose.");
    sideLayout->addWidget(secret);
    auto *privacy = new QLabel("Stored on your device.\nNo accounts. No cloud.");
    privacy->setObjectName("muted");
    sideLayout->addWidget(privacy);
    splitter->addWidget(sidebar);

    auto *paper = new QWidget;
    auto *paperLayout = new QVBoxLayout(paper);
    paperLayout->setContentsMargins(32, 28, 32, 24);
    title = new QLineEdit;
    title->setObjectName("entryTitle");
    title->setPlaceholderText("A moment worth keeping");
    title->setMaxLength(250);
    paperLayout->addWidget(title);
    dateLabel = new QLabel("Create or open a journal to begin.");
    dateLabel->setObjectName("muted");
    paperLayout->addWidget(dateLabel);
    auto *formatbar = new QToolBar;
    boldAction = formatbar->addAction("Bold");
    italicAction = formatbar->addAction("Italic");
    underlineAction = formatbar->addAction("Underline");
    boldAction->setShortcut(QKeySequence::Bold);
    italicAction->setShortcut(QKeySequence::Italic);
    underlineAction->setShortcut(QKeySequence::Underline);
    for (auto *action : {boldAction, italicAction, underlineAction}) action->setCheckable(true);
    paperLayout->addWidget(formatbar);
    editor = new Editor;
    paperLayout->addWidget(editor, 1);
    splitter->addWidget(paper);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({310, 850});
    setCentralWidget(splitter);
    setStyleSheet(
        "QMainWindow, QDialog { background: #faf8f3; color: #263d35; }"
        "QWidget { font-family: 'Segoe UI'; font-size: 14px; }"
        "QWidget#sidebar { background: #e5ece4; }"
        "QLabel#brand { font-size: 44px; font-weight: 700; color: #2c5344; }"
        "QLabel#journalName { font-size: 20px; font-weight: 600; }"
        "QLabel#muted { color: #61736a; padding: 5px 0; }"
        "QLineEdit { background: #fffdf8; border: 1px solid #c9d3c8; border-radius: 6px; padding: 10px; color: #263d35; }"
        "QLineEdit#entryTitle { font-size: 28px; font-weight: 600; border: none; background: transparent; }"
        "QTextEdit { background: #fffdf8; color: #283a32; border: 1px solid #e1e2d9; border-radius: 8px; selection-background-color: #c5dbc8; }"
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item { padding: 14px 10px; border-radius: 6px; }"
        "QListWidget::item:selected { background: #c9d9ca; color: #203f31; }"
        "QToolBar { border: none; spacing: 6px; padding: 9px; background: #f0f1e9; }"
        "QToolButton, QPushButton { padding: 8px 12px; border: 1px solid transparent; border-radius: 5px; }"
        "QToolButton:hover, QPushButton:hover { background: #d5e1d3; }"
        "QToolButton:checked { background: #c9d9ca; }"
        "QToolButton:disabled { color: #919a92; }"
        "QStatusBar { background: #f0f1e9; color: #52685b; }"
    );

    connect(newAction, &QAction::triggered, this, &MainWindow::newJournal);
    connect(openAction, &QAction::triggered, this, [this] { openJournal(); });
    connect(saveAction, &QAction::triggered, this, [this] { saveJournal(); });
    connect(addAction, &QAction::triggered, this, &MainWindow::addEntry);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteEntry);
    connect(imageAction, &QAction::triggered, this, &MainWindow::importImage);
    connect(lockAction, &QAction::triggered, this, &MainWindow::lockJournal);
    connect(exportEntryAction, &QAction::triggered, this, [this] { exportEntries(false); });
    connect(exportAllAction, &QAction::triggered, this, [this] { exportEntries(true); });
    connect(renameAction, &QAction::triggered, this, [this] {
        bool ok = false;
        auto name = QInputDialog::getText(this, "Rename journal", "Journal name:", QLineEdit::Normal, journal.name, &ok).trimmed();
        if (ok && !name.isEmpty()) { journal.name = name; markDirty(); }
    });
    connect(list, &QListWidget::currentRowChanged, this, &MainWindow::selectEntry);
    connect(search, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < list->count(); ++i)
            list->item(i)->setHidden(!journal.entries[i].title.contains(text, Qt::CaseInsensitive));
    });
    connect(title, &QLineEdit::textEdited, this, [this] {
        if (current >= 0) {
            journal.entries[current].title = title->text();
            list->item(current)->setText(title->text().isEmpty() ? "Untitled entry" : title->text());
            markDirty();
        }
    });
    connect(editor, &QTextEdit::textChanged, this, &MainWindow::markDirty);
    connect(secret, &QCheckBox::toggled, this, &MainWindow::changeSecret);
    connect(boldAction, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f; f.setFontWeight(on ? QFont::Bold : QFont::Normal); editor->mergeCurrentCharFormat(f); editor->setFocus();
    });
    connect(italicAction, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f; f.setFontItalic(on); editor->mergeCurrentCharFormat(f); editor->setFocus();
    });
    connect(underlineAction, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f; f.setFontUnderline(on); editor->mergeCurrentCharFormat(f); editor->setFocus();
    });
    connect(editor, &QTextEdit::currentCharFormatChanged, this, [this](const QTextCharFormat &f) {
        boldAction->setChecked(f.fontWeight() >= QFont::Bold);
        italicAction->setChecked(f.fontItalic());
        underlineAction->setChecked(f.fontUnderline());
    });
    updateState();
}
MainWindow::~MainWindow() { Crypto::wipe(key); }
void MainWindow::error(const QString &message) { QMessageBox::critical(this, "Jotter", message); }
void MainWindow::updateState() {
    const bool hasEntry = active && current >= 0 && current < journal.entries.size();
    setWindowTitle((active ? journal.name + (dirty ? " *" : "") + " — " : "") + "jotter");
    journalLabel->setText(active ? journal.name : (lockedPath.isEmpty() ? "Your next chapter" : "Journal locked"));
    saveAction->setEnabled(active);
    addAction->setEnabled(active);
    renameAction->setEnabled(active);
    secret->setEnabled(active);
    search->setEnabled(active);
    lockAction->setEnabled(active && !key.isEmpty());
    exportAllAction->setEnabled(active && !journal.entries.isEmpty());
    for (auto *a : {deleteAction, imageAction, exportEntryAction, boldAction, italicAction, underlineAction})
        a->setEnabled(hasEntry);
    title->setEnabled(hasEntry);
    editor->setEnabled(hasEntry);
    QSignalBlocker blocker(secret);
    secret->setChecked(!key.isEmpty());
    statusBar()->showMessage(!active ? (lockedPath.isEmpty() ? "Welcome to Jotter · Create or open a journal." : "Locked · Choose Open journal to unlock.") :
        (dirty ? "Unsaved changes · Ctrl+S to save" : "Saved locally") +
        QString(key.isEmpty() ? " · Standard journal" : " · Encrypted journal, currently unlocked"));
}
void MainWindow::markDirty() {
    if (loading || !active) return;
    dirty = true;
    updateState();
}
void MainWindow::syncEntry() {
    if (!active || current < 0 || current >= journal.entries.size()) return;
    journal.entries[current].title = title->text().trimmed().isEmpty() ? "Untitled entry" : title->text().trimmed();
    journal.entries[current].html = editor->document()->toHtml();
}
void MainWindow::refreshList() {
    QSignalBlocker blocker(list);
    list->clear();
    search->clear();
    for (const auto &entry : journal.entries) {
        list->addItem(entry.title);
        list->item(list->count()-1)->setToolTip(QDateTime::fromString(entry.created, Qt::ISODate).toLocalTime().toString("MMMM d, yyyy"));
    }
    list->setCurrentRow(current);
}
void MainWindow::selectEntry(int index) {
    syncEntry();
    loading = true;
    current = index;
    // Replace the entire document so undo history and image caches cannot cross entries.
    auto *old = editor->document();
    old->setParent(nullptr);
    editor->setDocument(new Document(editor));
    delete old;
    if (current >= 0 && current < journal.entries.size()) {
        title->setText(journal.entries[current].title);
        editor->setHtml(journal.entries[current].html);
        dateLabel->setText(QDateTime::fromString(journal.entries[current].created, Qt::ISODate).toLocalTime().toString("dddd, MMMM d, yyyy · h:mm AP"));
    } else {
        title->clear();
        dateLabel->setText(active ? "Add an entry to start writing." : "Create or open a journal to begin.");
    }
    loading = false;
    updateState();
}
void MainWindow::clearJournal() {
    active = false;
    current = -1;
    journal = {};
    path.clear();
    fileLock.reset();
    Crypto::wipe(key);
    Crypto::wipe(salt);
    dirty = false;
    refreshList();
    selectEntry(-1);
}
QString MainWindow::savePath(const QString &caption, const QString &filter, const QString &suffix) {
    QFileDialog dialog(this, caption);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(filter);
    dialog.setDefaultSuffix(suffix);
    return dialog.exec() == QDialog::Accepted ? dialog.selectedFiles().first() : QString();
}
bool MainWindow::confirmLeave() {
    if (!active || !dirty) return true;
    const auto answer = QMessageBox::question(this, "Save your journal?",
        "This journal has unsaved changes.", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveJournal());
}
void MainWindow::newJournal() {
    if (!confirmLeave()) return;
    bool ok;
    auto name = QInputDialog::getText(this, "New journal", "Journal name:", QLineEdit::Normal, "My journal", &ok).trimmed();
    if (!ok || name.isEmpty()) return;
    auto destination = savePath("Create journal", "Jotter journal (*.jotter)", "jotter");
    if (destination.isEmpty()) return;
    if (active && QFileInfo(destination).absoluteFilePath() == QFileInfo(path).absoluteFilePath()) {
        error("Choose a new filename rather than replacing the journal that is open."); return;
    }
    auto nextLock = std::make_unique<QLockFile>(destination + ".lock");
    if (!nextLock->tryLock()) { error("This journal is already open in another Jotter window, or its folder is not writable."); return; }
    Journal next{name, {JournalFile::newEntry()}};
    try { JournalFile::save(destination, next, {}, {}); }
    catch (const std::exception &e) { error(e.what()); return; }
    clearJournal();
    lockedPath.clear();
    journal = next; path = destination; fileLock = std::move(nextLock); active = true;
    refreshList();
    list->setCurrentRow(0);
    editor->setFocus();
}
QByteArray MainWindow::password(bool creating) {
    QDialog dialog(this);
    dialog.setWindowTitle(creating ? "Keep this journal secret" : "Unlock journal");
    QFormLayout layout(&dialog);
    QLabel info(creating ? "Use at least 12 characters. There is no password recovery.\nAll entries and images will be encrypted." : "Enter this journal’s password.");
    layout.addRow(&info);
    QLineEdit first, second;
    first.setEchoMode(QLineEdit::Password);
    second.setEchoMode(QLineEdit::Password);
    first.setMaxLength(1024); second.setMaxLength(1024);
    layout.addRow("Password", &first);
    if (creating) layout.addRow("Confirm password", &second);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (first.text().isEmpty() || (creating && (first.text().size() < 12 || first.text() != second.text()))) {
            QMessageBox::warning(&dialog, "Check your password", "Use at least 12 characters and make both passwords match.");
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted) return {};
    auto result = first.text().toUtf8();
    first.clear(); second.clear();
    return result;
}
void MainWindow::openJournal(const QString &requested) {
    if (!confirmLeave()) return;
    auto source = requested.isEmpty() ? QFileDialog::getOpenFileName(this, "Open journal", lockedPath, "Jotter journal (*.jotter)") : requested;
    if (source.isEmpty()) return;
    if (active && QFileInfo(source).absoluteFilePath() == QFileInfo(path).absoluteFilePath()) return;
    auto nextLock = std::make_unique<QLockFile>(source + ".lock");
    if (!nextLock->tryLock()) { error("This journal is already open elsewhere, or its folder is not writable."); return; }
    QByteArray nextKey, nextSalt, pass;
    try {
        auto envelope = JournalFile::read(source);
        if (JournalFile::isSecret(envelope)) {
            pass = password(false);
            if (pass.isEmpty()) return;
            nextSalt = JournalFile::salt(envelope);
            nextKey = Crypto::derive(pass, nextSalt);
            Crypto::wipe(pass);
        }
        auto next = JournalFile::decode(envelope, nextKey);
        clearJournal();
        lockedPath.clear();
        journal = std::move(next);
        path = source; key = std::move(nextKey); salt = std::move(nextSalt);
        fileLock = std::move(nextLock); active = true;
        refreshList();
        if (!journal.entries.isEmpty()) list->setCurrentRow(0);
        updateState();
    } catch (const std::exception &e) { Crypto::wipe(nextKey); Crypto::wipe(pass); error(e.what()); }
}
bool MainWindow::saveJournal() {
    if (!active) return true;
    syncEntry();
    try {
        JournalFile::save(path, journal, key, salt);
        dirty = false;
        updateState();
        return true;
    } catch (const std::exception &e) { error(e.what()); return false; }
}
void MainWindow::changeSecret(bool checked) {
    if (!active || loading) return;
    QByteArray nextKey, nextSalt, pass;
    try {
        if (checked) {
            pass = password(true);
            if (pass.isEmpty()) { updateState(); return; }
            nextSalt = Crypto::random(16);
            nextKey = Crypto::derive(pass, nextSalt);
            Crypto::wipe(pass);
        } else {
            if (QMessageBox::warning(this, "Remove password protection?",
                "This saves all entries and images without encryption. Anyone with this file can read them.",
                QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Yes) {
                updateState(); return;
            }
        }
        syncEntry();
        JournalFile::save(path, journal, nextKey, nextSalt);
        Crypto::wipe(key);
        key = std::move(nextKey); salt = std::move(nextSalt);
        dirty = false;
    } catch (const std::exception &e) { Crypto::wipe(nextKey); Crypto::wipe(pass); error(e.what()); }
    updateState();
}
void MainWindow::lockJournal() {
    if (!active || key.isEmpty()) return;
    if (dirty && !saveJournal()) return;
    lockedPath = path;
    clearJournal();
}
void MainWindow::addEntry() {
    syncEntry();
    journal.entries.append(JournalFile::newEntry());
    current = -1;
    refreshList();
    list->setCurrentRow(int(journal.entries.size()) - 1);
    markDirty();
    title->setFocus(); title->selectAll();
}
void MainWindow::deleteEntry() {
    if (current < 0) return;
    if (QMessageBox::warning(this, "Delete entry?", "Delete this entry and its images? This cannot be undone after saving.",
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Yes) return;
    auto next = qMax(0, current - 1);
    journal.entries.removeAt(current);
    current = -1;
    refreshList();
    if (journal.entries.isEmpty()) selectEntry(-1); else list->setCurrentRow(next);
    markDirty();
}
void MainWindow::importImage() {
    auto source = QFileDialog::getOpenFileName(this, "Import image", {}, "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (source.isEmpty()) return;
    QImageReader reader(source);
    reader.setAutoTransform(true);
    const auto size = reader.size();
    if (!size.isValid() || qint64(size.width()) * size.height() > 24000000 || QFileInfo(source).size() > 20 * 1024 * 1024) {
        error("Choose an image under 20 MB and 24 megapixels."); return;
    }
    auto image = reader.read();
    if (image.isNull()) { error("Could not read this image: " + reader.errorString()); return; }
    image = image.scaled(1600, 1600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG")) { error("Could not encode this image."); return; }
    QTextImageFormat format;
    format.setName("data:image/png;base64," + QString::fromLatin1(png.toBase64()));
    auto display = image.size().scaled(560, 700, Qt::KeepAspectRatio);
    format.setWidth(display.width()); format.setHeight(display.height());
    auto cursor = editor->textCursor();
    cursor.insertImage(format);
    cursor.insertBlock();
    editor->setTextCursor(cursor);
    editor->setFocus();
}
void MainWindow::exportEntries(bool all) {
    syncEntry();
    if (!key.isEmpty() && QMessageBox::warning(this, "Export secret content?",
        "The exported Word document or PDF will NOT be encrypted or password protected. Anyone with the exported file can read it.",
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) return;
    bool ok;
    auto format = QInputDialog::getItem(this, "Export", "Document format:", {"PDF (.pdf)", "Word document (.docx)"}, 0, false, &ok);
    if (!ok) return;
    bool pdf = format.startsWith("PDF");
    auto destination = savePath(all ? "Export journal" : "Export entry",
        pdf ? "PDF (*.pdf)" : "Word document (*.docx)", pdf ? "pdf" : "docx");
    if (destination.isEmpty()) return;
    if (QFileInfo(destination).absoluteFilePath() == QFileInfo(path).absoluteFilePath()) {
        error("An export cannot replace your journal file."); return;
    }
    auto entries = all ? journal.entries : QVector<Entry>{journal.entries[current]};
    try {
        if (pdf) Export::pdf(destination, journal.name, entries);
        else Export::docx(destination, journal.name, entries);
        QMessageBox::information(this, "Export complete", "Saved to:\n" + destination);
    } catch (const std::exception &e) { error(e.what()); }
}
void MainWindow::closeEvent(QCloseEvent *event) {
    if (confirmLeave()) event->accept(); else event->ignore();
}
