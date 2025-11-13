/**
 ******************************************************************************
 * @file           : control_pilot.c
 * @brief          : Control Pilot Functions
 ******************************************************************************
 * @attention
 *
 * Author: Nisal Bulathsinghala, Jul 27, 2022
 * Copyright (c) 2022 Vega Innovations, Sri Lanka.
 * All rights reserved.
 ******************************************************************************
 */

#include "control_pilot.h"

#define samples     75
#define samples2    25
#define samples3    5

uint32_t cp_min_sum = 0;
uint16_t cp_min_avg = 0;
uint32_t cp_max_sum = 0;
uint16_t cp_max_avg = 0;

uint32_t cp_min_count = 0;
uint32_t cp_max_count = 0;
uint32_t cp_min_count2 = 0;
uint32_t cp_max_count2 = 0;

uint16_t adcSample_buf1[samples];
uint16_t adcSample_buf1_count = 0;

uint16_t adc_high[samples] = {};
uint16_t adc_low[samples] = {};

bool dataCollected1 = false;
bool dataCollected2 = false;

bool max_done = false;
bool min_done = false;

uint16_t vehicleCheck_adc = 0;
uint16_t vehicleCheck_avg = 0;

//my parameters (H)
bool vehicle_pluged =false;
uint16_t cp_upper_when_pluged =3680;

uint32_t diodeCheck_adc = 0;
uint16_t diodeCheck_avg = 0;
uint16_t diodeCheck_count = 0;
volatile bool diodeCheck_flag = false;
volatile bool diodeCheck_passed = false;

#define PWMActive_count	(100 / PROTECTION_TICK_TIME)		//100ms
uint16_t PWMActive_InCounter = 0;
uint16_t PWMActive_OutCounter = 0;

#define CPFault_THRESHOLD	200
#define CPFault_errorCount	(1000 / PROTECTION_TICK_TIME)	//1s
uint16_t CPFault_errorInCounter = 0;
uint16_t CPFault_errorOutCounter = 0;

#define DC_errorCount	(250 / PROTECTION_TICK_TIME)		//250ms
uint16_t DC_errorInCounter = 0;
uint16_t DC_errorOutCounter = 0;

bool cp_readings_done = false;

#define vehicle_checkCount 25
uint8_t vehicle_checkInCounter = 0;
uint8_t vehicle_checkOutCounter = 0;

uint16_t avgHigh = 0;
uint16_t avgLow = 0;

volatile bool cpCalibrate_en = false;
volatile bool cpPositve_en = false;
volatile bool cpNegative_en = false;
volatile bool cpDuty_en = false;
volatile bool cpCalibrate_cmplt = false;

volatile uint16_t cpCalibrate_set[2] = {};
volatile int16_t cpCalibrate_get[2] = {};

float CP_POS_PARAM = 0.0;
float CP_NEG_PARAM = 0.0;

// Flags and variables
bool cp_calibrate = false;

void cpCalibrate_load(void)
{
	EEPROM_ReadData(CP_LOCATION, (uint16_t *)cpCalibrate_get, sizeof(cpCalibrate_get)/sizeof(cpCalibrate_get[0]));
	HAL_Delay(100);
}

void cpCalibrate_save(void)
{
	cpCalibrate_set[0] = charger_configSet.cp_positive;
	cpCalibrate_set[1] = charger_configSet.cp_negative;

	EEPROM_Erase(CP_LOCATION, EEPROM_SIZE);
	EEPROM_WriteData(CP_LOCATION, (uint16_t *)cpCalibrate_set, (sizeof(cpCalibrate_set)/sizeof(cpCalibrate_set[0])));
	HAL_Delay(100);
}

