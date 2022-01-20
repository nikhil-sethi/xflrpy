/****************************************************************************

    xflServer Class
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

#include "xflserver.h"
#include <globals/mainframe.h>
#include <twodwidgets/foildesignwt.h>
#include <design/afoil.h>
#include <xflobjects/objects2d/foil.h>
#include "rpc/server.h"
#include "RpcLibAdapters.h"
#include <xflobjects/objects2d/objects2d.h>
#include <iostream>
#include <QObject>
#include <QString>
#include <QVector>

#include "utils.h"

MainFrame* xflServer::s_pMainFrame = nullptr;

using namespace std;

xflServer::xflServer(int port) : server(port)
{
    QObject::connect(this, &xflServer::onNewProject, s_pMainFrame, &MainFrame::onNewProjectHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSaveProject, s_pMainFrame, &MainFrame::onSaveProject, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onLoadProject, s_pMainFrame, &MainFrame::onLoadFileHeadless,Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onXDirect, s_pMainFrame, &MainFrame::onXDirect, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onAFoil, s_pMainFrame, &MainFrame::onAFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onMiarex, s_pMainFrame, &MainFrame::onMiarex, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onXInverse, s_pMainFrame, &MainFrame::onXInverse, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onClose, s_pMainFrame, &MainFrame::close);
    QObject::connect(this, &xflServer::onFoilGeom, s_pMainFrame->m_pAFoil, &AFoil::onAFoilFoilGeomHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onAFoilNacaFoils, s_pMainFrame->m_pAFoil, &AFoil::onAFoilNacaFoilsHeadless, Qt::BlockingQueuedConnection);
    
    cout << "Starting Xflr server at port: "<< port << endl;

    server.bind("ping", []()->bool{
        return true;
        });
    server.bind("loadProject", [&](vector<string> files){
        emit onLoadProject(QStringList::fromVector(QStrQVecFromStrVec(files)));
        });
    server.bind("newProject", [&](){
        emit onNewProject();
        
        });
    server.bind("saveProject", [&](){
        emit onSaveProject();
        });    
    server.bind("getState", [&]()->RpcLibAdapters::StateAdapter{
        return RpcLibAdapters::StateAdapter(s_pMainFrame->m_FileName,s_pMainFrame->s_ProjectName,s_pMainFrame->m_iApp,s_pMainFrame->s_bSaved);
        });
    server.bind("setProjectPath",[&](string projectPath){
        s_pMainFrame->setProjectName(QString::fromStdString(projectPath));
        });
    server.bind("setApp",[&](int app){
        if (app==xfl::enumApp::NOAPP){
            return;
        }
        else if (app==xfl::enumApp::XFOILANALYSIS){
            emit onXDirect();
        }
        else if (app==xfl::enumApp::DIRECTDESIGN){
            emit onAFoil();
        }
        else if (app==xfl::enumApp::MIAREX){
            emit onMiarex();
        }
        else if (app==xfl::enumApp::INVERSEDESIGN){
            emit onXInverse();
        }
        });
    
    server.bind("foilExists", [&](string name)->bool{
        return Objects2d::foilExists(QString::fromStdString(name));
    });

    server.bind("getFoil",[&](string name = "")->RpcLibAdapters::FoilAdapter{
        if (name ==""){
            pFoil = Objects2d::curFoil();
        }
        else{
            pFoil = Objects2d::foil(QString::fromStdString(name))
        }
        return RpcLibAdapters::FoilAdapter(*pFoil);
    });

    server.bind("foilList", [&]()->vector<RpcLibAdapters::FoilAdapter>{
        return FoilVecFromQFoilQVec(*Objects2d::pOAFoil());
    });
    
    server.bind("foilCoords", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        vector<RpcLibAdapters::Coord> v;

        double(&x)[IBX] = pFoil->m_x;
        double(&y)[IBX] = pFoil->m_y;
        for (int i=0; i<pFoil->m_n; i++){
            v.push_back({x[i],y[i]});
        }
        return v;
    });
    
    server.bind("setCamber", [&](double val, string name){
        QString qname = QString::fromStdString(name);
        Foil* pFoil = new Foil();
        Foil* currFoil = Objects2d::foil(qname);
        pFoil->copyFoil(currFoil);
        pFoil->m_fCamber = val;
        emit onFoilGeom(pFoil, qname);
        pFoil->normalizeGeometry();
    });
    server.bind("setThickness", [&](double val, string name){
        QString qname = QString::fromStdString(name);
        Foil* pFoil = new Foil();
        Foil* currFoil = Objects2d::foil(qname);
        pFoil->copyFoil(currFoil);
        pFoil->m_fThickness = val;
        emit onFoilGeom(pFoil, qname);
        pFoil->normalizeGeometry();
    });
    server.bind("setCamberX", [&](double val, string name){
        QString qname = QString::fromStdString(name);
        Foil* pFoil = new Foil();
        Foil* currFoil = Objects2d::foil(qname);
        pFoil->copyFoil(currFoil);
        pFoil->m_fXCamber = val;
        emit onFoilGeom(pFoil, qname);
        pFoil->normalizeGeometry();

    });
    server.bind("setThickX", [&](double val, string name){
        QString qname = QString::fromStdString(name);
        Foil* pFoil = new Foil();
        Foil* currFoil = Objects2d::foil(qname);
        pFoil->copyFoil(currFoil);
        pFoil->m_fXThickness = val;
        emit onFoilGeom(pFoil, qname);
        pFoil->normalizeGeometry();

    });
    server.bind("createNACAFoil", [&](int digits, string name){
        emit onAFoilNacaFoils(digits, QString::fromStdString(name));
    });

    // server.bind("showFoil", [&](bool val, string name){
    //     Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
    //     // s_pMainFrame->m_pDirect2dWidget->showFoil(pFoil, val);
    // });

    server.bind("exit",[&]{
        stop();
        emit onClose();
        });
}

void xflServer::run(){
    server.run();
}

void xflServer::stop(){
    server.close_sessions();
    server.stop();
}

xflServer::~xflServer(){
   stop();
}

