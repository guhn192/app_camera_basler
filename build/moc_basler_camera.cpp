/****************************************************************************
** Meta object code from reading C++ file 'basler_camera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../app_camera_basler/basler_camera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'basler_camera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BaslerCamera_t {
    QByteArrayData data[12];
    char stringdata0[145];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BaslerCamera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BaslerCamera_t qt_meta_stringdata_BaslerCamera = {
    {
QT_MOC_LITERAL(0, 0, 12), // "BaslerCamera"
QT_MOC_LITERAL(1, 13, 12), // "imageUpdated"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 13), // "statusChanged"
QT_MOC_LITERAL(4, 41, 6), // "status"
QT_MOC_LITERAL(5, 48, 15), // "settingsChanged"
QT_MOC_LITERAL(6, 64, 16), // "frameRateUpdated"
QT_MOC_LITERAL(7, 81, 9), // "frameRate"
QT_MOC_LITERAL(8, 91, 14), // "frameIdUpdated"
QT_MOC_LITERAL(9, 106, 7), // "frameId"
QT_MOC_LITERAL(10, 114, 18), // "errorsCountUpdated"
QT_MOC_LITERAL(11, 133, 11) // "errorsCount"

    },
    "BaslerCamera\0imageUpdated\0\0statusChanged\0"
    "status\0settingsChanged\0frameRateUpdated\0"
    "frameRate\0frameIdUpdated\0frameId\0"
    "errorsCountUpdated\0errorsCount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BaslerCamera[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    1,   45,    2, 0x06 /* Public */,
       5,    0,   48,    2, 0x06 /* Public */,
       6,    1,   49,    2, 0x06 /* Public */,
       8,    1,   52,    2, 0x06 /* Public */,
      10,    1,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    7,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,   11,

       0        // eod
};

void BaslerCamera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BaslerCamera *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->imageUpdated(); break;
        case 1: _t->statusChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->settingsChanged(); break;
        case 3: _t->frameRateUpdated((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->frameIdUpdated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->errorsCountUpdated((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BaslerCamera::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::imageUpdated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BaslerCamera::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::statusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BaslerCamera::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::settingsChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (BaslerCamera::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::frameRateUpdated)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (BaslerCamera::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::frameIdUpdated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (BaslerCamera::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BaslerCamera::errorsCountUpdated)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject BaslerCamera::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_BaslerCamera.data,
    qt_meta_data_BaslerCamera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BaslerCamera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BaslerCamera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BaslerCamera.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BaslerCamera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void BaslerCamera::imageUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void BaslerCamera::statusChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BaslerCamera::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void BaslerCamera::frameRateUpdated(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void BaslerCamera::frameIdUpdated(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void BaslerCamera::errorsCountUpdated(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
