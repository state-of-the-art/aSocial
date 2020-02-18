#ifndef WDATE_H
#define WDATE_H

/**
 * Date Wrapper
 *
 * Replaces QML javascript Date object, due to next restrictions:
 *  - 1970.01.01 limited
 *  - Wrong adding year: 1991.01.01 00:00 + 1y = 1991.12.31 23:00, and same issue with 2006, 2007, 2008 years
 */

#include <QObject>
#include <QVariant>

class WDate : public QObject
{
    Q_OBJECT
    Q_ENUMS(DETAIL)

  public:
    explicit WDate(QObject *parent = 0);
    ~WDate();

    enum DETAIL { HOUR, DAY, MONTH, YEAR };

  public slots:
    QList<QVariant> getAxisMarks(DETAIL detail, qint64 munixtime_from, qint64 munixtime_interval);
    DETAIL getLevel(qint64 munixtime);

    QString format(qint64 munixtime, QString format);
    qint64 tzOffsetSec();
    qint64 currentUnixtime();

    bool checkFormat(QString datetime, QString format);
    qint64 unixtimeFromString(QString datetime, QString format, bool in_utc);
};

#endif  // WDATE_H
