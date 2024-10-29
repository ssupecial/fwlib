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
        PyErr_SetString(PyExc_RuntimeError, "Failed to start FANUC process.");
        return -1;
    }
#endif

    ret = cnc_allclibhndl3(host, port, timeout, &self->libh);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_ConnectionError, "Failed to connect to CNC: %d", ret);
        return -1;
    }
    self->connected = 1;

    return 0;
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

// CNC 기계 ID 읽기 [cnc_rdcncid]
static PyObject* Context_read_id(Context* self, PyObject* Py_UNUSED(ignored)) {
    uint32_t cnc_ids[4] = {0};
    char cnc_id[40] = "";
    int ret;

    ret = cnc_rdcncid(self->libh, (unsigned long*) cnc_ids);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "Failed to read CNC ID: %d", ret);
        return NULL;
    }

    snprintf(cnc_id, sizeof(cnc_id), "%08x-%08x-%08x-%08x",
             cnc_ids[0], cnc_ids[1], cnc_ids[2], cnc_ids[3]);

    return PyUnicode_FromString(cnc_id);
}

/*
CNC 단일 Spindle 속도 읽기 [cnc_acts] 
https://www.inventcom.net/fanuc-focas-library/position/cnc_acts
*/
static PyObject* Context_acts(Context* self, PyObject* Py_UNUSED(ignored)) {
    ODBACT actualspeed;
    int ret;

    ret = cnc_acts(self->libh, &actualspeed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "Failed to read CNC acts: %d", ret);
        return NULL;
    }

    return PyLong_FromLong(actualspeed.data);
}

/*
CNC 여러 Spindle 속도 읽기 [cnc_acts2]
https://www.inventcom.net/fanuc-focas-library/position/cnc_acts2
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
        PyErr_Format(PyExc_RuntimeError, "Failed to read CNC acts2: %d", ret);
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
CNC Axis Feedrate 읽기 [cnc_actf]
https://www.inventcom.net/fanuc-focas-library/position/cnc_actf
*/
static PyObject* Context_actf(Context* self, PyObject* Py_UNUSED(ignored)) {
    ODBACT actualfeed;
    int ret;

    ret = cnc_actf(self->libh, &actualfeed);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "Failed to read CNC actf: %d", ret);
        return NULL;
    }

    return PyLong_FromLong(actualfeed.data);
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

static PyMethodDef Context_methods[] = {
    {"read_id", (PyCFunction) Context_read_id, METH_NOARGS, "Reads the CNC ID."},
    {"acts", (PyCFunction) Context_acts, METH_NOARGS, "Reads the actual spindle speed."},
    {"acts2", (PyCFunction) Context_acts2, METH_VARARGS, "Reads actual speeds for multiple spindles."},
    {"actf", (PyCFunction) Context_actf, METH_NOARGS, "Reads the actual feed rate."},
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

