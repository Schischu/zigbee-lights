/*****************************************************************************
 *
 * MODULE:             JN-AN-1171
 *
 * COMPONENT:          App_Light_ColorLight.c
 *
 * DESCRIPTION:        ZLL Demo: Colored Light - Implementation
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5164,
 * JN5161, JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include "AppHardwareApi.h"
#include "zps_gen.h"
#include "App_Light_ColorLight.h"
#include "app_common.h"
#include "app_zcl_light_task.h"
#include "dbg.h"
#include <string.h>

#include "version.h"

#ifndef MONO_ON_OFF
#include "app_light_interpolation.h"
#endif
#include "DriverBulb_Shim.h"

#ifdef DEBUG_LIGHT_TASK
#define TRACE_LIGHT_TASK  TRUE
#else
#define TRACE_LIGHT_TASK FALSE
#endif

#ifdef DEBUG_PATH
#define TRACE_PATH  TRUE
#else
#define TRACE_PATH  FALSE
#endif

#define LIGHT_TO_ENDPOINT(light) \
    (LIGHT_COLORLIGHT_LIGHT_00_ENDPOINT + light)


#define ENDPOINT_TO_LIGHT(endpoint) \
    (endpoint - LIGHT_COLORLIGHT_LIGHT_00_ENDPOINT)

#define LIGHT_COMMISSION_ENDPOINT LIGHT_COLORLIGHT_COMMISSION_ENDPOINT

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/*Dimmable Light is a subset of Colour Light and if CLD_COLOUR_CONTROL is not set its actually the same*/

#if 1
tsZLL_ColourLightDevice sLight[ZLL_NUMBER_DEVICES];
tsIdentifyColour sIdEffect[ZLL_NUMBER_DEVICES];
#else
tsZLL_DimmableLightDevice sLight[ZLL_NUMBER_DEVICES];
tsIdentifyWhite sIdEffect[ZLL_NUMBER_DEVICES];
#endif

/*
typedef struct
{
    uint64  u64IEEEAddr;
    uint16  u16ProfileId;
    uint16 u16DeviceId;
    uint8   u8Endpoint;
    uint8 u8Version;
    uint8 u8NumberGroupIds;
    uint8 u8Sort;
} tsCLD_ZllDeviceRecord;
*/

tsCLD_ZllDeviceTable sDeviceTable = { ZLL_NUMBER_DEVICES,
                                      {
                                          { 0,
                                            ZLL_PROFILE_ID,
                                            COLOUR_LIGHT_DEVICE_ID,
                                            LIGHT_TO_ENDPOINT(0),
                                            2,
                                            0,
                                            0}
#if ZLL_NUMBER_DEVICES >= 2
                                          ,{ 0,
                                            ZLL_PROFILE_ID,
                                            COLOUR_LIGHT_DEVICE_ID,
                                            LIGHT_TO_ENDPOINT(1),
                                            2,
                                            0,
                                            0}
#if ZLL_NUMBER_DEVICES >= 3
                                          ,{ 0,
                                            ZLL_PROFILE_ID,
                                            COLOUR_LIGHT_DEVICE_ID,
                                            LIGHT_TO_ENDPOINT(2),
                                            2,
                                            0,
                                            0}
#if ZLL_NUMBER_DEVICES >= 4
                                          ,{ 0,
                                            ZLL_PROFILE_ID,
                                            COLOUR_LIGHT_DEVICE_ID,
                                            LIGHT_TO_ENDPOINT(3),
                                            2,
                                            0,
                                            0}
#endif /*ZLL_NUMBER_DEVICES >= 4*/
#endif /*ZLL_NUMBER_DEVICES >= 3*/
#endif /*ZLL_NUMBER_DEVICES >= 2*/
                                      }
};

#if 0
uint16 u16CurrentLevel;
uint16 u16TargetLevel;
int16 i16DeltaLevel = 0;

uint8 u8Update = 0xff;

uint16 u16CurrentRed;
uint16 u16TargetRed;
int16 i16DeltaRed = 0;

uint16 u16CurrentBlue;
uint16 u16TargetBlue;
int16 i16DeltaBlue = 0;

uint16 u16CurrentGreen;
uint16 u16TargetGreen;
int16 i16DeltaGreen = 0;
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PRIVATE void vOverideProfileId(uint8 u8Index, uint16* pu16Profile, uint8 u8Ep);
/****************************************************************************
 **
 ** NAME: eApp_ZLL_RegisterEndpoint
 **
 ** DESCRIPTION:
 ** Register ZLL endpoints
 **
 ** PARAMETER
 ** Type                                Name                    Descirption
 ** tfpZCL_ZCLCallBackFunction            fptr                    Pointer to ZCL Callback function
 ** tsZLL_CommissionEndpoint            psCommissionEndpoint    Pointer to Commission Endpoint
 **
 **
 ** RETURNS:
 ** teZCL_Status
 *
 ****************************************************************************/
