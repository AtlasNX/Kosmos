#include <switch.h>
#include <string.h>
#include <stdio.h>

#include "../common/common.h"

#ifdef ENABLE_AUDIO
#error "Audio is not supported currently."

#define SAMPLERATE 48000
#define BYTESPERSAMPLE 2
#define CHANNELCOUNT 2

static u8* raw_data, *raw_data2;
static u8 *audio_data;
static size_t audio_data_size;
static size_t audio_data_loopoffset;
static AudioOutBuffer source_buffer[2];

static Thread audio_thread;
static bool audio_thread_exitflag = 0;
static bool audio_thread_started = 0;

static void audio_playback_thread(void* arg)
{
    bool playing = 0;
    bool data_ready=0;
    int j, count;
    int bufi=0;

    u64 offset=0;
    u64 tmpsize=0;
    u64 totalsize = audio_data_size;

    AudioOutBuffer *released_buffer = NULL;
    AudioOutBuffer *src_buf = NULL;
    u32 released_count=0;

    while (!audio_thread_exitflag)
    {
        if (!playing)
        {
            count = 2;
            if (data_ready && released_count<2) count = 1;

            for (j=0; j<count; j++)
            {
                tmpsize = source_buffer[0].buffer_size;
                if (tmpsize > totalsize - offset) tmpsize = totalsize - offset;

                if (!data_ready || released_count==2) {
                    src_buf = &source_buffer[bufi];
                }
                else {
                    src_buf = released_buffer;
                }

                src_buf->data_size = tmpsize;

                memcpy(src_buf->buffer, &audio_data[offset], tmpsize);

                offset+= tmpsize;
                if (offset >= totalsize) offset = audio_data_loopoffset;

                audoutAppendAudioOutBuffer(src_buf);

                bufi = 1-bufi;
            }

            if (!data_ready) data_ready = 1;

            playing = 1;
        }

        if (R_SUCCEEDED(audoutWaitPlayFinish(&released_buffer, &released_count, U64_MAX)))
            playing = 0;
    }
}

void audio_initialize(void)
{
    Result rc=0;

    u8 *audio_intro, *audio_loop;
    size_t audio_intro_size, audio_loop_size;

    audio_intro = (u8*)audio_intro_bin;
    audio_intro_size = audio_intro_bin_size;

    audio_loop = (u8*)audio_loop_bin;
    audio_loop_size = audio_loop_bin_size;

    audio_data_loopoffset = audio_intro_size;
    audio_data_size = audio_intro_size + audio_loop_size;

    u32 SAMPLESPERBUF = SAMPLERATE/4;

    u32 raw_data_size = (SAMPLESPERBUF * CHANNELCOUNT * BYTESPERSAMPLE);
    u32 raw_data_size_aligned = (raw_data_size + 0xfff) & ~0xfff;

    audio_data = (u8*)malloc(audio_data_size);

    raw_data = (u8*)memalign(0x1000, raw_data_size_aligned);
    raw_data2 = (u8*)memalign(0x1000, raw_data_size_aligned);

    if (audio_data==NULL || raw_data == NULL || raw_data2==NULL) {
        free(audio_data);//free() checks NULL.
        free(raw_data);
        free(raw_data2);

        audio_data = NULL;
        raw_data = NULL;
        raw_data2 = NULL;

        return;
    }

    memset(audio_data, 0, audio_data_size);

    memset(raw_data, 0, raw_data_size_aligned);
    memset(raw_data2, 0, raw_data_size_aligned);

    memcpy(audio_data, audio_intro, audio_intro_size);
    memcpy(&audio_data[audio_data_loopoffset], audio_loop, audio_loop_size);

    source_buffer[0].next = 0;
    source_buffer[0].buffer = raw_data;
    source_buffer[0].buffer_size = raw_data_size;
    source_buffer[0].data_size = raw_data_size;
    source_buffer[0].data_offset = 0;

    memcpy(&source_buffer[1], &source_buffer[0], sizeof(AudioOutBuffer));
    source_buffer[1].buffer = raw_data2;

    if (R_SUCCEEDED(rc)) rc = audoutInitialize();
    if (R_SUCCEEDED(rc)) rc = audoutStartAudioOut();

    audio_thread_started = 0;

    if (R_SUCCEEDED(rc)) rc = threadCreate(&audio_thread, audio_playback_thread, 0, 0x4000, 28, -2);

    if (R_SUCCEEDED(rc)) rc = threadStart(&audio_thread);

    if (R_SUCCEEDED(rc)) audio_thread_started = 1;
}

void audio_exit(void)
{
    if (audio_thread_started) {
        audio_thread_exitflag = 1;
        threadWaitForExit(&audio_thread);
        threadClose(&audio_thread);
    }

    audoutStopAudioOut();
    audoutExit();

    free(audio_data);
    free(raw_data);
    free(raw_data2);
    audio_data = NULL;
    raw_data = NULL;
    raw_data2 = NULL;
}
#endif

