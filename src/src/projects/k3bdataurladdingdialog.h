/*
 *
 * Copyright (C) 2005-2008 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_DATA_URL_ADDING_DIALOG_H_
#define _K3B_DATA_URL_ADDING_DIALOG_H_

#include <kdialog.h>
#include <kurl.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qdir.h>
#include <QLabel>
#include <QList>

#include <kio/global.h>

class QProgressBar;
class QLabel;

namespace K3b {

    class DataItem;
    class DirItem;
    class EncodingConverter;
    class DirSizeJob;
    class DataDoc;

    class DataUrlAddingDialog : public KDialog
    {
        Q_OBJECT

    public:
        ~DataUrlAddingDialog();

        /**
         * shows DataUrlAddingDialog in non-blocking fashion
         * (doesn't wait till dialog is closed)
         */
        static void addUrls( const KUrl::List& urls, DirItem* dir,
                            QWidget* parent = 0 );

        static void moveItems( const QList<DataItem*>& items, DirItem* dir,
                              QWidget* parent = 0 );

        static void copyItems( const QList<DataItem*>& items, DirItem* dir,
                              QWidget* parent = 0 );

    private Q_SLOTS:
        void slotStartAddUrls();
        void slotStartCopyMoveItems();
        void slotAddUrls();
        void slotCopyMoveItems();
        void slotCancel();
        void slotDirSizeDone( bool );
        void updateProgress();

    private:
        DataUrlAddingDialog( const KUrl::List& urls, DirItem* dir, QWidget* parent = 0 );
        DataUrlAddingDialog( const QList<DataItem*>& items, DirItem* dir, bool copy, QWidget* parent = 0 );
        void init();
        bool getNewName( const QString& oldName, DirItem* dir, QString& newName );
        bool addHiddenFiles();
        bool addSystemFiles();
        QString resultMessage() const;

        QProgressBar* m_progressWidget;
        QLabel* m_infoLabel;
        QLabel* m_counterLabel;
        EncodingConverter* m_encodingConverter;

        KUrl::List m_urls;
        QList< QPair<KUrl, DirItem*> > m_urlQueue;
        QList< QPair<DataItem*, DirItem*> > m_items;
        QList<KUrl> m_dirSizeQueue;

        DataDoc* m_doc;
        bool m_bExistingItemsReplaceAll;
        bool m_bExistingItemsIgnoreAll;
        bool m_bFolderLinksFollowAll;
        bool m_bFolderLinksAddAll;
        int m_iAddHiddenFiles;
        int m_iAddSystemFiles;

        QStringList m_unreadableFiles;
        QStringList m_notFoundFiles;
        QStringList m_nonLocalFiles;
        QStringList m_tooBigFiles;
        QStringList m_mkisofsLimitationRenamedFiles;
        QStringList m_invalidFilenameEncodingFiles;

        bool m_bCanceled;
        bool m_copyItems;

        KIO::filesize_t m_totalFiles;
        KIO::filesize_t m_filesHandled;
        DirSizeJob* m_dirSizeJob;

        unsigned int m_lastProgress;
    };
}

#endif
