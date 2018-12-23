#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <mpg123.h>
#include <time.h>
#include "util.h"
#include "console.h"

#include <switch.h>


#define BUF_COUNT 2

static size_t			buffSize;
static mpg123_handle	*mh = NULL;
static uint32_t			rate;
static uint8_t			channels;

AudioOutBuffer*         audout_released_buf;

AudioOutBuffer          audiobuf[BUF_COUNT];
u8*                     buffData[BUF_COUNT];
int                     curBuf = 0;
#define swapbuf (curBuf = (curBuf+1)%(BUF_COUNT))

static Mutex mp3Mutex;

void mp3MutInit() {
	mutexInit(&mp3Mutex);
	
	buffSize = 80256;
    for(int curBuf = 0; curBuf < BUF_COUNT; curBuf++) {
        buffData[curBuf] = memalign(0x1000, buffSize);
    }

    audoutInitialize();
    audoutStartAudioOut();
}

int initMp3(const char* file)
{
    
	int err = 0;
	int encoding = 0;

	if((err = mpg123_init()) != MPG123_OK)
		return err;

    mpg123_pars* pars = mpg123_new_pars(&err);
    mpg123_par(pars, MPG123_FORCE_RATE, audoutGetSampleRate(), 0);
    mpg123_par(pars, MPG123_FORCE_STEREO, 1, 0);

	if((mh = mpg123_parnew(pars, NULL, &err)) == NULL)
	{
		printf("Error: %s\n", mpg123_plain_strerror(err));
		return err;
	}



	if(mpg123_open(mh, file) != MPG123_OK ||
			mpg123_getformat(mh, (long *) &rate, (int *) &channels, &encoding) != MPG123_OK)
	{
		printf("Trouble with mpg123: %s\n", mpg123_strerror(mh));
		return -1;
	}

	/*
	 * Ensure that this output format will not change (it might, when we allow
	 * it).
	 */
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);


	return 0;
}

/**
 * Get sampling rate of MP3 file.
 *
 * \return	Sampling rate.
 */
uint32_t rateMp3(void)
{
	return rate;
}

/**
 * Get number of channels of MP3 file.
 *
 * \return	Number of channels for opened file.
 */
uint8_t channelMp3(void)
{
	return channels;
}

/**
 * Decode part of open MP3 file.
 *
 * \param buffer	Decoded output.
 * \return			Samples read for each channel.
 */
uint64_t decodeMp3(void* buffer)
{
	memset(buffer, 0, buffSize);
	size_t done = 0;
	mpg123_read(mh, buffer, buffSize, &done);
	return done / (sizeof(int16_t));
}

/**
 * Free MP3 decoder.
 */
void exitMp3(void)
{
    /*for(int curBuf = 0; curBuf < BUF_COUNT; curBuf++) {
        free(buffData[curBuf]);
    }*/
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}

int fillBuf() {
    int count = decodeMp3(buffData[curBuf]);
    if(count == 0)
        return count;
    audiobuf[curBuf].next = 0;
    audiobuf[curBuf].buffer = buffData[curBuf];
    audiobuf[curBuf].buffer_size = buffSize;
    audiobuf[curBuf].data_size = buffSize;
    audiobuf[curBuf].data_offset = 0;
    audoutAppendAudioOutBuffer(&audiobuf[curBuf]);
    swapbuf;
    return count;
}


void playMp3(char* file) {
	mutexLock(&mp3Mutex);

    initMp3(file);

    u32 released_count = 0;
    int toPlayCount = 0;
    for(int curBuf = 0; curBuf < BUF_COUNT/2; curBuf++)
        toPlayCount += fillBuf() > 0;


    int lastFill = 1;
    while(appletMainLoop() && lastFill)
    {
	    for(int curBuf = 0; curBuf < BUF_COUNT/2; curBuf++) {
            lastFill = fillBuf();
			toPlayCount += lastFill > 0;
		}
        for(int curBuf = 0; curBuf < BUF_COUNT/2 && toPlayCount--; curBuf++)
            audoutWaitPlayFinish(&audout_released_buf, &released_count, 1000000000L);

    }

	while(toPlayCount--)
		audoutWaitPlayFinish(&audout_released_buf, &released_count, 1000000000L);

    exitMp3();
	mutexUnlock(&mp3Mutex);
}