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


#ifndef K3B_EXTERNALBIN_OPTIONTAB_H
#define K3B_EXTERNALBIN_OPTIONTAB_H

#include <qwidget.h>



namespace K3b {
    class ExternalBinManager;
}
namespace K3b {
    class ExternalBinWidget;
}


namespace K3b {
class ExternalBinOptionTab : public QWidget
{
Q_OBJECT

 public:
  ExternalBinOptionTab( ExternalBinManager*, QWidget* = 0 );
  ~ExternalBinOptionTab();

  void readSettings();
  void saveSettings();

 private:
  ExternalBinManager* m_manager;

  ExternalBinWidget* m_externalBinWidget;
};
}



#endif
