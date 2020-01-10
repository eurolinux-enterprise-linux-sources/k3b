/*
 *
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
 *           (C) 2009      Arthur Renato Mello <arthur@mandriva.com>
 *           (C) 2009      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *           (C) 2009      Michal Malek <michalm@jabster.pl>
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


#include "k3bmovixview.h"
#include "k3bmovixprojectmodel.h"
#include "k3bmovixdoc.h"
#include "k3bmovixburndialog.h"
#include "k3bmovixfileitem.h"

#include "k3bdatapropertiesdialog.h"
#include "k3baction.h"
#include "k3bvolumenamewidget.h"

#include <KAction>
#include <KDebug>
#include <KFileDialog>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KUrl>

#include <QList>
#include <KToolBar>

K3b::MovixView::MovixView( K3b::MovixDoc* doc, QWidget* parent )
    : K3b::StandardView( doc, parent ),
      m_doc(doc)
{
    m_model = new K3b::MovixProjectModel(m_doc, this);
    // set the model for the K3b::StandardView's views
    setModel(m_model);
    setViewExpanded(true);

    // and hide the side panel as the movix project has no tree hierarchy
    setShowDirPanel(false);

    // setup actions
    m_actionProperties = K3b::createAction( this, i18n("Properties"), "document-properties",
                                            0, this, SLOT(showPropertiesDialog()),
                                            actionCollection(), "movix_show_props" );
    m_actionRemove = K3b::createAction( this, i18n( "Remove" ), "edit-delete",
                                        Qt::Key_Delete, this, SLOT(slotRemoveSelectedIndexes()),
                                        actionCollection(), "movix_remove_item" );
    m_actionRemoveSubTitle = K3b::createAction( this, i18n( "Remove Subtitle File" ), "edit-delete",
                                                0, this, SLOT(slotRemoveSubTitleItems()),
                                                actionCollection(), "movix_remove_subtitle_item" );
    m_actionAddSubTitle = K3b::createAction( this, i18n("Add Subtitle File..."), 0,
                                             0, this, SLOT(slotAddSubTitleFile()),
                                             actionCollection(), "movix_add_subtitle" );

    m_popupMenu = new KMenu( this );
    m_popupMenu->addAction( m_actionRemove );
    m_popupMenu->addAction( m_actionRemoveSubTitle );
    m_popupMenu->addAction( m_actionAddSubTitle );
    m_popupMenu->addSeparator();
    m_popupMenu->addAction( m_actionProperties );
    m_popupMenu->addSeparator();
    m_popupMenu->addAction( actionCollection()->action("project_burn") );

    // Setup toolbar
    addPluginButtons();
    toolBox()->addWidget( new VolumeNameWidget( doc, toolBox() ) );
}


K3b::MovixView::~MovixView()
{
}


void K3b::MovixView::showPropertiesDialog()
{
    QModelIndexList selection = currentSelection();

    if ( selection.isEmpty() )
    {
        // show project properties
        slotProperties();
    }
    else
    {
        QList<K3b::DataItem*> items;

        foreach(const QModelIndex &index, selection)
            items.append(m_model->itemForIndex(index));

        K3b::DataPropertiesDialog dlg( items, this );
        dlg.exec();
    }
}


void K3b::MovixView::slotRemoveSubTitleItems()
{
    QModelIndexList selection = currentSelection();
    if ( !selection.count() )
        return;

    K3b::MovixFileItem *item = 0;
    foreach(const QModelIndex &index, selection)
    {
        item = m_model->itemForIndex(index);
        if (item)
            m_doc->removeSubTitleItem( item );
    }
}


void K3b::MovixView::slotAddSubTitleFile()
{
    QModelIndexList selection = currentSelection();
    if ( !selection.count() )
        return;

    K3b::MovixFileItem *item = 0;
    foreach(const QModelIndex &index, selection)
    {
        item = m_model->itemForIndex(index);
        if (item)
            break;
    }

    if( item ) {
        KUrl url = KFileDialog::getOpenUrl();
        if( url.isValid() ) {
            if( url.isLocalFile() )
                m_doc->addSubTitleItem( item, url );
            else
                KMessageBox::error( 0, i18n("K3b currently only supports local files.") );
        }
    }
}


void K3b::MovixView::selectionChanged( const QModelIndexList& indexes )
{
    if( indexes.count() >= 1 ) {
        m_actionRemove->setEnabled(true);

        bool subtitle = false;
        // check if any of the items have a subtitle
        foreach (const QModelIndex &index, indexes) {
            K3b::MovixFileItem *item = m_model->itemForIndex(index);
            if (item && item->subTitleItem()) {
                subtitle = true;
                break;
            }
        }
        m_actionRemoveSubTitle->setEnabled( subtitle );
        // only enable the subtitle adding if there is just one item selected
        m_actionAddSubTitle->setEnabled( indexes.count() == 1 );
    }
    else {
        m_actionRemove->setEnabled(false);
        m_actionRemoveSubTitle->setEnabled( false );
        m_actionAddSubTitle->setEnabled( false );
    }
}


void K3b::MovixView::contextMenu( const QPoint& pos )
{
    m_popupMenu->popup( pos );
}


K3b::ProjectBurnDialog* K3b::MovixView::newBurnDialog( QWidget* parent )
{
    return new K3b::MovixBurnDialog( m_doc, parent );
}

#include "k3bmovixview.moc"
