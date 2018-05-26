/*
 * Copyright (c) 2018 lactide
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hal.h"

#define WAKUPCR_EVWUPIEN_Mask 0x80000000uL
#define SCR_DEEPSLEEP_Mask 0x04uL

extern ioline_t row_list[MATRIX_ROWS];
extern ioline_t col_list[MATRIX_COLS];

__attribute__ ((weak))
void pok3r_led_suspend(void) {
};

__attribute__ ((weak))
void pok3r_led_wakeup(void) {
};

void suspend_power_down(void) {
    pok3r_led_suspend();
    /* enable all rows */
    for (int row = 0; row < MATRIX_ROWS; row++) {
        palClearLine(row_list[row]);
    }
    /* enable EXTI clock */
    CKCU->APBCCR0 |= CKCU_APBCCR0_EXTIEN;
    /* enable Event Wakeup Interrupt */
    NVIC_EnableIRQ(EVWUP_IRQn);
    /* enable wakeup interrupt */
    EXTI->WAKUPCR = WAKUPCR_EVWUPIEN_Mask;
    for (int col = 0; col < MATRIX_COLS; col++) {
        /* enable event wakeup for all column pads */
        EXTI->WAKUPCR |= (1 << PAL_PAD(col_list[col]));
        /* set wakeup polarity to low */
        EXTI->WAKUPPOLR |= (1 << PAL_PAD(col_list[col]));
    }
    /* enable deep sleep */
    SCB->SCR |= SCR_DEEPSLEEP_Mask;
    /* enable usb low power mode */
    USB->CSR |= USBCSR_LPMODE | USBCSR_PDWN;
    /* sleep */
    __WFI();
    /* disable usb low power mode */
    USB->CSR &= ~(USBCSR_LPMODE | USBCSR_PDWN);
    /* disable deep sleep */
    SCB->SCR &= ~SCR_DEEPSLEEP_Mask;
    /* disable all rows */
    for (int row = 0; row < MATRIX_ROWS; row++) {
        palSetLine(row_list[row]);
    }
    pok3r_led_wakeup();
}

OSAL_IRQ_HANDLER(HT32_EVWUP_IRQ_VECTOR) {
    OSAL_IRQ_PROLOGUE();
    /* acknowledge interrupt */
    EXTI->WAKUPFLG |= EXTI->WAKUPFLG;
    /* disable interrupt */
    EXTI->WAKUPCR = 0;
    OSAL_IRQ_EPILOGUE();
}
