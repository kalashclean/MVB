/**
 * @file
 *
 *  Created on: 22.05.2019
 *      Author: vmishkin
 */

#ifndef PACKAGE_SYSTEM_MVBC_LIB_SRC_INCLUDE_MVBC_APP_INTERFACE_H_
#define PACKAGE_SYSTEM_MVBC_LIB_SRC_INCLUDE_MVBC_APP_INTERFACE_H_

/** Get revision information */
int mvbc_get_library_version(int *major, int *minor, int* patch);
int mvbc_get_pld_firmware_version(int *version);

/**
 *
 * Setup MVBC('s) from project configuration file
 *
 * Parse configuration file
 * 	-> Reset MVBC('s)
 * 	 -> Execute RAM Test
 * 	  -> Configure MVBC('s)
 * 	   -> Configure Ports
 * 	    -> Run MVBC
 *
 * @param config_file
 * @return 0 in case of success, errorMask for error
 */
int mvbc_init(const char *config_file);

/**
 * Shutdown specified MVBC device, cleanup configured ports, reset register values.
 *
 * @param *dev
 * @return 0 in case of success, -1 for error
 */
int mvbc_shutdown(const char *dev);


#endif /* PACKAGE_SYSTEM_MVBC_LIB_SRC_INCLUDE_MVBC_APP_INTERFACE_H_ */
