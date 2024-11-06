#include <Python.h>
#include "fwlib32.h"
#include "gcode_mapp.h"

#define MACHINE_PORT_DEFAULT 8193
#define TIMEOUT_DEFAULT 10
#define MAX_AXIS 8

typedef struct {
    PyObject_HEAD
    unsigned short libh;
    int connected;
} Context;

struct aux_data {
    long aux_data;
    char flag1;
    char flag2;
};

#ifndef _WIN32
int cnc_startup() {
    return cnc_startupprocess(0, "focas.log");
}

void cnc_shutdown() {
    cnc_exitprocess();
}
#endif


static PyObject* parse_gdata(unsigned char g1shot_value);
static PyObject* create_aux_dict(void* aux_data_ptr);

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
        PyErr_Format(PyExc_ConnectionError, "FWLIB32[%d]", ret);
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
        return NULL;
    }

    PyObject* dict = PyDict_New();
    if (!dict) {
        return NULL;
    }

    if (PyDict_SetItemString(dict, "datano", PyLong_FromLong(actualspeed.datano)) < 0) {
        Py_DECREF(dict);
        return NULL;
    }

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

    if (PyDict_SetItemString(dict, "data", data_list) < 0) {
        Py_DECREF(dict);
        Py_DECREF(data_list);
        return NULL;
    }
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
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
    if (PyDict_SetItemString(feed_rate_dict, "data", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
    Py_DECREF(temp_obj);

    // Dec
    temp_obj = PyLong_FromLong(feed_rate.dec);
    if (!temp_obj) goto error;
    if (PyDict_SetItemString(feed_rate_dict, "dec", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
    Py_DECREF(temp_obj);

    // Unit
    temp_obj = PyLong_FromLong(feed_rate.unit);
    if (!temp_obj) goto error;
    if (PyDict_SetItemString(feed_rate_dict, "unit", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
    Py_DECREF(temp_obj);

    // Reserve
    temp_obj = PyLong_FromLong(feed_rate.reserve);
    if (!temp_obj) goto error;
    if (PyDict_SetItemString(feed_rate_dict, "reserve", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
    Py_DECREF(temp_obj);

    // Name
    temp_obj = PyUnicode_FromFormat("%c", feed_rate.name);
    if (!temp_obj) goto error;
    if (PyDict_SetItemString(feed_rate_dict, "name", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
    Py_DECREF(temp_obj);

    // Suffix
    temp_obj = PyUnicode_FromFormat("%c", feed_rate.suff);
    if (!temp_obj) goto error;
    if (PyDict_SetItemString(feed_rate_dict, "suff", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error;
    }
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
    if (PyDict_SetItemString(spindle_speed_dict, "data", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Dec
    temp_obj = PyLong_FromLong(spindle_speed.dec);
    if (!temp_obj) goto error_with_spindle;
    if(PyDict_SetItemString(spindle_speed_dict, "dec", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Unit
    temp_obj = PyLong_FromLong(spindle_speed.unit);
    if (!temp_obj) goto error_with_spindle;
    if (PyDict_SetItemString(spindle_speed_dict, "unit", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Reserve
    temp_obj = PyLong_FromLong(spindle_speed.reserve);
    if (!temp_obj) goto error_with_spindle;
    if (PyDict_SetItemString(spindle_speed_dict, "reserve", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Name
    temp_obj = PyUnicode_FromFormat("%c", spindle_speed.name);
    if (!temp_obj) goto error_with_spindle;
    if (PyDict_SetItemString(spindle_speed_dict, "name", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Suffix
    temp_obj = PyUnicode_FromFormat("%c", spindle_speed.suff);
    if (!temp_obj) goto error_with_spindle;
    if (PyDict_SetItemString(spindle_speed_dict, "suff", temp_obj) < 0) {
        Py_DECREF(temp_obj);
        goto error_with_spindle;
    }
    Py_DECREF(temp_obj);

    // Add dictionaries to main dict
    if (PyDict_SetItemString(dict, "feed_rate", feed_rate_dict) < 0) {
        goto error_with_spindle;
    }
    if (PyDict_SetItemString(dict, "spindle_speed", spindle_speed_dict) < 0) {
        goto error_with_spindle;
    }

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
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "hh", kwlist, &type, &block)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

    if (!(block >= 0 && block <= 2)) {
        PyErr_SetString(PyExc_ValueError, "Invalid block number, block number should be 0, 1, 2");
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
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
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
Read modal information [cnc_modal]
Returns the modal data of the CNC machine
The moddal data are G code or commanded data such as M,S,T,F
P.S this function cannot be used for Series 15i, so use cnc_rdgcode and cnc_rdcommand instead.
Reference: https://www.inventcom.net/fanuc-focas-library/misc/cnc_modal
*/
static PyObject* Context_modal(Context* self, PyObject* args, PyObject* kwds) {
    short type;
    short block;
    static char* kwlist[] = {"type", "block", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "hh", kwlist, &type, &block)) {
        PyErr_SetString(PyExc_TypeError, "Failed to read CNC modal: Invalid arguments");
        return NULL;
    }

    // Series 0i-D/F에서 지원하는 type 값 검증
    if (!((type >= 0 && type <= 20) ||      // Modal G code one by one
          (type >= 100 && type <= 126) ||   // Other than G code one by one
          (type >= 200 && type <= 207) ||   // Axis data one by one
          type == -4 ||                     // All 1 shot G code
          type == -3 ||                     // All axis data
          type == -2 ||                     // All other than G code
          type == -1 ||                     // All G code
          type == 300)) {                   // 1 shot G code one by one
        PyErr_SetString(PyExc_ValueError, "Invalid type value for Series 0i-D/F");
        return NULL;
    }

    // block 값 검증
    if (!((block == 0) ||                 // active block
          (block == 1) ||                 // next block
          (block == 2)                    // block after next block
    )) {
        PyErr_SetString(PyExc_ValueError, "Invalid block value for Series 0i-D/F");
        return NULL;
    }

    ODBMDL modal;
    int ret;

    ret = cnc_modal(self->libh, type, block, &modal);
    if (ret != EW_OK) {
        PyErr_Format(PyExc_RuntimeError, "FWLIB32[%d]", ret);
        return NULL;
    }

    PyObject* dict = PyDict_New();
    if (!dict) {
        return NULL;
    }

    // Add basic modal information
    PyObject* temp;
    temp = PyLong_FromLong(modal.datano);
    if (!temp) goto error;
    if (PyDict_SetItemString(dict, "datano", temp) < 0) goto error;
    Py_DECREF(temp);

    temp = PyLong_FromLong(modal.type);
    if (!temp) goto error;
    if (PyDict_SetItemString(dict, "type", temp) < 0) goto error;
    Py_DECREF(temp);

    // Process modal data based on type
    if (type >= 0 && type <= 20) {  // Modal G code one by one
        temp =  parse_gdata(type, (unsigned char)modal.modal.g_data);
        if (!temp) goto error;
        if (PyDict_SetItemString(dict, "g_data", temp) < 0) goto error;
        Py_DECREF(temp);
    }
    else if (type == -1) {  // All Modal G code data (0-20)
        PyObject* g_list = PyList_New(21);  // 0 to 20 = 21 items
        if (!g_list) goto error;
        for (int i = 0; i < 21; i++) {
            temp = parse_gdata(i, (unsigned char)modal.modal.g_rdata[i]);
            if (!temp) {
                Py_DECREF(g_list);
                goto error;
            }
            PyList_SET_ITEM(g_list, i, temp);  // PyList_SET_ITEM steals reference
        }
        if (PyDict_SetItemString(dict, "g_rdata", g_list) < 0) {
            Py_DECREF(g_list);
            goto error;
        }
        Py_DECREF(g_list);
    }
    else if (type == -4 || type == 300) {  // 1 shot G code
        if (type == 300) { // Single data
            temp = parse_gdata(type, (unsigned char)modal.modal.g_data);
            if (!temp) goto error;
            if (PyDict_SetItemString(dict, "g_data", temp) < 0) goto error;
            Py_DECREF(temp);
        } else { // All data
            PyObject* g_shot_list = PyList_New(4);
            if (!g_shot_list) goto error;
            for (int i = 0; i < 4; i++) {
                temp = parse_gdata(i, (unsigned char)modal.modal.g_1shot[i]);
                if (!temp) {
                    Py_DECREF(g_shot_list);
                    goto error;
                }
                PyList_SET_ITEM(g_shot_list, i, temp);
            }
            if (PyDict_SetItemString(dict, "g_1shot", g_shot_list) < 0) {
                Py_DECREF(g_shot_list);
                goto error;
            }
            Py_DECREF(g_shot_list);
        }
    }
    else if ((type >= 100 && type <= 126) || type == -2) {  // Other than G code
        PyObject* aux_data;
        if (type >= 100) {  // Single data
            aux_data = create_aux_dict(&modal.modal.aux);
        } else {  // All data
            aux_data = PyList_New(27);
            if (!aux_data) goto error;
            for (int i = 0; i < 27; i++) {
                temp = create_aux_dict(&modal.modal.raux1[i]);
                if (!temp) {
                    Py_DECREF(aux_data);
                    goto error;
                }
                PyList_SET_ITEM(aux_data, i, temp);
            }
        }
        if (!aux_data) goto error;
        if (PyDict_SetItemString(dict, type >= 100 ? "aux" : "raux1", aux_data) < 0) {
            Py_DECREF(aux_data);
            goto error;
        }
        Py_DECREF(aux_data);
    }
    else if (type == -3 || (type >= 200 && type <= 207)) {  // Axis data
        PyObject* axis_data;
        if (type >= 200) {  // Single axis data
            axis_data = create_aux_dict(&modal.modal.aux);
        } else {  // All axis data
            axis_data = PyList_New(MAX_AXIS);
            if (!axis_data) goto error;
            for (int i = 0; i < MAX_AXIS; i++) {
                temp = create_aux_dict(&modal.modal.raux2[i]);
                if (!temp) {
                    Py_DECREF(axis_data);
                    goto error;
                }
                PyList_SET_ITEM(axis_data, i, temp);
            }
        }
        if (!axis_data) goto error;
        if (PyDict_SetItemString(dict, type >= 200 ? "aux" : "raux2", axis_data) < 0) {
            Py_DECREF(axis_data);
            goto error;
        }
        Py_DECREF(axis_data);
    }

    return dict;

error:
    Py_XDECREF(dict);
    return NULL;
}

// Parse Gcode data (8 bit)
static PyObject* parse_gdata(int type, unsigned char g_data) {
    PyObject* dict = PyDict_New();
    if (!dict) return NULL;

    // G code number (bit 0-6)
    unsigned char g_code = g_data & 0x7F;  // 0x7F = 0111 1111
    PyObject* code = PyUnicode_FromString(mapGcode((type, g_code)));
    if (PyDict_SetItemString(dict, "code", code) < 0) {
        Py_DECREF(code);
        Py_DECREF(dict);
        return NULL;
    }
    Py_DECREF(code);

    // Command flag (bit 7)
    int is_commanded = (g_data & 0x80) >> 7;  // 0x80 = 1000 0000
    PyObject* commanded = PyBool_FromLong(is_commanded);
    if (PyDict_SetItemString(dict, "commanded", commanded) < 0) {
        Py_DECREF(commanded);
        Py_DECREF(dict);
        return NULL;
    }
    Py_DECREF(commanded);

    return dict;
}

// Helper function to create dictionary for aux data structure
static PyObject* create_aux_dict(void* aux_data_ptr) {
    struct aux_data* data = (struct aux_data*)aux_data_ptr;
    PyObject* aux_dict = PyDict_New();
    if (!aux_dict) return NULL;

    PyObject* temp;
    
    temp = PyLong_FromLong(data->aux_data);
    if (!temp) goto error;
    if (PyDict_SetItemString(aux_dict, "aux_data", temp) < 0) goto error;
    Py_DECREF(temp);

    temp = PyLong_FromSsize_t((unsigned char)data->flag1);
    if (!temp) goto error;
    if (PyDict_SetItemString(aux_dict, "flag1", temp) < 0) goto error;
    Py_DECREF(temp);

    temp = PyLong_FromSsize_t((unsigned char)data->flag2);
    if (!temp) goto error;
    if (PyDict_SetItemString(aux_dict, "flag2", temp) < 0) goto error;
    Py_DECREF(temp);

    return aux_dict;

error:
    Py_XDECREF(aux_dict);
    return NULL;
}

// Python Method Definition
static PyMethodDef Context_methods[] = {
    {"read_id", (PyCFunction) Context_read_id, METH_NOARGS, "Reads the CNC ID."},
    {"acts", (PyCFunction) Context_acts, METH_NOARGS, "Reads the actual spindle speed."},
    {"acts2", (PyCFunction) Context_acts2, METH_VARARGS, "Reads actual speeds for multiple spindles."},
    {"actf", (PyCFunction) Context_actf, METH_NOARGS, "Reads the actual feed rate."},
    {"rdspeed", (PyCFunction) Context_rdspeed, METH_VARARGS, "Reads the feed rate and spindle speed."},
    {"rdgcode", (PyCFunction) Context_rdgcode, METH_VARARGS | METH_KEYWORDS, "Reads the G code."},
    {"rdmodal", (PyCFunction) Context_modal, METH_VARARGS | METH_KEYWORDS, "Reads the modal information."},
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

