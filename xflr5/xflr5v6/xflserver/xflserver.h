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

#pragma once

#include "rpc/server.h"
#include <QThread>
#include <QObject>
#include <iostream>
#include <QString>
#include <xflcore/linestyle.h>
// #include <xflserver/RpcLibAdapters.h>   // need implementation to use as reference

class Foil; // only need pointer not actual implementation
class AFoil;
class Polar;
class MainFrame; // need only pointer 
class Plane;
class WPolar;
namespace RpcLibAdapters{

    class AnalysisSettings2D;
    class AnalysisSettings3D;
    class XDirectDisplayState;
}

class xflServer: public QThread
{   
    Q_OBJECT
    public:
        xflServer(int port);
        ~xflServer();
        // void start(int thread_count);
        void run();
        // rpc::server getServer();
        rpc::server server;
        void stop();
        // void foo();
        static MainFrame* s_pMainFrame;   
    // private:
    signals:

        // Mainframe signals
        void onMiarexSig();
        void onNewProject();
        void onSaveProject();
        void onCloseProject();
        void onLoadProject(QStringList files);
        void onXDirect();
        void onAFoil();
        void onMiarex();
        void onXInverse();
        void onClose();
        void onUpdate();
        
        // AFoil signals
        void onRenameFoil(Foil* foil, QString name);
        void onFoilGeom(Foil* foil, QString name);
        void onAFoilNacaFoils(int s_Digits, QString name);
        void onSelectFoil(Foil* foil);
        Foil* onDuplicateFoil(Foil* foil, QString toName);
        void onShowFoil(Foil* foil, bool flag);
        void onDeleteFoil(Foil* foil);
        void onNormalizeFoil();
        void onDerotateFoil();
        void onFoilStyle(Foil* foil, LineStyle ls);
        void onExportFoil(Foil* foil, QString FileName);
        void onSetFoilCoords(Foil* foil, double* x,  double* y);

        // XDirect signals
        void onDefinePolar(Polar* polar, Foil* foil);
        void onAnalyzeCurPolar();
        void onSetAnalysisSettings2D(RpcLibAdapters::AnalysisSettings2D* analysis_settings);
        void onSelectPolar(Polar* polar);
        void onSetXDirectDisplay(RpcLibAdapters::XDirectDisplayState* dsp_state);
        void onXDirectAnimate(bool flag);
        void onXDirectAnimateSpeed(int val);

        // Miarex signals
        void onNewPlane(Plane* plane);
        void onDefineWPolar(WPolar* polar, Plane* pPlane);
        void onSetAnalysisSettings3D(RpcLibAdapters::AnalysisSettings3D& analysis_settings);
        void onAnalyzeCurWPolar();
        void onSetWPolar(WPolar* pPolar);
        void onSetPlane(Plane* pPlane);
};
