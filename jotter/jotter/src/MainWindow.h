#pragma once
#include <QMainWindow>
#include <memory>
#include "Journal.h"

class Editor;
class QListWidget;
class QLineEdit;
class QCheckBox;
class QLabel;
class QAction;
class QLockFile;
class MainWindow : public QMainWindow {
public:
    MainWindow();
    ~MainWindow() override;
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Journal journal;
    QString path;
    QString lockedPath;
    QByteArray key, salt;
    std::unique_ptr<QLockFile> fileLock;
    bool active = false, dirty = false, loading = false;
    int current = -1;
    QListWidget *list;
    QLineEdit *title;
    QLineEdit *search;
    QCheckBox *secret;
    QLabel *journalLabel;
    QLabel *dateLabel;
    Editor *editor;
    QAction *saveAction, *addAction, *deleteAction, *imageAction, *lockAction;
    QAction *exportEntryAction, *exportAllAction, *renameAction;
    QAction *boldAction, *italicAction, *underlineAction;
    void updateState();
    void markDirty();
    void syncEntry();
    void refreshList();
    void selectEntry(int index);
    void clearJournal();
    void newJournal();
    void openJournal(const QString &requested = {});
    bool saveJournal();
    bool confirmLeave();
    void changeSecret(bool checked);
    void lockJournal();
    void addEntry();
    void deleteEntry();
    void importImage();
    void exportEntries(bool all);
    void error(const QString &message);
    QByteArray password(bool creating);
    QString savePath(const QString &caption, const QString &filter, const QString &suffix);
};
