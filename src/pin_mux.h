
/* SPDX-License-Identifier: MIT */

#ifndef PIN_MUX_H_
#define PIN_MUX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializacion de los pines de la placa */
void BOARD_InitBootPins(void);

/* Configuracion de pines y perifericos */
void BOARD_InitPins(void);

#ifdef __cplusplus
}
#endif

#endif /* PIN_MUX_H_ */
