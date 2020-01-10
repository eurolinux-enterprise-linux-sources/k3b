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

#include "k3bstatusbarmanager.h"
#include "k3bcore.h"
#include "k3bbusywidget.h"
#include "k3b.h"
#include "k3bversion.h"
#include "k3bglobals.h"
#include "k3bprojectmanager.h"
#include "k3bapplication.h"
#include "k3baudiodoc.h"
#include "k3bdatadoc.h"
#include "k3bmixeddoc.h"
#include "k3bvcddoc.h"
#include "k3bdiritem.h"
#include "k3bview.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kstatusbar.h>
#include <kaboutdata.h>
#include <kaction.h>

#include <qlabel.h>

#include <qfile.h>
#include <qtimer.h>
#include <qevent.h>
#include <qtooltip.h>
#include <kvbox.h>



K3b::StatusBarManager::StatusBarManager( K3b::MainWindow* parent )
    : QObject(parent),
      m_mainWindow(parent)
{
    // setup free temp space box
    KHBox* boxFreeTemp = new KHBox( m_mainWindow->statusBar() );
    boxFreeTemp->setSpacing(2);

    m_labelProjectInfo = new QLabel( m_mainWindow->statusBar() );

    m_pixFreeTemp = new QLabel( boxFreeTemp );
    (void)new QLabel( i18n("Temp:"), boxFreeTemp );
    m_pixFreeTemp->setPixmap( SmallIcon("folder-green") );
    m_labelFreeTemp = new QLabel( boxFreeTemp );
    boxFreeTemp->installEventFilter( this );

    // setup info area
    m_labelInfoMessage = new QLabel( " ", m_mainWindow->statusBar() );

    // setup version info
    m_versionBox = new QLabel( QString("K3b %1").arg(KGlobal::mainComponent().aboutData()->version()), m_mainWindow->statusBar() );
    m_versionBox->installEventFilter( this );

    // setup the statusbar
    m_mainWindow->statusBar()->addWidget( m_labelInfoMessage, 1 ); // for showing some info
    m_mainWindow->statusBar()->addPermanentWidget( m_labelProjectInfo, 0 );
    // a spacer item
    m_mainWindow->statusBar()->addPermanentWidget( new QLabel( "  ", m_mainWindow->statusBar() ), 0 );
    m_mainWindow->statusBar()->addPermanentWidget( boxFreeTemp, 0 );
    // a spacer item
    m_mainWindow->statusBar()->addPermanentWidget( new QLabel( "  ", m_mainWindow->statusBar() ), 0 );
    m_mainWindow->statusBar()->addPermanentWidget( m_versionBox, 0 );
    // a spacer item
    m_mainWindow->statusBar()->addPermanentWidget( new QLabel( "  ", m_mainWindow->statusBar() ), 0 );

    connect( m_mainWindow, SIGNAL(configChanged(KSharedConfig::Ptr)), this, SLOT(update()) );
    //FIXME kde4
#if 0
    connect( m_mainWindow->actionCollection(), SIGNAL(actionStatusText(const QString&)),
             this, SLOT(showActionStatusText(const QString&)) );
    connect( m_mainWindow->actionCollection(), SIGNAL(clearStatusText()),
             this, SLOT(clearActionStatusText()) );
#endif
    connect( k3bappcore->projectManager(), SIGNAL(activeProjectChanged(K3b::Doc*)),
             this, SLOT(slotActiveProjectChanged(K3b::Doc*)) );
    connect( k3bappcore->projectManager(), SIGNAL(projectChanged(K3b::Doc*)),
             this, SLOT(slotActiveProjectChanged(K3b::Doc*)) );
    connect( (m_updateTimer = new QTimer( this )), SIGNAL(timeout()), this, SLOT(slotUpdateProjectStats()) );

    update();
}


K3b::StatusBarManager::~StatusBarManager()
{
}


void K3b::StatusBarManager::update()
{
    QString path = K3b::defaultTempPath();

    if( !QFile::exists( path ) )
        path.truncate( path.lastIndexOf('/') );

    unsigned long size, avail;
    if( K3b::kbFreeOnFs( path, size, avail ) )
        slotFreeTempSpace( path, size, 0, avail );
    else
        m_labelFreeTemp->setText(i18n("No info"));
    if(path != m_labelFreeTemp->parentWidget()->toolTip())
        m_labelFreeTemp->parentWidget()->setToolTip( path );
}