void getCP_voltage(void)
{
	static uint16_t temp_adc;

	temp_adc = adc_store[MCU_CP_READ_INDEX];

	//V3
	if (dataCollected1 == false)
	{
		adcSample_buf1[adcSample_buf1_count] = temp_adc;
		adc_high[adcSample_buf1_count] = 0;
		adc_low[adcSample_buf1_count] = 0;

		adcSample_buf1_count++;

		if (adcSample_buf1_count >= samples)
		{
			adcSample_buf1_count = 0;
			dataCollected1 = true;
		}
	}
	else
	{
		if (dataCollected2 == false)
		{
			if (adcSample_buf1[adcSample_buf1_count] >= 1800)
			{
				if (cp_max_count < samples)
				{
					adc_high[cp_max_count] =
							adcSample_buf1[adcSample_buf1_count];
					cp_max_count++;
				}
			}
			else
			{
				if (cp_min_count < samples)
				{
					adc_low[cp_min_count] =
							adcSample_buf1[adcSample_buf1_count];
					cp_min_count++;
				}
			}

			adcSample_buf1_count++;

			if (adcSample_buf1_count >= samples)
			{
				adcSample_buf1_count = 0;
				dataCollected2 = true;
			}
		}
		else
		{
			if (cp_max_count > 0)
			{
				if (cp_max_count2 < cp_max_count)
				{
					cp_max_sum = cp_max_sum + adc_high[cp_max_count2];
					cp_max_count2++;
				}
				else
				{
					cp_max_avg = (uint16_t) (cp_max_sum / cp_max_count);

//					controlSide_data.controlPilot.cp_max = cp_max_avg;

					controlSide_data.controlPilot.cp_max = (0.2 * cp_max_avg)+((1 - 0.2) * controlSide_data.controlPilot.cp_max);

					float temp2 =
							((7.6807
									* ((float) (VREF / 4096.0)
											* (float) controlSide_data.controlPilot.cp_max))
									+ CP_POS_PARAM - (3.6707 * VREF));
					controlSide_data.controlPilot.cp_Vmax =(int) (temp2 * 10.0);

					max_done = true;
				}
			}
			else
			{
				max_done = true;
			}

			if (cp_min_count > 0)
			{
				if (cp_min_count2 < cp_min_count)
				{
					cp_min_sum = cp_min_sum + adc_low[cp_min_count2];
					cp_min_count2++;
				}
				else
				{
					cp_min_avg = (uint16_t) (cp_min_sum / cp_min_count);

//					controlSide_data.controlPilot.cp_min = cp_min_avg;

					controlSide_data.controlPilot.cp_min = (0.2 * cp_min_avg)+((1 - 0.2) * controlSide_data.controlPilot.cp_min);

					float temp1 =
							((7.6807
									* ((float) (VREF / 4096.0)
											* (float) controlSide_data.controlPilot.cp_min))
									- CP_NEG_PARAM - (3.6707 * VREF));
					controlSide_data.controlPilot.cp_Vmin =
							(int) (temp1 * 10.0);

					min_done = true;
				}
			}
			else
			{
				min_done = true;
			}

			if ((max_done == true) && (min_done == true))
			{
				adcSample_buf1_count = 0;
				cp_max_count = 0;
				cp_min_count = 0;
				cp_max_count2 = 0;
				cp_min_count2 = 0;
				max_done = false;
				min_done = false;

				cp_min_sum = 0;
				cp_min_avg = 0;
				cp_max_sum = 0;
				cp_max_avg = 0;

				dataCollected1 = false;
				dataCollected2 = false;
			}
		}
	}
}

