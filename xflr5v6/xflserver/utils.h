#include <QString>
#include <QVector>
#include "RpcLibAdapters.h"


QVector<QString> QStrQVecFromStrVec(std::vector<std::string> list){
    QVector<QString> out;
    for (uint i=0;i<list.size();i++){
        out.push_back(QString::fromStdString(list.at(i)));
    };
    return out;
};

std::vector<RpcLibAdapters::FoilAdapter> FoilVecFromQFoilQVec(QVector<Foil*> &QVec){
    std::vector<RpcLibAdapters::FoilAdapter> out;
    for (int i=0;i<QVec.size();i++){
        out.push_back(RpcLibAdapters::FoilAdapter(*QVec.at(i)));
    };
    return out;
};

std::vector<RpcLibAdapters::PolarAdapter> PolarVecFromQPolarQVec(QVector<Polar*> &QVec, QString foil_name){
    std::vector<RpcLibAdapters::PolarAdapter> out;
    for (int i=0;i<QVec.size();i++){
        if (QVec.at(i)->foilName() == foil_name){
            out.push_back(RpcLibAdapters::PolarAdapter(*QVec.at(i)));
        }
    };
    return out;
};

// std::vector<std::vector<double>> 2DMatFromxy(std::vector<double> x, std::vector<double> y){
//     std::vector<std::vector<double>> out;
//     for (uint i=0; i<x.size(), i++){
//         for (uint i=0; i<y.size(), i++){
//             out[i][j] = {}
//     } 
//     return; 
// }
