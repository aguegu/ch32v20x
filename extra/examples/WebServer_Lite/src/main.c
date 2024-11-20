#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "string.h"
#include "eth_driver.h"

u8 MACAddr[6];                                          //MAC address
u8 IPAddr[4] = { 192, 168, 0, 32 };                     //IP address
u8 GWIPAddr[4] = { 192, 168, 0, 1 };                    //Gateway IP address
u8 IPMask[4] = { 255, 255, 255, 0 };                    //subnet mask
u16 srcport = 80;                                     //source port

u8 SocketIdForListen;                                   //Socket for Listening
u8 SocketRecvBuf[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN];  //socket receive buffer

u8 requestBuffers[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN];
u16 requestBufferLengths[WCHNET_MAX_SOCKET_NUM];

extern unsigned char assets_index_html_gz[];
extern unsigned int assets_index_html_gz_len;

typedef struct {
  const char* path;
  void (*handler)(u8, const char* content);
} RouteHandler;

/*********************************************************************
 * @fn      mStopIfError
 *
 * @brief   check if error.
 *
 * @param   iError - error constants.
 *
 * @return  none
 */
void mStopIfError(u8 iError)
{
    if (iError == WCHNET_ERR_SUCCESS)
        return;
    printf("Error: %02X\r\n", (u16) iError);
}

/*********************************************************************
 * @fn      TIM2_Init
 *
 * @brief   Initializes TIM2.
 *
 * @return  none
 */
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = { 0 };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / 1000000;
    TIM_TimeBaseStructure.TIM_Prescaler = WCHNETTIMERPERIOD * 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/*********************************************************************
 * @fn      WCHNET_CreateTcpSocketListen
 *
 * @brief   Create TCP Socket for Listening
 *
 * @return  none
 */
void WCHNET_CreateTcpSocketListen(void)
{
    u8 i;
    SOCK_INF TmpSocketInf;

    memset((void *) &TmpSocketInf, 0, sizeof(SOCK_INF));
    TmpSocketInf.SourPort = srcport;
    TmpSocketInf.ProtoType = PROTO_TYPE_TCP;
    i = WCHNET_SocketCreat(&SocketIdForListen, &TmpSocketInf);
    printf("SocketIdForListen %d\r\n", SocketIdForListen);
    mStopIfError(i);
    i = WCHNET_SocketListen(SocketIdForListen);                   //listen for connections
    mStopIfError(i);
}

void dataSend(u8 id, uint8_t *dataptr, uint32_t datalen) {
  u32 len, totallen;
  u8 *p, timeout = 50;

  p = dataptr;
  totallen = datalen;

  while (totallen && timeout--) {
    len = totallen;
    WCHNET_SocketSend(id, p, &len);                         //Send the data
    totallen -= len;                                        //Subtract the sent length from the total length
    p += len;                                               //offset buffer pointer
  }
}

void textSend(u8 id, char * text) {
  dataSend(id, text, strlen(text));
}

void sendResponse(u8 socketid, uint16_t status, char * type, uint8_t *content) {
  textSend(socketid, "HTTP/1.1 ");

  switch (status) {
    case 200:
      textSend(socketid, "200 OK");
      break;
    case 204:
      textSend(socketid, "204 No Content");
      break;
    case 400:
      textSend(socketid, "400 Bad Request");
      break;
    case 404:
      textSend(socketid, "404 Not Found");
      break;
    case 405:
      textSend(socketid, "405 Method Not Allowed");
      break;
    case 413:
      textSend(socketid, "413 Request Entity Too Large");
      break;
  }

  textSend(socketid, "\r\nConnection: close\r\n");

  if (status == 204) {
    textSend(socketid, "\r\n");
  } else {
    if (type != NULL) {
      textSend(socketid, "Content-Type: ");
      textSend(socketid, type);
      textSend(socketid, "\r\n");
    }

    if (content) {
      textSend(socketid, "Content-Length: ");
      char s[6];
      sprintf(s, "%d", strlen(content));
      textSend(socketid, s);
      textSend(socketid, "\r\n\r\n");
      textSend(socketid, content);
    } else {
      textSend(socketid, "Content-Length: 0\r\n\r\n");
    }
  }


  WCHNET_SocketClose(socketid, TCP_CLOSE_NORMAL);
}

void getRoot(u8 socketid, const char * content) {
  textSend(socketid, "HTTP/1.1 302 Found\r\n");
  textSend(socketid, "Connection: close\r\n");
  textSend(socketid, "Content-Length: 0\r\n");
  textSend(socketid, "Location: /index.html\r\n");
  textSend(socketid, "\r\n");

  WCHNET_SocketClose(socketid, TCP_CLOSE_NORMAL);
}