PUBLIC teZCL_Status eApp_ZLL_RegisterEndpoint(tfpZCL_ZCLCallBackFunction fptr,
                                       tsZLL_CommissionEndpoint* psCommissionEndpoint)
{
    uint8 u8Index = 0;

    ZPS_vAplZdoRegisterProfileCallback(vOverideProfileId);
    zps_vSetIgnoreProfileCheck();

    eZLL_RegisterCommissionEndPoint(LIGHT_COMMISSION_ENDPOINT,
                                    fptr,
                                    psCommissionEndpoint);

    for (u8Index=0; u8Index < ZLL_NUMBER_DEVICES; u8Index++)
    {
		if (sDeviceTable.asDeviceRecords[u8Index].u16DeviceId == COLOUR_LIGHT_DEVICE_ID)
		{
        	eZLL_RegisterColourLightEndPoint(LIGHT_TO_ENDPOINT(u8Index),
                                                fptr,
                                                &sLight[u8Index]);
		}
		else
		{
        	eZLL_RegisterDimmableLightEndPoint(LIGHT_TO_ENDPOINT(u8Index),
                                                fptr,
                                                &sLight[u8Index]);
		}
    }

    return E_ZCL_SUCCESS;
}


/****************************************************************************
*
* NAME: vOverideProfileId
*
* DESCRIPTION: Allows the application to over ride the profile in the
* simple descriptor (0xc05e) with the ZHA profile id (0x0104)
* required for on air packets
*
*
* PARAMETER: pointer to the profile  to be used, the end point sending the data
*
* RETURNS: void
*
****************************************************************************/
PRIVATE void vOverideProfileId(uint8 u8Index, uint16* pu16Profile, uint8 u8Ep)
{
    if (u8Ep == LIGHT_TO_ENDPOINT(u8Index))
    {
        *pu16Profile = 0x0104;
    }
}

/****************************************************************************
 *
 * NAME: vApp_eCLD_ColourControl_GetRGB
 *
 * DESCRIPTION:
 * To get RGB value
 *
 * PARAMETER
 * Type                   Name                    Descirption
 * uint8 *                pu8Red                  Pointer to Red in RGB value
 * uint8 *                pu8Green                Pointer to Green in RGB value
 * uint8 *                pu8Blue                 Pointer to Blue in RGB value
 *
 * RETURNS:
 * teZCL_Status
 *
 ****************************************************************************/
PUBLIC void vApp_eCLD_ColourControl_GetRGB(uint8 u8Index, uint8 *pu8Red,uint8 *pu8Green,uint8 *pu8Blue)
{
    eCLD_ColourControl_GetRGB(LIGHT_TO_ENDPOINT(u8Index),
                              pu8Red,
                              pu8Green,
                              pu8Blue);
}

