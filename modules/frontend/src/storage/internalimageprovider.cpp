#include "internalimageprovider.h"

InternalImageProvider::InternalImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

InternalImageProvider::~InternalImageProvider()
{
}

QImage InternalImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QImage();
}