void getIndex(u8 socketid, const char * content) {
  textSend(socketid, "HTTP/1.1 200 OK\r\n");
  textSend(socketid, "Connection: close\r\n");
  textSend(socketid, "Content-Encoding: gzip\r\n");
  textSend(socketid, "Content-Type: text/html\r\n");
  textSend(socketid, "Content-Length: ");
  char s[6];
  sprintf(s, "%d", assets_index_html_gz_len);
  textSend(socketid, s);
  textSend(socketid, "\r\n");

  textSend(socketid, "\r\n");
  dataSend(socketid, assets_index_html_gz, assets_index_html_gz_len);
  WCHNET_SocketClose(socketid, TCP_CLOSE_NORMAL);
}

void getAbout(u8 socketid, const char * content) {
  sendResponse(socketid, 200, "application/json", "{\"chip\":\"CH32V208WBU6\"}");
}

const static RouteHandler getHandlers[] = {
  { "/", getRoot },
  { "/index.html", getIndex },
  { "/api/about", getAbout },
  { NULL, NULL}  // End marker
};

void url_decode(char *str) {
  char *src = str, *dst = str;
  while (*src) {
      if (*src == '%') {
          if (isxdigit(src[1]) && isxdigit(src[2])) {
              char hex[3] = { src[1], src[2], '\0' };
              *dst++ = (char)strtol(hex, NULL, 16);
              src += 3;
          } else {
              // Invalid percent encoding, copy as is
              *dst++ = *src++;
          }
      } else if (*src == '+') {
          *dst++ = ' '; // Replace '+' with a space
          src++;
      } else {
          *dst++ = *src++; // Copy other characters
      }
  }
  *dst = '\0'; // Null-terminate the decoded string
}

void postEcho(u8 socketid, const char * content) {
  char *s2 = strdup(content);
  char *pair = strtok(s2, "&");
  char body[RECE_BUF_LEN] = "{";
  uint16_t pairIndex = 0;
  while (pair) {
    char * key = pair;
    char * value = strchr(pair, '=');

    if (key) {
      url_decode(key); // Decode the key
    }

    if (value) {
      *value = '\0'; // Null-terminate the key
      value++;       // Move to the value part
      url_decode(value); // Decode the value
    }

    if (pairIndex) {
      strcat(body, ",");
    }

    strcat(body, "\"");
    strcat(body, key);
    strcat(body, "\":\"");
    strcat(body, value);
    strcat(body, "\"");
    pair = strtok(NULL, "&");
    pairIndex++;
  }

  strcat(body, "}");

  sendResponse(socketid, 200, "application/json", body);
  free(s2);
}

const static RouteHandler postHandlers[] = {
  { "/api/echo", postEcho },
  { NULL, NULL }
};

void httpResponse(u8 socketid) {
  char method[16], path[256], version[16];
  char *headers, *line, *statusEnd, *headersEnd;
  bool isHandled = false;

  const char *request = requestBuffers[socketid];

  statusEnd = strstr(request, "\r\n");

  if (!statusEnd) {
    return;
  }

  headersEnd = strstr(request, "\r\n\r\n");

  if (!headersEnd) {  // headers not completed yet
    return;
  }

  sscanf(request, "%15s %255s %15s", method, path, version);
  // printf("HTTP Method: %s, PATH: %s, Version: %s\r\n", method, path, version);
  
  headers = strstr(request, "\r\n");
  if (headers) {
    headers += 2; // Move past "\r\n"
  } else {
    isHandled = true;
    return sendResponse(socketid, 400, NULL, NULL);
  }

  u16 contentLength = 0;

  // printf("Headers:\n");
  line = strtok(headers, "\r\n"); // strtok would change the end of token to \0 and skip empty token
  while (line && line < headersEnd) {
    // printf("  %s\r\n", line);
    if (!strncmp(line, "Content-Length:", 15)) {
      contentLength = atoi(line + 15);
    }
    *(line + strlen(line)) = '\r';  // restore end of the token changed by strtok
    line = strtok(NULL, "\r\n");
  }

  printf("request length received: %d, expected: %d\r\n", requestBufferLengths[socketid], headersEnd - request + 4 + contentLength);
  if (requestBufferLengths[socketid] < headersEnd - request + 4 + contentLength) {  // content not completed yet
    return;
  }

  if (strcmp(version, "HTTP/1.1")) {
    isHandled = true;
    return sendResponse(socketid, 400, NULL, NULL);
  }

  if (!strcmp(method, "GET")) {
    for (uint8_t i = 0; getHandlers[i].path != NULL; i++) {
      if (!strcmp(path, getHandlers[i].path)) {
        getHandlers[i].handler(socketid, NULL);
        isHandled = true;
        break;
      }
    }
  } else if (!strcmp(method, "POST")) {
    for (uint8_t i = 0; postHandlers[i].path != NULL; i++) {
      if (!strcmp(path, postHandlers[i].path)) {
        postHandlers[i].handler(socketid, headersEnd + 4);
        isHandled = true;
        break;
      }
    }
  } else {
    isHandled = true;
    return sendResponse(socketid, 405, NULL, NULL);
  }

  if (!isHandled) {
    return sendResponse(socketid, 404, NULL, NULL);
  }
}

