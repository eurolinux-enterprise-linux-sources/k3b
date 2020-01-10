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



#ifndef _K3B_NOTIFY_OPTIONTAB_H_
#define _K3B_NOTIFY_OPTIONTAB_H_

#include <qwidget.h>

class KNotifyConfigWidget;

namespace K3b {
class NotifyOptionTab : public QWidget
{
  Q_OBJECT

 public:
  NotifyOptionTab( QWidget* parent = 0 );
  ~NotifyOptionTab();

  void readSettings();
  bool saveSettings();

 private:
   KNotifyConfigWidget *m_notifyWidget;
};
}

#endif
