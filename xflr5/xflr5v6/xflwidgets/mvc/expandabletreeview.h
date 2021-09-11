/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QTreeView>
#include <QAction>
#include <QLineEdit>

#include <xflcore/fontstruct.h>

class CrossCheckBox;

class ExpandableTreeView : public QTreeView
{
    Q_OBJECT

        friend class FoilTreeView;
        friend class PlaneTreeView;
        friend class BoatTreeView;

    public:
        ExpandableTreeView(QWidget *pParent = nullptr);

        void setOverallCheckedState(Qt::CheckState state);
        QWidget * cmdWidget() {return m_pfrControls;}

        int sizeHintForColumn(int column) const override;
        QSize sizeHint() const override;

        void enableSelectBox(bool bEnable);

        QString filter() const {return m_pleFilter->text();}

        static void setTreeFontStruct(FontStruct const &treefontstruct) {s_TreeFontStruct=treefontstruct;}
        static FontStruct const &treeFontStruct() {return s_TreeFontStruct;}

    private:
        void initETV();

    public slots:
        void onObjectLevel();
        void onPolarLevel();
        void onOpPointLevel();
        void onLevelMinus();
        void onLevelPlus();
//        void onCollapseAll();
        void onHideShowAll(bool bChecked);

    signals:
        void switchAll(bool);

    private:
        QLineEdit *m_pleFilter;

        QAction *m_pLevel0Action, *m_pLevel1Action, *m_pLevel2Action, *m_pLevelPlus, *m_pLevelMinus;
        QAction *m_pCollapseAll, *m_pExpandAll;
        CrossCheckBox *m_pchHideShowAll;

        QFrame *m_pfrControls;

        static FontStruct s_TreeFontStruct;
};

