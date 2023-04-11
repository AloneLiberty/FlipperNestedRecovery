#include "Python.h"
#include "library.h"

static PyObject *run_nested_python(PyObject *self, PyObject *args) {
    uint64_t uid, nt0, ks0, nt1, ks1;
    if (!PyArg_ParseTuple(args, "kkkkk", &uid, &nt0, &ks0, &nt1, &ks1)) {
        return NULL;
    }

    char *output = run_nested((uint32_t) uid, (uint32_t) nt0, (uint32_t) ks0, (uint32_t) nt1, (uint32_t) ks1);

    return Py_BuildValue("s", output);
}

static PyObject *run_full_nested_python(PyObject *self, PyObject *args) {
    uint64_t uid, nt0, ks0, par0, nt1, ks1, par1;
    int from, to, progress;
    if (!PyArg_ParseTuple(args, "kkkkkkkiip", &uid, &nt0, &ks0, &par0, &nt1, &ks1, &par1, &from, &to, &progress)) {
        return NULL;
    }

    char *output = run_full_nested((uint32_t) uid, (uint32_t) nt0, (uint32_t) ks0, (uint32_t) par0, (uint32_t) nt1,
                                   (uint32_t) ks1, (uint32_t) par1, from, to, progress);

    return Py_BuildValue("s", output);
}

static PyMethodDef nested_methods[] = {{"run_nested",      run_nested_python,      METH_VARARGS, "Run nested"},
                                       {"run_full_nested", run_full_nested_python, METH_VARARGS, "Run full nested"},
                                       {NULL, NULL,                                0, NULL}        /* Sentinel */
};

static struct PyModuleDef nested_module = {PyModuleDef_HEAD_INIT, "nested", NULL, -1, nested_methods};

PyMODINIT_FUNC PyInit_nested(void) {
    return PyModule_Create(&nested_module);
}