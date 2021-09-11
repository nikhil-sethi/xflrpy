/****************************************************************************

    xfl5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QTreeView>
#include <QToolBar>
#include <QSplitter>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>

#include <xflcore/core_enums.h>
#include <xflcore/linestyle.h>
#include <xflwidgets/customwts/plaintextoutput.h>

class MainFrame;
class XDirect;

class Foil;
class Polar;
class OpPoint;
class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;
class ObjectTreeItem;

class FoilTreeView : public QWidget
{
    Q_OBJECT

    public:
        FoilTreeView(QWidget *pParent = nullptr);
        ~FoilTreeView() override;

        void contextMenuEvent(QContextMenuEvent *event) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;

        void setupLayout();

        void setTreeFontStruct(FontStruct const &fntstruct);
        void setObjectFromIndex(const QModelIndex &filteredindex);

        void updateObjectView();
        void updateFoil(const Foil*pFoil);

        void insertPolar(Polar *pPolar);
        void insertFoil(Foil* pFoil);

        void fillModelView();
        void fillPolars(ObjectTreeItem *pFoilItem, Foil const*pFoil);
        void addOpps(Polar *pPolar);

        Qt::CheckState checkState(Foil *pFoil);
        Qt::CheckState checkState(Polar *pPolar);

        void removeOpPoint(OpPoint *pOpp);
        QString removePolar(Polar *pPolar);
        QString removeFoil(Foil* pFoil);
        QString removeFoil(QString foilName);

        void selectObjects();
        void setObjectProperties();

        void setCurveParams();
        void setPropertiesFont(QFont const &fnt) {m_pptoObjectProps->setFont(fnt);}

        void setOverallCheckStatus();

        QByteArray const &splitterSize() const {return m_SplitterSizes;}
        void setSplitterSize(QByteArray size) {m_SplitterSizes = size;}

        static void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}
        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}

    private:
        Qt::CheckState foilState(const Foil *pFoil) const;
        Qt::CheckState polarState(const Polar *pPolar) const;

    public slots:
        void onItemClicked(const QModelIndex &index);
        void onCurrentRowChanged(QModelIndex currentIndex, QModelIndex previousIndex);
        void onItemDoubleClicked(const QModelIndex &index);

        void selectFoil(Foil*pFoil);
        void selectPolar(Polar*pPolar);
        void selectOpPoint(OpPoint *pOpp=nullptr);

        void onSwitchAll(bool bChecked);
        void onSetFilter();

    protected:
        static MainFrame *s_pMainFrame;
        static XDirect *s_pXDirect;

        xfl::enumFoilSelectionType m_Selection;

        PlainTextOutput *m_pptoObjectProps;

    private:
        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QByteArray m_SplitterSizes;
        QSplitter *m_pMainSplitter;

};


