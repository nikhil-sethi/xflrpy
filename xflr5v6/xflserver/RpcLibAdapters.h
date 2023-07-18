#pragma once

#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects2d/oppoint.h>
#include <xdirect/xdirect.h>
#include <xflcore/linestyle.h>
#include "rpc/msgpack.hpp"
// class XDirect;
#include <iostream>

namespace RpcLibAdapters
{   
    // public:
        struct StateAdapter{
            std::string projectPath;
            std::string projectName;
            int app;
            bool saved;
            bool display;
            MSGPACK_DEFINE_MAP(projectPath, projectName, app, saved, display);

            StateAdapter(QString _projectPath, QString _projectName, xfl::enumApp _app, bool _saved, bool _display=true){
                projectPath = _projectPath.toStdString();
                projectName = _projectName.toStdString();
                app = _app;
                saved = _saved;
                display=_display;

            }
        };

        struct FoilAdapter{
            std::string name;
            double camber;
            double camber_x;
            double thickness;
            double thickness_x;
            int n;                               /**<  the number of points of the current foil */
            // double x[IBX];                       /**< the array of x-coordinates of the current foil points */
            // double y[IBX];                       /**< the array of y-coordinates of the current foil points*/
            MSGPACK_DEFINE_MAP(name, camber, camber_x, thickness, thickness_x, n);

            FoilAdapter(){}
            FoilAdapter(Foil& out){
                // if (&out==nullptr) return; 
                name = out.name().toStdString();
                camber = out.camber();
                camber_x = out.xCamber();
                thickness = out.thickness();
                thickness_x = out.xThickness();
                n = out.m_n;            
                }
        
        };

        struct Coord{
            double x,y;
            MSGPACK_DEFINE_ARRAY(x,y);
            
            std::vector<double> toVec(){
                return std::vector<double>{x,y}; 
            }
        };

        struct QColorAdapter{
            int red;
            int green;
            int blue;
            int alpha = 255;
            MSGPACK_DEFINE_ARRAY(red, green, blue, alpha);

            QColorAdapter(){}

            QColorAdapter(QColor& color){
                red = color.red();
                green = color.green();
                blue = color.blue();
                alpha = color.alpha();
            }

            static QColor from_msgpack(QColorAdapter& in){
                return QColor(in.red, in.green, in.blue, in.alpha);
            }
        };

        struct LineStyleAdapter{
            bool visible;
            int stipple;
            int point_style;
            int width;
            QColorAdapter color;
            std::string tag;

            MSGPACK_DEFINE_MAP(visible, stipple, point_style, width, color, tag);

            LineStyleAdapter(){}

            LineStyleAdapter(LineStyle& out){
                visible = out.m_bIsVisible;
                stipple = out.m_Stipple;
                point_style = out.m_Symbol;
                width = out.m_Width;
                color = QColorAdapter(out.m_Color);
                tag = (out.m_Tag).toStdString();
            }

            static LineStyle from_msgpack(LineStyleAdapter& in){
                // just convert stuff from client part to xflr/Qt acceptable values
                Line::enumLineStipple m_Stipple = LineStyle::convertLineStyle(in.stipple);
                Line::enumPointStyle m_Symbol = LineStyle::convertSymbol(in.point_style);
                QColor m_Color = QColorAdapter::from_msgpack(in.color);
                QString m_Tag = QString::fromStdString(in.tag);
                return LineStyle(in.visible, m_Stipple, in.width, m_Color, m_Symbol, m_Tag);
            }
        };

        struct PolarSpecAdapter{
            int polar_type;
            int re_type;
            int ma_type;
            double aoa;
            double mach;
            double ncrit;
            double xtop;
            double xbot;
            double reynolds;

            MSGPACK_DEFINE_MAP(polar_type, re_type, ma_type, aoa, mach, ncrit, xtop, xbot, reynolds);