void cpCalibrate()
{
	cpCalibrate_load();

	float cal_pos, cal_neg;

	cal_pos = 12.0;//(cpCalibrate_get[0]/100.0);
	cal_neg = 12.0;//(-(cpCalibrate_get[1]/100.0));

	//-------------------------------------------(+12V) Generation
	TIM3->CCR1 = (uint32_t) (PWMFULLON * 10);
	TIM3->CCR4 = (uint32_t) (PWMFULLON * 10);
	HAL_Delay(1000); //Give some time to settle

	vehicle_pluged  = ( controlSide_data.controlPilot.cp_max <= cp_upper_when_pluged);

	if(vehicle_pluged)
		{

			//CP_POS_PARAM = (cal_pos + (3.6707 * VREF) - (7.6807 * (VREF/4096.0)* cpCalibrate_get[0]));
			 CP_POS_PARAM = (cal_pos + (3.6707 * VREF) - (7.6807 * (VREF/4096.0)* 3940));

			//-------------------------------------------(-12V) Generation
			TIM3->CCR1 = (uint32_t) (PWMFULLOFF * 10);
			TIM3->CCR4 = (uint32_t) (PWMFULLOFF * 10);
			HAL_Delay(500);  // reduced time delay from 1000 to 500

			//CP_NEG_PARAM = (cal_neg - (3.6707 * VREF) + (7.6807 * (VREF/4096.0)* cpCalibrate_get[1]));
			CP_NEG_PARAM = (cal_neg - (3.6707 * VREF) + (7.6807 * (VREF/4096.0)*100));

		}

		else
		{

			CP_POS_PARAM = (cal_pos + (3.6707 * VREF) - (7.6807 * (VREF/4096.0)* controlSide_data.controlPilot.cp_max));

			//-------------------------------------------(-12V) Generation
			TIM3->CCR1 = (uint32_t) (PWMFULLOFF * 10);
			TIM3->CCR4 = (uint32_t) (PWMFULLOFF * 10);
			HAL_Delay(500); // reduced time delay from 1000 to 500

			CP_NEG_PARAM = (cal_neg - (3.6707 * VREF) + (7.6807 * (VREF/4096.0)* controlSide_data.controlPilot.cp_min));


		}

		//-------------------------------------------(+12V) Generation
		TIM3->CCR1 = (uint32_t) (PWMFULLON * 10);
		TIM3->CCR4 = (uint32_t) (PWMFULLON * 10);
		HAL_Delay(500); //Give some time to settle // reduced time delay from 1000 to 500

		cp_calibrate = true;


}

static uint8_t uv_handle_state = 0;
static uint8_t uv_stable_count = 0;
static uint16_t lowest_voltage = 0;

static inline uint16_t find_lowest(uint16_t a, uint16_t b, uint16_t c)
{
	uint16_t lowest = a; // Assume a is the lowest initially

	if (b < lowest)
	{
		lowest = b;
	}

	if (c < lowest)
	{
		lowest = c;
	}

	return lowest;
}

void load_balance(bool uv_en)
{
	if (load_balance6s.timeout_6s == false)
	{
		return;
	}

	tick_clear(&load_balance6s);

#if UV_dynamic
	if (controlSide_data.warnings.bits.UV_warn == 1)
	{
		switch (uv_handle_state)
		{
		case 0:
			if (powerSide_data.errorStatus.bit.error_UV == 1)
			{
				controlSide_data.controlPilot.PWMSET = 15;
				uv_handle_state = 3;
				uv_stable_count = 0;
			}
			break;
		case 1:
			if (powerSide_data.errorStatus.bit.error_UV == 0)
			{
				controlSide_data.controlPilot.PWMSET =
						(controlSide_data.controlPilot.PWMSET + 5);
				if (controlSide_data.controlPilot.PWMSET >= CPMAX)
				{
					controlSide_data.controlPilot.PWMSET = CPMAX;
				}
				uv_handle_state = 3;
				uv_stable_count = 0;
			}
			break;
		case 2:
			controlSide_data.controlPilot.PWMSET =
					(controlSide_data.controlPilot.PWMSET - 5);
			if (controlSide_data.controlPilot.PWMSET <= CPMIN)
			{
				controlSide_data.controlPilot.PWMSET = CPMIN;
			}
			uv_handle_state = 3;
			uv_stable_count = 0;
			break;
		case 3:
			if (threePhaseMeter)
			{
				lowest_voltage = find_lowest(powerSide_data.voltage.VA,
						powerSide_data.voltage.VB, powerSide_data.voltage.VC);
			}
			else
			{
				lowest_voltage = powerSide_data.voltage.VA;
			}

			if ((lowest_voltage - charger_configGet.uv_upper) > 0)
			{
				if ((lowest_voltage - charger_configGet.uv_upper) >=10)
				{
					uv_handle_state = 3;
					uv_stable_count++;

					if (uv_stable_count >= 10)
					{
						controlSide_data.warnings.bits.UV_warn = 0;
					}
				}
				else
				{
					uv_handle_state = 1;
				}
			}
			else
			{
				uv_handle_state = 2;
			}
			break;
		default:
			break;
		}
		return;
	}
#endif
//	else
//	{
//		controlSide_data.controlPilot.PWMSET = CPDUTY_DEFAULT;

		if (controlSide_data.errorStatus.bit.serialAError == 1)
		{
			return;
		}

		uint16_t temp = (uint16_t) (networkSide_data.maxCurrent_req / 0.6);

		if (temp >= CPMIN)
		{
			controlSide_data.controlPilot.PWMSET = temp;
		}

//	}
}

