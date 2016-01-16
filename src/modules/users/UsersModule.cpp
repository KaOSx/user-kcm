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

#include "UsersModule.h"
#include "ui_PageUsers.h"

#include <KAboutData>
#include <KAuth>
#include <KAuthAction>

#include <KPluginFactory>
K_PLUGIN_FACTORY( KcmUsersFactory,
                  registerPlugin<PageUsers>( "kcm_users" ); )

PageUsers::PageUsers( QWidget* parent, const QVariantList& args ) :
    KCModule( parent, args ),
    ui( new Ui::PageUsers )
{
    KAboutData* aboutData = new KAboutData( "kcm_users",
                                            tr( "User Accounts", "@title" ),
                                            PROJECT_VERSION,
                                            QStringLiteral( "" ),
                                            KAboutLicense::LicenseKey::GPL_V3,
                                            "Copyright 2016 demm" );

    aboutData->addAuthor( "demm",
                          tr( "Author", "@info:credit" ),
                          QStringLiteral( "demm@kaosx.us" ) );

    setAboutData( aboutData );
    setButtons( KCModule::NoAdditionalButton );

    ui->setupUi( this );


    // Connect signals and slots
    connect( ui->listWidget, SIGNAL( currentItemChanged( QListWidgetItem*,QListWidgetItem* ) )   ,   this, SLOT( setupUserData( QListWidgetItem* ) ) );
    connect( ui->buttonImage, SIGNAL( clicked() )  ,   this, SLOT( buttonImage_clicked() ) );
    connect( ui->buttonAdd, SIGNAL( clicked() )    ,   this, SLOT( buttonAddUser_clicked() ) );
    connect( ui->buttonRemove, SIGNAL( clicked() ) ,   this, SLOT( buttonRemoveUser_clicked() ) );
    connect( ui->buttonPassword, SIGNAL( clicked() )   ,   this, SLOT( buttonChangePassword_clicked() ) );
    connect( ui->buttonAccountType, SIGNAL( clicked() )    ,   this, SLOT( buttonChangeAccountType_clicked() ) );
}


PageUsers::~PageUsers()
{
    delete ui;
}


void
PageUsers::load()
{
    ui->listWidget->clear();

    QList<Global::User> users = Global::getAllUsers();

    for ( int i = 0; i < users.size(); i++ )
    {
        const Global::User* user = &users.at( i );

        ListWidgetItem* item = new ListWidgetItem( ui->listWidget );
        item->setText( user->username );
        item->user = *user;

        if ( QFile::exists( user->homePath + "/.face" ) )
            item->setIcon( QIcon( user->homePath + "/.face" ) );
        else
            item->setIcon( QIcon( ":/icons/user.png" ) );
    }

    setupUserData( ui->listWidget->currentItem() );
}


void
PageUsers::save()
{
    return;
}


void
PageUsers::defaults()
{
    this->load();
}


void
PageUsers::setupUserData( QListWidgetItem* current )
{
    if ( !current )
    {
        ui->buttonImage->setIcon( QIcon( ":/icons/user.png" ) );
        ui->labelUsername->setText( "" );
        ui->buttonAccountType->setText( tr( "Standard" ) );
        ui->userWidget->setEnabled( false );
        return;
    }

    ui->buttonImage->setIcon( current->icon() );
    ui->labelUsername->setText( current->text() );
    ui->buttonAccountType->setText( tr( "Standard" ) );
    ui->userWidget->setEnabled( true );

    QList<Global::Group> groups = Global::getAllGroups();

    for ( int i = 0; i < groups.size(); i++ )
    {
        const Global::Group* group = &groups.at( i );
        if ( group->name != ADMIN_GROUP || !group->members.contains( current->text() ) )
            continue;

        ui->buttonAccountType->setText( tr( "Administrator" ) );
        break;
    }
}


