#ifndef PLANETASKEVENT_H
#define PLANETASKEVENT_H


#include <QEvent>
#include <QString>


// Custom event identifier
const QEvent::Type PLANE_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 3);
const QEvent::Type PLANE_END_POPP_EVENT = static_cast<QEvent::Type>(QEvent::User + 4);


class PlaneTaskEvent : public QEvent
{

public:
    PlaneTaskEvent(void * pPlane, void *pWPolar): QEvent(PLANE_END_TASK_EVENT),
        m_pPlane(pPlane),
        m_pWPolar(pWPolar)
    {
    }

    void * planePtr() const    {return m_pPlane;}
    void * wPolarPtr() const    {return m_pWPolar;}

private:
    void *m_pPlane;
    void *m_pWPolar;
};



class PlanePOppEvent : public QEvent
{

public:
    PlanePOppEvent(): QEvent(PLANE_END_POPP_EVENT)    {
    }
};


#endif // PLANETASKEVENT_H
