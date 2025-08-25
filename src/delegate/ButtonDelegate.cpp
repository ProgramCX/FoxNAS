#include "ButtonDelegate.h"
#include <QApplication>
#include <QMouseEvent>
#include <QStyleOptionButton>

ButtonDelegate::ButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void ButtonDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QStyleOptionButton buttonOption;
    buttonOption.rect = option.rect.adjusted(4, 4, -4, -4); // 留点内边距
    buttonOption.state = QStyle::State_Enabled;

    if (option.state & QStyle::State_Selected)
        buttonOption.state |= QStyle::State_MouseOver;

    QVariant textVar = index.data(Qt::DisplayRole);
    buttonOption.text = textVar.isValid() ? textVar.toString() : "默认操作";
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);
}

bool ButtonDelegate::editorEvent(QEvent *event,
                                 QAbstractItemModel *,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        QRect buttonRect = option.rect.adjusted(4, 4, -4, -4);
        if (buttonRect.contains(e->pos())) {
            emit buttonClicked(index);
            return true; // 只在点击按钮时拦截事件
        }
    }
    return false; // 其他事件交给视图处理
}
