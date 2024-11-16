/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/05/31
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/*
 *@Note
TCP Server example, demonstrating that TCP Server
receives data and sends back after connecting to a client.
For details on the selection of engineering chips,
please refer to the "CH32V20x Evaluation Board Manual" under the CH32V20xEVT\EVT\PUB folder.
 */
#include "string.h"
#include "eth_driver.h"

u8 MACAddr[6];                                          //MAC address
u8 IPAddr[4] = { 192, 168, 0, 32 };                     //IP address
u8 GWIPAddr[4] = { 192, 168, 0, 1 };                    //Gateway IP address
u8 IPMask[4] = { 255, 255, 255, 0 };                    //subnet mask
u16 srcport = 80;                                     //source port

u8 SocketIdForListen;                                   //Socket for Listening
u8 SocketRecvBuf[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN];  //socket receive buffer
u8 requestBuffer[RECE_BUF_LEN];

extern unsigned char assets_index_html[];
extern unsigned char assets_index_html_gz[];
extern unsigned int assets_index_html_gz_len;

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
  }

  textSend(socketid, "\r\n");
  textSend(socketid, "Connection: close\r\n");

  if (status != 204) {
    if (type != NULL) {
      textSend(socketid, "content-type: ");
      textSend(socketid, type);
      textSend(socketid, "\r\n");
    }

    textSend(socketid, "content-encoding: gzip\r\n");

    if (content) {
      // textSend(socketid, "Content-Length: ");
      // char s[8];
      // memset(s, 0, 8);
      // sprintf(s, "%d", strlen(content));
      // textSend(socketid, s);
      // textSend(socketid, "\r\n");
      //
      // textSend(socketid, "\r\n");
      // textSend(socketid, content);
      // dataSend(id, content, ;
      // textSend(socketid, "\r\n");

      textSend(socketid, "content-length: ");
      char s[8];
      memset(s, 0, 8);
      sprintf(s, "%d", assets_index_html_gz_len);
      textSend(socketid, s);
      textSend(socketid, "\r\n");

      textSend(socketid, "\r\n");
      dataSend(socketid, content, assets_index_html_gz_len);
      textSend(socketid, "\r\n");
    } else {
      textSend(socketid, "Content-Length: 0\r\n");
    }
  }

    // textSend(socketid, "Content-Type: application/json\r\n");


  // textSend(socketid, "200 OK\r\n");

  // textSend(socketid, "Content-Length: 2\r\n");
  // textSend(socketid, "\r\n");
  // textSend(socketid, "{}");


  textSend(socketid, "\r\n");

  WCHNET_SocketClose(socketid, TCP_CLOSE_NORMAL);
}

void httpResponse(u8 socketid, const char *request) {
    char method[16], url[256], version[16];
    char *headers, *line;

    // Parse the request line (method, URL, version)
    sscanf(request, "%15s %255s %15s", method, url, version);

    printf("HTTP Method: %s\n", method);
    printf("URL: %s\n", url);
    printf("Version: %s\n", version);

    // Find the headers section
    headers = strstr(request, "\r\n");
    if (headers) {
      headers += 2; // Move past "\r\n"
    } else {
      return sendResponse(socketid, 400, NULL, NULL);
    }

    printf("Headers:\n");
    // Parse each header line
    line = strtok(headers, "\r\n");
    while (line) {
      printf("  %s\n", line);
      line = strtok(NULL, "\r\n");
    }

    if (strcmp(method, "GET")) {
      return sendResponse(socketid, 405, NULL, NULL);
    }

    // sendResponse(socketid, 204, NULL, NULL);
    // sendResponse(socketid, 200, "application/json", "{\"hello\": \"world\"}");
    sendResponse(socketid, 200, "text/html", assets_index_html_gz);
}

void WCHNET_HandleSockInt(u8 socketid, u8 intstat)
{
  u32 len;
    if (intstat & SINT_STAT_RECV)                                 //receive data
    {
        // WCHNET_DataLoopback(socketid);                            //Data loopback
        len = WCHNET_SocketRecvLen(socketid, NULL);
        WCHNET_SocketRecv(socketid, requestBuffer, &len);
        printf("socketid:%d Received data length: %d\r\n", socketid, len);
        // printf("SourPort: %d, DesPort: %d\r\n", SocketInf[socketid].SourPort, SocketInf[socketid].DesPort);
        // for (uint16_t i = 0; i < len; i++) {
        //   printf("%02x ", requestBuffer[i]);
        // }
        requestBuffer[len] = '\0';
        httpResponse(socketid, requestBuffer);
    }
    if (intstat & SINT_STAT_CONNECT)                              //connect successfully
    {
        WCHNET_ModifyRecvBuf(socketid, (u32) SocketRecvBuf[socketid], RECE_BUF_LEN);
        printf("TCP Connect Success\r\n");
        printf("socket id: %d\r\n", socketid);
    }
    if (intstat & SINT_STAT_DISCONNECT)                           //disconnect
    {
        printf("TCP Disconnect\r\n");
    }
    if (intstat & SINT_STAT_TIM_OUT)                              //timeout disconnect
    {
        printf("TCP Timeout\r\n");
    }
}

/*********************************************************************
 * @fn      WCHNET_HandleGlobalInt
 *
 * @brief   Global Interrupt Handle
 *
 * @return  none
 */
void WCHNET_HandleGlobalInt(void)
{
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

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program
 *
 * @return  none
 */
int main(void)
{
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
