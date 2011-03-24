/****************************************************************************
** Meta object code from reading C++ file 'MazeWidget.h'
**
** Created: Wed Mar 23 15:27:49 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/MazeWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MazeWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MazeWidget3D[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      37,   28,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      65,   60,   13,   13, 0x0a,
     101,   93,   13,   13, 0x0a,
     129,   28,  124,   13, 0x0a,
     143,   13,   13,   13, 0x0a,
     170,   13,   13,   13, 0x0a,
     198,   13,   13,   13, 0x0a,
     229,  227,   13,   13, 0x0a,
     257,  227,   13,   13, 0x0a,
     287,   13,   13,   13, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MazeWidget3D[] = {
    "MazeWidget3D\0\0zoomChanged()\0filename\0"
    "completedMaze(QString)\0name\0"
    "setParticipantName(QString)\0enabled\0"
    "setParticipating(bool)\0bool\0open(QString)\0"
    "slot_SwitchToEditingMode()\0"
    "slot_SwitchToOverviewMode()\0"
    "slot_SwitchToMouselookMode()\0x\0"
    "slot_SetVerticalOffset(int)\0"
    "slot_SetHorizontalOffset(int)\0"
    "slot_Advance()\0"
};

const QMetaObject MazeWidget3D::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_MazeWidget3D,
      qt_meta_data_MazeWidget3D, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MazeWidget3D::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MazeWidget3D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MazeWidget3D::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MazeWidget3D))
        return static_cast<void*>(const_cast< MazeWidget3D*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int MazeWidget3D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: zoomChanged(); break;
        case 1: completedMaze((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: setParticipantName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: setParticipating((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: { bool _r = open((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 5: slot_SwitchToEditingMode(); break;
        case 6: slot_SwitchToOverviewMode(); break;
        case 7: slot_SwitchToMouselookMode(); break;
        case 8: slot_SetVerticalOffset((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: slot_SetHorizontalOffset((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: slot_Advance(); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MazeWidget3D::zoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MazeWidget3D::completedMaze(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
