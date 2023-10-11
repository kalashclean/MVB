/**
 * @file
 *
 * Header file for ELTEC MVBC library.
 *
 * Copyright 2019 ELTEC Elektronik AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MVBC_LIB_INCLUDED
#define MVBC_LIB_INCLUDED 1

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>

#include "mvbc_ioctl_interface.h"

/** Error codes
 */
enum configErrors
{
	NO_ERROR = 0,                         ///< No error
	ERROR_CONFIG_INVALID_PARAMETER = -200,       ///< One of the function parameters is wrong
	ERROR_CONFIG_FILE_READ = -201,         ///< Error reading the configuration file
	ERROR_CONFIG_FILE_PARAMETER = -202,    ///< Error getting a parameter from the config file
};

/** maximal allowed port number */
#define MAX_PORT_COUNT 4095

/**
 * Error codes
 */
enum initErrors
{
	INIT_OK = 0,
	ERROR_PARSE_CONFIGURATION = 1,
	ERROR_RESET_MVBC = 2,
	ERROR_TEST_TRAFFIC_MEMORY = 4,
	ERROR_SET_DEVICE_CONFIG	 = 8,
	ERROR_SET_PORT_CONFIG = 16,
	ERROR_RUN_MVBC = 32,
};

/**
 * macro to format debug messages
 */
#define DEBUG_OUT(fmt, args...) fprintf(stderr, "%s (%d): "fmt,  __FUNCTION__, __LINE__, ##args)

/**
 * port configuration.
 */
struct sMvbcPortCfg
{
	 /** 0...4095 on MVB */
	int iPortAddr;

	/** LA = 0, DA = 1 or PP = 2 */
	int iPortType;

	/** 0 = SINK, 1 = SOURCE */
	int iPortDirection;

	/** 0...15 */
	int iFunctionCode;

	/** Check Port Status every X milliseconds, max polling interval 4096 milliseconds (0=disabled) */
	int iPollIntervalMS;

	/** Use Interrupt one of available Interrupts (DTI1 - DTI7) instead of polling (0=disabled) */
	int iIrqNumber;

	/** Port contains numerical data (1) or non numerical data (0) */
	int iNumData;
};

/**
 * MVB interface type
 */
enum eInterfaceType
{
	/** not supported */
	eOGF,

	/** ESD+ */
	eESD,

	/** EMD */
	eEMD
};

/**
 * decoder configuration
 *
 * single/redundant line
 *
 */
enum eLineMode
{
	/** LineA */
	eLineA,

	/** LineB */
	eLineB,

	/** both Lines A&B */
	eLineAB
};

/** MVBC operational mode */
enum eMode
{
	/** statically defined port(s) configuration */
	eStatic,

	/** sniffer mode is active
	 * 	-> automatically create sink port with default configuration for each port found by sniffer */
	eDynamic,

	/** combination of static and dynamic modes */
	eCombined
};

/** possible port directions */
enum ePortDirection
{
	/** Receive Data */
	eSink,

	/** Send Data */
	eSource
};

/** possible port types */
enum ePortType
{
	/** Process Data (F-Code 0..4) */
	eLA,
	/** Device Status (F-Code 15) */
	eDA,
	/** Message Data 	F-Code
	 * 			8,12,14,15 - DeviceAddress
	 *					 9 - Parameters
	 *					13 - Device Group Address */
	ePP

};

/**
 * port name/configuration.
 */
struct sMvbcPort
{
	/** e.g. ADC or TEMP */
	char cPortName[MAX_STRING_LENGTH];

	/** e.g. Address/Type/Direction ...*/
	struct sMvbcPortCfg portCfg;
};

/**
 * placeholder for maximal possible number of ports,
 * default port configuration and a variable with port_count number;
 */
struct sMvbcPorts
{
	/** number of configurations stored in port[MAX_PORT_COUNT] array.*/
	int mvbc_port_count;

	/** structure with default port config -> used for dynamically created ports (sniffer)*/
	struct sMvbcDefaultPortCfg defaultPortCfg;

	/** array with configurations for statical ports*/
	struct sMvbcPort port[MAX_PORT_COUNT];
};

/**
 * complete configuration for one MVBC device.
 */
struct sMvbcDevCfg
{
	/** e.g. MVBC1 */
	char cDescription[MAX_STRING_LENGTH];

	/** e.g. /dev/mvbc1 */
	char cDevPath[MAX_STRING_LENGTH];

	/** 1 = ESD, 2 = EMD */
	int iInterface;

	/** 0 = static -> configure ports from list */
	/** 1 = dynamic -> activate sniffer, dynamically add ports to list */
	/** 2= combined -> static port configuration + ports added/updated by sniffer */
	enum eOperationalMode iMode;

	/** switch to activate/deactivate memory test during MVBC initialization */
	int iTestTrafficMemory;

	/** MVBC device address */
	int iDeviceAddr;

	/** configuration for allowed number and types of ports */
	struct sMvbcPorts portSetup;

};

/**
 * complete configuration for one project containing max allowed count of MVBC devices.
 */
struct sProject
{
	/** project name */
	char cProjectName[MAX_STRING_LENGTH];

	/** project version*/
	char cProjectVersion[MAX_STRING_LENGTH];

	/** number of devices, parsed from JSON configuration */
	int mvbc_device_count;

	/** parameter, parsed from JSON configuration */
	struct sMvbcDevCfg mvbc[MAX_MVBC_DEVICES];
};

/** Location of the configuration file.
 *  Can be overwritten on the programs command line.
 *
 *  e.g. mvbc_init_test /path_to_config.json
 */
#define DEFAULT_PROJECT_CONFIG_FILE "/usr/share/mvbc_example.json"

int mvbc_parse_project_configuration(const char *configFile, struct sProject *pProject);

/** default project version */
#define MVBC_JSON_CONF_DEFAULT_PROJECT_VERSION "n/a"

/** default project name */
#define MVBC_JSON_CONF_DEFAULT_PROJECT_NAME "n/a"

/** default mvbc_device description */
#define MVBC_JSON_CONF_DEFAULT_DEVICE_DESCRIPTION "n/a"

/** default mvbc_device memory_test off */
#define MVBC_JSON_CONF_DEFAULT_DEVICE_MEMORY_TEST 0

/** default mvbc_port name */
#define MVBC_JSON_CONF_DEFAULT_PORT_NAME "n/a"

/** default mvbc_type name */
#define MVBC_JSON_CONF_DEFAULT_PORT_TYPE eLA

/** default mvbc_port direction */
#define MVBC_JSON_CONF_DEFAULT_PORT_DIRECTION eSink

/** default mvbc_port polling interval */
#define MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS 16

/** default mvbc_port interrupt number */
#define MVBC_JSON_CONF_DEFAULT_PORT_IRQ 0

/** default mvbc_port numerical data config */
#define MVBC_JSON_CONF_DEFAULT_PORT_NUM_DATA 0

#endif
