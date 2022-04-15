/*
 * web.c
 *
 *  Created on: 4 set. 2020
 *      Author: Renato
 */

#include "main.h"
//#include "esp_http_client.h"

#define WEB_SERVER "example.com"
#define WEB_PORT 80
#define WEB_URL "http://example.com/"

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";


void Web_Task(void *arg){

	while(1){
		const struct addrinfo hints = {
				.ai_family = AF_INET,
				.ai_socktype = SOCK_STREAM,
			};
		struct addrinfo *res;
		struct in_addr *addr;
		int s, r;
		char recv_buf[64]={0};

		int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);
		if(err != 0 || res == NULL) {
			Debug("DNS lookup failed err=%d res=%p", err, res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		/* Code to print the resolved IP.

		Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		Debug("DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

		if(socket(res->ai_family, res->ai_socktype, 0) < 0) {
			Debug("... Failed to allocate socket.");
			freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		Debug("... allocated socket");

		if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
			Debug("... socket connect failed errno=%d", errno);
			close(s);
			freeaddrinfo(res);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}

		Debug("... connected");
		freeaddrinfo(res);

		if (write(s, REQUEST, strlen(REQUEST)) < 0) {
			Debug("... socket send failed");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		Debug("... socket send success");

		struct timeval receiving_timeout;
		receiving_timeout.tv_sec = 5;
		receiving_timeout.tv_usec = 0;
		if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
				sizeof(receiving_timeout)) < 0) {
			Debug("... failed to set socket receiving timeout");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		Debug("... set socket receiving timeout success");

		/* Read HTTP response */
		do {
			bzero(recv_buf, sizeof(recv_buf));
			r = read(s, recv_buf, sizeof(recv_buf)-1);
			for(int i = 0; i < r; i++) {
				putchar(recv_buf[i]);
			}
		} while(r > 0);

		Debug("... done reading from socket. Last read return=%d errno=%d.", r, errno);
		close(s);
		for(int countdown = 10; countdown >= 0; countdown--) {
			Debug("%d... ", countdown);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		Debug("Starting again!");
	}
}