void
PageUsers::buttonImage_clicked()
{
    ListWidgetItem* item = dynamic_cast<ListWidgetItem*>( ui->listWidget->currentItem() );
    if ( !item )
        return;

    PreviewFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setNameFilter( tr( "Images (*.png *.jpg *.bmp)" ) );
    dialog.setViewMode( QFileDialog::Detail );
    dialog.setDirectory( FACES_IMAGE_FOLDER );

    if ( !dialog.exec() || dialog.selectedFiles().isEmpty() )
        return;

    QString filename = dialog.selectedFiles().first();

    // Set icon in our GUI
    item->setIcon( QIcon( filename ) );
    ui->buttonImage->setIcon( item->icon() );

    // Copy .face file to home dir
    QString faceDest;
    faceDest = QString( "%1/.face" ).arg( item->user.homePath );

    if ( QFile::exists( faceDest ) )
        QFile::remove( faceDest );
    if ( !QFile::copy( filename, faceDest ) )
    {
        QMessageBox::warning( this,
                              tr( "Error!" ),
                              tr( "Failed to copy image to '%1'!" ).arg( faceDest ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        return;
    }

    // Create symlinks to ~/.face
    QStringList symlinkHomeDest;
    symlinkHomeDest << QString( "%1/.face.icon" ).arg( item->user.homePath )
                    << QString( "%1/.icon" ).arg( item->user.homePath );

    for ( QString dest : symlinkHomeDest )
    {
        if ( QFile::exists( dest ) )
            QFile::remove( dest );

        if ( !QFile::link( ".face", dest ) )
        {
            QMessageBox::warning( this,
                                  tr( "Error!" ),
                                  tr( "Failed to symlink '%1' to '%2'!" ).arg( ".face", dest ),
                                  QMessageBox::Ok,
                                  QMessageBox::Ok );
            return;
        }
    }

    // Copy face image to dirs that need admin rights
    QStringList copyDest;
    if ( QDir( "/var/lib/AccountsService/icons/" ).exists() )
    {
        qDebug() << "/var/lib/AccountsService/icons/";
        copyDest << QString( "/var/lib/AccountsService/icons/%1" ).arg( item->user.username );
    }
    if ( QDir( "/usr/share/sddm/faces/" ).exists() )
    {
        qDebug() << "/usr/share/sddm/faces/";
        copyDest  << QString( "/usr/share/sddm/faces/%1.face.icon" ).arg( item->user.username );
    }

    if ( !copyDest.isEmpty() )
    {
        KAuth::Action installAction( QLatin1String( "org.kaosx.kcm.users.changeimage" ) );
        installAction.setHelperId( QLatin1String( "org.kaosx.kcm.users" ) );
        QVariantMap args;
        args["copyDest"] = copyDest;
        args["filename"] = filename;
        installAction.setArguments( args );
        KAuth::ExecuteJob* jobAdd = installAction.execute();
        connect( jobAdd, &KAuth::ExecuteJob::newData,
                 [=] ( const QVariantMap &data )
        {
            qDebug() << data;
        } );
        if ( jobAdd->exec() )
            qDebug() << "Change image job succesfull";
        else
        {
            QMessageBox::warning( this, tr( "Error!" ), QString( tr( "Failed to change user image" ) ), QMessageBox::Ok, QMessageBox::Ok );
            return;
        }
    }

}


void
PageUsers::buttonAddUser_clicked()
{
    AddUserDialog dialog( this );
    dialog.exec();

    // Refresh list if required
    if ( dialog.userDataChanged() )
        load();
}


void
PageUsers::buttonRemoveUser_clicked()
{
    ListWidgetItem* item = dynamic_cast<ListWidgetItem*>( ui->listWidget->currentItem() );
    if ( !item )
        return;

    QString username = item->text();

    if ( QMessageBox::No == QMessageBox::question( this,
            tr( "Continue?" ),
            tr( "Do you really want to remove the user %1?" ).arg( username ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No ) )
        return;

    QString removeHome = "";
    if ( QMessageBox::Yes == QMessageBox::question( this,
            tr( "Remove Home?" ),
            tr( "Do you want to remove the home folder of the user %1?" ).arg( username ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No ) )
        removeHome = "-r";

    // Remove user
    KAuth::Action installAction( QLatin1String( "org.kaosx.kcm.users.remove" ) );
    installAction.setHelperId( QLatin1String( "org.kaosx.kcm.users" ) );
    QVariantMap args;
    args["arguments"] = QStringList() << removeHome << username;
    installAction.setArguments( args );
    KAuth::ExecuteJob* jobAdd = installAction.execute();
    connect( jobAdd, &KAuth::ExecuteJob::newData,
             [=] ( const QVariantMap &data )
    {
        qDebug() << data;
    } );
    if ( jobAdd->exec() )
        qDebug() << "Remove user job succesfull";
    else
    {
        QMessageBox::warning( this, tr( "Error!" ), QString( tr( "Failed to remove user %1" ).arg( username ) ), QMessageBox::Ok, QMessageBox::Ok );
        close();
        return;
    }

    load();
}


void
PageUsers::buttonChangePassword_clicked()
{
    ListWidgetItem* item = dynamic_cast<ListWidgetItem*>( ui->listWidget->currentItem() );
    if ( !item )
        return;

    ChangePasswordDialog dialog( this );
    dialog.exec( item->text() );
}


void
PageUsers::buttonChangeAccountType_clicked()
{
    ListWidgetItem* item = dynamic_cast<ListWidgetItem*>( ui->listWidget->currentItem() );
    if ( !item )
        return;

    AccountTypeDialog dialog( this );
    dialog.exec( item->text() );

    // Update account type if required
    if ( dialog.userGroupsChanged() )
        setupUserData( item );
}

#include "UsersModule.moc"