/****************************************************************************
 *
 * NAME: vAPP_ZCL_DeviceSpecific_Init
 *
 * DESCRIPTION:
 * ZLL Device Specific initialization
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vAPP_ZCL_DeviceSpecific_Init(uint8 u8Index)
{
	//const char manufacture[] = "Philips";
	const char manufacture[] = "NXP";

    /* Initialise the strings in Basic */
    memcpy(sLight[u8Index].sBasicServerCluster.au8ManufacturerName, manufacture, sizeof(manufacture)); //32

    memcpy(sLight[u8Index].sBasicServerCluster.au8DateCode, fw_date_str, VERSION_DATE_LEN); //16

    //memcpy(sLight[u8Index].sBasicServerCluster.au8SWBuildID, fw_date_str, VERSION_DATE_LEN); //16
    //sLight[u8Index].sBasicServerCluster.au8SWBuildID[VERSION_DATE_LEN] = '-';
    //memcpy(sLight[u8Index].sBasicServerCluster.au8SWBuildID + VERSION_DATE_LEN + 1, fw_time_str, VERSION_TIME_LEN);
    memcpy(sLight[u8Index].sBasicServerCluster.au8SWBuildID, fw_time_str, VERSION_TIME_LEN);


	if (sDeviceTable.asDeviceRecords[u8Index].u16DeviceId == COLOUR_LIGHT_DEVICE_ID)
	{
#if 0
	    memcpy(sLight[u8Index].sBasicServerCluster.au8ModelIdentifier, "LCT015", CLD_BAS_MODEL_ID_SIZE);
	    memcpy(sLight[u8Index].sBasicServerCluster.au8SWBuildID, "1.46.13_r26312", CLD_BAS_SW_BUILD_SIZE);
#else
	    memcpy(sLight[u8Index].sBasicServerCluster.au8ModelIdentifier, "ZLL-ColorLight 0", CLD_BAS_MODEL_ID_SIZE);
	    sLight[u8Index].sBasicServerCluster.au8ModelIdentifier[15] = '0' + u8Index;
#endif
	}
	else
	{
#if 0
	    memcpy(sLight[u8Index].sBasicServerCluster.au8ModelIdentifier, "LCT015", CLD_BAS_MODEL_ID_SIZE);
	    memcpy(sLight[u8Index].sBasicServerCluster.au8SWBuildID, "1.46.13_r26312", CLD_BAS_SW_BUILD_SIZE);
#else
	    memcpy(sLight[u8Index].sBasicServerCluster.au8ModelIdentifier, "ZLL-DimmaLight 0", CLD_BAS_MODEL_ID_SIZE);
	    sLight[u8Index].sBasicServerCluster.au8ModelIdentifier[15] = '0' + u8Index;
#endif
	}

    sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_STOP_EFFECT;
    sIdEffect[u8Index].u8Tick = 0;

    DBG_vPrintf(TRACE_LIGHT_TASK, "\nLight %s | %s | %s | %s",
    		sLight[u8Index].sBasicServerCluster.au8ManufacturerName,
    		sLight[u8Index].sBasicServerCluster.au8ModelIdentifier,
    		sLight[u8Index].sBasicServerCluster.au8SWBuildID,
    		sLight[u8Index].sBasicServerCluster.au8DateCode);
}

/****************************************************************************
 *
 * NAME: APP_vHandleIdentify
 *
 * DESCRIPTION:
 * ZLL Device Specific identify
 *
 * PARAMETER: the identify time
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void APP_vHandleIdentify(uint8 u8Index, uint16 u16Time) {

    uint8 u8Red=0, u8Green=0, u8Blue=0;

    DBG_vPrintf(TRACE_LIGHT_TASK, "JP Time %d\n", u16Time);

    if (sIdEffect[u8Index].u8Effect < E_CLD_IDENTIFY_EFFECT_STOP_EFFECT) {
        /* do nothing */
        //DBG_vPrintf(TRACE_LIGHT_TASK, "Effect do nothing\n");
    	DBG_vPrintf(TRUE, "\n#%d, Identify Stop", u8Index);
    }
    else if (u16Time == 0)
    {
    	DBG_vPrintf(TRUE, "\n#%d, Identify End", u8Index);

            /*
             * Restore to off/off/colour state
             */


		if (sDeviceTable.asDeviceRecords[u8Index].u16DeviceId == COLOUR_LIGHT_DEVICE_ID)
		{
	        vApp_eCLD_ColourControl_GetRGB(u8Index, &u8Red, &u8Green, &u8Blue);
		}
        DBG_vPrintf(TRACE_LIGHT_TASK, "R %d G %d B %d L %d Hue %d Sat %d\n", u8Red, u8Green, u8Blue,
                            sLight[u8Index].sLevelControlServerCluster.u8CurrentLevel,
                            sLight[u8Index].sColourControlServerCluster.u8CurrentHue,
                            sLight[u8Index].sColourControlServerCluster.u8CurrentSaturation);

        //DBG_vPrintf(TRACE_LIGHT_TASK, "\nidentify stop");

        vRGBLight_SetLevels(u8Index,
                            sLight[u8Index].sOnOffServerCluster.bOnOff,
                            sLight[u8Index].sLevelControlServerCluster.u8CurrentLevel,
                            u8Red,
                            u8Green,
                            u8Blue);
    }
    else
    {
    	uint8 u8Level = CLD_LEVELCONTROL_MAX_LEVEL;

    	DBG_vPrintf(TRUE, "\n#%d, Identify Start", u8Index);

        /* Set the Identify levels */
		if (sDeviceTable.asDeviceRecords[u8Index].u16DeviceId == COLOUR_LIGHT_DEVICE_ID)
		{
			u8Red = 250;
			u8Green = 0;
			u8Blue = 0;
		}
    	vRGBLight_SetLevels(u8Index, 
							TRUE, 
							u8Level,
							u8Red,
	                        u8Green,
	                        u8Blue);
    }
}

