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

#include "xflrServer.h"
#include <globals/mainframe.h>

#include "rpc/server.h"
#include "RpcLibAdapters.h"
#include <xdirect/objects2d.h>
#include <iostream>
#include <QObject>
#include <QString>
#include <QVector>

#include "utils.h"

MainFrame* xflrServer::s_pMainFrame = nullptr;

using namespace std;

xflrServer::xflrServer(int port) : server(port)
{
    QObject::connect(this, &xflrServer::onNewProject, s_pMainFrame, &MainFrame::onNewProjectHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onSaveProject, s_pMainFrame, &MainFrame::onSaveProject, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onLoadProject, s_pMainFrame, &MainFrame::onLoadFileHeadless,Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onXDirect, s_pMainFrame, &MainFrame::onXDirect, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onAFoil, s_pMainFrame, &MainFrame::onAFoil, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onMiarex, s_pMainFrame, &MainFrame::onMiarex, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onXInverse, s_pMainFrame, &MainFrame::onXInverse, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflrServer::onClose, s_pMainFrame, &MainFrame::close);
    
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
        if (app==XFLR5::enumApp::NOAPP){
            return;
        }
        else if (app==XFLR5::enumApp::XFOILANALYSIS){
            emit onXDirect();
        }
        else if (app==XFLR5::enumApp::DIRECTDESIGN){
            emit onAFoil();
        }
        else if (app==XFLR5::enumApp::MIAREX){
            emit onMiarex();
        }
        else if (app==XFLR5::enumApp::INVERSEDESIGN){
            emit onXInverse();
        }
        });
    
    server.bind("foilExists", [&](string name)->bool{
        return Objects2d::foilExists(QString::fromStdString(name));
    });

    server.bind("getFoil",[&](string name)->RpcLibAdapters::FoilAdapter{
        return RpcLibAdapters::FoilAdapter(*Objects2d::foil(QString::fromStdString(name)));
    });

    server.bind("foilList", [&]()->vector<RpcLibAdapters::FoilAdapter>{
        return FoilVecFromQFoilQVec(*Objects2d::pOAFoil());
    });
    
    server.bind("foilCoords", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        vector<RpcLibAdapters::Coord> v;

        double(&x)[IBX] = pFoil->x;
        double(&y)[IBX] = pFoil->y;
        for (int i=0; i<pFoil->n; i++){
            v.push_back({x[i],y[i]});
        }
        return v;
    });

    server.bind("exit",[&]{
        stop();
        emit onClose();
        });
}

void xflrServer::run(){
    server.run();
}

void xflrServer::stop(){
    server.close_sessions();
    server.stop();
}

xflrServer::~xflrServer(){
   stop();
}

