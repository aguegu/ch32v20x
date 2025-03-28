#include "string.h"
#include "eth_driver.h"

// LED Link: PC0  Orange ELED2
// LED Data: PC1  Green  ELED1

#define KEEPALIVE_ENABLE                0                //Enable keep alive function

u8 MACAddr[6];                                          //MAC address
u8 IPAddr[4] = { 192, 168, 0, 32 };                     //IP address
u8 GWIPAddr[4] = { 192, 168, 0, 1 };                    //Gateway IP address
u8 IPMask[4] = { 255, 255, 255, 0 };                    //subnet mask
u16 srcport = 80;                                     //source port

u8 SocketIdForListen;                                   //Socket for Listening
u8 socket[WCHNET_MAX_SOCKET_NUM];                       //Save the currently connected socket
u8 SocketRecvBuf[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN];  //socket receive buffer
u8 MyBuf[RECE_BUF_LEN];

void mStopIfError(u8 iError) {
  if (iError == WCHNET_ERR_SUCCESS)
    return;
  printf("Error: %02X\r\n", (u16) iError);
}

void TIM2_Init(void) {
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

void WCHNET_CreateTcpSocketListen(void) {
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

void WCHNET_DataLoopback(u8 id) {
  u32 len;
  u32 endAddr = SocketInf[id].RecvStartPoint + SocketInf[id].RecvBufLen;       //Receive buffer end address

  // printf("%d: RecvStartPoint: %lx, RecvReadPoint: %lx, RecvBufLen: %lx, RecvRemLen: %lx \r\n", id, SocketInf[id].RecvStartPoint, SocketInf[id].RecvReadPoint, SocketInf[id].RecvBufLen, SocketInf[id].RecvRemLen);

  if ((SocketInf[id].RecvReadPoint + SocketInf[id].RecvRemLen) > endAddr) {    //Calculate the length of the received data
      len = endAddr - SocketInf[id].RecvReadPoint;
  } else {
      len = SocketInf[id].RecvRemLen;
  }
  uint8_t i = WCHNET_SocketSend(id, (u8 *) SocketInf[id].RecvReadPoint, &len);        //send data

  if (i == WCHNET_ERR_SUCCESS) {
    printf("%d: RecvReadPoint: %lx, RecvRemLen: %lx\r\n", id, SocketInf[id].RecvReadPoint, SocketInf[id].RecvRemLen);
    WCHNET_SocketRecv(id, NULL, &len);                                      //Clear SocketInf[id].RecvRemLen
  }
}

void WCHNET_HandleSockInt(u8 socketid, u8 intstat) {
  if (intstat & SINT_STAT_RECV) {                                 //receive data
    WCHNET_DataLoopback(socketid);                            //Data loopback
  }
  if (intstat & SINT_STAT_CONNECT) {                              //connect successfully

  #if KEEPALIVE_ENABLE
    WCHNET_SocketSetKeepLive(socketid, ENABLE);
  #endif
    WCHNET_ModifyRecvBuf(socketid, (u32) SocketRecvBuf[socketid], RECE_BUF_LEN);
    for (uint8_t i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
      if (socket[i] == 0xff) {                              //save connected socket id
        socket[i] = socketid;
        break;
      }
    }
    printf("TCP Connect Success\r\n");
    printf("socket id: %d\r\n", socketid);
  }

  if (intstat & SINT_STAT_DISCONNECT) {                           //disconnect
    for (uint8_t i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {             //delete disconnected socket id
      if (socket[i] == socketid) {
        socket[i] = 0xff;
        break;
      }
    }
    printf("TCP Disconnect\r\n");
  }

  if (intstat & SINT_STAT_TIM_OUT) {                             //timeout disconnect
    for (uint8_t i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {             //delete disconnected socket id
      if (socket[i] == socketid) {
        socket[i] = 0xff;
        break;
      }
    }
    printf("TCP Timeout\r\n");
  }
}

void WCHNET_HandleGlobalInt(void) {
  uint8_t intstat = WCHNET_GetGlobalInt();                              //get global interrupt flag
  if (intstat & GINT_STAT_UNREACH) {                              //Unreachable interrupt
    printf("GINT_STAT_UNREACH\r\n");
  }
  if (intstat & GINT_STAT_IP_CONFLI) {                            //IP conflict
    printf("GINT_STAT_IP_CONFLI\r\n");
  }
  if (intstat & GINT_STAT_PHY_CHANGE) {                          //PHY status change
    uint8_t i = WCHNET_GetPHYStatus();
    if (i & PHY_Linked_Status)
      printf("PHY Link Success\r\n");
  }
  if (intstat & GINT_STAT_SOCKET) {                             //socket related interrupt
    for (uint8_t i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
      uint8_t socketint = WCHNET_GetSocketInt(i);
      if (socketint)
        WCHNET_HandleSockInt(i, socketint);
    }
  }
}

int main(void) {
  Delay_Init();
  USART_Printf_Init(115200);                                    //USART initialize
  printf("TCPServer Test\r\n");
  if ((SystemCoreClock == 60000000) || (SystemCoreClock == 120000000)) {
    printf("SystemClk:%d\r\n", SystemCoreClock);
  } else {
    printf("Error: Please choose 60MHz and 120MHz clock when using Ethernet!\r\n");
  }

  printf("net version:%x\n", WCHNET_GetVer());
  if (WCHNET_LIB_VER != WCHNET_GetVer()) {
    printf("version error.\n");
  }

  WCHNET_GetMacAddr(MACAddr);                                   //get the chip MAC address

  printf("mac addr:");
  for (uint8_t i = 0; i < 6; i++) {
    printf("%02X ",MACAddr[i]);
  }
  printf("\n");

  TIM2_Init();

  uint8_t e = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);           //Ethernet library initialize
  mStopIfError(e);

  if (e == WCHNET_ERR_SUCCESS) {
    printf("WCHNET_LibInit Success\r\n");
  }

#if KEEPALIVE_ENABLE                                               //Configure keep alive parameters
  {
    struct _KEEP_CFG cfg;

    cfg.KLIdle = 20000;
    cfg.KLIntvl = 15000;
    cfg.KLCount = 9;
    WCHNET_ConfigKeepLive(&cfg);
  }
#endif

  memset(socket, 0xff, WCHNET_MAX_SOCKET_NUM);
  WCHNET_CreateTcpSocketListen();                               //Create TCP Socket for Listening

  while(1) {
    WCHNET_MainTask();
    if (WCHNET_QueryGlobalInt()) {
      WCHNET_HandleGlobalInt();
    }
  }
}
