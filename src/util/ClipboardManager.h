#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>

class ClipboardManager : public QObject
{
    Q_OBJECT
public:
    static ClipboardManager& instance();

    enum CLIPBOARDMODE { COPY, CUT };

    void setCopiedFiles(const QStringList& files);
    void setCutFiles(const QStringList& files);
    QStringList getCopiedFiles() const;
    QStringList getCutFiles() const;
    CLIPBOARDMODE getClipboardMode() const;

signals:
    void clipboardCopyChanged();
    void clipboardCutChanged();

private:
    explicit ClipboardManager(QObject* parent = nullptr);
    QStringList copiedFiles;
    QStringList cutFiles;

    CLIPBOARDMODE clipboardMode = COPY;
};

#endif // CLIPBOARDMANAGER_H
