/**
 * @file
 *
 * User Application interface to communicate with ELTEC MVB devices.
 *
 * Copyright (C) ELTEC Elektronik AG 2019
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

//#include "../include/mvbc_json_config.h_"
#include "mvbc_lib.h"


/** global variable gProject to hold parsed project configuration */
struct sProject gProject;

static int send_cmd(const char *dev, int cmd, void* arg);
static int mvbc_run_device(struct sMvbcDevCfg *mvbc);
static int mvbc_reset_device(struct sMvbcDevCfg *mvbc);
static int mvbc_set_device_configuration(struct sMvbcDevCfg *mvbc);
static int mvbc_get_device_configuration(struct sMvbcDevCfg *mvbc);
static int mvbc_set_port_configuration(struct sMvbcDevCfg *mvbc);

/**
 * call driver IOCTL function
 *
 * @param dev (e.g./dev/mvbc1)
 * @param cmd (e.g. EL_MVBC_SET_DEVICE_CONFIGURATION)
 * @param arg (e.g. (struct sMvbcDeviceConfig *)arg)
 * @return 0 in case of success, -1 for error
 */
static int send_cmd(const char *dev, int cmd, void* arg)
{
	int rc = NO_ERROR;
	int fd = -1;

	if (strncmp(dev,"/dev",4)==0)
	{
		fd = open(dev, O_RDWR);
		if (fd < 0)
		{
			DEBUG_OUT( "ERROR open device [%s] RC[%X]\n",dev,fd);
			rc = fd;
		}
		else
		{
			if (arg == NULL)
			{
				rc = ioctl(fd, cmd);
			}
			else
			{
				switch (cmd)
				{
					case EL_MVBC_RESET_DEVICE:
						rc = ioctl(fd, cmd, (struct sMvbcDeviceConfig *)arg);
						break;

					case EL_MVBC_SET_DEVICE_CONFIGURATION:
					case EL_MVBC_GET_DEVICE_CONFIGURATION:
						rc = ioctl(fd, cmd, (struct sMvbcDeviceConfig *)arg);
						break;

					case EL_MVBC_SET_PORT_CONFIGURATION:
					case EL_MVBC_GET_PORT_CONFIGURATION:
						rc = ioctl(fd, cmd, (struct sMvbcPortConfig *)arg);
						break;
					}
			}
			close(fd);
		}
	}
	else
	{
		rc = -1;
	}
	return rc;
}

/**
 * Reset MVBC devices
 *
 * @param mvbc sMvbcDevCfg
 * @return 0 in case of success, -1 for error
 */
static int mvbc_reset_device(struct sMvbcDevCfg *mvbc)
{
	int rc = NO_ERROR;

	if (mvbc != NULL)
	{
		struct sMvbcDeviceConfig deviceCfg;

		memset(&deviceCfg,0, sizeof(struct sMvbcDeviceConfig));

		/** QUIET: set Read-Only mode */
		deviceCfg.regs.wSCR |= (1 << 14);

		/** TMO: Timeout Coefficient -> set default 42,7 us */
		deviceCfg.regs.wSCR |= (1 << 10);

		/** WS: set minimum number of waitstates to 0 */
		/** !!!ATTENTION!!! BUG: changing of waitstates results in receiving wrong data! */
		deviceCfg.regs.wSCR |= (0 << 8);
		deviceCfg.regs.wSCR |= (0 << 9);

		/** IL: set configuration mode */
		deviceCfg.regs.wSCR |= 1;

		/** MCM: set memory layout */
		deviceCfg.regs.wMCR |= (3 << 0);

		/** QO: Range 00000H - 3FFFFH */
		/** MO: Range 00000H - 3FFFFH */

		deviceCfg.uiOperationMode = mvbc->iMode;

		deviceCfg.uiTestTrafficMemory = mvbc->iTestTrafficMemory;

		deviceCfg.defaultPortCfg = mvbc->portSetup.defaultPortCfg;

		rc |= send_cmd(mvbc->cDevPath, EL_MVBC_RESET_DEVICE,&deviceCfg);
	}

	DEBUG_OUT( "RC[%X]\n",rc);
	return rc;
}

/**
 *Configure MVBC device settings
 *
 * @param mvbc struct sMvbcDevCfg
 * @return 0 in case of success, -1 for error
 */
static int mvbc_set_device_configuration(struct sMvbcDevCfg *mvbc)
{
	int rc = ERROR_SET_DEVICE_CONFIG;

	if (mvbc != NULL)
	{
		struct sMvbcDeviceConfig deviceCfg;

		deviceCfg.uiLine = eLineAB;
		deviceCfg.uiDevAddr = mvbc->iDeviceAddr;
		deviceCfg.uiMode = mvbc->iInterface;

		deviceCfg.uiSinkTimeInterval = 6; //32ms
		deviceCfg.uiSinkTimeNumberOfDocks = 0xFFF; //activate sink-time supervision for all 4096 ports

		rc |= send_cmd(mvbc->cDevPath, EL_MVBC_SET_DEVICE_CONFIGURATION,&deviceCfg);
	}

	DEBUG_OUT( "RC[%X]\n", rc);

	return rc;
}

/**
 * Get MVBC device settings
 *
 * @param mvbc sMvbcDevCfg
 * @return 0 in case of success, -1 for error
 */
