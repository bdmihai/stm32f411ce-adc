/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2020 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 15.May.2020  |
 |                                                                            |
 |___________________________________________________________________________*/

#include "stm32f4xx.h"
#include "adc.h"

void adc_init()
{
    /* set the pis B0 as input ADC */
    /* configure LED pin */
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0_Msk, 11);      /* set the pin as analog */
    MODIFY_REG(GPIOB->PUPDR, GPIO_PUPDR_PUPD0_Msk,  0);      /* no pull up, no pull down */

    /* ADC prescalar 2*/
    MODIFY_REG(ADC1_COMMON->CCR, ADC_CCR_ADCPRE_Msk, 0);

    /* 12 bit ADC*/
    MODIFY_REG(ADC1->CR1, ADC_CR1_RES_Msk, 0);

    /* select B0 channel for conversion */
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_L_Msk, 0);
    MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1_Msk, 8);

    /* ADC 1 ON */
    MODIFY_REG(ADC1->CR2, ADC_CR2_ADON_Msk, ADC_CR2_ADON);
}

uint16_t adc_analog_read()
{
  CLEAR_BIT(ADC1->SR, ADC_SR_OVR | ADC_SR_STRT);
  MODIFY_REG(ADC1->CR2, ADC_CR2_SWSTART_Msk, ADC_CR2_SWSTART);

  /* wait for start of conversion */
  do {
  } while (!(ADC1->SR & ADC_SR_STRT));

  /* wait for end of conversion */
  do {
  } while (!(ADC1->SR & ADC_SR_EOC));

  return ADC1->DR;
}
