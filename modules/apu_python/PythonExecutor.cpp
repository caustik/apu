//
// File: PythonExecutor.cpp
// Desc: Definitions for PythonExecutor class
//

#include "apu_python.h"
#if defined(WIN32) && defined(_DEBUG)
#include <windows.h>
#endif

// global resources
PythonExecutor* g_executor = nullptr;
std::unique_ptr<py::scoped_interpreter> PythonExecutor::g_interpreter;
std::mutex PythonExecutor::g_mutex;
PyThreadState* PythonExecutor::g_mainState = nullptr;
PyInterpreterState* PythonExecutor::g_mainInterpreter = nullptr;

// thread local resources
static thread_local PyThreadState* thread_state = nullptr;
static thread_local PyThreadState* main_state = nullptr;

PythonExecutor::PythonExecutor()
{
#if defined(WIN32) && defined(_DEBUG)
    if (AllocConsole()) {
        SetConsoleTitleW(L"APU Debug Console");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
        freopen("CONOUT$", "wt", stdout);
        printf(" APU Debug Console\n");
    }
#endif

    retain();

    // create intepreter if necessary
    {
        std::lock_guard lock(g_mutex);
        const bool first = !g_interpreter;
        if (first) {
            g_interpreter.reset(new py::scoped_interpreter);
            PyEval_InitThreads();
            g_mainState = PyThreadState_Get();
            g_mainInterpreter = g_mainState->interp;
            // python doesn't let us create a new state for the main thread, otherwise
            // things break with very little indication as to why
            thread_state = g_mainState;
        }

        if (first)
            thread_state = PyEval_SaveThread();
    }

    // initialize the module context
    initContext();
}

PythonExecutor::~PythonExecutor()
{
    std::lock_guard lock(g_mutex);
    PyThreadState_Swap(g_mainState);
}

void PythonExecutor::execute(const char* filename, const char* script)
{
    // reset the module context
    initContext();

    PythonExecutor::lock();

    try {
        // determine the script's path
        std::string scriptPath = filename;
        scriptPath = scriptPath.substr(0, scriptPath.find_last_of("\\/"));
        // add the script's directory to sys.path so module imports will work
        py::module sys = py::module::import("sys");
        sys.attr("path").attr("append")(scriptPath.c_str());
        // execute the script with our module context
        py::dict dict = m_module.attr("__dict__");
        py::exec(script, dict, dict);
    }
    catch (...) {
    }

    PythonExecutor::unlock();
}

void PythonExecutor::lock()
{
    g_mutex.lock();
    g_executor = this;

    if (thread_state == nullptr) {
        thread_state = PyThreadState_New(g_mainInterpreter);
        PyThreadState_Swap(thread_state);
    }

    PyEval_RestoreThread(thread_state);
}

void PythonExecutor::unlock()
{
    thread_state = PyEval_SaveThread();

    g_executor = nullptr;
    g_mutex.unlock();
}

void PythonExecutor::initContext()
{
    PythonExecutor::lock();
    m_module = py::module_::create_extension_module("PythonExecutor", nullptr, new PyModuleDef());
    m_module.doc() = "apu module";
    PythonExecutor::unlock();
}

#ifdef _WIN32
#include <windows.h>
static void dummy() {}
void PythonExecutor::retain() const
{
    char modulePath[MAX_PATH]{};
    HMODULE hModule = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummy, &hModule) == 0)
        fprintf(stderr, "GetModuleHandleEx failed, error = 0x%.08X\n", GetLastError());
    if (GetModuleFileNameA(hModule, modulePath, sizeof(modulePath)) == 0)
        fprintf(stderr, "GetModuleFileName failed, error = 0x%.08X\n", GetLastError());
    if (LoadLibraryA(modulePath) == 0)
        fprintf(stderr, "LoadLibrary failed, error = 0x%.08X\n", GetLastError());
}
#else
void PythonExecutor::retain() const
{
#pragma message("retain is not implemented for this platform")
}
#endif
