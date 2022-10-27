/****************************************************************************
** Meta object code from reading C++ file 'QtPanel.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtPanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtPanel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x05,
      44,   33,    8,    8, 0x05,
      76,   71,    8,    8, 0x05,
     103,   71,    8,    8, 0x05,
     132,   71,    8,    8, 0x05,
     173,  162,    8,    8, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QtPanel[] = {
    "QtPanel\0\0xmlScheduleUpdatedSig()\0"
    "msg,status\0signalSendOPMsg(void*,int)\0"
    "data\0signalLayoutUpdated(void*)\0"
    "signalRTArrMsgUpdated(void*)\0"
    "signalTrainTimeUpdated(void*)\0playsource\0"
    "signalLiveSourceUpdated(int)\0"
};

void QtPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtPanel *_t = static_cast<QtPanel *>(_o);
        switch (_id) {
        case 0: _t->xmlScheduleUpdatedSig(); break;
        case 1: _t->signalSendOPMsg((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->signalLayoutUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 3: _t->signalRTArrMsgUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 4: _t->signalTrainTimeUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 5: _t->signalLiveSourceUpdated((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtPanel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtPanel::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtPanel,
      qt_meta_data_QtPanel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtPanel))
        return static_cast<void*>(const_cast< QtPanel*>(this));
    if (!strcmp(_clname, "Thread"))
        return static_cast< Thread*>(const_cast< QtPanel*>(this));
    return QObject::qt_metacast(_clname);
}

int QtPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtPanel::xmlScheduleUpdatedSig()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QtPanel::signalSendOPMsg(void * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtPanel::signalLayoutUpdated(void * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtPanel::signalRTArrMsgUpdated(void * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QtPanel::signalTrainTimeUpdated(void * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void QtPanel::signalLiveSourceUpdated(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