static void check_CPF(void)
{
	if ((controlSide_data.controlPilot.cp_Vmax > STATEF_MIN)
			&& (controlSide_data.controlPilot.cp_Vmax < STATEC_MIN))
	{
		CPFault_errorOutCounter = 0;
		CPFault_errorInCounter++;
		if (CPFault_errorInCounter >= CPFault_errorCount)
		{
			controlSide_data.errorStatus.bit.CPFault = 1;
		}
	}
	else
	{
		if (controlSide_data.status.bit.connector_state == 0)
		{
			CPFault_errorInCounter = 0;
			CPFault_errorOutCounter++;
			if (CPFault_errorOutCounter >= CPFault_errorCount)
			{
				CPFault_errorOutCounter = 0;
				controlSide_data.errorStatus.bit.CPFault = 0;
			}
		}
	}
}

void monitor_cp(void)
{
	if (charger_configGet.en_1.bit.cpf_en == 1)
	{
		check_CPF();
	}

	if (controlSide_data.controlPilot.PWMSET < CPMIN)
	{
		cpMin_reached = true;
		return;
	}

	cpMin_reached = false;

	if (controlSide_data.controlPilot.PWMSET >= CPMAX)
	{
		controlSide_data.controlPilot.PWMSET = CPMAX;
	}
}

void setCP_duty(void)
{
	if (sm.currentState == STATE_F)
	{
		if (controlSide_data.controlPilot.cp_enable == 1)
		{
			controlSide_data.controlPilot.cpDuty = PWMFULLON;
		}
		else
		{
			controlSide_data.controlPilot.cpDuty = PWMFULLOFF;
		}
	}
	else
	{
		if (controlSide_data.controlPilot.cp_enable == 1)
		{
			if (controlSide_data.controlPilot.cp_duty_enable == 1)
			{
				controlSide_data.controlPilot.cpDuty =
						controlSide_data.controlPilot.PWMSET;
			}
			else
			{
				controlSide_data.controlPilot.cpDuty = PWMFULLON;
			}

		}
		else
		{
			controlSide_data.controlPilot.cpDuty = PWMFULLOFF;
		}
	}

	if (cp_calibrate)
	{
		TIM3->CCR1 = (uint32_t) (controlSide_data.controlPilot.cpDuty * 10);
		TIM3->CCR4 = (uint32_t) (controlSide_data.controlPilot.cpDuty * 10);
	}

	if (cpCalibrate_en)
	{
		if (cpPositve_en)
		{
			TIM3->CCR1 = (uint32_t) (PWMFULLON * 10);
			TIM3->CCR4 = (uint32_t) (PWMFULLON * 10);
		}

		if (cpNegative_en)
		{
			TIM3->CCR1 = (uint32_t) (PWMFULLOFF * 10);
			TIM3->CCR4 = (uint32_t) (PWMFULLOFF * 10);
		}

		if (cpDuty_en)
		{
			TIM3->CCR1 = (uint32_t) (charger_configSet.cp_set * 10);
			TIM3->CCR4 = (uint32_t) (charger_configSet.cp_set * 10);
		}
	}
}

