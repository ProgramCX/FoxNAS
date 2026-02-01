#include "InfoForm.h"
#include "ui_InfoForm.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <ApiRequest.h>
#include <ApiUrl.h>
#include <MemStore.h>

#include <QClipboard>
#include <AboutDialog.h>
#include "../ErrorLogDialog.h"

InfoForm::InfoForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InfoForm)
{
    ui->setupUi(this);
    loadHardwareInfo();
}

InfoForm::~InfoForm()
{
    delete ui;
}

void InfoForm::loadHardwareInfo()
{
    ApiRequest *apiRequest = new ApiRequest(getFullApiPath(FULLHOST, NASHARDWAREINFO),
                                            ApiRequest::GET,
                                            this);
    connect(apiRequest,
            &ApiRequest::responseRecieved,
            this,
            [=](QString &rawContent, bool hasError, qint16 statusCode) {
                if (statusCode == 200) {
                    QJsonDocument dataDoc = apiRequest->getData(rawContent);

                    QJsonObject jsonObject = dataDoc.object();

                    QString result;

                    QString operatingSystem = jsonObject["operatingSystem"].toString();

                    result += tr("操作系统: %1\n\n").arg(operatingSystem);

                    // 获取 CPU 信息
                    QString cpuModel = jsonObject["cpuModel"].toString();
                    QString cpuVendor = jsonObject["cpuVendor"].toString();
                    result += tr("CPU型号: %1\nCPU厂商: %2\n").arg(cpuModel, cpuVendor);

                    result += "\n";
                    // 获取主板信息
                    QString mainBoardModel = jsonObject["mainBoardModel"].toString();
                    QString mainBoardVendor = jsonObject["mainBoardVendor"].toString();
                    result += tr("主板型号: %1\n主板厂商: %2\n").arg(mainBoardModel, mainBoardVendor);

                    result += "\n";
                    // 获取内存信息
                    int countMem = 0;
                    QJsonArray memoryList = jsonObject["memoryList"].toArray();
                    result += "内存信息:\n";
                    for (const QJsonValue &value : memoryList) {
                        QJsonObject memoryObject = value.toObject();
                        QString memoryModel = memoryObject["model"].toString();
                        QString memoryVendor = memoryObject["vendor"].toString();
                        double memorySizeGb = memoryObject["sizeGb"].toDouble();
                        if (memoryList.size() > 1) {
                            result += tr("内存 %1:\n").arg(++countMem);
                        }
                        result += tr("内存型号: %1\n内存厂商: %2\n内存大小: %3 GB\n")
                                      .arg(memoryModel, memoryVendor)
                                      .arg(memorySizeGb);
                    }

                    result += "\n";
                    // 获取硬盘信息
                    QJsonArray diskList = jsonObject["diskList"].toArray();
                    result += "硬盘信息:\n";
                    int countDisk = 0;
                    for (const QJsonValue &value : diskList) {
                        QJsonObject diskObject = value.toObject();
                        QString diskModel = diskObject["model"].toString();
                        QString diskVendorOrSerial = diskObject["vendorOrSerial"].toString();
                        double diskSizeGb = diskObject["sizeGb"].toDouble();
                        if (diskList.size() > 1) {
                            result += tr("硬盘 %1:\n").arg(++countDisk);
                        }
                        result += tr("硬盘型号: %1\n硬盘厂商/序列号: %2\n硬盘大小: %3 GB\n")
                                      .arg(diskModel, diskVendorOrSerial)
                                      .arg(diskSizeGb);
                    }

                    result += "\n";
                    // 获取显卡信息
                    QJsonArray gpuList = jsonObject["gpuList"].toArray();
                    result += "显卡信息:\n";
                    int countGpu = 0;
                    for (const QJsonValue &value : gpuList) {
                        QJsonObject gpuObject = value.toObject();
                        QString gpuModel = gpuObject["model"].toString();
                        QString gpuVendor = gpuObject["vendor"].toString();
                        if (gpuList.size() > 1) {
                            result += tr("显卡 %1:\n").arg(++countGpu);
                        }
                        result += tr("显卡型号: %1\n显卡厂商: %2\n").arg(gpuModel, gpuVendor);
                    }

                    ui->hardInfoEdit->setPlainText(result);
                }
            });

    apiRequest->sendRequest();
}

void InfoForm::on_pushButtonCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->hardInfoEdit->toPlainText());
}

void InfoForm::on_pushButtonAbout_clicked()
{
    AboutDialog about;
    about.exec();
}

void InfoForm::on_pushButtonViewLogs_clicked()
{
    ErrorLogDialog errorLogDialog(this);
    errorLogDialog.exec();
}
