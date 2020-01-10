/*
 * Copyright (C) 2008 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C) 2009 Arthur Mello <arthur@mandriva.com>
 * Copyright (C) 2010 Michal Malek <michalm@jabster.pl>
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

#ifndef _K3B_VCD_PROJECT_MODEL_H_
#define _K3B_VCD_PROJECT_MODEL_H_

#include <QAbstractTableModel>

namespace K3b {
    class VcdDoc;
    class VcdTrack;
    
    class VcdProjectModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        VcdProjectModel( VcdDoc* doc, QObject* parent = 0 );
        ~VcdProjectModel();

        enum Columns {
            NoColumn = 0,
            TitleColumn,
            TypeColumn,
            ResolutionColumn,
            HighResolutionColumn,
            FrameRateColumn,
            MuxRateColumn,
            DurationColumn,
            SizeColumn,
            FilenameColumn,
            NumColumns
        };

        VcdDoc* doc() const;

        VcdTrack* trackForIndex( const QModelIndex& index ) const;
        QModelIndex indexForTrack( VcdTrack* track, int column = NoColumn ) const;

        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
        virtual Qt::DropActions supportedDropActions() const;
        virtual QMimeData* mimeData( const QModelIndexList& indexes ) const;
        virtual QStringList mimeTypes() const;
        virtual bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );
        virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_aboutToAddRows(int, int))
        Q_PRIVATE_SLOT( d, void _k_addedRows())
        Q_PRIVATE_SLOT( d, void _k_aboutToRemoveRows(int, int))
        Q_PRIVATE_SLOT( d, void _k_removedRows())
    };
}

#endif
