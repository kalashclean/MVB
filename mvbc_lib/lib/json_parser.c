/**
 * @file
 *
 * Functions for parsing and validation of JSON project configuration files.
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

#include "mvbc_lib.h"
#include "parson.h"

static int validateMode(const char *mode);
static int validatePortType(const char *type);
static int validatePortDirection(const char *direction);
static int validateDeviceAddr(int addr);
static int validatePortAddr(int addr);
static int validateFunctionalCode(int fcode);
static int validatePollingTimeout(int poll_ms);
static int validateInterruptNumber(int irq);
static int validateNumericalData(int num_data);

/**
 * Validate MVB interface.
 * Allowed param values
 *  EMD
 *  ESD+
  * @param mode
 * @return -1 in case of unknown interface type, else interface number
 */
static int validateInterface(const char *mode)
{
	int rc =-1;

	if (strcmp(mode,"EMD") == 0)
	{
		rc = eEMD;
	}
	else if (strcmp(mode,"ESD+") == 0)
	{
		rc = eESD;
	}
	return rc;
}

/**
 * Validate MVB operational mode,
 * Allowed param values
 * 	static: all ports are statically defined
 * 	dynamic: no ports are defined, default port setup is used to setup ports found by sniffer
 * 	combined: mix of statically defined ports and ports found by sniffer
 *
 * @param mode
 * @return -1 in case of unknown mode, else mode number
 */
static int validateMode(const char *mode)
{
	int rc =-1;

	if (strcmp(mode,"static") == 0)
	{
		rc = eStatic;
	}
	else if (strcmp(mode,"dynamic") == 0)
	{
		rc = eDynamic;
	}
	else if (strcmp(mode,"combined") == 0)
	{
		rc = eCombined;
	}
	return rc;
}

/**
 * Validate MVB port type,
 * Allowed param values:
 * 	la: Process Data
 * 	da: Device Status
 * 	pp: Message Data/Events
 * @param type
 * @return -1 in case of unknown port type, else type number
 */
static int validatePortType(const char *type)
{
	int rc =-1;

	if (strcmp(type,"la") == 0)
	{
		rc = eLA;
	}
	else if (strcmp(type,"da") == 0)
	{
		rc = eDA;
	}
	else if (strcmp(type,"pp") == 0)
	{
		rc = ePP;
	}
	return rc;
}

/**
* Validate MVB port direction (function),
 * Allowed param values:
 * 	sink: port configured to receive data
 * 	source: port configured to send data (not yet supported)
 * @param direction
 * @return -1 in case of unknown port direction, else type number
 */
static int validatePortDirection(const char *direction)
{
	int rc =-1;

	if (strcmp(direction,"source") == 0)
	{
		rc = eSource;
	}
	else if (strcmp(direction,"sink") == 0)
	{
		rc = eSink;
	}
	return rc;
}

/**
 * * Validate MVB device address,
 * Allowed param values
 * 	4095 >= addr > 0
 * @param addr
 * @return -1 in case of wrong value, else device address number
 */
static int validateDeviceAddr(int addr)
{
	int rc =-1;

	if ( (addr <= 4095) && (addr > 0) )
	{
		rc = addr;
	}
	return rc;
}

/**
 * * Validate MVB port address,
 * Allowed param values
 * 	4095 >= addr > 0 (Memory Configuration Mode 3, see MVBC02D spec.)
 * @param addr
 * @return -1 in case of wrong value, else port address number
 */
static int validatePortAddr(int addr)
{
	int rc =-1;

	if ( (addr <= 4095) && (addr > 0) )
	{
		rc = addr;
	}
	return rc;
}

/**
 * * Validate MVB port functional code (F-Code),
 * Allowed param values
 * 	15 >= fcode >= 0 (see MVBC02D spec.)
 * @param fcode
 * @return -1 in case of wrong value, else port address number
 */
static int validateFunctionalCode(int fcode)
{
	int rc =-1;

	if ( (fcode <= 15) && (fcode >= 0) )
	{
		rc = fcode;
	}
	return rc;
}