            PolarSpecAdapter(){}
            PolarSpecAdapter(const Polar& out){
                polar_type = out.m_PolarType;
                re_type = out.m_ReType;
                ma_type = out.m_MaType;
                aoa =     out.m_ASpec;
                mach =    out.m_Mach;
                ncrit =   out.m_NCrit;
                xtop =    out.m_XTop;
                xbot =    out.m_XBot;
                reynolds = out.m_Reynolds;
            }
            
        };

        struct PolarResultAdapter{
            std::vector<double> alpha;
            std::vector<double> Cl;
            std::vector<double> XCp;
            std::vector<double> Cd;
            std::vector<double> Cdp;
            std::vector<double> Cm;
            std::vector<double> XTr1;
            std::vector<double> XTr2;
            std::vector<double> HMom;
            std::vector<double> Cpmn;
            std::vector<double> ClCd;
            std::vector<double> Cl32Cd;
            std::vector<double> RtCl;
            std::vector<double> Re;
            enum enumPolarResult{ALPHA, CL, XCP, CD, CDP, CM, XTR1, XTR2, HMOM, CPMN, CLCD, CL32CD, RTCL, RE};
            
            MSGPACK_DEFINE_MAP(alpha, Cl, XCp, Cd, Cdp, Cm, XTr1, XTr2, HMom, Cpmn, ClCd, Cl32Cd, RtCl, Re);

            PolarResultAdapter(){}
            PolarResultAdapter(const Polar& out, const vector<enumPolarResult>& result_list = vector<enumPolarResult>{}){
                if (result_list.size()==0) return;
                for (auto const& key: result_list){ //iterating once to avoid using find at every comparison
                    switch (key)
                    {
                    case ALPHA:
                        alpha = std::vector<double>(out.m_Alpha.begin(), out.m_Alpha.end());
                        break;
                    case CL:
                        Cl = std::vector<double>(out.m_Cl.begin(), out.m_Cl.end());
                        break;
                    case XCP:
                        XCp = std::vector<double>(out.m_XCp.begin(), out.m_XCp.end());
                        break;
                    case CD:
                        Cd = std::vector<double>(out.m_Cd.begin(), out.m_Cd.end());
                        break;
                    case CDP:
                        Cdp = std::vector<double>(out.m_Cdp.begin(), out.m_Cdp.end());
                        break;
                    case CM:
                        Cm = std::vector<double>(out.m_Cm.begin(), out.m_Cm.end());
                        break;
                    case XTR1:
                        XTr1 = std::vector<double>(out.m_XTr1.begin(), out.m_XTr1.end());
                        break;
                    case XTR2:
                        XTr2 = std::vector<double>(out.m_XTr2.begin(), out.m_XTr2.end());
                        break;
                    case HMOM:
                        HMom = std::vector<double>(out.m_HMom.begin(), out.m_HMom.end());
                        break;
                    case CPMN:
                        Cpmn = std::vector<double>(out.m_Cpmn.begin(), out.m_Cpmn.end());
                        break;
                    case CLCD:
                        ClCd = std::vector<double>(out.m_ClCd.begin(), out.m_ClCd.end());
                        break;
                    case CL32CD:
                        Cl32Cd = std::vector<double>(out.m_Cl32Cd.begin(), out.m_Cl32Cd.end());
                        break;
                    case RTCL:
                        RtCl = std::vector<double>(out.m_RtCl.begin(), out.m_RtCl.end());
                        break;
                    case RE:
                        Re = std::vector<double>(out.m_Re.begin(), out.m_Re.end());
                        break;
                    default:
                        break;
                    }
                }
            }
        };

        struct PolarAdapter{
            std::string name;
            std::string foil_name;
            PolarSpecAdapter spec;
            PolarResultAdapter result;

            MSGPACK_DEFINE_MAP(name, foil_name, spec, result);

            PolarAdapter(){}

            /**
             * Returns a client(python) usable Polar
             * @param out outgoing polar from server
             */
            PolarAdapter(const Polar& out){
                name = out.name().toStdString();
                foil_name = out.m_FoilName.toStdString();
                spec = PolarSpecAdapter(out);
                result = PolarResultAdapter(out);
            }

