/*****************************************************************************
 *
 * MODULE:             JN-AN-1171
 *
 * COMPONENT:          App_Light_ColorLight.h
 *
 * DESCRIPTION:        ZLL Demo: Colored Light - Interface
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

#ifndef APP_COLOR_LIGHT_H
#define APP_COLOR_LIGHT_H

#include "colour_light.h"
#include "dimmable_light.h"
#include "commission_endpoint.h"

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

extern tsZLL_ColourLightDevice sLight[ZLL_NUMBER_DEVICES];
//extern tsZLL_DimmableLightDevice sLight[ZLL_NUMBER_DEVICES];
extern tsCLD_ZllDeviceTable sDeviceTable;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC teZCL_Status eApp_ZLL_RegisterEndpoint(tfpZCL_ZCLCallBackFunction fptr,
                                       tsZLL_CommissionEndpoint* psCommissionEndpoint);
PUBLIC void vApp_eCLD_ColourControl_GetRGB(uint8 u8Index, uint8* pu8Red,uint8* pu8Green,uint8* pu8Blue);
PUBLIC void vAPP_ZCL_DeviceSpecific_Init(uint8 u8Index);
PUBLIC void vStartEffect(uint8 u8Index, uint8 u8Effect);
PUBLIC void vIdEffectTick(uint8 u8Endpoint);

PUBLIC void vRGBLight_SetLevels(uint8 u8Index, bool_t bOn, uint8 u8Level, uint8 u8Red,
                                uint8 u8Green, uint8 u8Blue);
PUBLIC void APP_vHandleIdentify(uint8 u8Index, uint16 u16Time);
PUBLIC void vCreateInterpolationPoints(uint8 u8Index);

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

#endif /* APP_COLOR_LIGHT_H */
