/*
 *
 * Copyright (C) 2003-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bdeviceoptiontab.h"
#include "k3bdevicemanager.h"
#include "k3bdevicewidget.h"
#include "k3bglobals.h"
#include "k3bcore.h"

#include <KConfigGroup>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KProcess>

#include <QApplication>
#include <QCursor>
#include <QGridLayout>
#include <QLabel>
#include <QString>


K3b::DeviceOptionTab::DeviceOptionTab( QWidget* parent )
    : QWidget( parent )
{
    QGridLayout* frameLayout = new QGridLayout( this );
    frameLayout->setMargin( 0 );


    // Info Label
    // ------------------------------------------------
    m_labelDevicesInfo = new QLabel( this );
    m_labelDevicesInfo->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    m_labelDevicesInfo->setText( "<p>" + i18n( "K3b tries to detect all your devices properly. "
                                               "If K3b is unable to detect your drive, you need to modify "
                                               "their permissions to give K3b write access to all devices." ) );
    m_labelDevicesInfo->setWordWrap(true);
    // ------------------------------------------------

    m_deviceWidget = new K3b::DeviceWidget( k3bcore->deviceManager(), this );

    frameLayout->addWidget( m_labelDevicesInfo, 0, 0 );
    frameLayout->addWidget( m_deviceWidget, 1, 0 );

    connect( m_deviceWidget, SIGNAL(modifyPermissionsButtonClicked()), this, SLOT(slotModifyPermissionsButtonClicked()) );
    connect( m_deviceWidget, SIGNAL(refreshButtonClicked()), this, SLOT(slotRefreshButtonClicked()) );
}


K3b::DeviceOptionTab::~DeviceOptionTab()
{
}


void K3b::DeviceOptionTab::readDevices()
{
    m_deviceWidget->init();
}



void K3b::DeviceOptionTab::saveDevices()
{
    // save the config
    k3bcore->deviceManager()->saveConfig( KGlobal::config()->group( "Devices" ) );
}


void K3b::DeviceOptionTab::slotModifyPermissionsButtonClicked()
{
    QStringList args;
    args << "k3bsetup" << "--lang" << KGlobal::locale()->language();
    if( !KProcess::startDetached( K3b::findExe("kcmshell4"), args ) )
        KMessageBox::error( this, i18n("Unable to start K3b::Setup.") );
}


void K3b::DeviceOptionTab::slotRefreshButtonClicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    k3bcore->deviceManager()->clear();
    k3bcore->deviceManager()->scanBus();
    m_deviceWidget->init();
    QApplication::restoreOverrideCursor();
}

#include "k3bdeviceoptiontab.moc"
