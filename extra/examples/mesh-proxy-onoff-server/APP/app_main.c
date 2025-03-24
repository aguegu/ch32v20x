#include "config.h"
#include "MESH_LIB.h"
#include "HAL.h"
#include "app_mesh_config.h"
#include "app.h"

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

uint8_t bt_mesh_lib_init(void)
{
    uint8_t ret;

    if(tmos_memcmp(VER_MESH_LIB, VER_MESH_FILE, strlen(VER_MESH_FILE)) == FALSE)
    {
        PRINT("mesh head file error...\n");
        while(1);
    }

    ret = RF_RoleInit();

#if((CONFIG_BLE_MESH_PROXY) ||   \
    (CONFIG_BLE_MESH_PB_GATT) || \
    (CONFIG_BLE_MESH_OTA))
    ret = GAPRole_PeripheralInit();
#endif /* PROXY || PB-GATT || OTA */

    MeshTimer_Init();
    MeshDeamon_Init();
    ble_sm_alg_ecc_init();

    return ret;
}

__attribute__((section(".highcode")))
__attribute__((noinline))
void Main_Circulation(void) {
  while(1) {
    TMOS_SystemProcess();
  }
}

int main(void) {
  SystemCoreClockUpdate();
  Delay_Init();

#ifdef DEBUG
  USART_Printf_Init( 115200 );
#endif

  PRINT("%s\n", VER_LIB);
  WCHBLE_Init();
  HAL_Init();
  bt_mesh_lib_init();
  App_Init();
  Main_Circulation();
}
