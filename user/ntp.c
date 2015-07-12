//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// richardaburton@gmail.com
//////////////////////////////////////////////////

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <osapi.h>
#include <mem.h>

#include "ntp.h"

// list of major public servers http://tf.nist.gov/tf-cgi/servers.cgi
uint8 ntp_server[] = {131, 107, 13, 100}; // microsoft

static time_t timestamp = 0;
time_t last_ntp_update = 1432390000;

static os_timer_t ntp_timeout;
static os_timer_t tic;
static struct espconn *pCon = 0;

void print(const char*);

static void ICACHE_FLASH_ATTR ntp_udp_timeout(void *arg) {
	os_timer_disarm(&ntp_timeout);
	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
}

time_t ICACHE_FLASH_ATTR gettime() {
    return timestamp;
}

static void ICACHE_FLASH_ATTR tictoc(void *arg) {
    timestamp++;
}

static void ICACHE_FLASH_ATTR ntp_udp_recv(void *arg, char *pdata, unsigned short len) {
	
	ntp_t *ntp;

	os_timer_disarm(&ntp_timeout);
	os_timer_disarm(&tic);

	// extract ntp time
	ntp = (ntp_t*)pdata;
    time_t ts = ntp->trans_time[0] << 24 | ntp->trans_time[1] << 16 |ntp->trans_time[2] << 8 | ntp->trans_time[3];
	// convert to unix time
	ts -= (2208988800UL - 7200UL);

    int diff = timestamp - ts;
    os_printf("Time slew: %d\n", diff);
    timestamp = ts;
    last_ntp_update = ts;

	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
	os_timer_arm(&tic, 1000, 1);
}



void ICACHE_FLASH_ATTR ntp_get_time() {

	ntp_t ntp;

	// set up the udp "connection"
	pCon = (struct espconn*)os_zalloc(sizeof(struct espconn));
	pCon->type = ESPCONN_UDP;
	pCon->state = ESPCONN_NONE;
	pCon->proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
	pCon->proto.udp->local_port = espconn_port();
	pCon->proto.udp->remote_port = 123;
	os_memcpy(pCon->proto.udp->remote_ip, ntp_server, 4);

	// create a really simple ntp request packet
	os_memset(&ntp, 0, sizeof(ntp_t));
	ntp.options = 0b00100011; // leap = 0, version = 4, mode = 3 (client)

	// set timeout timer
	os_timer_disarm(&ntp_timeout);
	os_timer_setfn(&ntp_timeout, (os_timer_func_t*)ntp_udp_timeout, pCon);
	os_timer_arm(&ntp_timeout, NTP_TIMEOUT_MS, 0);

	// send the ntp request
	espconn_create(pCon);
	espconn_regist_recvcb(pCon, ntp_udp_recv);
	espconn_sent(pCon, (uint8*)&ntp, sizeof(ntp_t));

    os_timer_disarm(&tic);
	os_timer_setfn(&tic, (os_timer_func_t*)tictoc, pCon);
}
