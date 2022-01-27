#pragma once
#include <xflobjects/objects2d/foil.h>
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

}; // namespace adapters
