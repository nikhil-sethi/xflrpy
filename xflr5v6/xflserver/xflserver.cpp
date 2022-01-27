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
    cout << "Starting Xflr server at port: "<< port << endl;

    //========================= Mainframe slots =========================//
    QObject::connect(this, &xflServer::onNewProject, s_pMainFrame, &MainFrame::onNewProjectHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSaveProject, s_pMainFrame, &MainFrame::onSaveProject, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onLoadProject, s_pMainFrame, &MainFrame::onLoadFileHeadless,Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onXDirect, s_pMainFrame, &MainFrame::onXDirect, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onAFoil, s_pMainFrame, &MainFrame::onAFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onMiarex, s_pMainFrame, &MainFrame::onMiarex, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onXInverse, s_pMainFrame, &MainFrame::onXInverse, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onClose, s_pMainFrame, &MainFrame::close);

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
    
    server.bind("deleteFoil", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onDeleteFoil(pFoil);
        });
    
    server.bind("exit",[&]{
        stop();
        emit onClose();
        });

    //====================== AFoil slots =======================// 
    QObject::connect(this, &xflServer::onFoilGeom, s_pMainFrame->m_pAFoil, &AFoil::onAFoilFoilGeomHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onAFoilNacaFoils, s_pMainFrame->m_pAFoil, &AFoil::onAFoilNacaFoilsHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onDuplicateFoil, s_pMainFrame->m_pAFoil, &AFoil::onDuplicateHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSelectFoil, s_pMainFrame->m_pAFoil, &AFoil::selectFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onShowFoil, s_pMainFrame->m_pAFoil, &AFoil::onShowFoilHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onRenameFoil, s_pMainFrame->m_pAFoil, &AFoil::onRenameFoilHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onDeleteFoil, s_pMainFrame->m_pAFoil, &AFoil::onDeleteFoilHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onNormalizeFoil, s_pMainFrame->m_pAFoil, &AFoil::onAFoilNormalizeFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onDerotateFoil, s_pMainFrame->m_pAFoil, &AFoil::onAFoilDerotateFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onFoilStyle, s_pMainFrame->m_pAFoil, &AFoil::onFoilStyleHeadless, Qt::BlockingQueuedConnection);
    
    server.bind("foilExists", [&](string name)->bool{
        return Objects2d::foilExists(QString::fromStdString(name));
    });

    server.bind("getFoil",[&](string name)->RpcLibAdapters::FoilAdapter{
        Foil* pFoil;
        if (name ==""){
            pFoil = Objects2d::curFoil();
            if (pFoil!=nullptr) {   // if the current foil is not the default splinefoil
                return RpcLibAdapters::FoilAdapter(*pFoil);
            }
            else {
                return RpcLibAdapters::FoilAdapter(); // return garbage foil if it's the spline foil
            }
        } 
        else {
            pFoil = Objects2d::foil(QString::fromStdString(name));
            return RpcLibAdapters::FoilAdapter(*pFoil);
        }
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

    server.bind("setGeom", [&](string name, double camber, double camber_x, double thickness, double thickness_x){
        QString qname = QString::fromStdString(name);
        Foil* pFoil = Objects2d::foil(qname);
        
        if (camber !=0.0)
            pFoil->m_fCamber = camber;
        if (camber_x !=0.0)
            pFoil->m_fXCamber = camber_x;
        if (thickness !=0.0)
            pFoil->m_fThickness = thickness;
        if (thickness_x !=0.0)
            pFoil->m_fXThickness = thickness_x;
        
        emit onFoilGeom(pFoil, qname);
        pFoil->normalizeGeometry();
    });

    server.bind("renameFoil", [&](string name, string newName){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onRenameFoil(pFoil, QString::fromStdString(newName));
    });

    server.bind("createNACAFoil", [&](int digits, string name){
        emit onAFoilNacaFoils(digits, QString::fromStdString(name));
    });

    server.bind("selectFoil", [&](string name){
        emit onSelectFoil(Objects2d::foil(QString::fromStdString(name)));
    });

    server.bind("duplicateFoil", [&](string fromName, string toName)->RpcLibAdapters::FoilAdapter{
        Foil* pFoil = Objects2d::foil(QString::fromStdString(fromName));
        Foil* newFoil = emit onDuplicateFoil(pFoil, QString::fromStdString(toName));
        return RpcLibAdapters::FoilAdapter(*newFoil);
    });

    server.bind("showFoil", [&](string name, bool flag){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onShowFoil(pFoil, flag);
    });

    server.bind("normalizeFoil", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onSelectFoil(pFoil);
        emit onNormalizeFoil();
    });

    server.bind("derotateFoil", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onSelectFoil(pFoil);
        emit onDerotateFoil();
    });

    server.bind("getLineStyle", [&](string name) -> RpcLibAdapters::LineStyleAdapter{
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        return RpcLibAdapters::LineStyleAdapter(pFoil->theStyle());
    });

    server.bind("setLineStyle", [&](string name, RpcLibAdapters::LineStyleAdapter& line_style){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        LineStyle ls = RpcLibAdapters::LineStyleAdapter::from_msgpack(line_style);
        
       emit onFoilStyle(pFoil, ls);

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

