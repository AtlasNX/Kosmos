#include "worker.h"

static bool s_workerInitialized = 0;
static thrd_t s_workerThread;
static cnd_t s_workerCdn;
static mtx_t s_workerMtx;

static volatile struct
{
	workerThreadFunc func;
	void* data;

	bool exit;
} s_workerParam;

static int workerThreadProc(void* unused)
{
    mtx_lock(&s_workerMtx);

    for (;;)
    {
        cnd_wait(&s_workerCdn, &s_workerMtx);

        if (s_workerParam.exit)
            break;

        s_workerParam.func(s_workerParam.data);
    }

    mtx_unlock(&s_workerMtx);

    return 0;
}

bool workerInit(void)
{
    if (s_workerInitialized) return 1;

    if (cnd_init(&s_workerCdn) != thrd_success) return 0;
    if (mtx_init(&s_workerMtx, mtx_plain) != thrd_success) {
        cnd_destroy(&s_workerCdn);
        return 0;
    }

    if (thrd_create(&s_workerThread, workerThreadProc, 0) != thrd_success) {
        mtx_destroy(&s_workerMtx);
        cnd_destroy(&s_workerCdn);
        return 0;
    }

    s_workerInitialized = 1;
    return 1;
}

void workerExit(void)
{
    int res=0;

    if (!s_workerInitialized) return;
    s_workerInitialized = 0;

    mtx_lock(&s_workerMtx);
    s_workerParam.exit = true;
    cnd_signal(&s_workerCdn);
    mtx_unlock(&s_workerMtx);

    thrd_join(s_workerThread, &res);
    mtx_destroy(&s_workerMtx);
    cnd_destroy(&s_workerCdn);
}

void workerSchedule(workerThreadFunc func, void* data)
{
    if (!s_workerInitialized) return;

    mtx_lock(&s_workerMtx);
    s_workerParam.func = func;
    s_workerParam.data = data;
    cnd_signal(&s_workerCdn);
    mtx_unlock(&s_workerMtx);
}

