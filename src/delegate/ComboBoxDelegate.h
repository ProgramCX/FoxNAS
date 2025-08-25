#pragma once
#include <QComboBox>
#include <QStyledItemDelegate>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboBoxDelegate(const QStringList &items, QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , m_items(items)
    {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &index) const override
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItems(m_items);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);

        int value = index.data(Qt::EditRole).toInt();
        combo->setCurrentIndex(value);
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);
        int value = combo->currentIndex();
        QString text = combo->currentText();

        model->setData(index, value, Qt::EditRole);
        model->setData(index, text, Qt::DisplayRole);
    }

private:
    QStringList m_items;
};
