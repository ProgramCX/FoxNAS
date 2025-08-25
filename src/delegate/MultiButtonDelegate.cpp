#include "MultiButtonDelegate.h"

MultiButtonDelegate::MultiButtonDelegate(const QStringList &buttons, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_buttons(buttons)
{}

void MultiButtonDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int btnWidth = 60;
    int btnHeight = option.rect.height() - 6;
    int spacing = 8;

    int x = option.rect.left() + 5;

    for (const QString &text : m_buttons) {
        QStyleOptionButton btn;
        btn.rect = QRect(x, option.rect.top() + 3, btnWidth, btnHeight);
        btn.text = text;
        btn.state = QStyle::State_Enabled;

        QApplication::style()->drawControl(QStyle::CE_PushButton, &btn, painter);
        x += btnWidth + spacing;
    }
}

bool MultiButtonDelegate::editorEvent(QEvent *event,
                                      QAbstractItemModel *,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        int btnWidth = 60;
        int spacing = 5;
        int x = option.rect.left() + 2;
        int y = option.rect.top() + 2;
        int h = option.rect.height() - 4;

        for (const QString &text : m_buttons) {
            QRect rect(x, y, btnWidth, h);
            if (rect.contains(e->pos())) {
                emit buttonClicked(index, text);
                return true;
            }
            x += btnWidth + spacing;
        }
    }
    return true;
}

void MultiButtonDelegate::setButtons(const QStringList &newButtons)
{
    m_buttons = newButtons;
}
