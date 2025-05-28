#include "ClipboardManager.h"

ClipboardManager &ClipboardManager::instance()
{
    static ClipboardManager instance;
    return instance;
}

void ClipboardManager::setCopiedFiles(const QStringList &files)
{
    this->copiedFiles = files;

    this->cutFiles = QStringList();
    this->clipboardMode = COPY;
    emit clipboardCopyChanged();
}

void ClipboardManager::setCutFiles(const QStringList &files)
{
    this->cutFiles = files;

    this->copiedFiles = QStringList();
    this->clipboardMode = CUT;
    emit clipboardCutChanged();
}

QStringList ClipboardManager::getCopiedFiles() const
{
    return copiedFiles;
}

QStringList ClipboardManager::getCutFiles() const
{
    return cutFiles;
}

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject{parent}
{}

ClipboardManager::CLIPBOARDMODE ClipboardManager::getClipboardMode() const
{
    return clipboardMode;
}
