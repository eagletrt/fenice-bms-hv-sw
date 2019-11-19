#ifndef _EAGLE_TRT_CAN_MSG_H
#define _EAGLE_TRT_CAN_MSG_H

#define ENC_L_FROM_ACU 20 	 //ACU sending to: SW CONTR --> B0: speed/256 | B1: speed%256 | B2: sign | B3: prescaler | B4: meters/256 | B5: meters%256 | B6: error | B7: 
#define ENC_R_FROM_ACU 10 	 //ACU sending to: SW CONTR --> B0: speed/256 | B1: speed%256 | B2: sign | B3: prescaler | B4: meters/256 | B5: meters%256 | B6: error | B7: 
#define ENC_C_FROM_ACU 84 	 //ACU sending to: SW CONTR --> B0: angle/256 | B1: angle%256 | B2: sign | B3: prescaler | B4: error | B5:  | B6:  | B7: 
#define PEDALS_FROM_ATC 97 	 //ATC sending to: ACU --> B0: APPS1/256 | B1: APPS1%256 | B2: APPS2/256 | B3: APPS%256 | B4: BRK1/256 | B5: BRK1%256 | B6: BRK2/256 | B7: BRK%256
#define PEDALS_FROM_ACU 148 	 //ACU sending to: SW CONTR --> B0: APPS1 | B1: APPS2 | B2: BRK1 | B3: BRK2 | B4: ERR | B5:  | B6:  | B7: 
#define HV_VOLT 177 	 //BMS_HV sending to: ACU SW --> B0: DATA_V0 | B1: DATA_V1 | B2: DATA_V2 | B3: DATA_V3 | B4: DATA_VMAX | B5: DATA_VMAX | B6: DATA_VMIN | B7: DATA_VMIN
#define HV_TEMP 241 	 //BMS_HV sending to: ACU SW BMS_LV --> B0: DATA_TAVG0 | B1: DATA_TAVG1 | B2: DATA_TMAX | B3: DATA_TMAX | B4: DATA_TMIN | B5: DATA_TMIN | B6:  | B7: 
#define HV_CURR_ERR_WARN 273 	 //BMS_HV sending to: ACU SW --> B0: SIGN | B1: CURR_0 | B2: CURR_1 | B3: PW | B4: ERR_CODE | B5: ERR_INDEX | B6: ERR_DATA0 | B7: ERR_DATA1
#define LV_STATUS 305 	 //BMS_LV sending to: SW ACU --> B0: LV_VOLT | B1: LV_TEMPAVG | B2: LV_TEMPMAX | B3: LV_CURR | B4: ERR | B5:  | B6:  | B7: 
#define LV_STATUS_JUDGE 289 	 //BMS_LV sending to: ACU --> B0: LV_VOLT | B1: LV_TEMPAVG | B2: LV_TEMPMAX | B3: LV_VOLT0 | B4: LV_VOLT1 | B5: LV_VOLT2 | B6: LV_VOLT3 | B7: ERR
#define INV_R_TEMP 369 	 //INV_R sending to: BMS_LV ACU SW --> B0: INV_R_TEMP | B1: INV_R_TEMP0 | B2: INV_R_TEMP1 | B3:  | B4:  | B5:  | B6:  | B7: 
#define INV_L_TEMP 401 	 //INV_L sending to: BMS_LV ACU SW --> B0: INV_L_TEMP | B1: INV_L_TEMP0 | B2: INV_L_TEMP1 | B3:  | B4:  | B5:  | B6:  | B7: 
#define MOT_R_TEMP 433 	 //INV_R sending to: BMS_LV ACU SW --> B0: MOT_R_TEMP | B1: MOT_R_TEMP0 | B2: MOT_R_TEMP1 | B3:  | B4:  | B5:  | B6:  | B7: 
#define MOT_L_TEMP 465 	 //INV_L sending to: BMS_LV ACU SW --> B0: MOT_L_TEMP | B1: MOT_L_TEMP0 | B2: MOT_L_TEMP1 | B3:  | B4:  | B5:  | B6:  | B7: 
#define CAL_SET_MIN_MAX 449 	 //STEER sending to: ACU --> B0: SET_CALIB_MAX/MIN[STEER,PEDALS] | B1:  | B2:  | B3:  | B4:  | B5:  | B6:  | B7: 
#define CAL_RESP 496 	 //ACU sending to: SW --> B0: MIN/MAX | B1: CALIB_OK/NOT_OK | B2:  | B3:  | B4:  | B5:  | B6:  | B7: 

#endif
