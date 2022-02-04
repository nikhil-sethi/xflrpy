#pragma once
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/polar.h>
#include <xflcore/linestyle.h>

class RpcLibAdapters
{   
    public:
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
            std::string polar_name;
            int polar_type;
            int re_type;
            int ma_type;
            double aoa;
            double mach;
            double ncrit;
            double xtop;
            double xbot;
            double reynolds;

            MSGPACK_DEFINE_MAP(polar_name, polar_type, re_type, ma_type, aoa, mach, ncrit, xtop, xbot, reynolds);

            PolarSpecAdapter(){}
            PolarSpecAdapter(const Polar& out){
                polar_name = out.name().toStdString();     
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

            MSGPACK_DEFINE_MAP(alpha, Cl, XCp, Cd, Cdp, Cm, XTr1, XTr2, HMom, Cpmn, ClCd, Cl32Cd, RtCl, Re);

            PolarResultAdapter(){}
            PolarResultAdapter(const Polar& out){
                alpha = out.m_Alpha.toStdVector();
                Cl = out.m_Cl.toStdVector();
                XCp = out.m_XCp.toStdVector();
                Cd = out.m_Cd.toStdVector();
                Cdp = out.m_Cdp.toStdVector();
                Cm = out.m_Cm.toStdVector();
                XTr1 = out.m_XTr1.toStdVector();
                XTr2 = out.m_XTr2.toStdVector();
                HMom = out.m_HMom.toStdVector();
                Cpmn = out.m_Cpmn.toStdVector();
                ClCd = out.m_ClCd.toStdVector();
                Cl32Cd = out.m_Cl32Cd.toStdVector();
                RtCl = out.m_RtCl.toStdVector();
                Re = out.m_Re.toStdVector();
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

        struct AnalysisSettings2DAdapter{
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
            int graph_view; // m_iPlrView
            bool active_opp_only;
            bool show_bl;
            bool show_pressure;
            bool show cpgraph; 
            
            MSGPACK_DEFINE_MAP(polar_view, graph_view, active_opp_only, show_bl, show_pressure, show_cpgraph);

        };

}; // namespace adapters
