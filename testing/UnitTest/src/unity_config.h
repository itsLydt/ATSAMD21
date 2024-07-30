/*
 * unity_config.h
 *
 * Created: 7/30/2024 12:54:36 PM
 */ 


#ifndef UNITY_CONFIG_H_
#define UNITY_CONFIG_H_

#include "stdio_serial.h"

#define UNITY_INCLUDE_EXEC_TIME
#define UNITY_OUTPUT_CHAR(a)                    putchar(a)
#define UNITY_OUTPUT_FLUSH()                    puts("\r\n")
#define UNITY_PRINT_EOL()						UNITY_OUTPUT_FLUSH()


void UnityHelperDeadLoop(void);
#define UNITY_OUTPUT_COMPLETE() UnityHelperDeadLoop()


#endif /* UNITY_CONFIG_H_ */