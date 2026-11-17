# Jotter User Manual

**Jotter - Jot down all of those ideas, thoughts and feelings**

For Jotter 1.0 · Windows 11 · Native C++ / Qt desktop application

---

## Contents

1. [Welcome to Jotter](#1-welcome-to-jotter)
2. [Requirements and installation](#2-requirements-and-installation)
3. [Quick start](#3-quick-start)
4. [Finding your way around](#4-finding-your-way-around)
5. [Creating and managing journals](#5-creating-and-managing-journals)
6. [Writing and managing entries](#6-writing-and-managing-entries)
7. [Importing images](#7-importing-images)
8. [Saving your work](#8-saving-your-work)
9. [Keeping a journal secret](#9-keeping-a-journal-secret)
10. [Exporting to Word or PDF](#10-exporting-to-word-or-pdf)
11. [Backups, transfers, and recovery](#11-backups-transfers-and-recovery)
12. [Keyboard shortcuts](#12-keyboard-shortcuts)
13. [Troubleshooting](#13-troubleshooting)
14. [Privacy and security](#14-privacy-and-security)
15. [Limits and current limitations](#15-limits-and-current-limitations)
16. [Frequently asked questions](#16-frequently-asked-questions)
17. [Build and distribution reference](#17-build-and-distribution-reference)
18. [Version and verification notes](#18-version-and-verification-notes)

---

## 1. Welcome to Jotter

Jotter is a local desktop journal for recording ideas, thoughts, feelings, and
memories. Write entries, add pictures, keep selected journals private, and export
your writing to share or print.

### Journals and entries

- A **journal** is a collection saved in one `.jotter` file.
- An **entry** is an individual piece of writing inside a journal. It has a
  title, creation date and time, and a body containing text and optional images.
- A **secret journal** is a journal whose contents are encrypted using a
  password you choose.
- An **export** is a separate Word document or PDF. It is not a replacement
  for the editable `.jotter` file.

You can create separate journals for different purposes, such as personal
reflections, creative ideas, or travel. Jotter opens one journal at a time in
each application window.

### Three things to remember

> **Save regularly.** Jotter does not have periodic autosave. Press **Ctrl+S**
> after changes you want to keep.

> **Keep your password safe.** There is no password reset or recovery service.

> **Exports are not secret.** Word and PDF exports are unencrypted, even when
> they come from a secret journal.

## 2. Requirements and installation

### If you already have a built copy

You need a Windows 11 computer and the complete Jotter application folder.
Keep `jotter.exe`, its DLL files, and its plugin folders together.

1. Place the complete application folder in a suitable location.
2. Install the Microsoft Visual C++ x64 Redistributable if it is not already
   present and the application requires it.
3. Double-click `jotter.exe`.
4. Create or open a journal using the toolbar.

Do not copy only `jotter.exe` to another computer. It depends on accompanying
runtime files. You can create a Windows shortcut to the executable instead
of moving it.

Jotter does not require an online account, Microsoft Word, a web server, or a
cloud database. Internet access may be required during the build process to
download dependencies; ordinary journaling is local.

### If you have the source project

The project includes source code and build tools, not a verified Windows
installer. To compile it, install:

| Requirement | What to select |
| --- | --- |
| Operating system | Windows 11, using the x64 build configuration |
| Visual Studio | Visual Studio 2026 or later |
| Visual Studio workload | Desktop development with C++ |
| Additional build tools | Windows SDK and C++ CMake tools for Windows |
| CMake | Version 3.28 or later |
| Qt for the automatic build script | Qt 6.8 or later in the Qt 6 series, MSVC x64 kit |
| Dependency manager | A bootstrapped vcpkg installation |
| Git | Needed when obtaining vcpkg from its Git repository |

**MinGW and ARM Qt kits do not work with the script's x64 MSVC build.**
A Qt kit folder may be named `msvc2022_64`; the script checks the kit's compiler
type, architecture, and Qt version rather than requiring a folder named after
Visual Studio 2026.

### Build using `build.bat`

1. Install the prerequisites above.
2. Open the project's `jotter` folder.
3. Double-click **`build.bat`**.
4. Read the detected Visual Studio and Qt paths in the console.
5. Wait for dependency installation and compilation to finish. The initial
   build may take considerably longer than later builds.
6. If successful, launch:

   ```text
   jotter\out\build\detected-qt\jotter.exe
   ```

The script finds Visual Studio, initializes its x64 compiler, locates Qt,
builds Jotter, and uses Qt's deployment tool to copy runtime files beside the
executable. It pauses at the end so you can read the result.

The script uses vcpkg to obtain **OpenSSL and libzip** if needed. It reuses
the detected Qt installation instead of downloading Qt again.

### Select Qt manually

If Qt is installed in a custom location, open Command Prompt in the `jotter`
folder and supply the kit path:

```bat
build.bat "D:\SDKs\Qt\6.10.0\msvc2022_64"
```

Replace the example with your actual installed kit. Keep quotation marks
around paths containing spaces.

Automatic discovery checks `QT_ROOT`, `QTDIR`, and `Qt6_DIR`, qmake on PATH,
and standard Qt folders on mounted drives and in your user profile. If several
automatically discovered kits qualify, it selects the highest version.
An explicitly supplied path takes priority.

### Set up vcpkg if needed

From PowerShell, with Git installed:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\dev\vcpkg", "User")
$env:VCPKG_ROOT = "C:\dev\vcpkg"
```

Use a current vcpkg version compatible with your compiler. Reopen terminals
and Visual Studio after changing user environment variables.

For other build methods, see [Build and distribution reference](#17-build-and-distribution-reference).

## 3. Quick start

1. Launch **Jotter**.
2. Click **New journal**.
3. Enter a name, such as `Everyday thoughts`.
4. Choose where to save its `.jotter` file.
5. Jotter creates an initial entry called **Untitled entry**.
6. If you want privacy, enable **Keep this journal secret** before writing
   sensitive content, and set a password.
7. Replace the entry title with something meaningful.
8. Click the writing area and start typing.
9. Optionally use **Import image** to add a picture.
10. Press **Ctrl+S** to save.
11. For a secret journal, click **Lock journal** when you are finished.
    For other journals, save and close the app.

To continue later, launch Jotter, choose **Open journal**, and select the
saved `.jotter` file. Enter its password if prompted.

## 4. Finding your way around

### Top toolbar

| Control | Purpose |
| --- | --- |
| **New journal** | Create a separate journal file. |
| **Open journal** | Select an existing `.jotter` file. |
| **Save** | Write the open journal's current contents to disk. |
| **+ New entry** | Add an entry to the open journal. |
| **Import image** | Insert an image into the current entry. |
| **Export entry** | Save the current entry as Word or PDF. |
| **Lock journal** | Save pending changes and close access to an open secret journal. |

### Left sidebar

The sidebar shows the journal name, a title-search field, the entry list,
and the **Keep this journal secret** checkbox.

Click an entry in the list to view or edit it. Hovering over a list item shows
its creation date as a tooltip.

### Writing area

The right side contains:

- The editable entry title.
- Its creation date and time, displayed in the computer's local time.
- **Bold**, **Italic**, and **Underline** controls.
- The main writing area.

The date is assigned when an entry is created. There is no date-editing control.

### Menu bar

- **File:** New journal, Open journal, Save, Export entire journal, Export entry,
  and Exit.
- **Journal:** New entry, Delete entry, Rename journal, and Lock journal.
- **Help → About Jotter:** Version information and a short usage/security summary.

### Status bar and title bar

The bottom status bar reports whether there are unsaved changes and whether
the journal is standard or encrypted.

An asterisk (`*`) beside the journal name in the window title indicates unsaved
changes. After a successful save, it disappears.

An **encrypted journal, currently unlocked** is readable in the open window.
Encryption does not mean the visible session is locked.

### Disabled controls

Controls are unavailable when they do not apply:

- Create or open a journal before adding entries.
- Select an entry before editing, importing an image, or exporting that entry.
- **Lock journal** is available only for an open secret journal.
- A journal with no entries cannot be exported.

## 5. Creating and managing journals

### Create a journal

1. Click **New journal**, or press **Ctrl+Shift+N**.
2. Enter a non-empty journal name.
3. Choose the file's folder and filename.
4. Save the file with the `.jotter` extension.

Jotter creates the file immediately, containing one blank entry. New journals
are initially unencrypted.

Choose a location you can write to, such as a dedicated folder within Documents.
Avoid opening a journal from a read-only location.

If another journal has unsaved changes, Jotter first asks whether to save,
discard, or cancel. Do not reuse an existing filename unless you intend to
replace that file.

### Open a journal

1. Click **Open journal**, or press **Ctrl+O**.
2. Browse to the `.jotter` file.
3. Select it and confirm.
4. If the journal is secret, enter its password.

Use this command from within Jotter. The current project does not set up a
Windows file association or support opening a journal by passing its filename
to the executable.

### Rename a journal

1. Open the journal.
2. Choose **Journal → Rename journal…**.
3. Enter the new name and confirm.
4. Save.

This changes the journal's display name, **not its filename or location**.
To rename the file itself, save, close Jotter, and use Windows File Explorer.
Keep the `.jotter` extension.

### Switch journals

Choose **Open journal** or **New journal**. Resolve any unsaved-change prompt
before continuing. Only the selected journal is then shown in that window.

### Move or delete a journal file

Jotter has no built-in command to move or delete a whole journal.

1. Save and close the journal.
2. Use Windows File Explorer to move, rename, or delete its `.jotter` file.
3. Open it from its new location when needed.

Deleting a journal file removes all entries in that file. Make a backup first.
Ordinary deletion is not secure erasure.

## 6. Writing and managing entries

### Create an entry

1. Open a journal.
2. Click **+ New entry**, choose **Journal → New entry**, or press **Ctrl+N**.
3. Type a title.
4. Click in the writing area and write.
5. Save.

New entries are appended to the list. Titles can contain up to 250 characters.
A blank or whitespace-only title is stored as **Untitled entry** when the
entry is synchronized for saving or another operation.

### Edit an entry

Select it in the sidebar. Change the title or body, then save. Moving to another
entry retains edits in the open session, but **does not save them to disk**.

### Format text

Select existing text and choose **Bold**, **Italic**, or **Underline**.
Alternatively, toggle a formatting option before typing.

Use the same control again to turn the formatting off. Formatting applies to
the body; the title uses the app's fixed title style.

### Paste text

Text pasted from another application is inserted as plain text. Jotter does
not preserve pasted web-page formatting or insert pasted images. Apply
formatting in Jotter and use **Import image** for pictures.

Dragging images into the editor is not supported.

### Find an entry

1. Click **Find an entry by title…**.
2. Type part of the entry's title.
3. Select a matching item.
4. Clear the search field to show all entries again.

Search is case-insensitive and searches **titles only**, not the entry body.
Filtering does not delete entries or restrict a whole-journal export.

### Delete an entry

1. Select the entry.
2. Choose **Journal → Delete entry…**.
3. Read the confirmation and choose **Yes** to delete it, or **Cancel**.
4. Save to make the deletion permanent in the journal file.

Deletion removes the entry and its embedded images. There is no trash or
restore-entry command.

If you delete something accidentally and have not saved or triggered a save,
you can close Jotter and choose **Discard**, then reopen the last saved journal.
This also discards **all other unsaved changes**. Locking or changing password
protection may already have saved the deletion; restore a backup in that case.

### Undo while editing

Standard text-editor undo can reverse recent typing while that entry remains
open. Do not rely on it to restore a deleted entry. Switching entries replaces
the editor document and clears that entry's editor undo history.

## 7. Importing images

### Insert a picture

1. Select the entry.
2. Place the text cursor where the image should appear.
3. Click **Import image**.
4. Choose the image file.
5. Continue writing below the inserted picture.
6. Save.

The image is embedded inside the journal. Moving or deleting the original
picture later does not remove the embedded copy.

### Supported formats and limits

| Item | Behavior |
| --- | --- |
| File types | PNG, JPEG/JPG, BMP, and WebP when the Qt installation supports it |
| Input file size | Up to 20 MB as enforced by the app; choose smaller files if rejected |
| Input dimensions | No more than 24 megapixels |
| Stored format | Re-encoded as PNG |
| Stored dimensions | Scaled to fit within 1600 × 1600 pixels, preserving aspect ratio |
| Initial display size | Fitted within 560 × 700 pixels in the editor |

The import process changes the embedded copy's dimensions and encoding; it
does not overwrite the source image. Keep originals separately if you need
full-resolution copies.

### Remove an image

Select the embedded image in the editor, or position the cursor immediately
after it and use Backspace to remove it. Check that the intended picture was
removed, then save.

There is no dedicated image library, crop tool, or image-size control.

### Images in secret journals and exports

Embedded images are encrypted along with the rest of a secret journal.
The original source images are **not** protected by the journal password.
Images included in Word or PDF exports are also unencrypted.

## 8. Saving your work

### Save manually

Click **Save**, choose **File → Save**, or press **Ctrl+S**.
Saving updates the open `.jotter` file, including all entries and images.

There is no periodic autosave, Save As command, or automatic backup history.

### When Jotter saves automatically as part of an action

- Creating a journal writes its initial file.
- Enabling password protection immediately saves the journal encrypted.
- Removing password protection immediately saves it unencrypted.
- Locking a secret journal saves pending changes before locking.

Selecting another entry, searching, and exporting do not replace the need
to save the journal.

### Save, Discard, or Cancel

When closing the app or switching journals with unsaved changes:

| Choice | Result |
| --- | --- |
| **Save** | Attempts to save before proceeding. |
| **Discard** | Proceeds without keeping changes since the last successful save. |
| **Cancel** | Keeps the current journal open without proceeding. |

If a save fails, read the error and keep the application open while addressing
the problem. Do not discard the journal if you still need the unsaved content.

### Safe file writes

Jotter uses an atomic-save mechanism to reduce the risk of replacing a good
file with an incomplete write. This is not a backup system and does not protect
against every storage failure, device loss, or accidental deletion.

## 9. Keeping a journal secret

### Enable password protection

1. Open the journal.
2. Tick **Keep this journal secret** in the sidebar.
3. Enter a password of at least **12 characters**.
4. Enter it again in **Confirm password**.
5. Confirm the dialog.
6. Check that the status bar identifies the journal as encrypted.

Both password fields must match. Use a strong, unique password or passphrase
and keep it in a trusted password manager.

Enabling protection encrypts **the entire journal**: its internal name, entry
titles, text, and embedded images. It immediately saves current changes.
It does not lock the journal's open window; you can continue writing.

> For a new private journal, enable protection before entering sensitive
> content. Encrypting a previously standard journal cannot erase older
> plaintext backups or filesystem history.

### Lock a journal

Click **Lock journal**, choose **Journal → Lock journal**, or press **Ctrl+L**.

If there are unsaved changes, Jotter tries to save them first. After a successful
lock, the entries disappear from the window and the sidebar says
**Journal locked**.

If saving fails, locking does not complete. The journal stays open; resolve
the save problem before leaving your computer.

There is **no automatic idle lock**. Lock the journal yourself, or save and close
Jotter. Locking Windows is also sensible when you leave your computer unattended.

### Unlock a journal

1. Choose **Open journal**.
2. Select the secret `.jotter` file.
3. Enter its password.

The file chooser starts from the last locked path when available in the current
session. There is no separate unlock button or permanent recent-journal list.

### Remove protection

1. Open and unlock the journal.
2. Untick **Keep this journal secret**.
3. Read the warning.
4. Choose **Yes** to save it without encryption, or **Cancel** to keep protection.

After confirming, anyone who can read the file can access its contents.
The journal is saved immediately.

### Change a password

There is no dedicated password-change command. The current procedure is:

1. Open the journal with its existing password.
2. Untick **Keep this journal secret** and confirm removal.
3. Tick it again.
4. Set and confirm the new password.
5. Lock and reopen the journal to confirm you can use the new password.

**Important:** Step 2 writes an unencrypted version to disk. Filesystem history
or backups may retain that intermediate version. Do not use this procedure if
you require password rotation without any plaintext intermediate save.

Older encrypted backup copies retain their original passwords.

### Forgotten passwords

Jotter cannot recover or reset a secret journal's password. There is no online
account or master recovery key.

An incorrect-password message can also indicate file damage. Check Caps Lock
and your keyboard layout, then try a separate known-good backup if available.
Keep the original file unchanged while investigating.

## 10. Exporting to Word or PDF

### Choose a format

| Format | Best suited to |
| --- | --- |
| **Word document (.docx)** | Sharing editable writing with Word or a compatible document editor |
| **PDF (.pdf)** | Reading, printing, or sharing a document with a fixed page layout |

Microsoft Word is not required to create either format. To read the export,
use an application that supports its file type.

### Export one entry

1. Select the entry.
2. Save the journal if you want the current edits retained in the `.jotter` file.
3. Click **Export entry**, or choose it from **File**.
4. If the journal is secret, acknowledge the unencrypted-export warning.
5. Choose **PDF (.pdf)** or **Word document (.docx)**.
6. Select a destination and filename.
7. Wait for **Export complete**.
8. Open the exported file to check the result before sharing it.

### Export the entire journal

1. Open and, if necessary, unlock the journal.
2. Save any changes you want retained in the original journal.
3. Choose **File → Export entire journal…**.
4. Acknowledge the privacy warning if shown.
5. Select the format and destination.
6. Confirm and wait for completion.

Whole-journal exports include every entry, including entries hidden by a title
search. They follow the journal's stored order, with each subsequent entry
starting on a new page.

### What exports include

- Entry titles.
- Entry creation dates and times.
- Body text.
- Supported basic text formatting.
- Embedded images.

Word and PDF rendering can differ from the editor and from each other.
Check pagination, image size, and text before sending or printing a document.

### Export does not save or protect the original

An export uses the entry's current content, including unsaved edits, but does
not itself save those edits into the `.jotter` file. Save separately.

Exports are not importable journal backups. Keep the original `.jotter` file
for continued editing in Jotter.

> **Secret-journal warning:** Exported Word documents and PDFs are not encrypted
> or password protected. Anyone with access to an export can read it.

Choose a trusted folder, verify recipients before sharing, and protect copies
stored in cloud-sync folders. Closing or locking Jotter does not protect
already exported documents.

### Print an entry or journal

Export to PDF, open it in your PDF viewer, and use that viewer's Print command.
Jotter does not provide its own Print command.

## 11. Backups, transfers, and recovery

### Make a backup

1. Save your journal.
2. Close Jotter, or lock the secret journal.
3. Locate its `.jotter` file in Windows File Explorer.
4. Copy it to your backup location.
5. Give the backup a clear filename, such as:

   ```text
   Everyday-thoughts-backup-2026-11-16.jotter
   ```

Images are embedded, so you do not need a separate image folder to restore
the journal. Keep original photos separately if you want their original quality.

Back up regularly, preferably to more than one protected location. A copy on
the same disk does not protect against that disk failing.

### Restore a backup

1. Close the current journal.
2. Keep the current file rather than overwriting it immediately.
3. Copy the backup into a writable local folder.
4. Open the copy using **Open journal**.
5. Enter the password associated with that backup.
6. Check its entries and images.

Work from the restored copy only after confirming that it contains the data
you need.

### Transfer to another computer

1. Save and close the journal on the first computer.
2. Copy its `.jotter` file using a trusted transfer method.
3. Install or copy a complete compatible Jotter application folder to the
   destination computer.
4. Open the transferred file in Jotter.

The destination needs the journal's password if it is secret.

### Avoid simultaneous editing

A lock file helps prevent multiple local Jotter instances from writing to
the same journal at once. It does not provide cloud conflict resolution.

Do not edit the same journal simultaneously on multiple computers. If using
a synchronized folder, close the journal, allow synchronization to finish,
and then open it on the other device.

### What is not a backup

- A PDF or Word export is not a fully editable Jotter backup.
- Undo history is not persistent recovery.
- The `.lock` file contains no journal backup.
- Encryption protects confidentiality, not against losing the file.

## 12. Keyboard shortcuts

### Jotter commands

| Shortcut | Action |
| --- | --- |
| **Ctrl+Shift+N** | Create a journal |
| **Ctrl+O** | Open a journal |
| **Ctrl+S** | Save the open journal |
| **Ctrl+N** | Create an entry |
| **Ctrl+L** | Lock an open secret journal |
| **Ctrl+B** | Toggle bold in the editor |
| **Ctrl+I** | Toggle italic in the editor |
| **Ctrl+U** | Toggle underline in the editor |

### Standard text-editing shortcuts

With focus in an editable field, Windows/Qt text controls support familiar
actions such as **Ctrl+C** to copy, **Ctrl+X** to cut, **Ctrl+V** to paste,
**Ctrl+A** to select all, and **Ctrl+Z** to undo recent text edits.

Selection and undo apply to the focused field. Undo is not a way to recover
a deleted journal or a previously saved deletion.

## 13. Troubleshooting

### Build and startup problems

| Problem | What to do |
| --- | --- |
| Visual Studio is not found | Install Visual Studio 2026+ with Desktop development with C++. Confirm the x64 C++ tools are installed. |
| CMake or Ninja is missing | Add C++ CMake tools for Windows using Visual Studio Installer. CMake must be 3.28+. |
| Qt is not found | Install Qt 6.8+ in the Qt 6 series with an MSVC x64 kit. Supply the kit path to `build.bat` if it is in a custom location. |
| Qt is installed but rejected | Check that it is not a MinGW, ARM, or older Qt kit. The script needs a complete kit containing qmake, CMake configuration files, and deployment tools. |
| vcpkg is not found | Bootstrap vcpkg and set `VCPKG_ROOT`; reopen the terminal and try again. |
| Dependencies cannot download | Check internet access, proxy settings, and the error output. Do not disable security software simply to force a download. |
| Build fails after changing toolchains | Close the build tools, preserve your journal files elsewhere, and remove only the generated `out\build\detected-qt` folder before rebuilding. |
| `windeployqt.exe` is missing | Add Qt's Windows deployment tools or repair the selected Qt kit. |
| A runtime DLL is missing | Keep the whole generated application folder together; rebuild deployment files and install the Microsoft Visual C++ x64 Redistributable if required. |
| A Qt platform-plugin error appears | Verify the plugin folders were copied with the executable. Do not run a lone copied `.exe`. |

The console stays open after `build.bat` finishes so you can read the failure.
Record the first meaningful error, not only the final “build failed” line.

### Journal and editing problems

| Problem | What to do |
| --- | --- |
| The editor is disabled | Create/open a journal and select an entry. Add an entry if the journal is empty. |
| An entry seems to be missing | Clear the title-search field and confirm you opened the correct journal file. |
| Searching does not find body text | Search only matches entry titles in this version. |
| “Already open elsewhere” appears | Close other Jotter instances and check that the journal folder is writable. |
| The journal folder is read-only | Close the journal and copy it to a writable local folder, then open the copy. |
| A save fails | Check free disk space, folder permissions, and whether the drive is connected. Keep Jotter open if you need unsaved changes. |
| The file exceeds 128 MB | Reduce embedded images or divide future writing across separate journals. Back up before removing content. |
| “Unsupported journal format or version” appears | Confirm it is an actual `.jotter` file from a compatible version. Renaming a PDF or Word file does not convert it. Preserve the original and try a backup. |
| Changes disappeared after restarting | They may not have been saved, or the app may have closed unexpectedly. Check the saved file and your backups; no automatic recovery history is provided. |

Do not remove a lock file while a Jotter instance might still be using the
journal. If an open-file error persists after all instances are closed,
first back up the `.jotter` file and investigate folder permissions or obtain
technical help before changing lock files.

### Password and privacy problems

| Problem | What to do |
| --- | --- |
| A new password is rejected | Use at least 12 characters and make both fields match exactly. |
| Unlocking fails | Check Caps Lock, keyboard layout, and the password used for this file. The file could also be damaged; try a known-good backup separately. |
| The password is forgotten | There is no reset or recovery method. Retain the file in case you recover the password later. |
| Lock is unavailable | The open journal must have password protection enabled. |
| Lock did not complete | A pending save may have failed. Read the error; the journal remains open until locking succeeds. |
| A PDF or Word export opens without a password | This is expected. Exports are not encrypted. |

### Image and export problems

| Problem | What to do |
| --- | --- |
| An image is rejected | Use a supported format within the size and pixel limits. Try a smaller PNG or JPEG. |
| WebP does not load | Your Qt installation may lack the relevant image plugin. Convert a copy to PNG or JPEG. |
| Pasting a picture does nothing | Use **Import image**; image paste and drag-and-drop are not supported. |
| An embedded picture is smaller than the original | Jotter scales imported images. Keep the source file for full-resolution use. |
| Export fails | Check the destination is writable and has free space. Close the existing output file in Word or a PDF viewer, then try a different filename. |
| An image cannot be read during export | Preserve a journal backup, then replace the affected image from its original source and retry. |
| Exported layout differs from the editor | Word/PDF rendering is not identical to the editor. Check the output and adjust entry content if needed. |

## 14. Privacy and security

### Local storage

Jotter stores journals where you choose on your computer. It has no built-in
cloud synchronization or online account system.

Windows, backup software, and cloud-sync services may still copy files from
the folder you choose. Their behavior is outside Jotter's control.

### Standard versus secret journals

A standard journal is not encrypted. Treat it as readable by anyone who can
access the file.

A secret journal uses:

- **AES-256-GCM** authenticated encryption.
- **PBKDF2-HMAC-SHA256**, with **600,000 iterations**, to derive a key from
  the password.
- A random salt when password protection is set and a fresh random nonce
  for each encrypted save.

The application does not store the password in the journal or application
settings. The derived key is held while the journal is open and is explicitly
cleared when the journal is closed, switched, or locked.

These are implementation details, not a claim that the application has
undergone an independent security audit.

### Information that remains visible

Even a secret journal exposes its external filename, size, filesystem
timestamps, and encryption-format information.

Choose a neutral filename if revealing a journal's topic would be sensitive.

### What encryption does not cover

- The contents displayed in an unlocked window.
- Copies made to the Windows clipboard.
- Screenshots or screen recordings.
- Original imported images.
- Word or PDF exports.
- Older unencrypted versions, backups, or filesystem snapshots.
- Malware or someone controlling your unlocked computer.
- All possible copies of text in RAM, operating-system paging, or crash dumps.

The application cannot guarantee secure erasure of every Qt text buffer.
Word export also creates a temporary unencrypted archive, normally removed
after export; file deletion is not secure erasure.

### Recommended habits

1. Protect your Windows account and use device encryption where appropriate.
2. Use a strong, unique journal password.
3. Store the password safely outside the journal itself.
4. Enable protection before writing private content.
5. Save frequently and lock secret journals when finished.
6. Back up encrypted journals and protect the backups.
7. Review exported files and their destinations before sharing.
8. Never send your journal password with a public bug report.

## 15. Limits and current limitations

| Area | Current behavior |
| --- | --- |
| Platform target | Windows 11, x64 MSVC build |
| Journal size | Maximum 128 MB on-disk file size |
| Entry titles | Up to 250 characters |
| New passwords | At least 12 characters; password fields accept up to 1,024 characters |
| Imported images | Up to 20 MB and 24 megapixels; stored as scaled PNG |
| Search | Entry titles only |
| Formatting | Bold, italic, and underline |
| Saving | Manual, plus saves performed by specific journal/protection actions |
| Locking | Manual; no idle-lock timer |
| Exports | Word `.docx` and PDF; unencrypted |
| Import | Images and existing `.jotter` files; not Word/PDF journal import |

This version does not include cloud sync, mobile apps, multiple-user accounts,
entry tags, full-text search, date editing, custom sorting, automatic backup
history, a recycle bin, a direct Print command, password recovery, or a
dedicated password-change dialog.

File size includes stored image data and encryption/encoding overhead. A
collection of input images totaling less than 128 MB may still produce a
journal that exceeds the limit.

## 16. Frequently asked questions

### Do I need an account?

No. Jotter stores local files and has no account-registration process.

### Can I use it without the internet?

Ordinary use is local. Building it may require internet access to obtain
libraries and tools.

### Can I protect just one entry?

No. The checkbox protects the entire journal. Put private entries in a
separate secret journal if other entries should remain unprotected.

### Can different journals have different passwords?

Yes. Password protection is set separately for each journal file.

### Does “secret” mean the window is locked?

No. A secret journal is encrypted on disk but readable while open.
Use **Lock journal** to remove it from the active window.

### Does exporting save my journal?

No. Export and Save are separate operations. Press **Ctrl+S** to retain
changes in the `.jotter` file.

### Can I edit an exported document and import it back?

Not in this version. Continue editing the original journal in Jotter.
External Word edits do not update its entries.

### Do I need Word installed to export a DOCX?

No. Jotter creates the document itself. You need a compatible application
to view or edit the resulting file.

### Can I delete the original photos after importing them?

The journal stores embedded copies, so it does not need the originals to
display those images. Keep originals if you need their original resolution
or want separate backups.

### Where is my journal stored?

In the folder you selected when creating it. Jotter does not choose a fixed
central journal folder. If you lose track of it, search Windows File Explorer
for `*.jotter`.

### Will uninstalling Jotter remove my journals?

There is no supplied installer/uninstaller managing journal files. Removing
an application folder does not intentionally remove separately stored
journals. However, deleting a folder that contains your `.jotter` files
will delete those files too. Back them up before cleaning up folders.

## 17. Build and distribution reference

This section is for users building the source or preparing a copy for another
Windows computer.

### Build inside Visual Studio

1. Open Visual Studio 2026 or later.
2. Choose **File → Open → Folder**.
3. Select the `jotter` folder containing `CMakeLists.txt`.
4. Select the **windows-release** CMake preset.
5. Wait for configuration and dependency installation.
6. Build the project and select `jotter.exe` as the startup target.

Unlike `build.bat`, this preset uses the main vcpkg manifest, which includes
Qt as well as OpenSSL and libzip. It may build/download Qt even if you already
have a separate Qt installation.

### Build from Developer PowerShell

From Developer PowerShell for the installed Visual Studio version:

```powershell
cd path\to\jotter
cmake --preset windows-release
cmake --build --preset windows-release
.\out\build\windows-release\jotter.exe
```

The preset expects `VCPKG_ROOT` to identify your vcpkg folder.

### Keep the two output locations distinct

| Method | Executable |
| --- | --- |
| `build.bat` | `out\build\detected-qt\jotter.exe` |
| `windows-release` preset | `out\build\windows-release\jotter.exe` |

The BAT script uses a separate OpenSSL/libzip dependency manifest at
`tools\windows-deps\vcpkg.json` and a PowerShell helper at
`tools\build-windows.ps1`. Keep these files with the project when copying
the build script.

For unattended command-line use, set `JOTTER_NO_PAUSE=1` before invoking
`build.bat`. The script returns a nonzero exit code if it fails.

### Prepare a distributable folder

For the Visual Studio preset:

```powershell
cmake --install out/build/windows-release
```

The configured install destination is `out\install`. Check that the application
folder includes the Qt runtime/plugins and the required OpenSSL, libzip, and
other runtime DLLs.

For the BAT-script build, runtime deployment is attempted directly into the
`detected-qt` build folder.

In either case:

1. Distribute a complete application folder, not only the executable.
2. Install the Microsoft Visual C++ x64 Redistributable where required.
3. Check the app on a clean Windows 11 computer.
4. Verify writing, saving, reopening, images, password protection, locking,
   and both export formats before distributing it to others.
5. Preserve required third-party license notices and comply with the applicable
   Qt, OpenSSL, and libzip licenses.

## 18. Version and verification notes

This manual documents the current **Jotter 1.0** source implementation,
including its automatic Windows build script.

The C++ source has been compiled on Linux and the native Qt window has been
launched and rendered there. That does **not** establish Windows compatibility.
The Windows build script, Visual Studio build configuration, and end-to-end
password/export flows have not been verified on Windows in this project.

Build and try the application with a non-sensitive sample journal before
depending on it for important data. Keep independent backups.

When reporting a problem, record the app version, Windows version, operation,
and exact error text. For build issues, include the selected Visual Studio and
Qt versions. Do not include passwords or private journal contents.

---

**Jotter - Jot down all of those ideas, thoughts and feelings**
