#include <Python.h>
#include "fwlib32.h"

#define MACHINE_PORT_DEFAULT 8193
#define TIMEOUT_DEFAULT 10

typedef struct {
    PyObject_HEAD
    unsigned short libh;
    int connected;
} Context;

#ifndef _WIN32
int cnc_startup() {
    return cnc_startupprocess(0, "focas.log");
}

void cnc_shutdown() {
    cnc_exitprocess();
}
#endif

static PyObject* Context_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Context* self;
    self = (Context*) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->libh = 0;
        self->connected = 0;
    }
    return (PyObject*) self;
}

static int Context_init(Context* self, PyObject* args, PyObject* kwds) {
    const char* host = "127.0.0.1";
    int port = MACHINE_PORT_DEFAULT;
    int timeout = TIMEOUT_DEFAULT;
    int ret;

    static char* kwlist[] = {"host", "port", "timeout", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sii", kwlist, &host, &port, &timeout)) {
        return -1;
    }

#ifndef _WIN32
    if (cnc_startup() != EW_OK) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot start FANUC process.");
        return -1;
    }
#endif

    ret = cnc_allclibhndl3(host, port, timeout, &self->libh);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_ConnectionError, "FWLIB32:%d", ret);
        return -1;
    }
    self->connected = 1;

    return 0;
}

static PyObject* Context_enter(PyObject* self) {
    Py_INCREF(self);
    return self;
}

static PyObject* Context_exit(Context* self, PyObject* exc_type, PyObject* exc_value, PyObject* traceback) {
    if (self->connected) {
        cnc_freelibhndl(self->libh);
        self->connected = 0;
    }

#ifndef _WIN32
    cnc_shutdown();
#endif

    Py_RETURN_NONE;
}

static void Context_dealloc(Context* self) {
    if (self->connected) {
        cnc_freelibhndl(self->libh);
        self->connected = 0;
    }

#ifndef _WIN32
    cnc_shutdown();
#endif

    Py_TYPE(self)->tp_free((PyObject*) self);
}

/* 
===============================================================================
Get data from the CNC machine 
===============================================================================
*/

/*
Read CNC Machine ID [cnc_rdcncid]
Returns the unique identifier of the CNC machine
Reference: https://www.inventcom.net/fanuc-focas-library/misc/cnc_rdcncid
*/
static PyObject* Context_read_id(Context* self, PyObject* Py_UNUSED(ignored)) {
    uint32_t cnc_ids[4] = {0};
    char cnc_id[40] = "";
    int ret;

    ret = cnc_rdcncid(self->libh, (unsigned long*) cnc_ids);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    snprintf(cnc_id, sizeof(cnc_id), "%08x-%08x-%08x-%08x",
             cnc_ids[0], cnc_ids[1], cnc_ids[2], cnc_ids[3]);

    return PyUnicode_FromString(cnc_id);
}

