/*
 *
 * Copyright (C) 2008 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_VIEW_COLUMN_ADJUSTER_H_
#define _K3B_VIEW_COLUMN_ADJUSTER_H_

#include <QtCore/QObject>
#include <QtCore/QList>

class QEvent;
class QTreeView;

namespace K3b {
    class ViewColumnAdjuster : public QObject
    {
        Q_OBJECT

    public:
        ViewColumnAdjuster( QObject* parent = 0 );
        ViewColumnAdjuster( QTreeView* parent );
        ~ViewColumnAdjuster();

        /**
         * Takes header into account if not hidden.
         */
        int columnSizeHint( int col ) const;

        void setView( QTreeView* view );
        void setFixedColumns( const QList<int>& );
        void addFixedColumn( int );

        void setColumnMargin( int column, int margin );
        int columnMargin( int column ) const;

        bool eventFilter( QObject* watched, QEvent* event );

    Q_SIGNALS:
        /**
         * If something is connected to this slot, adjustColumns
         * will not be called automatically.
         */
        void columnsNeedAjusting();

    public Q_SLOTS:
        void adjustColumns();

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_adjustColumns() )
    };
}

#endif
