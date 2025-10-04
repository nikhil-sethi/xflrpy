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
#include <design/afoil.h>
#include "RpcLibAdapters.h" // redundant
#include <xdirect/xdirect.h>
#include <miarex/miarex.h>
#include <xflobjects/objects2d/foil.h>
#include "rpc/server.h"

#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects3d/objects3d.h>
#include <iostream>
#include <QObject>
#include <QString>
#include <QVector>
#include <QSignalSpy>

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
    QObject::connect(this, &xflServer::onUpdate, s_pMainFrame, &MainFrame::updateView, Qt::BlockingQueuedConnection);

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

    // ====================== AFoil slots =======================// 
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
    QObject::connect(this, &xflServer::onExportFoil, s_pMainFrame->m_pAFoil, &AFoil::onExportFoilHeadless, Qt::BlockingQueuedConnection);
    // QObject::connect(this, &xflServer::onSetFoilCoords, s_pMainFrame->m_pAFoil, &AFoil::onSetFoilCoordsHeadless, Qt::BlockingQueuedConnection);
    
    server.bind("foilExists", [&](string name)->bool{
        return Objects2d::foilExists(QString::fromStdString(name));
    });

    server.bind("getFoil",[&](string name)->RpcLibAdapters::FoilAdapter{
        Foil* pFoil;
        if (name =="") pFoil = Objects2d::curFoil();
        else pFoil = Objects2d::foil(QString::fromStdString(name));
            
        if (pFoil!=nullptr) {   // if the current foil is not the default splinefoil
            return RpcLibAdapters::FoilAdapter(*pFoil);
        }
        else {
            // return garbage foil if it's the spline foil
            // do further checks in python because msgpack needs to this adapter as a return type at all costs 
            return RpcLibAdapters::FoilAdapter(); 
        }
    });

    server.bind("foilList", [&]()->vector<RpcLibAdapters::FoilAdapter>{
        return FoilVecFromQFoilQVec(*Objects2d::pOAFoil());
    });
    
    server.bind("getFoilCoords", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        vector<RpcLibAdapters::Coord> v;

        double(&x)[IBX] = pFoil->m_x;
        double(&y)[IBX] = pFoil->m_y;
        for (int i=0; i<pFoil->m_n; i++){
            v.push_back({x[i],y[i]});
        }
        return v;
    });

    server.bind("setFoilCoords", [&](string name, vector<RpcLibAdapters::Coord> v){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        // double x[v.size()];
        // double y[v.size()];
        for (uint i=0; i<v.size(); i++){
            // vector<double> row = v[i].toVec();
            pFoil->m_xb[i] = v[i].x;
            pFoil->m_yb[i] = v[i].y;
            pFoil->m_x[i] = v[i].x;
            pFoil->m_y[i] = v[i].y;
        }
        // emit onSetFoilCoords(pFoil);
        emit onUpdate();
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

    server.bind("setCurFoil", [&](string name, bool select){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        Objects2d::setCurFoil(pFoil);
        if (select) emit onSelectFoil(pFoil);
        
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
        emit onNormalizeFoil(); // Operates on current foil
    });

    server.bind("derotateFoil", [&](string name){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        emit onSelectFoil(pFoil);
        emit onDerotateFoil(); // Operates on current foil
    });

    server.bind("getLineStyle", [&](string name) -> RpcLibAdapters::LineStyleAdapter{
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        return RpcLibAdapters::LineStyleAdapter(pFoil->theStyle());
    });

    server.bind("setLineStyle", [&](string name, RpcLibAdapters::LineStyleAdapter& lineStyle){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(name));
        LineStyle ls = RpcLibAdapters::LineStyleAdapter::from_msgpack(lineStyle);
        
       emit onFoilStyle(pFoil, ls);

    });

    server.bind("exportFoil", [&](string foilName, string fileName){
        Foil* pFoil = Objects2d::foil(QString::fromStdString(foilName));        
        emit onExportFoil(pFoil, QString::fromStdString(fileName));
    });

    // ===================== XDirect ====================== //
    QObject::connect(this, &xflServer::onAnalyzeCurPolar, s_pMainFrame->m_pXDirect, &XDirect::onAnalyze, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onDefinePolar, s_pMainFrame->m_pXDirect, &XDirect::onDefinePolarHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSetAnalysisSettings2D, s_pMainFrame->m_pXDirect, &XDirect::onSetAnalysisSettings2DHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSelectPolar, s_pMainFrame->m_pXDirect, &XDirect::onSelectPolarHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSetXDirectDisplay, s_pMainFrame->m_pXDirect, &XDirect::onSetDisplayHeadless, Qt::BlockingQueuedConnection);

    server.bind("defineAnalysis2D", [&](RpcLibAdapters::PolarAdapter polar){
        // creates a new polar on the heap everytime. use carefully
        Foil* pFoil = Objects2d::foil(QString::fromStdString(polar.foil_name));
        Polar* pPolar = RpcLibAdapters::PolarAdapter::from_msgpack(polar); 
        
        Objects2d::setCurFoil(pFoil);   // need to set these for posterity
        
        Objects2d::setCurPolar(pPolar);
        emit onDefinePolar(pPolar, pFoil);
    });

    server.bind("analyzeCurPolar", [&](RpcLibAdapters::AnalysisSettings2D analysis_settings, vector<RpcLibAdapters::PolarResultAdapter::enumPolarResult> result_list){
        emit onSetAnalysisSettings2D(&analysis_settings);
        emit onAnalyzeCurPolar();
        return RpcLibAdapters::PolarResultAdapter(*Objects2d::curPolar(), result_list);
    });

    server.bind("setCurPolar", [&](string polar_name, string foil_name, bool select = false){
        Polar* pPolar = Objects2d::getPolar(QString::fromStdString(foil_name), QString::fromStdString(polar_name));
        Objects2d::setCurPolar(pPolar);
        if (select) emit onSelectPolar(pPolar);
    });

    server.bind("getPolar", [&](string foil_name, string polar_name){
        Polar* pPolar = Objects2d::getPolar(QString::fromStdString(foil_name), QString::fromStdString(polar_name));
        return RpcLibAdapters::PolarAdapter(*pPolar); //argument is a const reference
    });

    server.bind("setXDirectDisplay", [&](RpcLibAdapters::XDirectDisplayState dsp_state){
        emit onSetXDirectDisplay(&dsp_state); // need to send pointers when dealing with custom rpc adapters
    });

    server.bind("getXDirectDisplay", [&](){
        const XDirect* xdirect_ref = s_pMainFrame->m_pXDirect;
        return RpcLibAdapters::XDirectDisplayState(*xdirect_ref);
    });

    server.bind("polarList", [&](string foil_name){
        return  PolarVecFromQPolarQVec(*Objects2d::pOAPolar(), QString::fromStdString(foil_name));
    });

    server.bind("getOpPoint", [&](double alpha, string polar_name, string foil_name){
        Foil* pFoil;  // preassigned here to create memory on stack. As it might point to null after below functions
        Polar* pPolar;

        pFoil = Objects2d::foil(QString::fromStdString(foil_name));
        if (pFoil==nullptr)  pFoil = Objects2d::curFoil();

        pPolar = Objects2d::getPolar(pFoil->name(), QString::fromStdString(polar_name));
        if (pPolar==nullptr) pPolar = Objects2d::curPolar();  
        
        OpPoint* pOpPoint = Objects2d::getOpp(pFoil, pPolar, alpha);
        if (!pOpPoint) OpPoint* pOpPoint = Objects2d::curOpp();
        
        return  RpcLibAdapters::OpPointAdapter(*pOpPoint);
    });

    // ===================== Miarex ====================== //
    QObject::connect(this, &xflServer::onNewPlane, s_pMainFrame->m_pMiarex, &Miarex::onNewPlaneHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onDefineWPolar, s_pMainFrame->m_pMiarex, &Miarex::onDefineWPolarHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSetWPolar, s_pMainFrame->m_pMiarex, QOverload<WPolar*>::of(&Miarex::setWPolar), Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSetPlane, s_pMainFrame->m_pMiarex, QOverload<Plane*>::of(&Miarex::setPlane), Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onSetAnalysisSettings3D, s_pMainFrame->m_pMiarex, &Miarex::setAnalysisParamsHeadless, Qt::BlockingQueuedConnection);
    QObject::connect(this, &xflServer::onAnalyzeCurWPolar, s_pMainFrame->m_pMiarex, &Miarex::onAnalyze, Qt::BlockingQueuedConnection);

    server.bind("getPlane", [&](string name){
        Plane* pPlane = Objects3d::plane(QString::fromStdString(name));
            
        if (pPlane!=nullptr) {   // if the current foil is not the default splinefoil
            return RpcLibAdapters::PlaneAdapter(*pPlane);
        }
        else {
            // return garbage plane if it's the spline foil
            // do further checks in python because msgpack needs to this adapter as a return type at all costs 
            return RpcLibAdapters::PlaneAdapter(); 
        }
    });

    server.bind("addDefaultPlane", [&](string name){
        Plane* pPlane = new Plane();
        pPlane->setName(QString::fromStdString(name));
        
        emit onNewPlane(pPlane);
        return RpcLibAdapters::PlaneAdapter(*Objects3d::addPlane(pPlane));

    });

    server.bind("addPlane", [&](RpcLibAdapters::PlaneAdapter plane){
        Plane* pPlane = RpcLibAdapters::PlaneAdapter::from_msgpack(plane);
        emit onNewPlane(pPlane);
        return RpcLibAdapters::PlaneAdapter(*Objects3d::addPlane(pPlane));
    });

    server.bind("getPlaneData", [&](string name){
        return Objects3d::plane(QString::fromStdString(name))->planeData(false).toStdString();
    });

    server.bind("defineAnalysis3D", [&](RpcLibAdapters::WPolarAdapter wpolar){
        Plane* pPlane = Objects3d::plane(QString::fromStdString(wpolar.plane_name));
        WPolar* pWPolar = RpcLibAdapters::WPolarAdapter::from_msgpack(wpolar); 
        std::cout<<pWPolar<<std::endl;
        emit onDefineWPolar(pWPolar, pPlane);
    });

    server.bind("analyzeWPolar", [&](string polar_name, string plane_name, RpcLibAdapters::AnalysisSettings3D analysis_settings, vector<RpcLibAdapters::WPolarResult::enumWPolarResult> result_list){
        emit onSetAnalysisSettings3D(analysis_settings);

        Plane* pPlane = Objects3d::plane(QString::fromStdString(plane_name));
        WPolar* pPolar = Objects3d::getWPolar(pPlane, QString::fromStdString(polar_name));
        emit onSetPlane(pPlane);
        emit onSetWPolar(pPolar);
        emit onAnalyzeCurWPolar();

        // this spy is needed to make sure that the analysis is completed before results are returned.
        QSignalSpy spy(s_pMainFrame->m_pMiarex->m_pPanelAnalysisDlg, &PanelAnalysisDlg::analysisFinished);
        spy.wait(1000);
        
        return RpcLibAdapters::WPolarResult(*pPolar, result_list);
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

