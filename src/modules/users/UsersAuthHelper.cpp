/*
 *  User KCM
 *  Roland Singer <roland@manjaro.org>
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

#include "UsersAuthHelper.h"

#include <QtCore/QFile>
#include <QtCore/QProcess>

ActionReply
UsersAuthHelper::add( const QVariantMap& args )
{
    QProcess* useradd = new QProcess();
    useradd->start( "/usr/sbin/useradd", args["arguments"].toStringList() );
    connect( useradd, &QProcess::readyRead,
             [=] ()
    {
        QString data = QString::fromUtf8( useradd->readAll() ).trimmed();
        if ( !data.isEmpty() )
        {
            QVariantMap map;
            map.insert( QLatin1String( "Data" ), data );
            HelperSupport::progressStep( map );
        }
    } );
    if ( !useradd->waitForStarted( 5000 ) )
        return ActionReply::HelperErrorReply();

    if ( !useradd->waitForFinished( 15000 ) )
        return ActionReply::HelperErrorReply();

    if ( useradd->exitCode() != QProcess::NormalExit )
        return ActionReply::HelperErrorReply();

    return ActionReply::SuccessReply();
}


ActionReply
UsersAuthHelper::remove( const QVariantMap& args )
{
    QProcess* userdel = new QProcess();
    userdel->start( "/usr/sbin/userdel", args["arguments"].toStringList() );
    connect( userdel, &QProcess::readyRead,
             [=] ()
    {
        QString data = QString::fromUtf8( userdel->readAll() ).trimmed();
        if ( !data.isEmpty() )
        {
            QVariantMap map;
            map.insert( QLatin1String( "Data" ), data );
            HelperSupport::progressStep( map );
        }
    } );
    if ( !userdel->waitForStarted( 5000 ) )
        return ActionReply::HelperErrorReply();

    if ( !userdel->waitForFinished( 15000 ) )
        return ActionReply::HelperErrorReply();

    if ( userdel->exitCode() != QProcess::NormalExit )
        return ActionReply::HelperErrorReply();

    return ActionReply::SuccessReply();
}


ActionReply
UsersAuthHelper::changepassword( const QVariantMap& args )
{
    QProcess* passwd = new QProcess();
    passwd->start( "/usr/bin/passwd", args["arguments"].toStringList() );

    connect( passwd, &QProcess::readyRead,
             [=] ()
    {
        QString data = QString::fromUtf8( passwd->readAll() ).trimmed();
        if ( !data.isEmpty() )
        {
            QVariantMap map;
            map.insert( QLatin1String( "Data" ), data );
            HelperSupport::progressStep( map );
        }
    } );

    if ( !passwd->waitForStarted( 5000 ) )
        return ActionReply::HelperErrorReply();

    for ( QString arg : args["writeArgs"].toStringList() )
        passwd->write( QString( arg + "\n" ).toUtf8() );

    passwd->closeWriteChannel();

    if ( !passwd->waitForFinished( 15000 ) )
        return ActionReply::HelperErrorReply();

    if ( passwd->exitCode() != QProcess::NormalExit )
        return ActionReply::HelperErrorReply();

    return ActionReply::SuccessReply();
}


ActionReply
UsersAuthHelper::changeaccounttype( const QVariantMap& args )
{
    QProcess* usermod = new QProcess();
    usermod->start( "/usr/sbin/usermod", args["arguments"].toStringList() );
    connect( usermod, &QProcess::readyRead,
             [=] ()
    {
        QString data = QString::fromUtf8( usermod->readAll() ).trimmed();
        if ( !data.isEmpty() )
        {
            QVariantMap map;
            map.insert( QLatin1String( "Data" ), data );
            HelperSupport::progressStep( map );
        }
    } );
    if ( !usermod->waitForStarted( 5000 ) )
        return ActionReply::HelperErrorReply();

    if ( !usermod->waitForFinished( 15000 ) )
        return ActionReply::HelperErrorReply();

    if ( usermod->exitCode() != QProcess::NormalExit )
        return ActionReply::HelperErrorReply();

    return ActionReply::SuccessReply();
}


ActionReply
UsersAuthHelper::changeimage( const QVariantMap& args )
{
    QString filename = args["filename"].toString();
    for ( QString dest : args["copyDest"].toStringList() )
    {
        if ( QFile::exists( dest ) )
            QFile::remove( dest );

        if ( !QFile::copy( filename, dest ) )
            return ActionReply::HelperErrorReply();
    }

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN( "org.kaosx.kcm.users", UsersAuthHelper )
#include "moc_UsersAuthHelper.cpp"
