#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef _WIN32
#include <intrin.h>
#endif

static PyObject* py_rdtsc(PyObject* self, PyObject* args) {
#ifdef _WIN32
    unsigned long long cycles = __rdtsc();
#else
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long long cycles = ((unsigned long long)hi << 32) | lo;
#endif
    return PyLong_FromUnsignedLongLong(cycles);
}

static PyMethodDef RdtscMethods[] = {
    {"rdtsc", py_rdtsc, METH_NOARGS, "Read CPU cycles."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef rdtscmodule = {
    PyModuleDef_HEAD_INIT,
    "rdtsc",
    NULL,
    -1,
    RdtscMethods
};

PyMODINIT_FUNC PyInit_rdtsc(void) {
    return PyModule_Create(&rdtscmodule);
}