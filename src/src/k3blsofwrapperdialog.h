/*
 *
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_LSOF_WRAPPER_DIALOG_H_
#define _K3B_LSOF_WRAPPER_DIALOG_H_

#include <kdialog.h>

namespace K3b {
    class RichTextLabel;

    namespace Device {
        class Device;
    }

    class LsofWrapperDialog : public KDialog
    {
        Q_OBJECT

    public:
        ~LsofWrapperDialog();

        /**
         * Check if other applications are currently using the device and if so
         * warn the user and provide a quick solution to shut down these other
         * applications.
         *
         * If the device is not in use this method simply returns.
         */
        static void checkDevice( Device::Device* dev, QWidget* parent = 0 );

    private Q_SLOTS:
        bool slotCheckDevice();
        void slotQuitOtherApps();

    private:
        LsofWrapperDialog( QWidget* parent );

        Device::Device* m_device;
        RichTextLabel* m_label;
    };
}

#endif