void K3b::StatusBarManager::slotFreeTempSpace(const QString&,
                                            unsigned long kbSize,
                                            unsigned long,
                                            unsigned long kbAvail)
{
    m_labelFreeTemp->setText(KIO::convertSizeFromKiB(kbAvail)  + "/" +
                             KIO::convertSizeFromKiB(kbSize)  );

    // if we have less than 640 MB that is not good
    if( kbAvail < 655360 )
        m_pixFreeTemp->setPixmap( SmallIcon("folder-red") );
    else
        m_pixFreeTemp->setPixmap( SmallIcon("folder-green") );

    // update the display every second
    QTimer::singleShot( 2000, this, SLOT(update()) );
}


void K3b::StatusBarManager::showActionStatusText( const QString& text )
{
    m_mainWindow->statusBar()->showMessage( text );
}


void K3b::StatusBarManager::clearActionStatusText()
{
    m_mainWindow->statusBar()->clearMessage();
}


bool K3b::StatusBarManager::eventFilter( QObject* o, QEvent* e )
{
    if( e->type() == QEvent::MouseButtonDblClick ) {
        if( o == m_labelFreeTemp->parentWidget() )
            m_mainWindow->showOptionDialog( K3b::OptionDialog::Misc );
        else if( o == m_versionBox )
            if( QAction* a = m_mainWindow->action( "help_about_app" ) )
                a->activate(QAction::Trigger);
    }

    return QObject::eventFilter( o, e );
}


static QString dataDocStats( K3b::DataDoc* dataDoc )
{
    return i18np( "1 file in %2",
                  "%1 files in %2",
                  dataDoc->root()->numFiles(),
                  i18np("1 folder", "%1 folders", dataDoc->root()->numDirs()+1 ) );
}


void K3b::StatusBarManager::slotActiveProjectChanged( K3b::Doc* doc )
{
    if( doc && doc == k3bappcore->projectManager()->activeProject() ) {
        // cache updates
        if( !m_updateTimer->isActive() ) {
            slotUpdateProjectStats();
            m_updateTimer->setSingleShot( false );
            m_updateTimer->start( 1000 );
        }
    }
    else if( !doc ) {
        m_labelProjectInfo->setText( QString() );
    }
}


void K3b::StatusBarManager::slotUpdateProjectStats()
{
    K3b::Doc* doc = k3bappcore->projectManager()->activeProject();
    if( doc ) {
        switch( doc->type() ) {
        case K3b::Doc::AudioProject: {
            K3b::AudioDoc* audioDoc = static_cast<K3b::AudioDoc*>( doc );
            m_labelProjectInfo->setText( i18np("Audio CD (1 track)", "Audio CD (%1 tracks)", audioDoc->numOfTracks() ) );
            break;
        }

        case K3b::Doc::DataProject: {
            K3b::DataDoc* dataDoc = static_cast<K3b::DataDoc*>( doc );
            m_labelProjectInfo->setText( i18n("Data Project (%1)",dataDocStats(dataDoc)) );
            break;
        }

        case K3b::Doc::MixedProject: {
            K3b::AudioDoc* audioDoc = static_cast<K3b::MixedDoc*>( doc )->audioDoc();
            K3b::DataDoc* dataDoc = static_cast<K3b::MixedDoc*>( doc )->dataDoc();
            m_labelProjectInfo->setText( i18np("Mixed CD (1 track and %2)", "Mixed CD (%1 tracks and %2)", audioDoc->numOfTracks(),
                                               dataDocStats(dataDoc)) );
            break;
        }

        case K3b::Doc::VcdProject: {
            K3b::VcdDoc* vcdDoc = static_cast<K3b::VcdDoc*>( doc );
            m_labelProjectInfo->setText( i18np("Video CD (1 track)", "Video CD (%1 tracks)", vcdDoc->numOfTracks() ) );
            break;
        }

        case K3b::Doc::MovixProject: {
            K3b::DataDoc* dataDoc = static_cast<K3b::DataDoc*>( doc );
            m_labelProjectInfo->setText( i18n("eMovix Project (%1)",dataDocStats(dataDoc)) );
            break;
        }

        case K3b::Doc::VideoDvdProject: {
            K3b::DataDoc* dataDoc = static_cast<K3b::DataDoc*>( doc );
            m_labelProjectInfo->setText( i18n("Video DVD (%1)",dataDocStats(dataDoc)) );
            break;
        }
        }
    }
    else {
        m_labelProjectInfo->setText( QString() );
    }
}

#include "k3bstatusbarmanager.moc"