void check_diode(void)
{
	if ((controlSide_data.controlPilot.cp_Vmin > STATEF_MIN)
			&& (sm.currentState == STATE_B2)
			&& (controlSide_data.status.bit.cpPWM_active == 1))
	{
		DC_errorOutCounter = 0;
		DC_errorInCounter++;
		if (DC_errorInCounter >= DC_errorCount)
		{
			DC_errorInCounter = 0;
			controlSide_data.errorStatus.bit.diodeCheck_failed = 1;
			diodeCheck_passed = false;
		}
	}
	else
	{
		DC_errorInCounter = 0;
		DC_errorOutCounter++;
		if (DC_errorOutCounter >= DC_errorCount)
		{
			DC_errorOutCounter = 0;
			controlSide_data.errorStatus.bit.diodeCheck_failed = 0;
			diodeCheck_passed = true;
		}
	}
}

void bootup_vehicleCheck(void)
{
	if (controlSide_data.controlPilot.cp_Vmax > STATEA_MIN)
	{
		controlSide_data.networkSide_request.bit.vehicle_Check = 0;
		stateEntry_flag = true;
		changeState((StateMachine *)&sm, STATE_A1, 0);
	}
	else
	{
		controlSide_data.networkSide_request.bit.vehicle_Check = 1;

		if ((controlSide_data.controlPilot.cp_Vmax > STATEB_MIN) //Jump to STATE B1
		&& (controlSide_data.controlPilot.cp_Vmax < STATEB_MAX))
		{
			stateEntry_flag = true;
			changeState((StateMachine *)&sm, STATE_B1, 0);
		}

		if ((controlSide_data.controlPilot.cp_Vmax > STATEC_MIN) //Jump to STATE C1
		&& (controlSide_data.controlPilot.cp_Vmax < STATEC_MAX))
		{
			stateEntry_flag = true;
			changeState((StateMachine *)&sm, STATE_C1, 0);
		}
	}
}

void check_vehicleConnector(void)
{
	if (controlSide_data.controlPilot.cp_enable)
	{
		if ((controlSide_data.controlPilot.cp_Vmax > 0)
				&& (controlSide_data.controlPilot.cp_Vmax < STATEA_MIN))
		{
			vehicle_checkOutCounter = 0;
			vehicle_checkInCounter++;
			if (vehicle_checkInCounter >= vehicle_checkCount)
			{
				controlSide_data.status.bit.connector_state = 1;
			}
		}
		else
		{
			vehicle_checkInCounter = 0;
			vehicle_checkOutCounter++;
			if (vehicle_checkOutCounter >= vehicle_checkCount)
			{
				controlSide_data.status.bit.connector_state = 0;
			}
		}
	}
}

void check_PWMActive(void)
{
	if (charger_init_flag == true)
	{

		static uint16_t diff;

		if (controlSide_data.controlPilot.cp_duty_enable == 1)
		{
			diff = (controlSide_data.controlPilot.cp_max
					- controlSide_data.controlPilot.cp_min);

			if (diff > 1000)
			{
				PWMActive_OutCounter = 0;
				PWMActive_InCounter++;
				if (PWMActive_InCounter >= PWMActive_count)
				{
					PWMActive_InCounter = 0;
					controlSide_data.status.bit.cpPWM_active = 1;
				}
			}
			else
			{
				PWMActive_InCounter = 0;
				PWMActive_OutCounter++;
				if (PWMActive_OutCounter >= PWMActive_count)
				{
					PWMActive_OutCounter = 0;
					controlSide_data.status.bit.cpPWM_active = 0;
				}
			}
		}
		else
		{
			controlSide_data.status.bit.cpPWM_active = 0;
		}
	}
}
