#include <Python.h>
#include <structmember.h>
#include "qkd/models.h"
#include "qkd/interfaces.h"


static PyObject *qkd_open_connect_wrapper(PyObject *self, PyObject *args) {
    char *kms_address = "localhost:1234";
    char *source_s = "qkd://app1@n1";
    char *destination_s = "qkd://app2@n2";
    char *key_stream_id_s = "12345678-1234-1234-1234-123456789abc";

    char *meta = "hello";
    qkd_uri_t source = {strlen(source_s), source_s};
    qkd_uri_t destination = {strlen(destination_s), destination_s};
    qkd_qos_t qos = {1, 2, 3, 4, 5, 6, 7, {strlen(meta), meta}};

    uuid_t key_stream_id;
    uuid_parse(key_stream_id_s, key_stream_id);

    qkd_status_t result = qkd_open_connect(kms_address, &source, &destination, qos, &key_stream_id);

    char key_stream_id_r[UUID_STR_LEN];
    uuid_unparse_lower(key_stream_id, key_stream_id_r);
    PyObject *qos_r = Py_BuildValue(
            "{s:I,s:I,s:I,s:I,s:I,s:I,s:I,s:s}",
            "key_chunk_size", qos.key_chunk_size,
            "max_bps", qos.max_bps,
            "min_bps", qos.min_bps,
            "jitter", qos.jitter,
            "priority", qos.priority,
            "timeout", qos.timeout,
            "ttl", qos.ttl,
            "metadata_mimetype", qos.metadata_mimetype.text
    );

    PyObject *ret = Py_BuildValue("(IsO)", result, key_stream_id_r, qos_r);
    return ret;
}

static PyObject *qkd_get_key_wrapper(PyObject *self, PyObject *args) {
    char *kms_address = "localhost:1234";
    char *source_s = "qkd://app1@n1";
    char *destination_s = "qkd://app2@n2";
    char *key_stream_id_s = "12345678-1234-1234-1234-123456789abc";

    char *meta = "hello";
    qkd_uri_t source = {strlen(source_s), source_s};
    qkd_uri_t destination = {strlen(destination_s), destination_s};
    qkd_qos_t qos = {1, 2, 3, 4, 5, 6, 7, {strlen(meta), meta}};

    uuid_t key_stream_id;
    uuid_parse(key_stream_id_s, key_stream_id);

    qkd_status_t result = qkd_open_connect(kms_address, &source, &destination, qos, &key_stream_id);

    char key_stream_id_r[UUID_STR_LEN];
    uuid_unparse_lower(key_stream_id, key_stream_id_r);
    PyObject *qos_r = Py_BuildValue(
            "{s:I,s:I,s:I,s:I,s:I,s:I,s:I,s:s}",
            "key_chunk_size", qos.key_chunk_size,
            "max_bps", qos.max_bps,
            "min_bps", qos.min_bps,
            "jitter", qos.jitter,
            "priority", qos.priority,
            "timeout", qos.timeout,
            "ttl", qos.ttl,
            "metadata_mimetype", qos.metadata_mimetype.text
    );

    PyObject *ret = Py_BuildValue("(IsO)", result, key_stream_id_r, qos_r);
    return ret;
}

static PyObject *qkd_close_wrapper(PyObject *self, PyObject *args) {
    char *kms_address = "localhost:1234";
    char *key_stream_id_s = "12345678-1234-1234-1234-123456789abc";

    uuid_t key_stream_id;
    uuid_parse(key_stream_id_s, key_stream_id);

    qkd_status_t result = qkd_close(kms_address, &key_stream_id);

    PyObject *ret = Py_BuildValue("I", result);
    return ret;
}

static PyMethodDef CudaextMethods[] = {
        {"open_connect", qkd_open_connect_wrapper, METH_NOARGS},
        {"get_key", qkd_get_key_wrapper, METH_NOARGS},
        {"close", qkd_close_wrapper, METH_NOARGS},
//        {"return_nothing",                   return_nothing_wrapper,                     METH_NOARGS},
//        {"return_by_value",                  return_by_value_wrapper,                    METH_VARARGS},
//        {"return_by_reference",              "return_by_reference_wrapper",              METH_VARARGS},
//        {"return_by_value_and_by_reference", "return_by_value_and_by_reference_wrapper", METH_VARARGS},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef qkd_ext_definition = {
        PyModuleDef_HEAD_INIT,
        "interface",
        "A Python module extension for the standard \"etsi-gs-qkd-004\" library",
        -1,
        CudaextMethods
};

PyMODINIT_FUNC PyInit_interface(void) {
    return PyModule_Create(&qkd_ext_definition);
}