void WCHNET_HandleSockInt(u8 socketid, u8 intstat) {
  u32 len;
  if (intstat & SINT_STAT_RECV) {                                 //receive data
    len = WCHNET_SocketRecvLen(socketid, NULL);

    // printf("SourPort: %d, DesPort: %d\r\n", SocketInf[socketid].SourPort, SocketInf[socketid].DesPort);
    printf("socketid:%d Received %ld bytes\r\n", socketid, len);

    if (requestBufferLengths[socketid] > RECE_BUF_LEN) {
      WCHNET_SocketRecv(socketid, NULL, &len);
      WCHNET_SocketClose(socketid, TCP_CLOSE_ABANDON);
      requestBufferLengths[socketid] += len;
      printf("dump request buffer left, %ld received\r\n", requestBufferLengths[socketid]);
    } else if (requestBufferLengths[socketid] + len > RECE_BUF_LEN) {
      requestBufferLengths[socketid] += len;
      WCHNET_SocketRecv(socketid, NULL, &len);
      sendResponse(socketid, 413, NULL, NULL);
      printf("413 sent\r\n");
    } else {
      WCHNET_SocketRecv(socketid, requestBuffers[socketid] + requestBufferLengths[socketid], &len);
      requestBufferLengths[socketid] += len;
      printf("received: %d/%d\r\n", requestBufferLengths[socketid], RECE_BUF_LEN);
      requestBuffers[socketid][requestBufferLengths[socketid]] = 0;
      httpResponse(socketid);
    }
  }
  if (intstat & SINT_STAT_CONNECT)                              //connect successfully
  {
    WCHNET_ModifyRecvBuf(socketid, (u32) SocketRecvBuf[socketid], RECE_BUF_LEN);
    printf("TCP Connect Success: %d\r\n", socketid);
    requestBufferLengths[socketid] = 0;
  }
  if (intstat & SINT_STAT_DISCONNECT)                           //disconnect
  {
    printf("TCP Disconnect: %d\r\n", socketid);
  }
  if (intstat & SINT_STAT_TIM_OUT)                              //timeout disconnect
  {
      printf("TCP Timeout: %d\r\n", socketid);
  }
}

void WCHNET_HandleGlobalInt(void) {
  u8 intstat;
  u16 i;
  u8 socketint;

  intstat = WCHNET_GetGlobalInt();                              //get global interrupt flag
  if (intstat & GINT_STAT_UNREACH)                              //Unreachable interrupt
  {
      printf("GINT_STAT_UNREACH\r\n");
  }
  if (intstat & GINT_STAT_IP_CONFLI)                            //IP conflict
  {
      printf("GINT_STAT_IP_CONFLI\r\n");
  }
  if (intstat & GINT_STAT_PHY_CHANGE)                           //PHY status change
  {
      i = WCHNET_GetPHYStatus();
      if (i & PHY_Linked_Status)
          printf("PHY Link Success\r\n");
  }
  if (intstat & GINT_STAT_SOCKET) {                             //socket related interrupt
      for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
          socketint = WCHNET_GetSocketInt(i);
          if (socketint)
              WCHNET_HandleSockInt(i, socketint);
      }
  }
}

int main(void) {
    u8 i;
    Delay_Init();
    USART_Printf_Init(115200);                                    //USART initialize
    printf("TCPServer Test\r\n");
    if((SystemCoreClock == 60000000) || (SystemCoreClock == 120000000))
        printf("SystemClk:%d\r\n", SystemCoreClock);
    else
        printf("Error: Please choose 60MHz and 120MHz clock when using Ethernet!\r\n");
    printf("net version:%x\n", WCHNET_GetVer());
    if (WCHNET_LIB_VER != WCHNET_GetVer()) {
        printf("version error.\n");
    }
    WCHNET_GetMacAddr(MACAddr);                                   //get the chip MAC address
    printf("mac addr:");
    for(i = 0; i < 6; i++)
        printf("%x ",MACAddr[i]);
    printf("\n");
    TIM2_Init();
    i = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);           //Ethernet library initialize
    mStopIfError(i);
    if (i == WCHNET_ERR_SUCCESS)
        printf("WCHNET_LibInit Success\r\n");

    WCHNET_CreateTcpSocketListen();                               //Create TCP Socket for Listening

    while(1)
    {
        /*Ethernet library main task function,
         * which needs to be called cyclically*/
        WCHNET_MainTask();
        /*Query the Ethernet global interrupt,
         * if there is an interrupt, call the global interrupt handler*/
        if(WCHNET_QueryGlobalInt())
        {
            WCHNET_HandleGlobalInt();
        }
    }
}
