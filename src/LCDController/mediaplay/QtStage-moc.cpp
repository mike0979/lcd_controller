/****************************************************************************
** Meta object code from reading C++ file 'QtStage.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtStage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtStage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtStage[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,    9,    8,    8, 0x08,
      77,   47,    8,    8, 0x08,
     122,  112,    8,    8, 0x08,
     168,  161,    8,    8, 0x08,
     193,    8,    8,    8, 0x08,
     220,  215,    8,    8, 0x08,
     249,  215,    8,    8, 0x08,
     286,  275,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtStage[] = {
    "QtStage\0\0msg,status\0onOPSMsgUpdated(void*,int)\0"
    "bstart,currOps,display_region\0"
    "onOPMsgPlay(bool,OPSMsgParam*,int)\0"
    "id,status\0onOPMsgPlayReply(int,OPSMsgPlayStatus)\0"
    "schpkg\0onScheduleUpdated(void*)\0"
    "onSchedulePlayReply()\0data\0"
    "onRTArrivalMsgUpdated(void*)\0"
    "onTrainTimeUpdated(void*)\0playsource\0"
    "onLiveSourceUpdated(int)\0"
};

void QtStage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtStage *_t = static_cast<QtStage *>(_o);
        switch (_id) {
        case 0: _t->onOPSMsgUpdated((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->onOPMsgPlay((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< OPSMsgParam*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->onOPMsgPlayReply((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const OPSMsgPlayStatus(*)>(_a[2]))); break;
        case 3: _t->onScheduleUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 4: _t->onSchedulePlayReply(); break;
        case 5: _t->onRTArrivalMsgUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 6: _t->onTrainTimeUpdated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 7: _t->onLiveSourceUpdated((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtStage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtStage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtStage,
      qt_meta_data_QtStage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtStage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtStage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtStage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtStage))
        return static_cast<void*>(const_cast< QtStage*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtStage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
