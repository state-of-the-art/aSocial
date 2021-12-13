#ifndef UIWORKER_H
#define UIWORKER_H

#include <QThread>

#include "cli/cli.h"

class UiWorker : public QThread
{
    Q_OBJECT

public:
    void configure();

private:
    void run() override;
    std::unique_ptr<cli::Menu> root_menu;
};

#endif // UIWORKER_H