static int mvbc_get_device_configuration(struct sMvbcDevCfg *mvbc)
{
	int rc = NO_ERROR;

	if (mvbc != NULL)
	{
		struct sMvbcDeviceConfig deviceCfg;

		rc |= send_cmd(mvbc->cDevPath, EL_MVBC_GET_DEVICE_CONFIGURATION,&deviceCfg);

		DEBUG_OUT( "MCR[%X]\n", deviceCfg.regs.wMCR);
		DEBUG_OUT( "DR[%X]\n", deviceCfg.regs.wDR);
		DEBUG_OUT( "SCR[%X]\n", deviceCfg.regs.wSCR);
	}

	DEBUG_OUT( "RC[%X]\n", rc);

	return rc;
}

/**
 * Configure MVBC ports
 *
 * @param mvbc sMvbcDevCfg
 * @return 0 in case of success, -1 for error
 */
static int mvbc_set_port_configuration(struct sMvbcDevCfg *mvbc)
{
	int rc = ERROR_SET_PORT_CONFIG;

	if (mvbc != NULL)
	{
		/* go through all configured ports for each mvbc device */
		for (int j = 0; j < mvbc->portSetup.mvbc_port_count; j++)
		{
			/* MVBC driver port config struct */
			struct sMvbcPortConfig portCfg;

			memset(&portCfg, 0, sizeof(struct sMvbcPortConfig));

			int direction = mvbc->portSetup.port[j].portCfg.iPortDirection;
			int num_data = mvbc->portSetup.port[j].portCfg.iNumData;
			int irq_num = mvbc->portSetup.port[j].portCfg.iIrqNumber;

			portCfg.bStaticConf = 1;

			portCfg.wPortAddr = mvbc->portSetup.port[j].portCfg.iPortAddr;
			portCfg.wFuncCode = mvbc->portSetup.port[j].portCfg.iFunctionCode;
			portCfg.wPortType = mvbc->portSetup.port[j].portCfg.iPortType;

			/* start with zero */
			portCfg.wPCS_W0 = 0;

			/* F-Code */
			portCfg.wPCS_W0 |= (portCfg.wFuncCode << 12);
			/* Source/Sink */
			portCfg.wPCS_W0 |= (1 << (10 + direction));
			/* Num. Data */
			portCfg.wPCS_W0 |= (num_data << 1);

			/* either interrupt or pollInterval should be used */
			if (irq_num == 0)
			{
				portCfg.wPollInterval = mvbc->portSetup.port[j].portCfg.iPollIntervalMS;
			}
			else
			{
				/* Interrupt */
				portCfg.wPCS_W0 |= (irq_num << 5);
			}

			DEBUG_OUT( "wPCS_W0[%X]\n", portCfg.wPCS_W0);

			rc |= send_cmd(mvbc->cDevPath, EL_MVBC_SET_PORT_CONFIGURATION,&portCfg);
		}
	}

	DEBUG_OUT( "RC[%X]\n", rc);

	return rc;
}

/**
 * Run MVBC devices
 *
 * @param mvbc sMvbcDevCfg
 * @return 0 in case of success, -1 for error
 */
static int mvbc_run_device(struct sMvbcDevCfg *mvbc)
{
	int rc = ERROR_RUN_MVBC;

	if (mvbc != NULL)
	{
		rc |= send_cmd(mvbc->cDevPath, EL_MVBC_RUN_DEVICE,NULL);
	}

	DEBUG_OUT( "RC[%X]\n", rc);

	return rc;
}

/**
 * Get the libraries version numbers.
 *
 * The library number has the form "major.minor.patch" (e.g. 1.0.0).
 *
 * @param  major returns the major number
 * @param  minor returns the minor number
 * @param  patch returns the patch
 *
 * @return 0 in case of success, -1 for error
 */
int mvbc_get_library_version(int *major, int *minor, int *patch)
{
    *major = LIBMVBC_VERSION_MAJOR;
    *minor = LIBMVBC_VERSION_MINOR;
    *patch = LIBMVBC_VERSION_PATCH;

    return 0;
}

/**
 * Get the PLD firmware version.
 *
 * The PLD firmware version is returned as int.
 *
 * @param  version returns the MVBC version
 *
 * @return 0 in case of success, -1 for error
 */
int mvbc_get_pld_firmware_version(int *version)
{
	//TODO: return PLD firmware version
    *version = 1;

    return 0;
}

int mvbc_shutdown(const char *devPath)
{
	return send_cmd(devPath,EL_MVBC_SHUTDOWN_DEVICE,NULL);
}

int mvbc_init(const char *config_file)
{
	int rc = 0;

	rc |= mvbc_parse_project_configuration(config_file,&gProject);

	/* if config valid -> init mvbc devices in the loop */
	for (int i = 0; i < gProject.mvbc_device_count; i++)
	{
		rc = mvbc_shutdown(gProject.mvbc[i].cDevPath);
		if (rc < 0)
			return rc;

		rc |= mvbc_reset_device(&gProject.mvbc[i]);
		if (rc < 0)
			return rc;

		rc |= mvbc_set_device_configuration(&gProject.mvbc[i]);
		if (rc < 0)
			return rc;

		rc |= mvbc_set_port_configuration(&gProject.mvbc[i]);
		if (rc < 0)
			return rc;

		rc |= mvbc_run_device(&gProject.mvbc[i]);
	}

	DEBUG_OUT( "RC[%X]\n", rc);
	return rc;
}
