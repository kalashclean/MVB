/**
 * @file
 */

#include <stdio.h>
#include <string.h>
#include "mvbc_app_interface.h"

#define DEFAULT_MVB_DEVICE	"/dev/mvbc0"

/**
 * Main entry for test application
 *
 * @param argc
 * @param argv
 *
 * @return 0
 */

int main(int argc, char* argv[])
{
	int rc = -1;
    int major, minor, patch;
    int pld_firmware_version;
	char Dev_Name[20] = "n/a";
	char MVBC_Dev[20] = "n/a";

    printf("MVBC Lib Test\n");

    rc = mvbc_get_library_version(&major, &minor, &patch);
    printf("Library version: %d.%d.%d RC[%d]\n", major, minor, patch,rc);

    rc = mvbc_get_pld_firmware_version(&pld_firmware_version);
    printf("PLD firmware version: %d RC[%d]\n", pld_firmware_version,rc);

	if (argc > 1)
	{
		//DevName
		strcpy(Dev_Name, argv[1]);
	}

	if (strcmp(Dev_Name, "n/a") == 0)
	{
		strcpy(MVBC_Dev, DEFAULT_MVB_DEVICE);
	}
	else
	{
		strcpy(MVBC_Dev, Dev_Name);
	}

	rc = mvbc_shutdown(MVBC_Dev);
    printf("shutdown device[%s] rc[%X]\n",MVBC_Dev,rc);

    return rc;
}
