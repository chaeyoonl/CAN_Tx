#include "config.h"
#include "stm32f4xx_hal.h"
#include "can.h"

extern CAN_HandleTypeDef hcan1;

CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

uint8_t CanWatchTimer100ms, CanWatchTimer1s;
uint8_t CanSleepTimer100ms;
uint8_t CanSleep;
uint8_t CanAcc;
uint8_t CanCheck;


void CAN_SetFilterId(CAN_FilterTypeDef* sFilterConfig, uint8_t ide, uint32_t id, uint32_t mask)
{
  if(ide) // ExtId Range 0 ~ 1FFFFFFF
  {
    sFilterConfig->FilterIdHigh = (id & 0x1FFFE000) >> 13;
    sFilterConfig->FilterIdLow = ((id & 0x1FFF) << 3) | CAN_ID_EXT;
    sFilterConfig->FilterMaskIdHigh = (mask & 0x1FFFE000) >> 13;
    sFilterConfig->FilterMaskIdLow = ((mask & 0x1FFF) << 3) | CAN_ID_EXT;
  }
  else // StdId Range 0 ~ 7FF
  {
    sFilterConfig->FilterIdHigh = (id & 0x7FF) << 5;
    sFilterConfig->FilterIdLow = 0x0000;
    sFilterConfig->FilterMaskIdHigh = (mask & 0x7FF) << 5;
    sFilterConfig->FilterMaskIdLow = 0x0000;
  }
}

void CAN1_SetFilter(void)
{
  CAN_FilterTypeDef sFilterConfig;

  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;


  sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST; // ID List Mode
  CAN_SetFilterId(&sFilterConfig, CAN_ID_EXT, CAN_ID1, CAN_ID2);

  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.SlaveStartFilterBank = 14; // meaningless at single CAN

  if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
  {
    /* Filter configuration Error */
    Error_Handler();
  }
}

void CAN1_Start(void)
{
	HAL_StatusTypeDef status1 = HAL_CAN_Start(&hcan1);
	HAL_StatusTypeDef status2 = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	if(HAL_OK != status1) printf("[CAN1] Start Error! - %d\n", status1);
	if(HAL_OK != status2)
		printf("[CAN1] Active Error! - %d\n", status2);
}

void CAN1_Stop(void)
{
  HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_Stop(&hcan1);
}


void CAN1_Rx(void)
{
  HAL_StatusTypeDef status1 = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);


  if(HAL_OK != status1)
  {
    D_PRINTF("[CAN1] RX Get Error! - %X\n", status1);
    return;
  }

  // Check IDE
  if(RxHeader.IDE != CAN_ID_EXT)
  {
    D_PRINTF("[CAN1] IDE Error!\n");
    return;
  }

  // Check DLC
  if(RxHeader.DLC != 8)
  {
    D_PRINTF("[CAN1] DLC Error!\n");
    return;
  }


#if (1 != TEST_CAN)
  printf("%X %X %X %X %X %X %X %X %X %X\n",
    RxHeader.ExtId, RxHeader.DLC, RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7]);
#endif /* TEST_CAN */
}


void CAN1_Tx_Test(void)
{
//	TxHeader.StdId = 0x321;
	TxHeader.ExtId = 0xCEC0AC;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 8;
	TxHeader.TransmitGlobalTime = DISABLE;
	TxData[0] = 1;
	TxData[1] = 2;
	TxData[2] = 3;
	TxData[3] = 4;
	TxData[4] = 5;
	TxData[5] = 6;
	TxData[6] = 7;
	TxData[7] = 8;


  // Transmit
  HAL_StatusTypeDef status1 = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
  if(HAL_OK != status1) {
	  printf("[CAN1] TX Error! - %d %08X\n", status1, hcan1.Instance->ESR); // hcan1.Instance->TSR
  } else {
	  printf("[CAN1] TX HAL_OK !! \n");
  }




}