/*
Read Single Spindle Speed [cnc_acts]
Returns the actual speed of a single spindle
Reference: https://www.inventcom.net/fanuc-focas-library/position/cnc_acts
*/
static PyObject* Context_acts(Context* self, PyObject* Py_UNUSED(ignored)) {
    ODBACT actualspeed;
    int ret;

    ret = cnc_acts(self->libh, &actualspeed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    return PyLong_FromLong(actualspeed.data);
}

/*
Read Multiple Spindle Speeds [cnc_acts2]
Returns speeds for multiple spindles
Reference: https://www.inventcom.net/fanuc-focas-library/position/cnc_acts2
Parameters:
    sp_no: Spindle number (-1: all spindles)
Returns:
    Dictionary containing:
    - datano: Number of spindles
    - data: List of spindle speeds
*/
static PyObject* Context_acts2(Context* self, PyObject* args) {
    short sp_no;
    // 인자로 Spindle 번호를 받음 (-1: 모든 Spindle)
    if (!PyArg_ParseTuple(args, "h", &sp_no)) {
        return NULL;
    }


    ODBACT2 actualspeed;
    int ret;

    ret = cnc_acts2(self->libh, sp_no, &actualspeed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    PyObject* dict = PyDict_New();
    if (!dict) {
        return NULL;
    }

    PyDict_SetItemString(dict, "datano", PyLong_FromLong(actualspeed.datano));

    // 스핀들 데이터 추가
    PyObject* data_list = PyList_New(0);
    if (!data_list) {
        Py_DECREF(dict);
        return NULL;
    }
    
    long* data = actualspeed.data;
    for (int i = 0; i < actualspeed.datano; i++) {
        PyList_Append(data_list, PyLong_FromLong(data[i]));
    }

    PyDict_SetItemString(dict, "data", data_list);
    Py_DECREF(data_list);

    return dict;
}

/*
Read CNC Axis Feedrate [cnc_actf]
Returns the actual feedrate of the CNC machine axis
Reference: https://www.inventcom.net/fanuc-focas-library/position/cnc_actf
*/
static PyObject* Context_actf(Context* self, PyObject* Py_UNUSED(ignored)) {
    ODBACT actualfeed;
    int ret;

    ret = cnc_actf(self->libh, &actualfeed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    return PyLong_FromLong(actualfeed.data);
}

/*
Read CNC Feed Rate and Spindle Speed [cnc_rdspeed]
Parameters:
   type    : Data type to read
             0      : Feed rate only
             1      : Spindle speed only
             -1     : Both feed rate and spindle speed
Returns:
   Dictionary containing:
   - feed_rate      : Dictionary of feed rate data
     - data        : Actual feed rate value (raw)
     - dec         : Decimal point position
     - unit        : Unit type (0:mm/min, 1:inch/min, 2:rpm, 3:mm/rev, 4:inch/rev)
     - reserve     : Reserved value
     - name        : Data identifier ('F')
     - suff        : Suffix for identification
   - spindle_speed : Dictionary of spindle speed data
     - data        : Actual spindle speed value (raw)
     - dec         : Decimal point position
     - unit        : Unit type (2:rpm)
     - reserve     : Reserved value
     - name        : Data identifier ('S')
     - suff        : Spindle number in ASCII
Reference: https://www.inventcom.net/fanuc-focas-library/position/cnc_rdspeed
*/
static PyObject* Context_rdspeed(Context* self, PyObject* args) {
    short type = -1;
    if (!PyArg_ParseTuple(args, "h", &type)) {
        return NULL;
    }

    ODBSPEED speed;
    int ret;

    ret = cnc_rdspeed(self->libh, type, &speed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    SPEEDELM feed_rate = speed.actf;
    SPEEDELM spindle_speed = speed.acts;

    PyObject* dict = PyDict_New();
    if (!dict) {
        return NULL;
    }

    /*
    ------------------------------------------------
    Feed Rate
    ------------------------------------------------
    */
    // Create feed rate dictionary
    PyObject* feed_rate_dict = PyDict_New();
    if (!feed_rate_dict) {
        Py_DECREF(dict);
        return NULL;
    }
    // Add feed rate data
    PyObject* temp_obj;
    
    // Data
    temp_obj = PyLong_FromLong(feed_rate.data);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "data", temp_obj);
    Py_DECREF(temp_obj);

    // Dec
    temp_obj = PyLong_FromLong(feed_rate.dec);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "dec", temp_obj);
    Py_DECREF(temp_obj);

    // Unit
    temp_obj = PyLong_FromLong(feed_rate.unit);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "unit", temp_obj);
    Py_DECREF(temp_obj);

    // Reserve
    temp_obj = PyLong_FromLong(feed_rate.reserve);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "reserve", temp_obj);
    Py_DECREF(temp_obj);

    // Name
    temp_obj = PyUnicode_FromFormat("%c", feed_rate.name);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "name", temp_obj);
    Py_DECREF(temp_obj);

    // Suffix
    temp_obj = PyUnicode_FromFormat("%c", feed_rate.suff);
    if (!temp_obj) goto error;
    PyDict_SetItemString(feed_rate_dict, "suff", temp_obj);
    Py_DECREF(temp_obj);


    /*
    ------------------------------------------------
    Spindle Speed
    ------------------------------------------------
    */
    // Create spindle speed dictionary
    PyObject* spindle_speed_dict = PyDict_New();
    if (!spindle_speed_dict) {
        Py_DECREF(dict);
        Py_DECREF(feed_rate_dict);
        return NULL;
    }

    // Data
    temp_obj = PyLong_FromLong(spindle_speed.data);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "data", temp_obj);
    Py_DECREF(temp_obj);

    // Dec
    temp_obj = PyLong_FromLong(spindle_speed.dec);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "dec", temp_obj);
    Py_DECREF(temp_obj);

    // Unit
    temp_obj = PyLong_FromLong(spindle_speed.unit);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "unit", temp_obj);
    Py_DECREF(temp_obj);

    // Reserve
    temp_obj = PyLong_FromLong(spindle_speed.reserve);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "reserve", temp_obj);
    Py_DECREF(temp_obj);

    // Name
    temp_obj = PyUnicode_FromFormat("%c", spindle_speed.name);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "name", temp_obj);
    Py_DECREF(temp_obj);

    // Suffix
    temp_obj = PyUnicode_FromFormat("%c", spindle_speed.suff);
    if (!temp_obj) goto error_with_spindle;
    PyDict_SetItemString(spindle_speed_dict, "suff", temp_obj);
    Py_DECREF(temp_obj);

    // Add dictionaries to main dict
    PyDict_SetItemString(dict, "feed_rate", feed_rate_dict);
    PyDict_SetItemString(dict, "spindle_speed", spindle_speed_dict);

    Py_DECREF(feed_rate_dict);
    Py_DECREF(spindle_speed_dict);

    return dict;

