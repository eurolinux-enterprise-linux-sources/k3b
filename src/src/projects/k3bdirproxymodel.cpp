/*
 *
 * Copyright (C) 2009      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include "k3bdirproxymodel.h"
#include "k3bmodeltypes.h"


K3b::DirProxyModel::DirProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

K3b::DirProxyModel::~DirProxyModel()
{
}


bool K3b::DirProxyModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
{
    QAbstractItemModel *model = sourceModel();
    QModelIndex index = model->index(source_row, 0, source_parent);

    QVariant data = index.data(K3b::ItemTypeRole);
    K3b::ItemType type = K3b::FileItemType;

    if (data.isValid())
        type = (K3b::ItemType) index.data(K3b::ItemTypeRole).toInt();

    return (type == K3b::DirItemType);
}
