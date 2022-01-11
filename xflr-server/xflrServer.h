/****************************************************************************

    xflrServer Class
    Copyright (C) 2021-2022 Nikhil Sethi 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#ifndef XFLRSERVER_H
#define XFLRSERVER_H

#include <globals/mainframe.h>
#include "rpc/server.h"
#include <QThread>
#include <QObject>
#include <iostream>
#include <QString>

class Foil;
class AFoil;
class FoilDesignWt;

class xflrServer: public QThread
{   
    Q_OBJECT
    public:
        xflrServer(int port);
        ~xflrServer();
        // void start(int thread_count);
        void run();
        // rpc::server getServer();
        rpc::server server;
        void stop();
        // void foo();
        static MainFrame* s_pMainFrame;   
    // private:
    signals:
        void onMiarexSig();
        void onSetPlane(QString planeName);
        void onNewProject();
        void onSaveProject();
        void onCloseProject();
        void onLoadProject(QStringList files);
        void onXDirect();
        void onAFoil();
        void onMiarex();
        void onXInverse();
        void onClose();
        void onFoilGeom(Foil* foil, QString name);

};

#endif