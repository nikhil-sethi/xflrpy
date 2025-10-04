/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QModelIndex>
#include <QSplitter>

#include <xflcore/fontstruct.h>
#include <xflcore/core_enums.h>

class Plane;
class WPolar;
class PlaneOpp;
class ExpandableTreeView;
class ObjectTreeModel;
class ObjectTreeItem;
class ObjectTreeDelegate;
class PlainTextOutput;
class MainFrame;
class Miarex;

class PlaneTreeView : public QWidget
{
    Q_OBJECT
    friend class BatchModeDlg;

    public:
        PlaneTreeView(QWidget *pParent = nullptr);
        ~PlaneTreeView();

        void insertPlane(Plane *pPlane);
        void insertWPolar(const WPolar *pWPolar);

        QString removePlane(const QString &planeName);
        QString removePlane(Plane *pPlane);
        QString removeWPolar(WPolar *pWPolar);
        void removeWPolars(const Plane *pPlane);
        void removePlaneOpp(PlaneOpp *pPOpp);
        void removeWPolarPOpps(const WPolar *pWPolar);

        void selectPlane(Plane* pPlane=nullptr);
        void selectWPolar(WPolar *pWPolar, bool bSelectPOpp);
        void selectPlaneOpp(PlaneOpp *pPOpp=nullptr);

        void selectCurrentObject();
        void setObjectFromIndex(QModelIndex filteredindex);

        void addPOpps(const WPolar *pWPolar=nullptr);
        void fillModelView();
        void fillWPolars(ObjectTreeItem *pPlaneItem, const Plane *pPlane);
        void selectObjects();
        void setCurveParams();

        void updatePlane(Plane const *pPlane);

        QByteArray const &splitterSize() {return s_SplitterSizes;}
        void setSplitterSize(QByteArray size) {s_SplitterSizes = size;}

        void setObjectProperties();
//        void updateObjectView();
        void setPropertiesFont(QFont const &fnt);
        void setTreeFontStruct(const FontStruct &fntstruct);
        void fillEigenThings(QString &props);

        void setOverallCheckStatus();

        Qt::CheckState planeState(const Plane *pPlane) const;
        Qt::CheckState wPolarState(const WPolar *pWPolar) const;

        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}
        static void setMiarex(Miarex*pMiarex) {s_pMiarex = pMiarex;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;

    private slots:
        void onItemClicked(const QModelIndex &filteredindex);
        void onItemDoubleClicked(const QModelIndex &index);
        void onCurrentRowChanged(QModelIndex currentfilteredidx);
        void onSetFilter();
        void onSplitterMoved() {s_SplitterSizes = m_pMainSplitter->saveState();}

    public slots:
        void onSwitchAll(bool bChecked);

    private:
        void setupLayout();

    private:
        ExpandableTreeView *m_pTreeView;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QSplitter *m_pMainSplitter;

        xfl::enumPlaneSelectionType m_Selection;

        PlainTextOutput *m_pptObjectProps;

        static QByteArray s_SplitterSizes;
        static MainFrame *s_pMainFrame;
        static Miarex *s_pMiarex;
};

