#include "Python.h"
#include "library.h"

static PyObject *run_hardnested_python(PyObject *self, PyObject *args) {
    uint64_t uid;
    char* path;
    if (!PyArg_ParseTuple(args, "ks", &uid, &path)) {
        return NULL;
    }

    char *output = run_hardnested(uid, path);

    return Py_BuildValue("s", output);
}

static PyMethodDef hardnested_methods[] = {{"run_hardnested",      run_hardnested_python,      METH_VARARGS, "Run hardnested"},
                                       {NULL, NULL,                                0, NULL}        /* Sentinel */
};

static struct PyModuleDef hardnested_module = {PyModuleDef_HEAD_INIT, "hardnested", NULL, -1, hardnested_methods};

PyMODINIT_FUNC PyInit_hardnested(void) {
    return PyModule_Create(&hardnested_module);
}
