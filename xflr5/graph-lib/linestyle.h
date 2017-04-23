#ifndef LINESTYLE_H
#define LINESTYLE_H

#include <QColor>
#include "graph-lib_global.h"

struct GRAPHLIBSHARED_EXPORT LineStyle
{
	bool m_bIsVisible;       /**< true if the curve is visible in the active view >*/
	int m_Style;             /**< the index of the style with which to draw the curve >*/
	int m_Width;             /**< the width with which to draw the curve >*/
	QColor m_Color;          /**< the color with which to draw the curve >*/

	int m_PointStyle;        /**< defines the point display. O = no points, 1 = small circles, 2 = large circles,3 = small squares, 4 = large squares >*/
};

#endif // LINESTYLE_H