/**
 * * Validate MVB port poll interval,
 * Allowed param values
 * 1 ms (not recommended, will use a very lot of MVB resources)
 * 2 ms (not recommended, will use a very lot of MVB resources)
 * 4 ms (not recommended, will use a very lot of MVB resources)
 * 8 ms (not recommended, will use a very lot of MVB resources)
 * 16 ms
 * 32 ms
 * 64 ms
 * 128 ms
 * 256 ms
 * 512 ms
 * 1024 ms
 * @param poll_ms
 * @return -1 in case of wrong value, else polling timeout number
 */
static int validatePollingTimeout(int poll_ms)
{
	int rc = -1;

	switch(poll_ms)
	{
	case 1:
	case 2:
	case 4:
	case 8:
		DEBUG_OUT( "WARNING: poll_ms value is %d! Recommended values are 16/32/64/128/512/1024ms.\n",poll_ms);
		rc = poll_ms;
		break;
	case 16:
	case 32:
	case 64:
	case 128:
	case 256:
	case 512:
	case 1024:
		rc = poll_ms;
		break;
	default:
		DEBUG_OUT( "WARNING: not_specified poll_ms value %d! Set default poll_ms = %d ms!\n",poll_ms, MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS);
		rc = MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS;
		break;
	}
	return rc;
}

/**
 * Validate interrupt number
 *
 * Allowed param values
 *  7 (DTI7) >= irg >= 0 (No Interrupt)
 *
 * @param irq
* @return -1 in case of wrong value, else interrupt number
 */
static int validateInterruptNumber(int irq)
{
	int rc = -1;

	/* 0=No interrupt/1=DTI1..7=DTI7 */
	if ( (irq <= 7) && (irq >= 0) )
	{
		rc = irq;
	}
	return rc;
}

/**
 * Validate num_data param
 *
 * Allowed param values
 *  0 / 1
 *
 * @param num_data
* @return -1 in case of wrong value, else num_data param
 */
static int validateNumericalData(int num_data)
{
	int rc =-1;

	/* 0/1 */
	if ((num_data == 0) || (num_data == 1))
	{
		rc = num_data;
	}
	return rc;
}

/**
 * Parse port/s configuration from given JSON_Object *structObject
 * depending on operational mode param (enum eMode mode).
 *
 * @param structObject
 * @param portSetup
 * @param mode
* @return error_code ERROR_CONFIG_FILE_PARAMETER in case of error, else NO_ERROR
 */
