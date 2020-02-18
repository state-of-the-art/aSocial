#ifndef INTERNALIMAGEPROVIDER_H
#define INTERNALIMAGEPROVIDER_H

#include <QQuickImageProvider>

class InternalImageProvider : public QQuickImageProvider
{
  public:
    InternalImageProvider();
    ~InternalImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif  // INTERNALIMAGEPROVIDER_H
