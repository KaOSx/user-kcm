/*
 *  User KCM
 *  Ramon Buldó <ramon@manjaro.org>
 *  Demm <demm@kaosx.us>
 *
 *  Copyright (C) 2007 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LocaleAuthHelper.h"

#include <QtDBus/QDBusInterface>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QSet>

#include <QDebug>

ActionReply
LocaleAuthHelper::save( const QVariantMap& args )
{
    if ( args["isLocaleListModified"].toBool() )
    {
        updateLocaleGen( args["locales"].toStringList() );
        generateLocaleGen();
    }
    if ( args["isSystemLocalesModified"].toBool() )
        setSystemLocale( args["localeList"].toStringList() );
    return ActionReply::SuccessReply();
}


bool
LocaleAuthHelper::updateLocaleGen( QStringList locales )
{

    const QString localeGen = "/etc/locale.gen";
    QFile file( localeGen );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        qDebug() << QString( "error: failed to open file '%1'" ).arg( localeGen );
        return false;
    }

    QStringList content;

    QTextStream in( &file );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        content.append( line );
        line = line.trimmed();

        bool found = false;

        for ( QString locale : locales )
        {
            if ( line.startsWith( locale + " " ) )
            {
                found = true;
                locales.removeAll( locale );
                break;
            }

            if ( line.startsWith( "#" + locale + " " ) )
            {
                content.removeLast();
                content.append( line.remove( 0, 1 ) );
                found = true;
                locales.removeAll( locale );
                break;
            }
        }

        if ( !found && !line.split( "#", QString::KeepEmptyParts ).first()
                .trimmed().isEmpty() )
        {
            content.removeLast();
            content.append( "#" + line );
        }
    }
    file.close();

    // Add missing locales in the file
    for ( QString locale : locales )
    {
        QString validLocale = localeToLocaleGenFormat( locale );
        if ( validLocale.isEmpty() )
        {
            qDebug() << QString( "warning: failed to obtain valid locale string"
                                 "for locale '%1'!" ).arg( locale );
            continue;
        }
        content.append( validLocale );
    }

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        qDebug() << QString( "error: failed to open file '%1'!" ).arg( localeGen );
        return false;
    }
    QTextStream out( &file );
    out << content.join( "\n" );
    out.flush();
    file.close();

    return true;
}


bool
LocaleAuthHelper::generateLocaleGen()
{
    QProcess localeGen;
    localeGen.start( "/usr/bin/locale-gen" );
    connect( &localeGen, &QProcess::readyRead,
             [&] ()
    {
        QString data = QString::fromUtf8( localeGen.readAll() ).trimmed();
        if ( !data.isEmpty() )
        {
            QVariantMap map;
            map.insert( QLatin1String( "Data" ), data );
            HelperSupport::progressStep( map );
        }
    } );
    if ( !localeGen.waitForStarted() )
        return false;
    if ( !localeGen.waitForFinished( 60000 ) )
        return false;
    return true;
}


bool
LocaleAuthHelper::setSystemLocale( const QStringList localeList )
{
    QDBusInterface dbusInterface( "org.freedesktop.locale1",
                                  "/org/freedesktop/locale1",
                                  "org.freedesktop.locale1",
                                  QDBusConnection::systemBus() );
    /*
     * asb
     * array_string -> locale
     * boolean -> arg_ask_password
     */
    QDBusMessage reply;
    reply = dbusInterface.call( "SetLocale", localeList, true );
    if ( reply.type() == QDBusMessage::ErrorMessage )
        return false;
    return true;
}


QString
LocaleAuthHelper::localeToLocaleGenFormat( const QString locale )
{
    QSet<QString> localeList;

    QFile localeGen( "/etc/locale.gen" );
    QString lines;
    if ( localeGen.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QTextStream in( &localeGen );
        lines.append( in.readAll() );
    }

    QFile localeGenPacnew( "/etc/locale.gen.pacnew" );
    if ( localeGenPacnew.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QTextStream in( &localeGenPacnew );
        lines.append( in.readAll() );
    }

    for ( const QString line : lines.split( '\n' ) )
    {
        if ( line.startsWith( "# " ) || line.simplified() == "#"
                || line.isEmpty() )
            continue;

        if ( line.simplified().startsWith( "#" ) )
            localeList.insert( line.simplified().remove( '#' ) );
        else
            localeList.insert( line.simplified() );
    }

    for ( const QString line : localeList )
    {
        if ( line.startsWith( locale + " " ) )
            return line;
    }

    return "";
}

KAUTH_HELPER_MAIN( "org.kaosx.kcm.locale", LocaleAuthHelper )
#include "moc_LocaleAuthHelper.cpp"
