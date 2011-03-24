/****************************************************************************
** Meta object code from reading C++ file 'QtMaze.h'
**
** Created: Wed Mar 23 15:27:50 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/QtMaze.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtMaze.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtMaze[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x09,
      23,    7,    7,    7, 0x09,
      39,    7,    7,    7, 0x09,
      55,    7,    7,    7, 0x09,
      84,   73,    7,    7, 0x09,
     110,    7,    7,    7, 0x09,
     137,    7,    7,    7, 0x09,
     165,    7,    7,    7, 0x09,
     194,    7,    7,    7, 0x09,
     205,    7,    7,    7, 0x09,
     226,  217,    7,    7, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_QtMaze[] = {
    "QtMaze\0\0slot_FileNew()\0slot_FileOpen()\0"
    "slot_FileSave()\0slot_FileSaveAs()\0"
    "fullscreen\0slot_ViewFullscreen(bool)\0"
    "slot_SwitchToEditingMode()\0"
    "slot_SwitchToOverviewMode()\0"
    "slot_SwitchToMouselookMode()\0slot_Run()\0"
    "slot_Test()\0filename\0slot_MazeCompleted(QString)\0"
};

const QMetaObject QtMaze::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_QtMaze,
      qt_meta_data_QtMaze, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtMaze::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtMaze::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtMaze::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtMaze))
        return static_cast<void*>(const_cast< QtMaze*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int QtMaze::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slot_FileNew(); break;
        case 1: slot_FileOpen(); break;
        case 2: slot_FileSave(); break;
        case 3: slot_FileSaveAs(); break;
        case 4: slot_ViewFullscreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: slot_SwitchToEditingMode(); break;
        case 6: slot_SwitchToOverviewMode(); break;
        case 7: slot_SwitchToMouselookMode(); break;
        case 8: slot_Run(); break;
        case 9: slot_Test(); break;
        case 10: slot_MazeCompleted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
