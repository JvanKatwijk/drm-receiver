/****************************************************************************
** Meta object code from reading C++ file 'spyserver-client.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.17)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "devices/spy-server/spyserver-client.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'spyserver-client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.17. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_spyServer_client_t {
    QByteArrayData data[9];
    char stringdata0[112];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_spyServer_client_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_spyServer_client_t qt_meta_stringdata_spyServer_client = {
    {
QT_MOC_LITERAL(0, 0, 16), // "spyServer_client"
QT_MOC_LITERAL(1, 17, 13), // "setConnection"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11), // "wantConnect"
QT_MOC_LITERAL(4, 44, 7), // "setGain"
QT_MOC_LITERAL(5, 52, 15), // "handle_autogain"
QT_MOC_LITERAL(6, 68, 17), // "handle_checkTimer"
QT_MOC_LITERAL(7, 86, 14), // "set_portNumber"
QT_MOC_LITERAL(8, 101, 10) // "data_ready"

    },
    "spyServer_client\0setConnection\0\0"
    "wantConnect\0setGain\0handle_autogain\0"
    "handle_checkTimer\0set_portNumber\0"
    "data_ready"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_spyServer_client[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    1,   51,    2, 0x08 /* Private */,
       5,    1,   54,    2, 0x08 /* Private */,
       6,    0,   57,    2, 0x08 /* Private */,
       7,    1,   58,    2, 0x08 /* Private */,
       8,    0,   61,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void spyServer_client::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<spyServer_client *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setConnection(); break;
        case 1: _t->wantConnect(); break;
        case 2: _t->setGain((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->handle_autogain((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->handle_checkTimer(); break;
        case 5: _t->set_portNumber((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->data_ready(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject spyServer_client::staticMetaObject = { {
    QMetaObject::SuperData::link<deviceHandler::staticMetaObject>(),
    qt_meta_stringdata_spyServer_client.data,
    qt_meta_data_spyServer_client,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *spyServer_client::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *spyServer_client::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_spyServer_client.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_spyServer_widget_8"))
        return static_cast< Ui_spyServer_widget_8*>(this);
    return deviceHandler::qt_metacast(_clname);
}

int spyServer_client::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = deviceHandler::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
