#ifndef RPCLIBADAPTERS_H
#define RPCLIBADAPTERS_H
#include <objects/objects2d/foil.h>

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

            StateAdapter(QString _projectPath, QString _projectName, XFLR5::enumApp _app, bool _saved, bool _display=true){
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
                name = out.foilName().toStdString();
                camber = out.camber();
                camber_x = out.xCamber();
                thickness = out.thickness();
                thickness_x = out.thickness();
                n = out.n;            
                }

            // static Foil from(FoilAdapter& in){
            //     std::vector<Foil> foil; 
            //     for (const auto& item : in)
            //         foil.push_back(item.to());
            //     return foil;
            // }
        };


}; // namespace adapters

#endif
