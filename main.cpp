#include <QtWidgets>
#include <QSystemTrayIcon>
#include <QTimer>
#include <Windows.h>
#include <pdh.h>

class TaskManager : public QWidget
{
    Q_OBJECT

public:
    TaskManager(QWidget *parent = nullptr) : QWidget(parent)
    {
        QLabel *ramLabel = new QLabel("RAM Usage:", this);
        ramUsageLabel = new QLabel(this);

        QLabel *diskLabel = new QLabel("Disk Usage:", this);
        diskUsageLabel = new QLabel(this);

        QLabel *cpuLabel = new QLabel("CPU Usage:", this);
        cpuUsageLabel = new QLabel(this);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(ramLabel);
        layout->addWidget(ramUsageLabel);
        layout->addWidget(diskLabel);
        layout->addWidget(diskUsageLabel);
        layout->addWidget(cpuLabel);
        layout->addWidget(cpuUsageLabel);

        // Установка значка в системный трей
        trayIcon = new QSystemTrayIcon(QIcon(":/icon.png"), this);
        trayIcon->show();

        // Создание таймера для обновления данных о загрузке
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TaskManager::updateUsage);
        timer->start(1000); // Обновление каждую секунду
    }

private slots:
    void updateUsage()
    {
        // Получение информации о загрузке ОЗУ
        MEMORYSTATUSEX memoryStatus;
        memoryStatus.dwLength = sizeof(memoryStatus);
        GlobalMemoryStatusEx(&memoryStatus);
        double ramUsage = 100.0 - ((double)memoryStatus.ullAvailPhys / memoryStatus.ullTotalPhys) * 100.0;
        ramUsageLabel->setText(QString::number(ramUsage, 'f', 2) + "%");

        // Получение информации о загрузке диска
        ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
        GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes);
        double diskUsage = 100.0 - ((double)totalNumberOfFreeBytes.QuadPart / totalNumberOfBytes.QuadPart) * 100.0;
        diskUsageLabel->setText(QString::number(diskUsage, 'f', 2) + "%");

        // Получение информации о загрузке процессора
        PDH_FMT_COUNTERVALUE counterValue;
        PDH_HCOUNTER counter;
        PdhOpenQuery(NULL, NULL, &queryHandle);
        PdhAddCounter(queryHandle, L"\\Processor(_Total)\\% Processor Time", NULL, &counter);
        PdhCollectQueryData(queryHandle);
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterValue);
        double cpuUsage = counterValue.doubleValue;
        cpuUsageLabel->setText(QString::number(cpuUsage, 'f', 2) + "%");

        // Обновление значка в системном трее
        QString tooltip = "RAM Usage: " + QString::number(ramUsage, 'f', 2) + "%\n";
        tooltip += "Disk Usage: " + QString::number(diskUsage, 'f', 2) + "%\n";
        tooltip += "CPU Usage: " + QString::number(cpuUsage, 'f', 2) + "%";
        trayIcon->setToolTip(tooltip);
    }

private:
    QLabel *ramUsageLabel;
    QLabel *diskUsageLabel;
    QLabel *cpuUsageLabel;
    QSystemTrayIcon *trayIcon;
    PDH_HQUERY queryHandle;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TaskManager taskManager;
    taskManager.show();

    return app.exec();
}

#include "resources.qrc"
