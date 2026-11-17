# jotter

A native C++20 / Qt 6 Widgets journal application for Windows 11. Open this `jotter` folder in Visual Studio to build the Windows app.

## Features

- Create and open local `.jotter` journals, each containing multiple dated entries.
- Create, edit, rename, search entry titles, and delete entries.
- Bold, italic, and underline formatting.
- Import PNG, JPEG, BMP, or supported WebP images directly into entries.
  Images are embedded in the journal; the original files are not needed later.
- Export an entry or the whole journal to a real `.docx` Word document or PDF.
  Text, basic formatting, and images are included. Word does not need to be installed.
- **Keep this journal secret** encrypts the whole journal, including its name,
  entry titles, text, and images, using a user-chosen password.
- Save with Ctrl+S. Unsaved changes are marked in the title bar; switching
  journals or closing the app prompts to save, discard, or cancel.
- Lock a secret journal with Ctrl+L. Locking saves pending changes, removes the
  open journal from the UI, and requires the password to reopen it.

## Build on Windows 11 with Visual Studio 2026 or later

### Build automatically with an installed Qt kit

Double-click **`build.bat`** in this folder. It locates Visual Studio 2026+,
initializes its x64 compiler, finds an installed **Qt 6.8+ MSVC x64** kit,
compiles Jotter, and copies Qt runtime files beside the executable.
It uses Windows PowerShell, included with Windows 11.

Qt detection checks `QT_ROOT`, `QTDIR`, `Qt6_DIR`, qmake on PATH, and standard
Qt folders on mounted drives and in your user profile. For a custom location:

```bat
build.bat "D:\SDKs\Qt\6.10.0\msvc2022_64"
```

Visual Studio's C++ workload and CMake tools must be installed. The script also
requires a bootstrapped vcpkg installation for OpenSSL and libzip; it checks
`VCPKG_ROOT`, Visual Studio's bundled vcpkg, PATH, and common installation paths.
Missing OpenSSL/libzip packages are downloaded automatically; an existing Qt
installation is reused rather than downloaded again. See prerequisites below
for vcpkg setup. MinGW and ARM Qt kits are not compatible with this x64 MSVC build.

Output: **`out\build\detected-qt\jotter.exe`**. Keep all DLLs and plugin folders
beside it. The script pauses so you can read the result. Set `JOTTER_NO_PAUSE=1`
when invoking it from automation; failures return a nonzero exit code.
The script has not been executed on Windows in this Linux environment.

### Prerequisites

1. Install **Visual Studio 2026 or later**, with the **Desktop development with C++**
   workload, Windows SDK, and **C++ CMake tools for Windows**.
