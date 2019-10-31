#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "coap-log.h"
#include "sys/rtimer.h"

#define LOG_MODULE "App"
#define LOG_LEVEL  LOG_LEVEL_APP
#define SERVER_EP "coap://[fd00::212:4b00:14b5:d8fb]"

PROCESS(er_example_client, "Erbium Example Client");
AUTOSTART_PROCESSES(&er_example_client);

static struct etimer et;
static rtimer_clock_t start_count, end_count;
static rtimer_clock_t start_all, end_all;
int m; //messages counter

char *service_urls[1] = { "test/hello", "test/energy" };

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    puts("Request timed out");
    return;
  }
  int len = coap_get_payload(response, &chunk);
  printf("|%.*s", len, (char *)chunk);
}

PROCESS_THREAD(er_example_client, ev, data) {
  static coap_endpoint_t server_ep;
  PROCESS_BEGIN();
  static coap_message_t request[1];
  coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
  etimer_set(&et, 2 * CLOCK_SECOND);
  m = 1;
  LOG_INFO_("START \n");

  // test/energy
  coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
  coap_set_header_uri_path(request, service_urls[0]);
  LOG_INFO_COAP_EP(&server_ep);
  COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

  start_all = RTIMER_NOW();
  while(m <= 100) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      start_count = RTIMER_NOW();
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[0]);
      LOG_INFO_COAP_EP(&server_ep);
      COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
      end_count = RTIMER_NOW();
      printf(" | message #: %d ", m);
      printf("| start_count %lu ", (unsigned long)(start_count));
      printf("| end_count %lu ", (unsigned long)(end_count));
      printf("| elapsed %lu \n", (unsigned long)(end_count - start_count));
      etimer_reset(&et);
      m += 1;
    }
  }
  end_all = RTIMER_NOW();
  printf("| start_all %lu ", (unsigned long)(start_all));
  printf("| end_all %lu ", (unsigned long)(end_all));
  printf("| elapsed_all %lu \n", (unsigned long)(end_all - start_all));
  LOG_INFO_("STOP \n");
  PROCESS_END();
}
