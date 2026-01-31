#include "GeneralSettingForm.h"
#include "ui_GeneralSettingForm.h"
#include "LanguageManager.h"

#include <QEvent>
#include <QMessageBox>

GeneralSettingForm::GeneralSettingForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneralSettingForm)
{
    ui->setupUi(this);
    initLanguageComboBox();

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingForm::onLanguageChanged);
}

GeneralSettingForm::~GeneralSettingForm()
{
    delete ui;
}

void GeneralSettingForm::initLanguageComboBox()
{
    ui->comboBox->blockSignals(true);
    ui->comboBox->clear();

    LanguageManager &langMgr = LanguageManager::instance();
    const auto languages = langMgr.supportedLanguages();

    for (const auto &lang : languages) {
        // 显示格式: "本地名称 (English Name)"
        QString displayText = QString("%1 (%2)").arg(lang.nativeName, lang.englishName);
        ui->comboBox->addItem(displayText, lang.code);
    }

    // 设置当前选中的语言
    int currentIndex = langMgr.languageIndex(langMgr.currentLanguage());
    ui->comboBox->setCurrentIndex(currentIndex);

    ui->comboBox->blockSignals(false);
}

void GeneralSettingForm::onLanguageChanged(int index)
{
    LanguageManager &langMgr = LanguageManager::instance();
    QString languageCode = langMgr.languageCodeAt(index);

    if (languageCode != langMgr.currentLanguage()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("切换语言"),
            tr("切换语言需要重启应用程序才能完全生效。\n\n是否现在切换？"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            langMgr.switchLanguage(languageCode);

            QMessageBox::information(
                this,
                tr("语言已切换"),
                tr("语言设置已保存。部分界面将在重启后完全生效。")
            );
        } else {
            // 恢复原来的选择
            ui->comboBox->blockSignals(true);
            ui->comboBox->setCurrentIndex(langMgr.languageIndex(langMgr.currentLanguage()));
            ui->comboBox->blockSignals(false);
        }
    }
}

void GeneralSettingForm::retranslateUi()
{
    ui->retranslateUi(this);
}

void GeneralSettingForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}
