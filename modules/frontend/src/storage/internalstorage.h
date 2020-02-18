#ifndef INTERNALMEDIASTORAGE_H
#define INTERNALMEDIASTORAGE_H

#include <QObject>

class InternalMediaStorage : public QObject
{
    Q_OBJECT
  public:
    explicit InternalMediaStorage(QObject *parent = 0);
    ~InternalMediaStorage();

  signals:

  public slots:
};

#endif  // INTERNALMEDIASTORAGE_H