            /**
             * Returns a server(xflr) usable Polar
             * @param in Incoming polar from client
             */
            static Polar* from_msgpack(const PolarAdapter& in){ 
                Polar* pPolar = new Polar();
                pPolar->setName(QString::fromStdString(in.name));
                pPolar->setFoilName(QString::fromStdString(in.foil_name));
                pPolar->m_PolarType = xfl::enumPolarType(in.spec.polar_type);
                pPolar->m_ReType = in.spec.re_type; 
                pPolar->m_MaType = in.spec.ma_type; 
                pPolar->m_ASpec = in.spec.aoa; 
                pPolar->m_Mach = in.spec.mach; 
                pPolar->m_NCrit = in.spec.ncrit; 
                pPolar->m_XTop = in.spec.xtop; 
                pPolar->m_XBot = in.spec.xbot; 
                pPolar->m_Reynolds = in.spec.reynolds; 

                return pPolar;
            }

        };

        struct SequenceAdapter{
            double start;
            double end;
            double delta;
            MSGPACK_DEFINE_ARRAY(start, end, delta);
        };

        struct AnalysisSettings2D{
            int sequence_type;
            SequenceAdapter sequence;
            bool is_sequence;
            bool init_BL;
            bool store_opp;
            bool viscous;
            bool keep_open_on_error;

            MSGPACK_DEFINE_MAP(sequence_type, sequence, is_sequence, init_BL, store_opp, viscous, keep_open_on_error);

        };

        struct XDirectDisplayState{
            bool polar_view; // m_bPolarView
            // Polar View
            xfl::enumGraphView graph_view; // m_iPlrView
            int which_graph;

            //OpPoint View
            bool active_opp_only;
            bool show_bl;
            bool show_pressure;
            bool show_cpgraph; 
            bool animated;
            int ani_speed;
            
            MSGPACK_DEFINE_MAP(polar_view, graph_view, which_graph, active_opp_only, show_bl, show_pressure, show_cpgraph, animated, ani_speed);
            XDirectDisplayState(){}
            XDirectDisplayState(const XDirect& out){
                polar_view = out.bPolarView();
                graph_view = out.iPlrView();
                which_graph = out.iPlrGraph();
                active_opp_only = out.bActiveOppOnly();
                show_bl = out.bShowBL();
                show_pressure = out.bShowPressure();
                show_cpgraph = out.bCpGraph();
                animated = out.bAnimate();
                ani_speed = out.slAnimateSpeed();
            }
        };

        struct OpPointAdapter{
            string polar_name;
            string foil_name;
            double alpha;
            double Cl;
            double XCp;
            double Cd;
            double Cdp;
            double Cm;
            double XTr1;
            double XTr2;
            double HMom;
            double Cpmn;
            double Re;
            double mach;

            MSGPACK_DEFINE_MAP(polar_name, foil_name, alpha, Cl, XCp, Cd, Cdp, Cm, XTr1, XTr2, HMom, Cpmn, Re, mach);

            OpPointAdapter(){}

            OpPointAdapter(const OpPoint& out){
                polar_name = out.polarName().toStdString();
                foil_name = out.foilName().toStdString();
                alpha = out.m_Alpha;
                Cl = out.Cl;
                XCp = out.m_XCP;
                Cd = out.Cd;
                Cdp = out.Cdp;
                Cm = out.Cm;
                XTr1 = out.Xtr1;
                XTr2 = out.Xtr2;
                HMom = out.m_TEHMom;  // LEHMom is not really used 
                Cpmn = out.Cpmn;
                Re = out.m_Reynolds;
                mach = out.m_Mach;
            }

        };

        struct WingSectionAdapter{
            double y_position;
            double chord;
            double offset;
            double dihedral;
            double twist;        
            std::string right_foil_name;
            std::string left_foil_name;
            int n_x_panels;
            xfl::enumPanelDistribution x_panel_dist;
            int n_y_panels;
            xfl::enumPanelDistribution y_panel_dist;

