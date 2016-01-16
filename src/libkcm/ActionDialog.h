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

#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

#include <KAuth/KAuthAction>

#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>

class ActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionDialog( QWidget* parent = 0 );

    KAuth::Action installAction() const;
    void setInstallAction( const KAuth::Action& installAction );

    QString message() const;
    void setMessage( const QString& message );

    QString information() const;
    void setInformation( const QString& information );

    bool isJobSuccesful() const;

private:
    void startJob();

    QTextEdit* m_terminal;
    QLabel* m_messageLabel;
    QLabel* m_informationLabel;
    QDialogButtonBox* m_buttonBox;

    KAuth::Action m_installAction;
    QString m_message;
    QString m_information;
    bool m_jobSuccesful;

    QString m_lastMessage;
};

#endif // ACTIONDIALOG_H
