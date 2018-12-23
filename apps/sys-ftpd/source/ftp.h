#pragma once

/*! Loop status */
typedef enum
{
  LOOP_CONTINUE, /*!< Continue looping */
  LOOP_RESTART,  /*!< Reinitialize */
  LOOP_EXIT,     /*!< Terminate looping */
} loop_status_t;

void ftp_pre_init(void);
int ftp_init(void);
loop_status_t ftp_loop(void);
void ftp_exit(void);
void ftp_post_exit(void);
