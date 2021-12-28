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

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QString>
#include <QVector>

MainFrame* xflrServer::s_pMainFrame = nullptr;

QVector<QString> QStrlVecFromPyList(std::vector<std::string> list){
    QVector<QString> out;
    for (uint i=0;i<list.size();i++){
        out.push_back(QString::fromStdString(list.at(i)));
    };
    return out;
};

namespace adapters
{   
    struct StateAdapter{
        std::string projectPath;
        std::string projectName;
        int app;
        bool saved;
        bool display;
        MSGPACK_DEFINE_MAP(projectPath, projectName, app, saved, display);

        StateAdapter(QString _projectPath, QString _projectName, XFLR5::enumApp _app, bool _saved, bool _display=true){
            projectPath = _projectPath.toStdString();
            projectName = _projectName.toStdString();
            app = _app;
            saved = _saved;
            display=_display;

        }
    };
} // namespace adapters


xflrServer::xflrServer(int port) : server(port)
{
    QObject::connect(this, &xflrServer::onNewProject, s_pMainFrame, &MainFrame::onNewProjectHeadless);
    QObject::connect(this, &xflrServer::onSaveProject, s_pMainFrame, &MainFrame::onSaveProject);
    QObject::connect(this, &xflrServer::onLoadProject, s_pMainFrame, &MainFrame::onLoadFileHeadless);
    QObject::connect(this, &xflrServer::onXDirect, s_pMainFrame, &MainFrame::onXDirect);
    QObject::connect(this, &xflrServer::onAFoil, s_pMainFrame, &MainFrame::onAFoil);
    QObject::connect(this, &xflrServer::onMiarex, s_pMainFrame, &MainFrame::onMiarex);
    QObject::connect(this, &xflrServer::onXInverse, s_pMainFrame, &MainFrame::onXInverse);
    QObject::connect(this, &xflrServer::onClose, s_pMainFrame, &MainFrame::close);
    
    using namespace std;
    {
    cout << "Starting Xflr server at port: "<< port << endl;

    server.bind("ping", []()->bool{
        return true;
        });
    server.bind("loadProject", [&](vector<string> files){
        emit onLoadProject(QStringList::fromVector(QStrlVecFromPyList(files)));
        });
    server.bind("newProject", [&](){
        emit onNewProject();
        
        });
    server.bind("saveProject", [&](){
        emit onSaveProject();
        });    
    server.bind("getState", [&](){
        return adapters::StateAdapter(s_pMainFrame->m_FileName,s_pMainFrame->s_ProjectName,s_pMainFrame->m_iApp,s_pMainFrame->s_bSaved);
        });
    server.bind("setProjectPath",[&](std::string projectPath){
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
    server.bind("exit",[&]{
        stop();
        emit onClose();
        });
    } // namespace std   
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