static int parse_port_config(JSON_Object *structObject, struct sMvbcPorts *portSetup, enum eMode mode)
{
	int rc = NO_ERROR;
	int i;
	int iLocalValue = -1;

	if (( mode == eStatic ) || (mode == eCombined))
	{
		JSON_Array *staticList;
		int count = 0;

		staticList = json_object_dotget_array(structObject, "config.static");

		count = json_array_get_count(staticList);

		portSetup->mvbc_port_count = count;

		for (i = 0; i < count; i++)
		{
			JSON_Object *port = json_array_get_object(staticList, i);
			const char *strResult;

			DEBUG_OUT( "\t\t**********static***********\n");

			/** OPTIONAL config.static.name (string) */

			if (json_object_dothas_value_of_type(port, "name", JSONString))
			{
				strResult = json_object_dotget_string(port, "name");
				if (strResult)
				{
					DEBUG_OUT( "\t\t\tPORT[%d] name[%s]\n",i,strResult);
					strncpy(portSetup->port[i].cPortName, strResult, MAX_STRING_LENGTH);
				}
				else
				{
					DEBUG_OUT( "'name' parser error\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'name' is not a string -> set default [%s]\n",MVBC_JSON_CONF_DEFAULT_PORT_NAME);
				strncpy(portSetup->port[i].cPortName, MVBC_JSON_CONF_DEFAULT_PORT_NAME, MAX_STRING_LENGTH);
			}

			/** MANDATORY config.static.addr (number) */

			if (json_object_dothas_value_of_type(port, "addr", JSONNumber))
			{
				iLocalValue = validatePortAddr((int)json_object_get_number(port, "addr"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iPortAddr = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] addr[%d]\n",i, portSetup->port[i].portCfg.iPortAddr);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'addr' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				rc = ERROR_CONFIG_FILE_PARAMETER;
				DEBUG_OUT( "'addr' is not a number\n");
				break;
			}

			/** OPTIONAL config.static.type (string) */

			if (json_object_dothas_value_of_type(port, "type", JSONString))
			{
				iLocalValue = validatePortType(json_object_dotget_string(port, "type"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iPortType = iLocalValue;
					DEBUG_OUT( " \t\t\tPORT[%d] type[%d]\n",i,portSetup->port[i].portCfg.iPortType);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'type' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'type' is not a string -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_TYPE);
				portSetup->port[i].portCfg.iPortType = MVBC_JSON_CONF_DEFAULT_PORT_TYPE;
			}

			/** OPTIONAL config.static.direction (string) */

			if (json_object_dothas_value_of_type(port, "direction", JSONString))
			{
				iLocalValue = validatePortDirection(json_object_dotget_string(port, "direction"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iPortDirection = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] direction[%d]\n",i,portSetup->port[i].portCfg.iPortDirection);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'direction' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'direction' is not a string -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_DIRECTION);
				portSetup->port[i].portCfg.iPortDirection = MVBC_JSON_CONF_DEFAULT_PORT_DIRECTION;
			}

			/** MANDATORY config.static.fcode (number) */

			if (json_object_dothas_value_of_type(port, "fcode", JSONNumber))
			{
				iLocalValue = validateFunctionalCode((int)json_object_get_number(port, "fcode"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iFunctionCode = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] fcode[%d]\n",i,portSetup->port[i].portCfg.iFunctionCode);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'fcode' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				rc = ERROR_CONFIG_FILE_PARAMETER;
				DEBUG_OUT( "'fcode' is not a number\n");
				break;
			}

			/** OPTIONAL config.static.poll_ms (number) */

			if (json_object_dothas_value_of_type(port, "poll_ms", JSONNumber))
			{
				iLocalValue = validatePollingTimeout((int)json_object_get_number(port, "poll_ms"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iPollIntervalMS = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] poll_ms[%d]\n",i,portSetup->port[i].portCfg.iPollIntervalMS);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'poll_ms' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'poll_ms' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS);
				portSetup->port[i].portCfg.iPollIntervalMS = MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS;
			}

			/** OPTIONAL config.static.irq (number) */

			if (json_object_dothas_value_of_type(port, "irq", JSONNumber))
			{
				iLocalValue = validateInterruptNumber((int)json_object_get_number(port, "irq"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iIrqNumber = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] irq[%d]\n",i,portSetup->port[i].portCfg.iIrqNumber);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'irq' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'irq' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_IRQ);
				portSetup->port[i].portCfg.iIrqNumber = MVBC_JSON_CONF_DEFAULT_PORT_IRQ;
			}

			/** OPTIONAL config.static.num_data (number) */

			if (json_object_dothas_value_of_type(port, "num_data", JSONNumber))
			{
				iLocalValue = validateNumericalData((int)json_object_get_number(port, "num_data"));
				if (iLocalValue != -1)
				{
					portSetup->port[i].portCfg.iNumData = iLocalValue;
					DEBUG_OUT( "\t\t\tPORT[%d] num_data[%d]\n",i,portSetup->port[i].portCfg.iNumData);

					iLocalValue = -1;
				}
				else
				{
					DEBUG_OUT( "'num_data' validation failed\n");
					rc = ERROR_CONFIG_FILE_PARAMETER;
					break;
				}
			}
			else
			{
				DEBUG_OUT( "'num_data' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_NUM_DATA);
				portSetup->port[i].portCfg.iNumData = MVBC_JSON_CONF_DEFAULT_PORT_NUM_DATA;
			}
		}
	}

	if (( mode == eDynamic ) || (mode == eCombined))
	{
		const char *strResult;

		DEBUG_OUT( "\t\t**********dynamic***********\n");

		/** OPTIONAL config.default.type (string) */

		if (json_object_dothas_value_of_type(structObject, "config.default.type", JSONString))
		{
			iLocalValue = validatePortType(json_object_dotget_string(structObject, "config.default.type"));
			if (iLocalValue != -1)
			{
				portSetup->defaultPortCfg.wPortType = iLocalValue;
				DEBUG_OUT( " \t\t\tdefault.type[%d]\n",portSetup->defaultPortCfg.wPortType);

				iLocalValue = -1;
			}
			else
			{
				DEBUG_OUT( "'default.type' validation failed\n");
				rc = ERROR_CONFIG_FILE_PARAMETER;
			}
		}
		else
		{
			DEBUG_OUT( "'default.type' is not a string -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_TYPE);
			portSetup->defaultPortCfg.wPortType = MVBC_JSON_CONF_DEFAULT_PORT_TYPE;
		}

		/** OPTIONAL config.default.poll_ms (number) */

		if (json_object_dothas_value_of_type(structObject, "config.default.poll_ms", JSONNumber))
		{
			iLocalValue = validatePollingTimeout(json_object_dotget_number(structObject, "config.default.poll_ms"));
			if (iLocalValue != -1)
			{
				portSetup->defaultPortCfg.wPollInterval = iLocalValue;
				DEBUG_OUT( " \t\t\tdefault.poll_ms[%d]\n",portSetup->defaultPortCfg.wPollInterval);

				iLocalValue = -1;
			}
			else
			{
				DEBUG_OUT( "'default.poll_ms' validation failed\n");
				rc = ERROR_CONFIG_FILE_PARAMETER;
			}
		}
		else
		{
			DEBUG_OUT( "'default.poll_ms' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS);
			portSetup->defaultPortCfg.wPollInterval = MVBC_JSON_CONF_DEFAULT_PORT_POLL_MS;
		}

		/** OPTIONAL config.default.irq (number) */

		if (json_object_dothas_value_of_type(structObject, "config.default.irq", JSONNumber))
		{
			iLocalValue = validateInterruptNumber(json_object_dotget_number(structObject, "config.default.irq"));
			if (iLocalValue != -1)
			{
				portSetup->defaultPortCfg.iIrqNumber = iLocalValue;
				DEBUG_OUT( " \t\t\tdefault.irq[%d]\n",portSetup->defaultPortCfg.iIrqNumber);
				iLocalValue = -1;
			}
			else
			{
				DEBUG_OUT( "'default.irq' validation failed\n");
				rc = ERROR_CONFIG_FILE_PARAMETER;
			}
		}
		else
		{
			DEBUG_OUT( "'default.irq' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_IRQ);
			portSetup->defaultPortCfg.iIrqNumber = MVBC_JSON_CONF_DEFAULT_PORT_IRQ;
		}

		/** OPTIONAL config.default.num_data (number) */

		if (json_object_dothas_value_of_type(structObject, "config.default.num_data", JSONNumber))
		{
			iLocalValue = validateNumericalData(json_object_dotget_number(structObject, "config.default.num_data"));
			if (iLocalValue != -1)
			{
				portSetup->defaultPortCfg.iNumData = iLocalValue;
				DEBUG_OUT( " \t\t\tdefault.num_data[%d]\n",portSetup->defaultPortCfg.iNumData);

				iLocalValue = -1;
			}
			else
			{
				DEBUG_OUT( "'default.num_data' validation failed\n");
				rc = ERROR_CONFIG_FILE_PARAMETER;
			}
		}
		else
		{
			DEBUG_OUT( "'default.num_data' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_PORT_NUM_DATA);
			portSetup->defaultPortCfg.iNumData  = MVBC_JSON_CONF_DEFAULT_PORT_NUM_DATA;
		}
	}

	return rc;
}

/**
 *
 * Parse the project configuration file.
 *
 * Some parameters are mandatory (e.g. mvbc interface name "/dev/mvbc0").
 * Others are optional (e.g. interrupt number).
 *
 * In case of at least one of mandatory parameters has a wrong type or validation returns error, further parsing is aborted.
 * If optional parameters have a wrong type or validation returns error, default values are used and parsing is continued.
 *
 * @param  configFile contains the path of JSON configuration file
 * @param  pProject structure to return the data
 * @return 0 in case of success, error_code (enum configErrors) in case of error
 */
int mvbc_parse_project_configuration(const char *configFile, struct sProject *pProject)
{
	int rc = 0;
	JSON_Value *rootValue;
	const char* jsonFile = configFile ? configFile : DEFAULT_PROJECT_CONFIG_FILE;
	const char *strResult;

	if (pProject == 0 )
	{
		return ERROR_CONFIG_INVALID_PARAMETER;
	}
	else
	{
		memset(pProject, 0, sizeof(struct sProject));
	}

	rootValue = json_parse_file(jsonFile);

	if (json_value_get_type(rootValue) != JSONObject)
	{
		DEBUG_OUT( "Error reading json file %s\n", jsonFile);
		rc = ERROR_CONFIG_FILE_READ;
	}
	else
	{
		JSON_Array *devices;
		JSON_Object *rootObject = json_value_get_object(rootValue);

		/** OPTIONAL project.name (string) */

		strResult = json_object_dotget_string(rootObject, "project.name");

		if (strResult)
		{
			strncpy(pProject->cProjectName, strResult, MAX_STRING_LENGTH);
		}
		else
		{
			DEBUG_OUT( "'project.name' is not a string -> set default [%s]\n",MVBC_JSON_CONF_DEFAULT_PROJECT_NAME);
			strncpy(pProject->cProjectName, MVBC_JSON_CONF_DEFAULT_PROJECT_NAME, MAX_STRING_LENGTH);
		}

		/** OPTIONAL project.version (string) */

		strResult = json_object_dotget_string(rootObject, "project.version");

		if (strResult)
		{
			strncpy(pProject->cProjectVersion, strResult, MAX_STRING_LENGTH);
		}
		else
		{
			DEBUG_OUT( "'project.version' is not a string -> set default [%s]\n",MVBC_JSON_CONF_DEFAULT_PROJECT_VERSION);
			strncpy(pProject->cProjectVersion, MVBC_JSON_CONF_DEFAULT_PROJECT_VERSION, MAX_STRING_LENGTH);
		}

		/** MANDATORY project.devices (array) */

		devices = json_object_dotget_array(rootObject, "project.devices");

		if(devices)
		{
			int count = json_array_get_count(devices);

			if (count > MAX_MVBC_DEVICES)
			{
				DEBUG_OUT( "Error: DEVICE NODE FOUND[%d] > ALLOWED [%d]\n",count,MAX_MVBC_DEVICES);
				rc = ERROR_CONFIG_INVALID_PARAMETER;
			}
			else
			{
				int i;
				int iLocalValue = -1;

				pProject->mvbc_device_count = count;

				for (i = 0; i < count; i++)
				{
					JSON_Object *structObject = json_array_get_object(devices, i);

					DEBUG_OUT( "\t*********************\n");

					/** MANDATORY project.devices[i].path (string) */

					strResult = json_object_dotget_string(structObject, "path");

					if (strResult)
					{
						DEBUG_OUT( "DEVICE[%d]\tpath [%s]\n",i,strResult);
						strncpy(pProject->mvbc[i].cDevPath, strResult, MAX_STRING_LENGTH);
					}
					else
					{
						DEBUG_OUT( "Entry 'path' is not a string\n");
						rc = ERROR_CONFIG_FILE_PARAMETER;
						break;
					}

					/** OPTIONAL project.devices[i].description (string) */

					strResult = json_object_dotget_string(structObject, "description");

					if (strResult)
					{
						DEBUG_OUT( "DEVICE[%d]\tdescription[%s]\n",i,strResult);
						strncpy(pProject->mvbc[i].cDescription, strResult, MAX_STRING_LENGTH);
					}
					else
					{
						DEBUG_OUT( "'project.description' is not a string -> set default [%s]\n",MVBC_JSON_CONF_DEFAULT_DEVICE_DESCRIPTION);
						strncpy(pProject->mvbc[i].cDescription, MVBC_JSON_CONF_DEFAULT_DEVICE_DESCRIPTION, MAX_STRING_LENGTH);
				}

					/** MANDATORY project.devices[i].interface (string) */

					if (json_object_dothas_value_of_type(structObject, "interface", JSONString))
					{
						iLocalValue = validateInterface(json_object_dotget_string(structObject, "interface"));
						if (iLocalValue != -1)
						{
							pProject->mvbc[i].iInterface = iLocalValue;
							DEBUG_OUT( "DEVICE[%d]\tinterface[%d]\n",i,pProject->mvbc[i].iInterface);

							iLocalValue = -1;
						}
						else
						{
							DEBUG_OUT( "'interface' validation failed\n");
							rc = ERROR_CONFIG_FILE_PARAMETER;
							break;
						}
					}
					else
					{
						rc = ERROR_CONFIG_FILE_PARAMETER;
						DEBUG_OUT( "'interface' is not a string\n");
						break;
					}

					/** MANDATORY project.devices[i].device_addr (number) */
					if (json_object_dothas_value_of_type(structObject, "device_addr", JSONNumber))
					{
						iLocalValue = validateDeviceAddr((int)json_object_get_number(structObject, "device_addr"));
						if (iLocalValue != -1)
						{
							pProject->mvbc[i].iDeviceAddr = iLocalValue;
							DEBUG_OUT( "DEVICE[%d]\taddr[%d]\n",i, pProject->mvbc[i].iDeviceAddr);

							iLocalValue = -1;
						}
						else
						{
							DEBUG_OUT( "'addr' validation failed\n");
							rc = ERROR_CONFIG_FILE_PARAMETER;
							break;
						}
					}
					else
					{
						rc = ERROR_CONFIG_FILE_PARAMETER;
						DEBUG_OUT( "'device_addr' is not a number\n");
						break;
					}

					/** MANDATORY project.devices[i].mode (string) */

					if (json_object_dothas_value_of_type(structObject, "mode", JSONString))
					{
						iLocalValue = validateMode(json_object_dotget_string(structObject, "mode"));
						if (iLocalValue != -1)
						{
							pProject->mvbc[i].iMode = iLocalValue;
							DEBUG_OUT( "DEVICE[%d]\tmode[%d]\n",i,pProject->mvbc[i].iMode);

							iLocalValue = -1;
						}
						else
						{
							DEBUG_OUT( "'mode' validation failed\n");
							rc = ERROR_CONFIG_FILE_PARAMETER;
							break;
						}
					}
					else
					{
						rc = ERROR_CONFIG_FILE_PARAMETER;
						DEBUG_OUT( "'mode' is not a string\n");
						break;
					}

					/** OPTIONAL project.devices[i].traffic_memory (number) */
					if (json_object_dothas_value_of_type(structObject, "traffic_memory", JSONNumber))
					{
						pProject->mvbc[i].iTestTrafficMemory = (int)json_object_get_number(structObject, "traffic_memory");
						DEBUG_OUT( "DEVICE[%d]\ttraffic_memory[%d]\n",i, pProject->mvbc[i].iTestTrafficMemory);
					}
					else
					{
						DEBUG_OUT( "'traffic_memory' is not a number -> set default [%d]\n",MVBC_JSON_CONF_DEFAULT_DEVICE_MEMORY_TEST);
						pProject->mvbc[i].iTestTrafficMemory = MVBC_JSON_CONF_DEFAULT_DEVICE_MEMORY_TEST;
					}

					/* depending on device mode static/dynamic/combined -> parse config values */
					rc = parse_port_config(structObject,&pProject->mvbc[i].portSetup,pProject->mvbc[i].iMode);
				}
			}
		}
		else
		{
			DEBUG_OUT( "'devices' not found\n");
			rc = ERROR_CONFIG_FILE_PARAMETER;
		}
	}

	/* cleanup */
	json_value_free(rootValue);

	return rc;
}
