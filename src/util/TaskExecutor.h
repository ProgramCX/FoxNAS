#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H
#include <QDebug>
#include <QObject>
#include <QQueue>
#include <functional>
class TaskExecutor : public QObject
{
    Q_OBJECT
public:
    TaskExecutor() = default;

    // 添加任务到队列
    void addTask(std::function<void()> task) { taskQueue.enqueue(task); }

    // 执行队列中的任务
    void executeTasks()
    {
        while (!taskQueue.isEmpty()) {
            std::function<void()> task = taskQueue.dequeue();
            task(); // 执行任务
        }
    }

private:
    QQueue<std::function<void()>> taskQueue; // 任务队列
};
#endif // TASKEXECUTOR_H
