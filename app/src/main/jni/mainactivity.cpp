#include <jni.h>
#include <signal.h>
#include <android/log.h>

#include <unwind.h>
#include <dlfcn.h>

#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

#include <pthread.h>

#include "HttpClient.h"
//#include "demangle.h"

struct android_backtrace_state
{
    void **current;
    void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context,
                                            void* arg)
{
    struct android_backtrace_state* state = (struct android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

void dump_stack(void)
{
    __android_log_print(ANDROID_LOG_INFO, "testjni", "android stack dump");

    const int max = 100;
    void* buffer[max];

    struct android_backtrace_state state;
    state.current = buffer;
    state.end = buffer + max;

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    for (int idx = 0; idx < count; idx++)
    {
        const void* addr = buffer[idx];
        const char* symbol = "";
        const char* fname = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname)
        {
            symbol = info.dli_sname;
            fname = info.dli_fname;
        }
        int status = 0;
        char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

        __android_log_print(ANDROID_LOG_INFO, "testjni", "%03d: 0x%p %s %s",
                idx,
                addr,
                fname,
                (NULL != demangled && 0 == status) ? demangled : symbol);

        if (NULL != demangled)
           free(demangled);
    }

    __android_log_print(ANDROID_LOG_INFO, "testjni", "android stack dump done");
}

struct sigaction oldhandler;

void testNative() {
    __android_log_print(ANDROID_LOG_INFO, "testjni", "calling http");
    HttpGet("52.193.50.148", "/hello");
}

void sighandler(int signo, struct siginfo *info, void *reserved)
{
    __android_log_print(ANDROID_LOG_INFO, "testjni", "got handler");

    //oldhandler.sa_handler(signo);
    oldhandler.sa_sigaction(signo, info, reserved);

    testNative();

    dump_stack();
}

void func2() {
    //g_env->FindClass(NULL);
    ((char*)(0))[0] = 'A';
}

void func1() {
    func2();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_chaeyk_testjni_MainActivity_test(JNIEnv *env, jobject instance) {
    testNative();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_chaeyk_testjni_MainActivity_makeCrash(JNIEnv *env, jobject instance) {
    func1();
    //((char*)("abcd"))[0] = 'A';
}

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {
    struct sigaction handler;
    memset(&handler, 0, sizeof(handler));
    handler.sa_sigaction = &sighandler;
    handler.sa_flags = SA_RESETHAND | SA_SIGINFO;
    sigaction(SIGSEGV, &handler, &oldhandler);
    //sigaction(SIGILL, &handler, &oldhandler);
    return JNI_VERSION_1_2;
}