/****************************************************************************
 *
 * NAME: vIdEffectTick
 *
 * DESCRIPTION:
 * ZLL Device Specific identify tick
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vIdEffectTick(uint8 u8Endpoint) {

    //DBG_vPrintf(TRUE, "\n#%d, Effect Tick", u8Index, sIdEffect[u8Index].u8Tick);
    if (u8Endpoint < LIGHT_COLORLIGHT_LIGHT_00_ENDPOINT || u8Endpoint >= LIGHT_COLORLIGHT_COMMISSION_ENDPOINT) {
        return;
    }
	uint8 u8Index = ENDPOINT_TO_LIGHT(u8Endpoint);

    //DBG_vPrintf(TRUE, "\n#%d, %d, Effect Tick", u8Index, u8Endpoint, sIdEffect[u8Index].u8Tick);

    if (sIdEffect[u8Index].u8Effect < E_CLD_IDENTIFY_EFFECT_STOP_EFFECT)
    {
        if (sIdEffect[u8Index].u8Tick > 0)
        {
            DBG_vPrintf(TRACE_PATH, "\nPath 5");

            sIdEffect[u8Index].u8Tick--;

            /* Set the light parameters */
            vRGBLight_SetLevels(u8Index,
            		TRUE,
            		sIdEffect[u8Index].u8Level,
            		sIdEffect[u8Index].u8Red,
            		sIdEffect[u8Index].u8Green,
            		sIdEffect[u8Index].u8Blue);

            /* Now adjust parameters ready for for next round */
            switch (sIdEffect[u8Index].u8Effect) {
                case E_CLD_IDENTIFY_EFFECT_BLINK:
                    break;

                case E_CLD_IDENTIFY_EFFECT_BREATHE:
                    if (sIdEffect[u8Index].bDirection) {
                        if (sIdEffect[u8Index].u8Level >= 250) {
                            sIdEffect[u8Index].u8Level -= 50;
                            sIdEffect[u8Index].bDirection = 0;
                        } else {
                            sIdEffect[u8Index].u8Level += 50;
                        }
                    } else {
                        if (sIdEffect[u8Index].u8Level == 0) {
                            // go back up, check for stop
                            sIdEffect[u8Index].u8Count--;
                            if ((sIdEffect[u8Index].u8Count) && ( !sIdEffect[u8Index].bFinish)) {
                                sIdEffect[u8Index].u8Level += 50;
                                sIdEffect[u8Index].bDirection = 1;
                            } else {
                                //DBG_vPrintf(TRACE_LIGHT_TASK, "\n>>set tick 0<<");
                                /* lpsw2773 - stop the effect on the next tick */
                                sIdEffect[u8Index].u8Tick = 0;
                            }
                        } else {
                            sIdEffect[u8Index].u8Level -= 50;
                        }
                    }
                    break;
                default:
                    if ( sIdEffect[u8Index].bFinish ) {
                        sIdEffect[u8Index].u8Tick = 0;
                    }
                }
        } else {
            /*
             * Effect finished, restore the light
             */
            DBG_vPrintf(TRACE_PATH, "\nPath 6");
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_STOP_EFFECT;
            sIdEffect[u8Index].bDirection = FALSE;
            APP_ZCL_vSetIdentifyTime(u8Index, 0);

            uint8 u8Red, u8Green, u8Blue;
            vApp_eCLD_ColourControl_GetRGB(u8Index, &u8Red, &u8Green, &u8Blue);
            DBG_vPrintf(TRACE_LIGHT_TASK, "EF - R %d G %d B %d L %d Hue %d Sat %d\n",
                                u8Red,
                                u8Green,
                                u8Blue,
                                sLight[u8Index].sLevelControlServerCluster.u8CurrentLevel,
                                sLight[u8Index].sColourControlServerCluster.u8CurrentHue,
                                sLight[u8Index].sColourControlServerCluster.u8CurrentSaturation);

            vRGBLight_SetLevels(u8Index,
                                sLight[u8Index].sOnOffServerCluster.bOnOff,
                                sLight[u8Index].sLevelControlServerCluster.u8CurrentLevel,
                                u8Red,
                                u8Green,
                                u8Blue);
        }
    }
}

