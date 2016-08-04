#include "bsp_ts.h"

static uint8_t  ts_orientation;
static uint8_t  I2C_Address = 0;


uint8_t BSP_TS_Init(uint16_t ts_SizeX, uint16_t ts_SizeY)
{
	uint8_t ts_status = TS_OK;

	/**/
	ft6x06_Init(I2C_Address);

	if(ft6x06_ReadID(TS_I2C_ADDRESS) == FT6206_ID_VALUE)
	{
		I2C_Address = TS_I2C_ADDRESS;
		/*获取/设置LCD的方向*/
		if (ts_SizeX < ts_SizeY)
		{
			ts_orientation = TS_SWAP_NONE;
		}
		else
		{
			ts_orientation = TS_SWAP_XY | TS_SWAP_Y;
		}
		if (ts_status == TS_OK)
		{
			/*复位触摸屏*/
			ft6x06_Reset(I2C_Address);
			/*校正、配置、启动触摸屏驱动*/
			ft6x06_TS_Startt(I2C_Address);
		}
	}
	else
	{
		ts_status = TS_DEVICE_NOT_FOUND;
	}

return ts_status;

}
uint8_t BSP_TS_ITConfig(void)
{
	uint8_t ts_status = TS_OK;
	GPIO_InitTypeDef GPIO_InitStructure;

	TS_INT_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Pin = TS_INT_PIN;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;

 	HAL_GPIO_Init(TS_INT_GPIO_PORT, &GPIO_InitStructure);

 /* Enable and set the TS_INT EXTI Interrupt to an intermediate priority */
  HAL_NVIC_SetPriority((IRQn_Type)(TS_INT_EXTI_IRQn), 0x05, 0x00);
  HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));

  ft6x06_TS_EnableIT(I2C_Address);

  return ts_status;
}
uint8_t BSP_TS_GetState(TS_StateTypeDef *TS_State)
{
	static uint16_t x[TS_MAX_NB_TOUCH] = {0, 0};
	static uint16_t y[TS_MAX_NB_TOUCH] = {0, 0};
	uint8_t ts_status = TS_OK;
	uint8_t index;
	uint16_t temp;
	uint16_t Raw_x[TS_MAX_NB_TOUCH];
	uint16_t Raw_y[TS_MAX_NB_TOUCH];
	uint16_t xDiff;
	uint16_t yDiff;
#if (TS_MULTI_TOUCH_SUPPORTED == 1)
  uint32_t weight = 0;
  uint32_t area = 0;
  uint32_t event = 0;
#endif
  TS_State->touchDetected = ft6x06_TS_DetectTouch(I2C_Address);
  if(TS_State->touchDetected)
  {
  		for (index = 0; index < TS_State->touchDetected; index++)
  		{
			ft6x06_TS_GetXY(I2C_Address, &Raw_x[index], &Raw_y[index]);
  			if (ts_orientation & TS_SWAP_XY)
  			{
  				temp = Raw_x[index];
  				Raw_x[index] = Raw_y[index];
  				Raw_y[index] = temp;
  			}
  			if(ts_orientation & TS_SWAP_X)
  			{
  				Raw_x[index] = FT_6206_MAX_WIDTH - 1 - Raw_x[index];
  			}
  			if(ts_orientation &TS_SWAP_Y)
  			{
  				Raw_y[index] = FT_6206_MAX_HEIGHT - 1 - Raw_y[index];
  			}
  /*计算两个Index触摸测量的结果的差，如果偏移大于5，则采用最新的测量值，反之这则采用Index 1的测量值*/
  			xDiff = Raw_x[index] > x[index]? (Raw_x[index] - x[index]): (x[index] - Raw_x[index]);
      	yDiff = Raw_y[index] > y[index]? (Raw_y[index] - y[index]): (y[index] - Raw_y[index]);
      	if ((xDiff + yDiff) > 5)
      	{
        		x[index] = Raw_x[index];
        		y[index] = Raw_y[index];
			}
			TS_State->touchX[index] = x[index];
	      TS_State->touchY[index] = y[index];

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

      	/*获得当前触摸的相关信息*/
			ft6x06_TS_GetTouchInfo(I2C_Address, index, &weight, &area, &event);

      	/*更新触摸状态结构体*/
      	TS_State->touchWeight[index] = weight;
      	TS_State->touchArea[index]   = area;

      	/*判断相关触摸事件*/
      	switch(event)
      	{
	        case FT6206_TOUCH_EVT_FLAG_PRESS_DOWN  :
	          TS_State->touchEventId[index] = TOUCH_EVENT_PRESS_DOWN;
	          break;
	        case FT6206_TOUCH_EVT_FLAG_LIFT_UP :
	          TS_State->touchEventId[index] = TOUCH_EVENT_LIFT_UP;
	          break;
	        case FT6206_TOUCH_EVT_FLAG_CONTACT :
	          TS_State->touchEventId[index] = TOUCH_EVENT_CONTACT;
	          break;
	        case FT6206_TOUCH_EVT_FLAG_NO_EVENT :
	          TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
	          break;
	        default :
	          ts_status = TS_ERROR;
	          break;
      	} 

#endif
  		}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
    /*获取手势Id，将其保存在TS*/
		ts_status = BSP_TS_Get_GestureId(TS_State);
#endif
  }
	return ts_status;
}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

uint8_t BSP_TS_Get_GestureId(TS_StateTypeDef *TS_State)
{
	uint32_t gestureId = 0;
	uint8_t ts_status = TS_OK;

	ft6x06_TS_GetGestureID(I2C_Address,&gestureId);

	switch(gestureId)
	{
		case FT6206_GEST_ID_NO_GESTURE :
      TS_State->gestureId = GEST_ID_NO_GESTURE;
      break;
    case FT6206_GEST_ID_MOVE_UP :
      TS_State->gestureId = GEST_ID_MOVE_UP;
      break;
    case FT6206_GEST_ID_MOVE_RIGHT :
      TS_State->gestureId = GEST_ID_MOVE_RIGHT;
      break;
    case FT6206_GEST_ID_MOVE_DOWN :
      TS_State->gestureId = GEST_ID_MOVE_DOWN;
      break;
    case FT6206_GEST_ID_MOVE_LEFT :
      TS_State->gestureId = GEST_ID_MOVE_LEFT;
      break;
    case FT6206_GEST_ID_ZOOM_IN :
      TS_State->gestureId = GEST_ID_ZOOM_IN;
      break;
    case FT6206_GEST_ID_ZOOM_OUT :
      TS_State->gestureId = GEST_ID_ZOOM_OUT;
      break;
    default :
      ts_status = TS_ERROR;
      break;
  } /* of switch(gestureId) */

  return(ts_status);

}
uint8_t BSP_TS_ResetTouchData(TS_StatusTypeDef *TS_State)
{
	uint8_t ts_status = TS_ERROR;
	uint32_t index;
	if(TS_State != (TS_StateTypeDef *)NULL)
	{

		TS_State->gestureId = GEST_ID_NO_GESTURE;
		TS_State->touchDetected = 0;
		for (index = 0; index < TS_MAX_NB_TOUCH; index++)
		{
			TS_State->touchX[index] = 0;
			TS_State->touchY[index] = 0;
			TS_State->touchArea[index] = 0;
			TS_State->touchEventId[index] = 0;
			TS_State->touchWeight[index] = 0;
		}
		ts_status = TS_OK;
	}

	return ts_status;
}
#endif

