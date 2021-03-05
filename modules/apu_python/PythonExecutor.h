//
// File: PythonExecutor.h
// Desc: Declarations for PythonExecutor class
//

#ifndef PYTHON_EXECUTOR_H
#define PYTHON_EXECUTOR_H

#include "apu_python.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <mutex>
#include <thread>

//
// PythonExecutor
//
// Provides capability to execute Python code
//

class PythonExecutor
{
public:
    PythonExecutor();
    ~PythonExecutor();

    // execute python script
    virtual void execute(const char* filename, const char* script);

    // enter/exit thread interpreter
    void lock();
    void unlock();

    // bind variable to module context
    void bind(const char* name, py::object obj) { m_module.attr("__dict__")[name] = obj; }
    py::module_& getModule() { return m_module; }

private:
    //
    // Retain an extra reference to our own dll
    //
    // This is (sadly) necessary because otherwise the host can unload and then re-load our VST
    // which will then expect a fresh and clean Python DLL state. Python has a major design flaw
    // whereby native modules stay loaded and expect a persistent state. Unloading our DLL will
    // cause a disconnect between the Python C API / pybind11 and the internal state of the Python
    // DLLs and its loaded native modules, causing crashes.
    //
    // Downside: Our plugin module will always be locked, making it impossible to delete/replace
    // while the host is still running. It should be reported in release notes as a known issue.
    //

    void retain() const;

    // (re)initialize the execution context
    void initContext();

    // static resources
    static std::unique_ptr<py::scoped_interpreter> g_interpreter;
    static std::mutex g_mutex;
    static PyThreadState* g_mainState;
    static PyInterpreterState* g_mainInterpreter;

    // per-instance module context
    py::module_ m_module;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonExecutor)
};

#endif /* PYTHON_EXECUTOR_H */