/****************************************************************************
 *
 * NAME: vStartEffect
 *
 * DESCRIPTION:
 * ZLL Device Specific identify effect set up
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vStartEffect(uint8 u8Index, uint8 u8Effect) {
    switch (u8Effect) {
        case E_CLD_IDENTIFY_EFFECT_BLINK:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Blink", u8Index);
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_BLINK;
            sIdEffect[u8Index].u8Level = 250;
            sIdEffect[u8Index].u8Red = 255;
            sIdEffect[u8Index].u8Green = 0;
            sIdEffect[u8Index].u8Blue = 0;
            sIdEffect[u8Index].bFinish = FALSE;
            APP_ZCL_vSetIdentifyTime(u8Index, 2);
            sIdEffect[u8Index].u8Tick = 10;
            break;
        case E_CLD_IDENTIFY_EFFECT_BREATHE:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Breath", u8Index);
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_BREATHE;
            sIdEffect[u8Index].bDirection = 1;
            sIdEffect[u8Index].bFinish = FALSE;
            sIdEffect[u8Index].u8Level = 0;
            sIdEffect[u8Index].u8Count = 15;
            eCLD_ColourControl_GetRGB( LIGHT_TO_ENDPOINT(u8Index), &sIdEffect[u8Index].u8Red, &sIdEffect[u8Index].u8Green, &sIdEffect[u8Index].u8Blue);
            APP_ZCL_vSetIdentifyTime(u8Index, 17);
            sIdEffect[u8Index].u8Tick = 200;
            break;
        case E_CLD_IDENTIFY_EFFECT_OKAY:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Okay", u8Index);
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_OKAY;
            sIdEffect[u8Index].bFinish = FALSE;
            sIdEffect[u8Index].u8Level = 250;
            sIdEffect[u8Index].u8Red = 0;
            sIdEffect[u8Index].u8Green = 255;
            sIdEffect[u8Index].u8Blue = 0;
            APP_ZCL_vSetIdentifyTime(u8Index, 2);
            sIdEffect[u8Index].u8Tick = 10;
            break;
        case E_CLD_IDENTIFY_EFFECT_CHANNEL_CHANGE:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Channel Change", u8Index);
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_CHANNEL_CHANGE;
            sIdEffect[u8Index].u8Level = 250;
            sIdEffect[u8Index].u8Red = 255;
            sIdEffect[u8Index].u8Green = 127;
            sIdEffect[u8Index].u8Blue = 4;
            sIdEffect[u8Index].bFinish = FALSE;
            APP_ZCL_vSetIdentifyTime(u8Index, 9);
            sIdEffect[u8Index].u8Tick = 80;
            break;

        case E_CLD_IDENTIFY_EFFECT_FINISH_EFFECT:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Finish", u8Index);
            if (sIdEffect[u8Index].u8Effect < E_CLD_IDENTIFY_EFFECT_STOP_EFFECT)
            {
                DBG_vPrintf(TRACE_LIGHT_TASK, "\n<FINISH>");
                sIdEffect[u8Index].bFinish = TRUE;
            }
            break;
        case E_CLD_IDENTIFY_EFFECT_STOP_EFFECT:
        	DBG_vPrintf(TRUE, "\n#%d, Effect Stop", u8Index);
            sIdEffect[u8Index].u8Effect = E_CLD_IDENTIFY_EFFECT_STOP_EFFECT;
            APP_ZCL_vSetIdentifyTime(u8Index, 1);
            break;
    }
}


/****************************************************************************
 *
 * NAME: vRGBLight_SetLevels
 *
 * DESCRIPTION:
 * Set the RGB and levels
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

PUBLIC void vRGBLight_SetLevels(uint8 u8Index, bool_t bOn, uint8 u8Level, uint8 u8Red, uint8 u8Green, uint8 u8Blue)
{
    if (bOn == TRUE)
    {
#ifndef MONO_ON_OFF
        vLI_Start(u8Index, u8Level, u8Red, u8Green, u8Blue, 0);
#else
        vBULB_SetLevel(u8Index, u8Level);
        vBULB_SetColour(u8Index, u8Red, u8Green, u8Blue);
#endif
    }
#ifndef MONO_ON_OFF
    else
    {
        vLI_Stop(u8Index);
    }
#endif
    vBULB_SetOnOff(u8Index, bOn);
}

/****************************************************************/
/* OS Stub functions to allow single osconfig diagram (ZLL/ZHA) */
/* to be used for all driver variants (just clear interrupt)    */
/****************************************************************/

#ifndef DR1192

OS_ISR(vISR_Timer3)
{
    (void) u8AHI_TimerFired(E_AHI_TIMER_3);
}

OS_ISR(vISR_Timer4)
{
    (void) u8AHI_TimerFired(E_AHI_TIMER_4);
}

#endif
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
