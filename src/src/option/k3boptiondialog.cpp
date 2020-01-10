/*
 *
 * $Id$
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#include "k3boptiondialog.h"
#include "k3bcore.h"
#include "k3bdeviceoptiontab.h"
#include "k3badvancedoptiontab.h"
#include "k3bexternalbinoptiontab.h"
#include "k3bmiscoptiontab.h"
#include "k3bthemeoptiontab.h"
#include "k3bpluginoptiontab.h"
#include "k3bcddboptiontab.h"
#include "k3bsystemproblemdialog.h"
#include "k3bnotifyoptiontab.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QTabWidget>
#include <QToolTip>
#include <QVBoxLayout>

#include <KLocale>
#include <KIconLoader>
#include <KConfig>
#include <KConfigGroup>

// TODO: handle the default-settings

K3b::OptionDialog::OptionDialog(QWidget *parent )
    : KPageDialog( parent )
{
    setFaceType( List );
    setCaption( i18n("Settings") );
    showButtonSeparator( true );

    setupMiscPage();
    setupDevicePage();
    setupProgramsPage();
    setupNotifyPage();
    setupPluginPage();
    setupThemePage();
    setupCddbPage();
    setupAdvancedPage();

    m_miscOptionTab->readSettings();
    m_deviceOptionTab->readDevices();
    m_externalBinOptionTab->readSettings();
    m_notifyOptionTab->readSettings();
    m_themeOptionTab->readSettings();
    m_advancedOptionTab->readSettings();

    // if we don't do this the dialog start really huge
    // because of the label in the device-tab
    resize( 700, 500 );

    connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );
    connect( this, SIGNAL( defaultClicked() ), SLOT( slotDefault() ) );
    connect( this, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
}


K3b::OptionDialog::~OptionDialog()
{
}


void K3b::OptionDialog::setCurrentPage( ConfigPage page )
{
    Pages::const_iterator pageIt = m_pages.constFind( page );
    if( pageIt != m_pages.constEnd() ) {
        KPageDialog::setCurrentPage( *pageIt );
    }
}


void K3b::OptionDialog::slotOk()
{
    if( saveSettings() ) {
        accept();

        KConfigGroup grp( KGlobal::config(), "General Options" );
        if( grp.readEntry( "check system config", true ) )
            K3b::SystemProblemDialog::checkSystem();
    }
}

void K3b::OptionDialog::slotApply()
{
    saveSettings();
}


bool K3b::OptionDialog::saveSettings()
{
    m_deviceOptionTab->saveDevices();
    m_externalBinOptionTab->saveSettings();
    m_notifyOptionTab->saveSettings();
    m_themeOptionTab->saveSettings();
    m_cddbOptionTab->apply();
    m_advancedOptionTab->saveSettings();

    if( !m_miscOptionTab->saveSettings() )
        return false;

    return true;
}


void K3b::OptionDialog::slotDefault()
{
}


void K3b::OptionDialog::setupMiscPage()
{
    m_miscOptionTab = new K3b::MiscOptionTab;
    m_miscPage = addPage( m_miscOptionTab, i18n("Misc") );
    m_miscPage->setHeader( i18n("Miscellaneous Settings") );
    m_miscPage->setIcon( KIcon( "preferences-other" ) );
    m_pages.insert( Misc, m_miscPage );
}


void K3b::OptionDialog::setupDevicePage()
{
    m_deviceOptionTab = new K3b::DeviceOptionTab;
    m_devicePage = addPage( m_deviceOptionTab, i18n("Devices") );
    m_devicePage->setHeader( i18n("Setup Devices") );
    m_devicePage->setIcon( KIcon( "drive-optical" ) );
    m_pages.insert( Devices, m_devicePage );
}


void K3b::OptionDialog::setupProgramsPage()
{
    m_externalBinOptionTab = new K3b::ExternalBinOptionTab( k3bcore->externalBinManager() );
    m_programsPage = addPage( m_externalBinOptionTab, i18n("Programs") );
    m_programsPage->setHeader( i18n("Setup External Programs") );
    m_programsPage->setIcon( KIcon( "system-run" ) );
    m_pages.insert( Programs, m_programsPage );
}


void K3b::OptionDialog::setupNotifyPage()
{
    m_notifyOptionTab = new K3b::NotifyOptionTab;
    m_notifyPage = addPage( m_notifyOptionTab, i18n("Notifications") );
    m_notifyPage->setHeader( i18n("System Notifications") );
    m_notifyPage->setIcon( KIcon( "preferences-desktop-notification" ) );
    m_pages.insert( Notifications, m_notifyPage );
}


void K3b::OptionDialog::setupPluginPage()
{
    m_pluginOptionTab = new K3b::PluginOptionTab;
    m_pluginPage = addPage( m_pluginOptionTab, i18n("Plugins") );
    m_pluginPage->setHeader( i18n("K3b Plugin Configuration") );
    m_pluginPage->setIcon( KIcon( "preferences-plugin" ) );
    m_pages.insert( Plugins, m_pluginPage );
}


void K3b::OptionDialog::setupThemePage()
{
    m_themeOptionTab = new K3b::ThemeOptionTab;
    m_themePage = addPage( m_themeOptionTab, i18n("Themes") );
    m_themePage->setHeader( i18n("K3b GUI Themes") );
    m_themePage->setIcon( KIcon( "preferences-desktop-theme" ) );
    m_pages.insert( Themes, m_themePage );
}


void K3b::OptionDialog::setupCddbPage()
{
    m_cddbOptionTab = new K3b::CddbOptionTab;
    m_cddbPage = addPage( m_cddbOptionTab, i18n("CDDB") );
    m_cddbPage->setHeader( i18n("CDDB Audio CD Info Retrieval") );
    m_cddbPage->setIcon( KIcon( "media-optical-audio" ) );
    m_pages.insert( Cddb, m_cddbPage );
}


void K3b::OptionDialog::setupAdvancedPage()
{
    m_advancedOptionTab = new K3b::AdvancedOptionTab;
    m_advancedPage = addPage( m_advancedOptionTab, i18n("Advanced") );
    m_advancedPage->setHeader( i18n("Advanced Settings") );
    m_advancedPage->setIcon( KIcon( "media-optical-recordable" ) );
    m_pages.insert( Advanced, m_advancedPage );
}

#include "k3boptiondialog.moc"
