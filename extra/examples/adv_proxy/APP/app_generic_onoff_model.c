/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_generic_onoff_model.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/24
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#include "config.h"
#include "MESH_LIB.h"
#include "app_mesh_config.h"
#include "app_generic_onoff_model.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define ALI_DEF_TTL    (10)

/*********************************************************************
 * @fn      read_led_state
 *
 * @brief   ��ȡ��ǰ��״̬.
 *
 * @param   led_pin - ����.
 *
 * @return  ��״̬
 */
BOOL read_led_state(uint32_t led_pin)
{
    return (GPIO_ReadOutputDataBit(MSG_GPIO, led_pin) > 0) ? 0 : 1;
}

/*********************************************************************
 * @fn      set_led_state
 *
 * @brief   ���õ�ǰ��״̬.
 *
 * @param   led_pin - ����.
 * @param   on      - ״̬.
 *
 * @return  none
 */
void set_led_state(uint32_t led_pin, BOOL on)
{
    RCC_APB2PeriphClockCmd(MSG_PCENR, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = led_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MSG_GPIO, &GPIO_InitStructure);

    on ? GPIO_WriteBit(MSG_GPIO, led_pin, Bit_RESET) : GPIO_WriteBit(MSG_GPIO, led_pin, Bit_SET);
}

/*********************************************************************
 * @fn      toggle_led_state
 *
 * @brief   ��ת��ǰ��״̬
 *
 * @param   led_pin - ����.
 *
 * @return  none
 */
void toggle_led_state(uint32_t led_pin)
{
    RCC_APB2PeriphClockCmd(MSG_PCENR, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = led_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_WriteBit(MSG_GPIO, led_pin, Bit_SET-GPIO_ReadOutputDataBit(MSG_GPIO, led_pin));
}

/*********************************************************************
 * @fn      gen_onoff_status
 *
 * @brief   �ظ���è���鿪��״̬
 *
 * @param   model   - ģ�Ͳ���.
 * @param   ctx     - ���ݲ���.
 *
 * @return  none
 */
static void gen_onoff_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx)
{
    NET_BUF_SIMPLE_DEFINE(msg, 32);
    int err;

    bt_mesh_model_msg_init(&msg, BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS);
    net_buf_simple_add_u8(&msg, read_led_state(MSG_PIN));

    APP_DBG("ttl: 0x%02x dst: 0x%04x", ctx->recv_ttl, ctx->recv_dst);

    if(ctx->recv_ttl != ALI_DEF_TTL)
    {
        ctx->send_ttl = BLE_MESH_TTL_DEFAULT;
    }
    else
    {
        ctx->send_ttl = 0;
    }

    err = bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
    if(err)
    {
        APP_DBG("send status failed: %d", err);
    }
}

/*********************************************************************
 * @fn      gen_onoff_get
 *
 * @brief   ��è�����·��Ļ�ȡ����״̬����
 *
 * @param   model   - ģ�Ͳ���.
 * @param   ctx     - ���ݲ���.
 * @param   buf     - ��������.
 *
 * @return  none
 */
static void gen_onoff_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");

    gen_onoff_status(model, ctx);
}

/*********************************************************************
 * @fn      gen_onoff_set
 *
 * @brief   ��è�����·������ÿ���״̬��������뵱ǰ״̬��ͬ,����Ҫ����ind����è
 *
 * @param   model   - ģ�Ͳ���.
 * @param   ctx     - ���ݲ���.
 * @param   buf     - ��������.
 *
 * @return  none
 */
static void gen_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG("ttl: 0x%02x dst: 0x%04x rssi: %d", ctx->recv_ttl, ctx->recv_dst, ctx->recv_rssi);

    set_led_state(MSG_PIN, buf->data[0]);
    gen_onoff_status(model, ctx);
}

/*********************************************************************
 * @fn      gen_onoff_set_unack
 *
 * @brief   ��è�����·������ÿ���״̬����(��Ӧ��)
 *
 * @param   model   - ģ�Ͳ���.
 * @param   ctx     - ���ݲ���.
 * @param   buf     - ��������.
 *
 * @return  none
 */
static void gen_onoff_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    uint8_t status;

    APP_DBG(" ");

    status = read_led_state(MSG_PIN);
    if(status != buf->data[0])
    {
        set_led_state(MSG_PIN, buf->data[0]);
    }
}

const struct bt_mesh_model_op gen_onoff_op[] = {
    {BLE_MESH_MODEL_OP_GEN_ONOFF_GET, 0, gen_onoff_get},
    {BLE_MESH_MODEL_OP_GEN_ONOFF_SET, 2, gen_onoff_set},
    {BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK, 2, gen_onoff_set_unack},
    BLE_MESH_MODEL_OP_END,
};

/******************************** endfile @ main ******************************/
