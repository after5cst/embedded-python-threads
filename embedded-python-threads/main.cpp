#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include <Python.h>

#define DO_NOT_RUN_ANY_PYTHON_IN_MAIN_THREAD

void main_thread_GIL_not_locked();

int main()
{
    Py_Initialize();
    PyEval_InitThreads();
    // Do any Python setup (loading modules) here
    auto threadState = PyEval_SaveThread();
    // Python NOT safe to access from here on without GIL

    main_thread_GIL_not_locked();

    // Put back our original thread state so we can finalize.
    PyEval_RestoreThread(threadState);
    Py_Finalize();
    return 0;
}

// Redirect Python prints to an object so we can
// print them in the console.
void intercept_python_stdout()
{
    std::cout << "Intercepting Python output" << std::endl;
    auto state = PyGILState_Ensure();

    auto result = PyRun_SimpleString("\
class StdoutCatcher:\n\
    def __init__(self):\n\
        self.data = ''\n\
    def write(self, stuff):\n\
        self.data = self.data + stuff\n\
import sys\n\
sys.stdout = StdoutCatcher()");
    assert(0 == result);

    PyGILState_Release(state);
}

// Grab the object and print the redirection.
void print_intercepted_stdout()
{
    auto state = PyGILState_Ensure();

    PyObject *sysmodule;
    PyObject *pystdout;
    PyObject *pystdoutdata;
    char *stdoutstring;
    sysmodule = PyImport_ImportModule("sys");
    pystdout = PyObject_GetAttrString(sysmodule, "stdout");
    pystdoutdata = PyObject_GetAttrString(pystdout, "data");
    stdoutstring = PyString_AsString(pystdoutdata);

    std::cout << stdoutstring;

    PyGILState_Release(state);
    std::cout << "Python output printed" << std::endl;
}

void worker_thread_GIL_not_locked(int id)
{
    std::stringstream sstr;
    sstr << "print('Worker Thread " << id << "')";
    auto state = PyGILState_Ensure();

    auto result = PyRun_SimpleString(sstr.str().c_str());
    assert(0 == result);
    PyGILState_Release(state);
}

void main_thread_GIL_not_locked()
{
#ifdef DO_NOT_RUN_ANY_PYTHON_IN_MAIN_THREAD
    std::thread(intercept_python_stdout).join();
#else
    intercept_python_stdout();
#endif

    // Launch a bunch of worker threads that will lock and
    // do Python things.
    const auto t_count = 10;
    std::thread t[t_count];
    for (auto i=0; i<t_count; ++i)
    {
        t[i] = std::thread(worker_thread_GIL_not_locked, i);
    }
    // Wait for them to complete.
    for (auto i=0; i<t_count; ++i)
    {
        t[i].join();
    }

#ifdef DO_NOT_RUN_ANY_PYTHON_IN_MAIN_THREAD
    std::thread(print_intercepted_stdout).join();
#else
    print_intercepted_stdout();
#endif
}
