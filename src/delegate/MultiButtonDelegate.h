#ifndef MULTIBUTTONDELEGATE_H
#define MULTIBUTTONDELEGATE_H

#include <QApplication>
#include <QMouseEvent>
#include <QStyledItemDelegate>
class MultiButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MultiButtonDelegate(const QStringList &buttons, QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    void setButtons(const QStringList &newButtons);

signals:
    void buttonClicked(const QModelIndex &index, const QString &button) const;

private:
    QStringList m_buttons;
};

#endif // MULTIBUTTONDELEGATE_H
