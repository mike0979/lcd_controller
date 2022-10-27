/****************************************************************************
** Meta object code from reading C++ file 'QtMediaDone.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtMediaDone.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtMediaDone.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtMediaDone[] = {

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
      13,   12,   12,   12, 0x0a,
      34,   27,   12,   12, 0x0a,
      54,   12,   12,   12, 0x2a,
      79,   70,   12,   12, 0x0a,
     114,  107,   12,   12, 0x0a,
     154,   12,   12,   12, 0x0a,
     173,   12,   12,   12, 0x0a,
     194,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtMediaDone[] = {
    "QtMediaDone\0\0slotTimeout()\0paused\0"
    "onQtMediaDone(bool)\0onQtMediaDone()\0"
    "position\0onQtPositionChanged(qint64)\0"
    "status\0onmediaStatusChanged(QtAV::MediaStatus)\0"
    "OnCheckVideoStop()\0slotsingletimeshot()\0"
    "setDefaultValue()\0"
};

void QtMediaDone::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtMediaDone *_t = static_cast<QtMediaDone *>(_o);
        switch (_id) {
        case 0: _t->slotTimeout(); break;
        case 1: _t->onQtMediaDone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onQtMediaDone(); break;
        case 3: _t->onQtPositionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 4: _t->onmediaStatusChanged((*reinterpret_cast< QtAV::MediaStatus(*)>(_a[1]))); break;
        case 5: _t->OnCheckVideoStop(); break;
        case 6: _t->slotsingletimeshot(); break;
        case 7: _t->setDefaultValue(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtMediaDone::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtMediaDone::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtMediaDone,
      qt_meta_data_QtMediaDone, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtMediaDone::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtMediaDone::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtMediaDone::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtMediaDone))
        return static_cast<void*>(const_cast< QtMediaDone*>(this));
    return QObject::qt_metacast(_clname);
}

int QtMediaDone::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
