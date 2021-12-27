
TEMPLATE = subdirs


SUBDIRS = xflr5-engine \
    XFoil-lib \
    xflr5-gui \
    xflr-server\

TRANSLATIONS = translations/xflr5v6.ts \
    translations/xflr5v6_fr.ts \
    translations/xflr5v6_de.ts \
    translations/xflr5v6_ja.ts


OTHER_FILES += License.txt \
    doc/ReleaseNotes.txt \
    qss/appwidget.css \
    qss/xflr5_style.qss \
    qss/default.qss
 