error_with_spindle:
    Py_DECREF(spindle_speed_dict);
error:
    Py_DECREF(dict);
    Py_DECREF(feed_rate_dict);
    return NULL;
}

/*
Read G code [cnc_rdgcode]
Returns the G code data of the CNC machine
Reference: https://www.inventcom.net/fanuc-focas-library/Misc/cnc_rdgcode
*/
static PyObject* Context_rdgcode(Context* self, PyObject* args, PyObject* kwds) {
    short type;
    short block;
    static char* kwlist[] = {"type", "block", NULL};
    if (!(block >= 0 && block <= 2)) {
        PyErr_SetString(PyExc_ValueError, "Invalid block number, block number should be 0, 1, 2");
        return NULL;
    }
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "hh", kwlist, &type, &block)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

    ODBGCD* gcode;
    short num_gcd;

    if (type == -1 || type == -2) {
        // Allocate maximum size
        num_gcd = 50;
        gcode = (ODBGCD*) malloc(sizeof(ODBGCD) * num_gcd);
    } else {
        // Allocate single size
        num_gcd = 1;
        gcode = (ODBGCD*) malloc(sizeof(ODBGCD));
    }

    if (!gcode) {
        PyErr_SetString(PyExc_MemoryError, "Cannot allocate memory for gcode");
        return NULL;
    }


    int ret;
    ret = cnc_rdgcode(self->libh, type, block, &num_gcd, gcode);
    if (ret != EW_OK) {
        free(gcode);
        PyErr_Format(PyExc_RuntimeError, "FWLIB32:%d", ret);
        return NULL;
    }

    PyObject* return_list = PyList_New(num_gcd);
    if (!return_list) {
        free(gcode);
        return NULL;
    }

    for (int i = 0; i < num_gcd; i++) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }

        // Add gcode data
        PyObject* py_group = PyLong_FromLong(gcode[i].group);
        if (!py_group) {
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        if (PyDict_SetItemString(dict, "group", py_group) < 0) {
            Py_DECREF(py_group);
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        Py_DECREF(py_group);

        // Add flag
        PyObject* py_flag = PyLong_FromLong(gcode[i].flag);
        if (!py_flag) {
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        if (PyDict_SetItemString(dict, "flag", py_flag) < 0) {
            Py_DECREF(py_flag);
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        Py_DECREF(py_flag);

        // Add code
        PyObject* py_code = PyUnicode_FromString(gcode[i].code);
        if (!py_code) {
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        if (PyDict_SetItemString(dict, "code", py_code) < 0) {
            Py_DECREF(py_code);
            Py_DECREF(dict);
            Py_DECREF(return_list);
            free(gcode);
            return NULL;
        }
        Py_DECREF(py_code);

        PyList_SET_ITEM(return_list, i, dict);
    }

    free(gcode);
    return return_list;
}


/*
미완성
Read modal information [cnc_modal]
Returns the modal data of the CNC machine
The moddal data are G code or commanded data such as M,S,T,F
P.S this function cannot be used for Series 15i, so use cnc_rdgcode and cnc_rdcommand instead.
Reference: https://www.inventcom.net/fanuc-focas-library/misc/cnc_modal
*/
// static PyObject* Context_modal(Context* self, PyObject* args, PyObject* kwds) {
//     short type;
//     short block;
//     static char* kwlist[] = {"type", "block", NULL};
//     if (!PyArg_ParseTupleAndKeywords(args, kwds, "hh", kwlist, &type, &block)) {
//         PyErr_SetString(PyExc_TypeError, "Failed to read CNC modal: Invalid arguments");
//         return NULL;
//     }


//     ODBMDL modal;
//     int ret;

//     ret = cnc_modal(self->libh, type, block, &modal);
//     if (ret != EW_OK) {
//         PyErr_Format(PyExc_RuntimeError, "Failed to read CNC modal: %d", ret);
//         return NULL;
//     }

//     PyObject* dict = PyDict_New();
//     if (!dict) {
//         return NULL;
//     }

//     // Add modal data
//     PyObject* temp_obj;
//     for (int i = 0; i < modal.datano; i++) {
//         temp_obj = PyLong_FromLong(modal.data[i]);
//         if (!temp_obj) {
//             Py_DECREF(dict);
//             return NULL;
//         }
//         PyDict_SetItemString(dict, modal.name[i], temp_obj);
//     }

//     return dict;
// }

// Python Method Definition
static PyMethodDef Context_methods[] = {
    {"read_id", (PyCFunction) Context_read_id, METH_NOARGS, "Reads the CNC ID."},
    {"acts", (PyCFunction) Context_acts, METH_NOARGS, "Reads the actual spindle speed."},
    {"acts2", (PyCFunction) Context_acts2, METH_VARARGS, "Reads actual speeds for multiple spindles."},
    {"actf", (PyCFunction) Context_actf, METH_NOARGS, "Reads the actual feed rate."},
    {"rdspeed", (PyCFunction) Context_rdspeed, METH_VARARGS, "Reads the feed rate and spindle speed."},
    {"rdgcode", (PyCFunction) Context_rdgcode, METH_VARARGS | METH_KEYWORDS, "Reads the G code."},
    {"__enter__", (PyCFunction) Context_enter, METH_NOARGS, "Enter the context."},
    {"__exit__", (PyCFunction) Context_exit, METH_VARARGS, "Exit the context."},
    {NULL}  /* Sentinel */
};

static PyTypeObject ContextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "fwlib.Context",
    .tp_doc = "FANUC Context Manager",
    .tp_basicsize = sizeof(Context),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Context_new,
    .tp_init = (initproc) Context_init,
    .tp_dealloc = (destructor) Context_dealloc,
    .tp_methods = Context_methods,
};

static PyModuleDef fwlibmodule = {
    PyModuleDef_HEAD_INIT,
    "fwlib",
    "Python wrapper for FANUC fwlib32 library",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_fwlib(void) {
    PyObject* m;
    if (PyType_Ready(&ContextType) < 0)
        return NULL;

    m = PyModule_Create(&fwlibmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ContextType);
    if (PyModule_AddObject(m, "Context", (PyObject*) &ContextType) < 0) {
        Py_DECREF(&ContextType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

