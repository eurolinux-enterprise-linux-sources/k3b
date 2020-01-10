/*
 *
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef K3B_DEVICE_OPTIONTAB_H
#define K3B_DEVICE_OPTIONTAB_H

#include <QWidget>

class QLabel;

namespace K3b {
    class DeviceWidget;

    class DeviceOptionTab : public QWidget
    {
        Q_OBJECT

    public:
        DeviceOptionTab( QWidget* = 0 );
        ~DeviceOptionTab();

        void readDevices();
        void saveDevices();

    private Q_SLOTS:
        void slotModifyPermissionsButtonClicked();
        void slotRefreshButtonClicked();

    private:
        QLabel* m_labelDevicesInfo;
        DeviceWidget* m_deviceWidget;
    };
}



#endif
