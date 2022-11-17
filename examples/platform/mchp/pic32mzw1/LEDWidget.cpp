/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "LEDWidget.h"
#include "AppConfig.h"
#include <platform/CHIPDeviceLayer.h>
#include <peripheral/gpio/plib_gpio.h>

/*******************************************************************************
 * Macro Definitions
 *******************************************************************************/
/* Allowed TCPWM compare value for maximum brightness */
#define LED_MAX_BRIGHTNESS (100u)

/* Allowed TCPWM compare value for minimum brightness*/
#define LED_MIN_BRIGHTNESS (1u)

#define PWM_LED_FREQ_HZ (1000000u) /* in Hz */

/* subtracting from 100 since the LED is connected in active low configuration */
#define GET_DUTY_CYCLE(x) (100 - x)


#define LED_STATE_OFF 0
#define LED_STATE_ON 1


void LEDWidget::Init(int ledNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLedNum           = ledNum;
    mState            = 0;
    mbrightness       = LED_MAX_BRIGHTNESS;

    GPIO_PinOutputEnable((GPIO_PIN) mLedNum);
    GPIO_PinSet((GPIO_PIN) mLedNum); // turn off yellow led by default
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

bool LEDWidget::Get()
{
    return mState;
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = mState ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    if (mState != state)
    {
        if (state == LED_STATE_ON)
            GPIO_PinClear((GPIO_PIN) mLedNum);   
        else
            GPIO_PinSet((GPIO_PIN) mLedNum);
    }
    mState = state;
}

void LEDWidget::RGB_init()
{
    // Not Implment
}

void LEDWidget::PWM_start()
{
    // Not Implment
}

void LEDWidget::PWM_stop()
{
    // Not Implment
}

void LEDWidget::RGB_set(bool state)
{
    // Not Implment
}
void LEDWidget::SetBrightness(uint32_t led_brightness)
{
    // Not Implment
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    // Not Implment
}

void LEDWidget::HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness)
{
    // Not Implment
}
