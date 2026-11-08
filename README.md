Monitor analogico con alarma - FRDM-MCXA156
Proyecto de Microcontroladores (ITLA)  
Lenguaje: C  
Placa: NXP FRDM-MCXA156 (MCXA156, Arm Cortex-M33)  
Entorno: Visual Studio Code + MCUXpresso for VS Code  
Perifericos: GPIO, LPADC1, CTIMER1 y LPUART0
Objetivo
Desarrollar un monitor analogico que lea un potenciometro mediante ADC, convierta la lectura a porcentaje y encienda una alarma mediante un LED cuando se alcanza un umbral. El sistema dispone de modo RUN/PAUSE, control por SW2 y comandos por UART. Se emplea histeresis para evitar oscilaciones de la alarma cerca del umbral.
Conexiones
Elemento	Pin FRDM-MCXA156	Descripcion
Potenciometro, cursor	A0 / J4-2 / P1_10 / ADC1_A8	Entrada ADC, 0 a 3.3 V
Potenciometro, extremo 1	3.3 V / J3-8	Alimentacion
Potenciometro, extremo 2	GND / J3-12 o J3-14	Tierra
LED rojo de actividad	D2 / J1-6 / P3_1	Salida GPIO3 pin 1, con resistor de 330 ohmios
LED blanco de alarma	D4 / J1-10 / P3_31	Salida GPIO3 pin 31, con resistor de 330 ohmios
SW2 (integrado)	P1_7	Boton activo en bajo
UART de monitor	MCU-Link VCOM / LPUART0	USB-C MCU-Link, 115200 8N1
Seguridad: conectar el potenciometro entre 3.3 V y GND, no entre 5 V y GND; apagar la alimentacion antes de modificar cables. La actividad y alarma requieren resistencias limitadoras en los LED.
Material: el enunciado solicita potenciometro de 10 kOhm. Durante las pruebas previas se utilizo uno de 5 kOhm. Confirmar con el docente o utilizar 10 kOhm para la entrega.
Arquitectura y configuracion
ADC1: canal 8 (A0/P1_10), 16 bits, muestreo solicitado cada 100 ms.
Conversion: `PCT = (ADC * 100 + 32767) / 65535`, con operaciones de 32 bits.
CTIMER1: interrupcion cada 1 ms, que alimenta los intervalos periodicos.
LED rojo: alterna su estado cada 500 ms.
LPUART0: 115200 baudios, 8 bits, sin paridad, 1 bit de parada; RX/TX por interrupciones y buffers circulares.
Telemetria: una linea con estado y mediciones cada 1000 ms.
SW2: antirrebote de 30 ms; una pulsacion alterna RUN/PAUSE.
Inicializacion: RUN, umbral del 70 %, alarma apagada.
Histeresis: al subir enciende cuando `PCT >= THR`; al bajar apaga cuando `PCT < THR - 5`; entre ambas condiciones conserva su estado anterior.
Comandos UART
Comando	Resultado esperado
`STATUS`	Muestra `t`, `ADC`, `PCT`, `THR`, `MODE` y `ALARM`
`START`	Activa RUN y responde `OK RUN`
`STOP`	Activa PAUSE, apaga alarma y responde `OK PAUSE`
`THR 50`	Actualiza umbral y responde `OK THR=50`
`THR 95`	Actualiza umbral y responde `OK THR=95`
`THR 96` o comando desconocido	Responde `ERR`
Formato de ejemplo de una respuesta de estado:
```text
t=200;ADC=0;PCT=0;THR=70;MODE=RUN;ALARM=0
```
Estructura de archivos
```text
thaliaparcial2micro/
|-- CMakeLists.txt
|-- prj.conf
|-- README.md
|-- src/
|   |-- lpadc_polling.c       # Aplicacion principal
|   |-- hardware_init.c       # Inicializacion de placa
|   |-- pin_mux.c             # Configuracion de pines
|   |-- app.h
|   |-- pin_mux.h
|   |-- alarm_core.h          # Comandos, estado, alarma y debounce
|   |-- adc_control.h         # Modulo auxiliar preparado
|   |-- led_control.h         # Modulo auxiliar preparado
|   |-- timer_control.h       # Modulo auxiliar preparado
|   |-- uart_control.h        # Modulo auxiliar preparado
|   `-- periodic_tasks.h      # Modulo auxiliar preparado
`-- evidencias/
    `-- RESULTADOS_P01_P10.md
```
Estado de integracion: `src/lpadc_polling.c` implementa directamente la inicializacion y el manejo de los perifericos, usando `alarm_core.h` para la logica. Los encabezados auxiliares `adc_control.h`, `led_control.h`, `timer_control.h`, `uart_control.h` y `periodic_tasks.h` forman parte del desarrollo incremental, pero todavia no se utilizan desde el archivo principal. No son prueba de integracion independiente.
Compilacion y ejecucion
Instalar el MCUXpresso SDK para FRDM-MCXA156, Arm GNU Toolchain, LinkServer y la extension MCUXpresso for VS Code.
Abrir esta carpeta del repositorio en Visual Studio Code e importar el proyecto CMake con la placa FRDM-MCXA156 seleccionada y el SDK instalado.
Comprobar que `CMakeLists.txt` contenga `set(board frdmmcxa156)` y `set(board_root examples/_boards)`, y que incluya `src/lpadc_polling.c`.
Seleccionar el proyecto y ejecutar Build Project. Corregir los errores reales de compilacion, si aparecen.
Conectar la placa por el USB-C MCU-Link y cargar el firmware mediante Debug/Flash.
Abrir el monitor serie correspondiente a MCU-Link VCOM (por ejemplo COM3, si Windows lo asigna) en 115200 8N1, enviar `STATUS` y observar los LED y el potenciometro.
Importante: las lineas rojas debajo de `#include` en VS Code pueden indicar que IntelliSense aun no encuentra los encabezados del SDK. La comprobacion definitiva es el resultado de Build, no solo el editor. No es suficiente compilar con `gcc` de escritorio sin el SDK para Arm.
Estado de verificacion y evidencias
Este es un repositorio nuevo, construido en commits incrementales. Se reutilizo como referencia un firmware previo que si fue compilado y ejecutado sobre la FRDM-MCXA156. Esa validacion anterior no certifica automaticamente esta version del repositorio `thaliaparcial2micro`.
Las pruebas realizadas el 6 de noviembre de 2026 corresponden al proyecto original `examen2microthalia`:
Se observo lectura ADC, cambio del porcentaje, dos LED, SW2, modos RUN/PAUSE y comandos UART.
En P10 se registraron 150 respuestas a 150 comandos, con una latencia maxima aproximada de 16 ms medida desde Python/Windows, y 30 intervalos de telemetria de 1000 ms segun el tiempo interno reportado por el MCU.
No hubo un barrido documentado del potenciometro durante los 30 segundos de P10; tampoco se completaron todas las mediciones instrumentales de la rubrica.
Consultar `evidencias/RESULTADOS_P01_P10.md` para conocer las pruebas observadas, su estado y lo que falta realizar.
Validacion pendiente de este repositorio: ejecutar Build, Flash y las pruebas funcionales sobre la placa, y anotar el hash exacto del commit compilado. Hasta realizarlo, no afirmar que el nuevo firmware esta validado fisicamente.
Los registros originales pueden consultarse en el repositorio probado:
Log original de P10 (TXT)
Resultados P10 por comando (CSV)
Informe original de pruebas P01-P10
Referencias
NXP: primeros pasos con FRDM-MCXA156
Manual de la placa FRDM-MCXA156 UM12121
Ejemplo LPADC del SDK de NXP
Proximos pasos despues de la documentacion
Compilar el codigo de este repositorio con MCUXpresso, grabarlo en la placa, repetir al menos `STATUS`, `START`, `STOP`, `THR`, SW2, ADC, histéresis y temporizacion, y anadir capturas y resultados reales. Si algo falla, corregirlo en commits adicionales sin alterar ni inventar el historial.