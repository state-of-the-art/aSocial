#include "wdate.h"

#include <QDateTime>
#include <QDebug>
#include <QTimeZone>

WDate::WDate(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Create WDate date wrapper";
}

WDate::~WDate()
{
    qDebug() << "Destroy WDate date wrapper";
}

QList<QVariant> WDate::getAxisMarks(WDate::DETAIL detail, qint64 munixtime_from, qint64 munixtime_interval)
{
    QList<QVariant> out;

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(munixtime_from);
    QDate d = dt.date();
    QTime t = QTime::currentTime();

    // Resetting date & time basing on the detail level
    if( detail > WDate::MONTH )
        d.setDate(d.year(), 1, d.day());
    if( detail > WDate::DAY )
        d.setDate(d.year(), d.month(), 1);
    if( detail > WDate::HOUR )
        t.setHMS(0, 0, 0);
    else
        t.setHMS(dt.time().hour(), 0, 0);

    dt = QDateTime::currentDateTime();
    dt.setDate(d);
    dt.setTime(t);

    // Filling array with marks unixtime data
    qint64 ut = 0;
    qint64 to = munixtime_from + munixtime_interval;
    while( true ) {
        ut = dt.toMSecsSinceEpoch();

        if( detail != WDate::HOUR && dt.time().hour() != 0 ) {
            dt.setTime(t);  // Fix for switching to summer time
        }

        if( detail == WDate::YEAR )
            dt = dt.addYears(1);
        if( detail == WDate::MONTH )
            dt = dt.addMonths(1);
        if( detail == WDate::DAY )
            dt = dt.addDays(1);
        if( detail == WDate::HOUR )
            dt = dt.addSecs(3600);

        if( ut < munixtime_from )
            continue;  // Skip dates before
        else if( ut > to )
            break;  // Skip dates after

        out.append(QVariant::fromValue(ut / 1000));
    }

    return out;
}

WDate::DETAIL WDate::getLevel(qint64 munixtime)
{
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(munixtime);

    // Get level of mark by unixtime
    DETAIL level = WDate::HOUR;
    if( dt.time().hour() == 0 ) {
        level = WDate::DAY;
        if( dt.date().day() == 1 ) {
            level = WDate::MONTH;
            if( dt.date().month() == 1 )
                level = WDate::YEAR;
        }
    }

    return level;
}

QString WDate::format(qint64 munixtime, QString format)
{
    return QDateTime::fromMSecsSinceEpoch(munixtime).toString(format);
}

qint64 WDate::tzOffsetSec()
{
    return QDateTime::currentDateTime().offsetFromUtc();
}

qint64 WDate::currentUnixtime()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000;
}

bool WDate::checkFormat(QString datetime, QString format)
{
    return QDateTime::fromString(datetime, format).isValid();
}

qint64 WDate::unixtimeFromString(QString datetime, QString format, bool in_utc)
{
    QDateTime dt = QDateTime::fromString(datetime, format);
    if( in_utc )
        dt.setTimeSpec(Qt::UTC);
    return dt.toMSecsSinceEpoch() / 1000;
}