            MSGPACK_DEFINE_MAP(y_position, chord, offset, dihedral, twist, right_foil_name, left_foil_name, n_x_panels, x_panel_dist, n_y_panels, y_panel_dist);

            WingSectionAdapter(){}
            WingSectionAdapter(const WingSection& out){
                chord = out.m_Chord;
                y_position = out.m_YPosition;
                offset = out.m_Offset;
                dihedral = out.m_Dihedral;
                twist = out.m_Twist;
                right_foil_name = out.m_RightFoilName.toStdString();
                left_foil_name = out.m_LeftFoilName.toStdString();
                n_x_panels = out.m_NXPanels;
                x_panel_dist = out.m_XPanelDist;
                n_y_panels = out.m_NYPanels;
                y_panel_dist = out.m_YPanelDist;
            }

            static WingSection* from_msgpack(const WingSectionAdapter& in){
                WingSection* section = new WingSection();
                section->m_Chord = in.chord;
                section->m_YPosition = in.y_position;
                section->m_Offset = in.offset;
                section->m_Dihedral = in.dihedral;
                section->m_Twist = in.twist;
                section->m_RightFoilName = QString::fromStdString(in.right_foil_name);
                section->m_LeftFoilName = QString::fromStdString(in.left_foil_name);
                section->m_NXPanels = in.n_x_panels;
                section->m_XPanelDist = in.x_panel_dist;
                section->m_NYPanels = in.n_y_panels;
                section->m_YPanelDist = in.y_panel_dist;

                return section;
            }

        };

        struct WingAdapter{
            xfl::enumWingType type;
            std::vector<WingSectionAdapter> sections;

            MSGPACK_DEFINE_MAP(type, sections);

            WingAdapter(){};
            WingAdapter(Wing& out){
                type = out.wingType();
                for (WingSection section: out.m_Section){
                    WingSectionAdapter section_adapter(section);
                    sections.push_back(section_adapter);
                }   
            }

            static Wing* from_msgpack(WingAdapter& in){
                Wing* wing = new Wing();
                wing->setWingType(in.type);
                for (int i; i<in.sections.size(); i++){
                    wing->setWingSection(i, *WingSectionAdapter::from_msgpack(in.sections[i]));
                }
                
                return wing;
            }

        };

        struct PlaneAdapter{
            string name;
            WingAdapter wing;
            WingAdapter wing2;
            WingAdapter elevator;
            WingAdapter fin;

            MSGPACK_DEFINE_MAP(name, wing, wing2, elevator, fin);

            PlaneAdapter(){};
            PlaneAdapter(Plane& out){
                name = out.name().toStdString();
                wing = WingAdapter(out.m_Wing[0]);
                wing2 = WingAdapter(out.m_Wing[1]);
                elevator = WingAdapter(out.m_Wing[2]);
                fin = WingAdapter(out.m_Wing[3]);
            }

            static Plane* from_msgpack(PlaneAdapter& in){
                Plane* plane = new Plane();
                plane->setName(QString::fromStdString(in.name));
                plane->m_Wing[0] = *WingAdapter::from_msgpack(in.wing);

                if (in.wing2.sections.size()==0) plane->setSecondWing(false);
                else plane->m_Wing[1] = *WingAdapter::from_msgpack(in.wing2);

                if (in.elevator.sections.size()==0) plane->setElevator(false);
                else plane->m_Wing[2] = *WingAdapter::from_msgpack(in.elevator);

                if (in.fin.sections.size()==0) plane->setFin(false);
                else plane->m_Wing[3] = *WingAdapter::from_msgpack(in.fin);

                return plane;
            }

        };


}; // namespace adapters

MSGPACK_ADD_ENUM(xfl::enumGraphView);
MSGPACK_ADD_ENUM(xfl::enumPanelDistribution);
MSGPACK_ADD_ENUM(xfl::enumWingType);
MSGPACK_ADD_ENUM(RpcLibAdapters::PolarResultAdapter::enumPolarResult);