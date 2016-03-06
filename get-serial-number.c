#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>

#include "model_numbers.h"


int get_sn(char *iface, int model_id, unsigned int *si);
int check_model_name(char *m, int *id);
int write_hostname(const char *hn);
int write_machine_id(unsigned int id);
int sn2hostname(int sn, char *hostname);

static struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "write-files", no_argument, NULL, 'w' },
	{ "interface", required_argument, NULL, 'i' },
	{ "model", required_argument, NULL, 'm' },
	{ NULL, 0, 0, 0 }
};


int main(int argc, char **argv)
{
	unsigned int sn = 0;
	int n = 0;
	char iface[25] = {0};
	char model[10] = {0};
	int write_files = 0;

	while (n >= 0) {
		n = getopt_long(argc, argv, "hwm:i:", options, NULL);
		if (argc > 1 && n < 0)
			continue;
		switch (n) {
		case 'i':
			strncpy(iface, optarg, sizeof(iface));
			iface[sizeof(iface) - 1] = '\0';
			break;
		case 'm':
			strncpy(model, optarg, sizeof(model));
			model[sizeof(model) - 1] = '\0';
			break;
		case 'w':
			write_files = 1;
			break;
		case 'h':
		default:
			fprintf(stderr,	"Usage: " PROGNAME " <--interface <if>> <--model <model> [--write-files]\n");
			exit(EXIT_FAILURE);
		}
	}

	if (!*iface) {
		fprintf(stderr, "No interface specified\n");
		exit(EXIT_FAILURE);
	}

	if (!*model) {
		fprintf(stderr, "No model specified\n");
		exit(EXIT_FAILURE);
	}

	int model_number = 0;
	if (0 != check_model_name(model, &model_number)) {
		fprintf(stderr, "Model %s not recognised\n", model);
		exit(EXIT_FAILURE);
	}

	if (0 != get_sn(iface, model_number, &sn)) {
		perror("Couldn't get serial number.");
		exit(EXIT_FAILURE);
	}

	char hostname[25];
	sn2hostname(sn, hostname);

	if (write_files) {
		if (0 != write_machine_id(sn)) {
			fprintf(stderr, "Couldn't write machine id\n");
			exit(EXIT_FAILURE);
		}
		if (0 != write_hostname(hostname)) {
			fprintf(stderr, "Couldn't write host name\n");
			exit(EXIT_FAILURE);
		}
	}


	printf("Model: %s\n", model);
	printf("Machine ID: %032d\n", sn);
	printf("Hostname: %s\n", hostname);

	return 0;
}

int sn2hostname(int sn, char *hostname)
{
	assert(hostname);
	sprintf(hostname, "%s-%d", "bufferupr", sn);
	return 0;
}

int check_model_name(char *m, int *number)
{
	struct model *mt = model_n2n;
	while (mt->number) {
		if (0 == strcmp(m, mt->name)) {
			/* found match. */
			*number = mt->number;
			return 0;
		}
		mt++;
	}
	return -1;
}

int check_model_number(int m)
{
	switch(m) {
	case TEST:
	case APU1:
	case APU2:
		return 0;
	default:
		return -1;
	}
}

int write_hostname(const char *hn)
{
	FILE *f = fopen("/etc/hostname", "w+");
	if (!f) {
		perror("Couldn't open hostname file.");
		return -1;
	}
	if(0 > fprintf(f, "%s\n", hn)) {
		perror("Couldn't write id.");
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

int write_machine_id(unsigned int id)
{
	FILE *f = fopen("/etc/machine-id", "w+");
	if (!f) {
		perror("Couldn't open machine-id file.");
		return -1;
	}
	if(0 > fprintf(f, "%032d\n", id)) {
		perror("Couldn't write id.");
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

int get_sn(char *iface, int model_id, unsigned int *si)
{
	unsigned char mac_address[6];
	unsigned char sn[4] = {0};
	struct ifreq ifr;

	assert(iface);
	assert(si);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		fprintf(stderr, "couldn't create socket\n");
		return -1;
	};

	strcpy(ifr.ifr_name, iface);
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
		fprintf(stderr, "couldn't ioctl iface: %s\n",
		        ifr.ifr_name);
		return -1;
	}

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
		memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
		memcpy(sn, &mac_address[3], 3);

		*si = sn[2] + 256 * sn[1] + 65536 * sn[0];
		*si = *si / 4 - 64;
		*si = *si + model_id;
		return 0;
	}
	return -1;
}
