#include "uplink.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "os_type.h"
#include "user_interface.h"


#include "c_types.h"
#include "user_interface.h"
#include "mem.h"
#include "osapi.h"
#include "upgrade.h"
#include "espconn.h" 

static struct espconn uplink_conn;
static esp_tcp uplink_tcp_conn;
static void uplink_connectedCb(void *arg);
static void uplink_disconCb(void *arg);
static void uplink_reconCb(void *arg, sint8 err);
static void uplink_recvCb(void *arg, char *data, unsigned short len);
static void uplink_sentCb(void *arg);
const char* esp_errstr(sint8 err);
void uplink_start() {
}

void uplink_stop() {
}

void uplink_init() {
  uplink_conn.type=ESPCONN_TCP;
  uplink_conn.state=ESPCONN_NONE;
  uplink_conn.proto.tcp=&uplink_tcp_conn;
  uplink_conn.proto.tcp->local_port=espconn_port();
  uplink_conn.proto.tcp->remote_port=80;

  uint32_t ip = ipaddr_addr("192.168.0.99");
  os_memcpy(uplink_conn.proto.tcp->remote_ip, &ip, 4);

  espconn_regist_connectcb(&uplink_conn, uplink_connectedCb);
  espconn_regist_disconcb(&uplink_conn, uplink_disconCb);
  espconn_regist_reconcb(&uplink_conn, uplink_reconCb);
  espconn_regist_recvcb(&uplink_conn, uplink_recvCb);
  espconn_regist_sentcb(&uplink_conn, uplink_sentCb);
  espconn_connect(&uplink_conn);
}

static void ICACHE_FLASH_ATTR uplink_sentCb(void *arg) {
  print("sent");
}

static void ICACHE_FLASH_ATTR uplink_recvCb(void *arg, char *data, unsigned short len) {
  print("recv");

  struct espconn *conn=(struct espconn *)arg;
  int x;
  uart0_tx_buffer(data,len);
}

static void ICACHE_FLASH_ATTR uplink_connectedCb(void *arg) {

  print("conn");
  struct espconn *conn=(struct espconn *)arg;

  char *data = "GET / HTTP/1.0\r\n\r\n\r\n";
  sint8 d = espconn_sent(conn,data,strlen(data));

  espconn_regist_recvcb(conn, uplink_recvCb);
  print("cend");
}

static void ICACHE_FLASH_ATTR uplink_reconCb(void *arg, sint8 err) {
  print("rcon");
}

static void ICACHE_FLASH_ATTR uplink_disconCb(void *arg) {
  print("dcon");
}



const char* esp_errstr(sint8 err) {
    switch(err) {
        case ESPCONN_OK:
            return "No error, everything OK.";
        case ESPCONN_MEM:
            return "Out of memory error.";
        case ESPCONN_TIMEOUT:
            return "Timeout.";
        case ESPCONN_RTE:
            return "Routing problem.";
        case ESPCONN_INPROGRESS:
            return    "Operation in progress";
        case ESPCONN_ABRT:
            return    "Connection aborted.";
        case ESPCONN_RST:
            return    "Connection reset.";
        case ESPCONN_CLSD:
            return   "Connection closed.";
        case ESPCONN_CONN:
            return   "Not connected.";
        case ESPCONN_ARG:
            return   "Illegal argument.";
        case ESPCONN_ISCONN:
            return   "Already connected.";
    }
}