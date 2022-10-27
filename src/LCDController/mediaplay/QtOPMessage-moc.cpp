/****************************************************************************
** Meta object code from reading C++ file 'QtOPMessage.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtOPMessage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtOPMessage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtOPMessage[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      43,   13,   12,   12, 0x05,
      97,   82,   12,   12, 0x25,
     142,  132,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     185,   12,   12,   12, 0x0a,
     205,  199,   12,   12, 0x0a,
     232,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtOPMessage[] = {
    "QtOPMessage\0\0bstart,currOps,display_region\0"
    "signalOPMsgPlay(bool,OPSMsgParam*,int)\0"
    "bstart,currOps\0signalOPMsgPlay(bool,OPSMsgParam*)\0"
    "id,status\0signalOPMsgPlayReply(int,OPSMsgPlayStatus)\0"
    "slotTimeout()\0event\0resizeEvent(QResizeEvent*)\0"
    "invalidate()\0"
};

void QtOPMessage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtOPMessage *_t = static_cast<QtOPMessage *>(_o);
        switch (_id) {
        case 0: _t->signalOPMsgPlay((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< OPSMsgParam*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->signalOPMsgPlay((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< OPSMsgParam*(*)>(_a[2]))); break;
        case 2: _t->signalOPMsgPlayReply((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const OPSMsgPlayStatus(*)>(_a[2]))); break;
        case 3: _t->slotTimeout(); break;
        case 4: _t->resizeEvent((*reinterpret_cast< QResizeEvent*(*)>(_a[1]))); break;
        case 5: _t->invalidate(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtOPMessage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtOPMessage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtOPMessage,
      qt_meta_data_QtOPMessage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtOPMessage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtOPMessage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtOPMessage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtOPMessage))
        return static_cast<void*>(const_cast< QtOPMessage*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtOPMessage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void QtOPMessage::signalOPMsgPlay(bool _t1, OPSMsgParam * _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 2
void QtOPMessage::signalOPMsgPlayReply(const int _t1, const OPSMsgPlayStatus _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
