/****************************************************************************
** Meta object code from reading C++ file 'QtSchedule.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtSchedule.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtSchedule.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtSchedule[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x05,
      49,   11,   11,   11, 0x05,
      96,   76,   11,   11, 0x05,
     160,   11,   11,   11, 0x05,
     186,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
     213,   12,   11,   11, 0x08,
     241,   11,   11,   11, 0x08,
     266,   76,   11,   11, 0x08,
     331,  328,   11,   11, 0x08,
     357,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSchedule[] = {
    "QtSchedule\0\0done\0signalQtMediaDone(QtMediaDone*)\0"
    "signalStartPlaynewLayout()\0"
    "pPartation,pContent\0"
    "signalPlayNextContent(Json::PartitionDetail*,Json::MediaBasic*)\0"
    "signalPauseStreamDetect()\0"
    "signalResumeStreamDetect()\0"
    "onQtMediaDone(QtMediaDone*)\0"
    "slotStartPlaynewLayout()\0"
    "slotPlayNextContent(Json::PartitionDetail*,Json::MediaBasic*)\0"
    "dc\0onProbeStreamReport(bool)\0"
    "slotLanguageSwitch()\0"
};

void QtSchedule::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSchedule *_t = static_cast<QtSchedule *>(_o);
        switch (_id) {
        case 0: _t->signalQtMediaDone((*reinterpret_cast< QtMediaDone*(*)>(_a[1]))); break;
        case 1: _t->signalStartPlaynewLayout(); break;
        case 2: _t->signalPlayNextContent((*reinterpret_cast< Json::PartitionDetail*(*)>(_a[1])),(*reinterpret_cast< Json::MediaBasic*(*)>(_a[2]))); break;
        case 3: _t->signalPauseStreamDetect(); break;
        case 4: _t->signalResumeStreamDetect(); break;
        case 5: _t->onQtMediaDone((*reinterpret_cast< QtMediaDone*(*)>(_a[1]))); break;
        case 6: _t->slotStartPlaynewLayout(); break;
        case 7: _t->slotPlayNextContent((*reinterpret_cast< Json::PartitionDetail*(*)>(_a[1])),(*reinterpret_cast< Json::MediaBasic*(*)>(_a[2]))); break;
        case 8: _t->onProbeStreamReport((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->slotLanguageSwitch(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSchedule::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSchedule::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtSchedule,
      qt_meta_data_QtSchedule, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSchedule::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSchedule::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSchedule::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSchedule))
        return static_cast<void*>(const_cast< QtSchedule*>(this));
    if (!strcmp(_clname, "Handler"))
        return static_cast< Handler*>(const_cast< QtSchedule*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtSchedule::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QtSchedule::signalQtMediaDone(QtMediaDone * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtSchedule::signalStartPlaynewLayout()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QtSchedule::signalPlayNextContent(Json::PartitionDetail * _t1, Json::MediaBasic * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtSchedule::signalPauseStreamDetect()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void QtSchedule::signalResumeStreamDetect()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE
