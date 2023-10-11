/**
 * @file
 */

#include <time.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

#include <mvbc_app_interface.h>

typedef __u64	SC_ADDR;
typedef __u16	SC_UINT;
typedef __u16	SC_WORD;
typedef __u64	SC_DWORD;

#define DEFAULT_MVB_DEVICE	"/dev/mvbc0"

#define MAX_PORT_DATA_LENGTH 16

struct sPortData
{
	SC_WORD  wPortAddr;		// MVB Port Address
	SC_WORD  wPortType;		// LA, DA, PP type
	SC_WORD  wNumOfWords;
	SC_WORD  wTACK;
	struct timeval sTimeStamp;
	SC_WORD  wPortData[MAX_PORT_DATA_LENGTH];
}__attribute__((packed));

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
	int pMvbFile = -1;
	char Dev_Name[20] = "n/a";
	char MVBC_Dev[20] = "n/a";
	struct pollfd pollDesc;

    printf("MVBC Lib Read Test\n");

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

	/* Open MVB Device File */
	pMvbFile = open(MVBC_Dev, O_RDWR);
	/* Check if file opened successfully */
	if (pMvbFile < 0)
	{
		fprintf(stderr, "unable to open file %d", pMvbFile);
		return 0;
	}

	/* Init POLL structure */
	pollDesc.fd = pMvbFile;
	pollDesc.events = POLLIN;
	pollDesc.revents = 0;

	printf("\n** poll FIFO **\n");

	while (1)
	{
		// Poll with 1 second timeout
		int rc = poll(&pollDesc, 1, 10);

		if (rc > 0)
		{
			if (pollDesc.revents & POLLIN)
			{
				// Read everything
				struct sPortData data;

				// We have set up non blocking read,
				// to be able to end the thread.
				// So it is possible that read returns -1
				int count = read(pollDesc.fd, &data,
						sizeof(struct sPortData));

				if (count == sizeof(struct sPortData))
				{
					printf("ADDR[%d] TYPE[%d] NR_WORDS[%d] TACK[0x%X] TIME[%ld.%ld]\n",
							data.wPortAddr,
							data.wPortType,
							data.wNumOfWords,
							data.wTACK,
							data.sTimeStamp.tv_sec,
							data.sTimeStamp.tv_usec);

					for(int i = 0; i < data.wNumOfWords; i++)
					{
						printf("\tDATA_%d [0x%X]\n",i,data.wPortData[i]);
					}
					printf("***********************\n");
				}
				else if (count > 0)
				{
					printf("\t\t\tCount: %d\n", count);
				}
			}
			else if (pollDesc.revents & POLLHUP)
			{
				puts("hup");
				close(pollDesc.fd);
			}
		}
	}

	// call mvbc_release
	close(pMvbFile);

	return rc;
}
