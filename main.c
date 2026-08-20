#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspaudiorouting.h>
#include <psphprm.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("RouteFix", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define ROUTE_MODE_HEADPHONE_ONLY 0
#define ROUTE_MODE_SPEAKER_ON 1
#define ENFORCED_ROUTE_MODE ROUTE_MODE_SPEAKER_ON

#define WORKER_NAME "routefix_worker"
#define LOG_PATH "ms0:/seplugins/routefix.log"
#define POLL_DELAY_US 500000

static volatile int g_running = 0;
static SceUID g_thread_id = -1;

static void write_log_line(const char *line)
{
    SceUID fd = sceIoOpen(LOG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0) {
        return;
    }

    sceIoWrite(fd, line, strlen(line));
    sceIoClose(fd);
}

static void log_status(const char *tag, int hprm_hp, int route_before, int route_after)
{
    char line[160];
    snprintf(
        line,
        sizeof(line),
        "%s hprm_hp=%d route_before=%d route_after=%d forced=%d\n",
        tag,
        hprm_hp,
        route_before,
        route_after,
        ENFORCED_ROUTE_MODE
    );
    write_log_line(line);
}

static int apply_route_once(const char *tag)
{
    int hprm_hp = sceHprmIsHeadphoneExist();
    int route_before = sceAudioRoutingGetMode();
    sceAudioRoutingSetMode(ENFORCED_ROUTE_MODE);
    int route_after = sceAudioRoutingGetMode();

    log_status(tag, hprm_hp, route_before, route_after);
    return route_after;
}

static int worker_thread(SceSize args, void *argp)
{
    int last_hp = -99;
    int last_route = -99;

    (void)args;
    (void)argp;

    while (g_running) {
        int hprm_hp = sceHprmIsHeadphoneExist();
        int route_before = sceAudioRoutingGetMode();
        sceAudioRoutingSetMode(ENFORCED_ROUTE_MODE);
        int route_after = sceAudioRoutingGetMode();

        if (hprm_hp != last_hp || route_after != last_route) {
            log_status("tick", hprm_hp, route_before, route_after);
            last_hp = hprm_hp;
            last_route = route_after;
        }

        sceKernelDelayThread(POLL_DELAY_US);
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

int module_start(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    g_running = 1;
    apply_route_once("start");

    g_thread_id = sceKernelCreateThread(WORKER_NAME, worker_thread, 0x18, 0x1000, PSP_THREAD_ATTR_USER, NULL);
    if (g_thread_id >= 0) {
        sceKernelStartThread(g_thread_id, 0, NULL);
        return 0;
    }

    write_log_line("error create_thread_failed\n");
    g_running = 0;
    return 1;
}

int module_stop(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    g_running = 0;
    if (g_thread_id >= 0) {
        sceKernelWaitThreadEnd(g_thread_id, NULL);
        g_thread_id = -1;
    }

    write_log_line("stop\n");
    return 0;
}
