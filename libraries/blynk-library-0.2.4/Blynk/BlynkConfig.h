/**
 * @file       BlynkConfig.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Configuration of different aspects of library
 *
 */

#ifndef BlynkConfig_h
#define BlynkConfig_h

/***************************************************
 * Change these settings to match your need
 ***************************************************/

#define BLYNK_DEFAULT_DOMAIN "cloud.blynk.cc"
#define BLYNK_DEFAULT_PORT   8442

/***************************************************
 * Professional settings
 ***************************************************/
// Library version.
#define BLYNK_VERSION        "0.2.4"

// Heartbeat period in seconds.
#define BLYNK_HEARTBEAT      10

// Network timeout in milliseconds.
#ifndef BLYNK_TIMEOUT_MS
#define BLYNK_TIMEOUT_MS     2000UL
#endif

// Limit the amount of outgoing commands.
#ifndef BLYNK_MSG_LIMIT
#define BLYNK_MSG_LIMIT      20
#endif

// Limit the incoming command length.
#ifndef BLYNK_MAX_READBYTES
#define BLYNK_MAX_READBYTES  256
#endif

// Uncomment to disable built-in analog and digital operations.
//#define BLYNK_NO_BUILTIN

// Uncomment to disable providing info about device to the server.
//#define BLYNK_NO_INFO

// Uncomment to enable debug prints.
//#define BLYNK_DEBUG

// Uncomment to enable experimental functions.
//#define BLYNK_EXPERIMENTAL

#endif