2. Install Git for Windows.
3. Install a current vcpkg checkout in a short path, for example `C:\dev\vcpkg`.
   From PowerShell:

   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
   C:\dev\vcpkg\bootstrap-vcpkg.bat
   [Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\dev\vcpkg", "User")
   $env:VCPKG_ROOT = "C:\dev\vcpkg"
   ```

   Restart Visual Studio after setting the environment variable. Use a current
   vcpkg version that supports your installed MSVC toolset. A future Visual Studio
   version may require updates to vcpkg and Qt; compatibility is not guaranteed
   with toolchains that have not been tested.

### Visual Studio

1. Choose **File → Open → Folder** and select this `jotter` directory (the one
   containing `CMakeLists.txt`, not the surrounding web workspace).
2. Select the **windows-release** CMake preset.
3. Let CMake configure. The vcpkg manifest installs Qt, OpenSSL, and libzip.
   Qt is large; the first build can take a while and requires free disk space.
4. Choose **Build → Build All**, then select `jotter.exe` as the startup target.
5. Run without debugging, or debug with F5.

The preset uses Ninja and the MSVC compiler provided by Visual Studio. It avoids
a version-specific Visual Studio CMake generator, so the project is not tied to a
single IDE generator version.

### Command line alternative

Open **Developer PowerShell for Visual Studio 2026** (or later), then:

```powershell
cd path\to\jotter
cmake --preset windows-release
cmake --build --preset windows-release
.\out\build\windows-release\jotter.exe
```

Requires CMake 3.28+, Qt 6.8+, OpenSSL 3+, and libzip. The manifest supplies the
native libraries. No Node.js, pnpm, web server, cloud database, or account is needed.

### Deploy to another Windows 11 machine

```powershell
cmake --install out/build/windows-release
```

The install step uses Qt's deployment script. Distribute the entire
`out/install` directory, not just the executable. Confirm that the OpenSSL and
libzip runtime DLLs and their dependencies are included alongside the executable
(the vcpkg DLLs are in `out/build/windows-release/vcpkg_installed/x64-windows/bin`).
Install the Microsoft Visual C++ x64 Redistributable on the destination machine.
Smoke-test on a clean Windows 11 machine before distributing.

Qt, OpenSSL, and libzip have separate licenses. Preserve their license notices
and comply with the applicable Qt LGPL/GPL or commercial license when distributing.

## Using Jotter

1. **New journal** → choose a name and local `.jotter` file.
2. Write in the first entry; use **New entry** to add more.
3. Use **Import image** to insert an image at the cursor.
4. Check **Keep this journal secret** and enter the same password twice
   (at least 12 characters). This immediately saves the journal encrypted.
5. Use **Save** regularly. There is no periodic autosave.
6. Use **Export entry**, or **File → Export entire journal**, and select Word or PDF.
7. Lock secret journals when finished. Choose **Open journal** to unlock again.

The checkbox protects **the entire journal**, not individual entries. Separate
journal files can have different passwords. Unchecking it, after a confirmation,
saves the journal without encryption. To change a password, first uncheck and
then re-enable protection; note that the intermediate save is unencrypted.
There is no password reset or recovery.

## Security and data handling

- AES-256-GCM authenticated encryption; fresh random 12-byte nonce on every save.
- PBKDF2-HMAC-SHA256, 600,000 iterations, and a random 16-byte salt per password.
- The derived key is held only while the journal is unlocked and explicitly
  cleansed when locking, switching journals, or closing the app. Passwords are
  never written to the journal or application settings.
- Atomic saves use `QSaveFile`; failed writes do not silently replace the old file.
- A lock file prevents two Jotter instances from writing the same journal at once.
- Locked journals contain no plaintext entry metadata. The filename, file size,
  file timestamps, and encryption parameters are still visible.
- Imported images are re-encoded as PNG and downscaled to fit within 1600×1600.
  Input limits: 20 MB / 24 megapixels. Journal files are limited to 128 MB.
- Rich-text HTML cannot load remote URLs or local files. Paste is plain text;
  use Import image for pictures.
- PDF and DOCX exports are **not encrypted**. Secret-journal export requires an
  explicit warning acknowledgement. DOCX generation uses a temporary plaintext
  archive that is removed after export.
- Encryption does not erase older unencrypted versions, deleted entries in
  backups, filesystem snapshots, exported files, or the original imported images.
  QSaveFile is not secure erasure. Use Windows device encryption/BitLocker and
  protect your backups.
- An unlocked app, clipboard copies, process memory, OS swap/crash dumps,
  screenshots, and malware are outside the protection provided here. Qt text
  buffers cannot be guaranteed to be securely erased from RAM.
- There is **no automatic idle lock**. Use Ctrl+L or close the app when leaving.
- Back up `.jotter` files and retain passwords safely. There is no recovery service.

## Source layout

- `MainWindow`: native Qt UI and user actions.
- `Journal`: journal model, file validation, atomic persistence.
- `Crypto`: OpenSSL password derivation and authenticated encryption.
- `Document`: safe embedded image resource loading and text-only paste.
- `Export`: PDF rendering and OpenXML `.docx` packaging.

## Verification status

The C++ sources compiled successfully with GCC and Qt 6.9 on Linux using
pkg-config, and the native window launched and rendered with Qt's offscreen
platform. Local CMake configuration was blocked by Nix OpenGL dependency
discovery; the direct compiler check does not verify the CMake preset.
No end-to-end export or password-flow tests have been run.

The source is intended for the Windows configuration above. Development checks
in this project run on Linux, not Windows. A successful Linux compilation does
not establish Visual Studio compatibility or validate the Windows UI. Before
release, build with your Visual Studio toolchain, open exported files in Word
and a PDF viewer, and exercise save/reopen, images, wrong passwords, and locking.
