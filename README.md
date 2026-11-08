# Monitor analógico con alarma — FRDM-MCXA156

**Asignatura:** Microcontroladores — ITLA  
**Estudiante:** Thalía Aminisha Pimentel  
**Matrícula:** 2025-0531  
**Lenguaje de programación:** C  
**Repositorio:** `thaliaparcial2micro`  
**Placa:** NXP FRDM-MCXA156  
**Microcontrolador:** NXP MCXA156VLL — núcleo Arm Cortex-M33

## 1. Objetivo del proyecto

Desarrollar un monitor de señal analógica que lea la posición de un potenciómetro mediante el convertidor analógico-digital (ADC), convierta el resultado a porcentaje y controle un LED de alarma según un umbral configurable.

La aplicación combina **ADC, GPIO, TIMER y UART**. Tiene estados **RUN** y **PAUSE**, un pulsador **SW2**, telemetría por puerto serie y una banda de histéresis para evitar cambios repetidos de la alarma cerca del umbral.

## 2. Herramientas y versiones

| Herramienta | Uso | Versión / estado |
|---|---|---|
| Visual Studio Code | Edición y desarrollo en C | Versión exacta por confirmar en el equipo |
| Extensión MCUXpresso for VS Code (NXP) | Importación, compilación y depuración | Versión exacta por confirmar |
| MCUXpresso SDK para FRDM-MCXA156 | Drivers y soporte de la placa | **26.9.0**, registrada en el proyecto original; confirmar para este repositorio |
| Arm GNU Toolchain | Compilador cruzado para Arm | **14.2.1**, registrada en el entorno anterior; confirmar instalación actual |
| LinkServer | Programación y depuración mediante MCU-Link | **26.6.137**, registrada en las pruebas previas; confirmar versión actual |
| GitHub Desktop | Control de versiones | Versión exacta por confirmar |
| Windows | Sistema anfitrión | Versión exacta por confirmar |

> Las versiones indicadas proceden del entorno donde se probó el firmware original. Antes de entregar la práctica, comprobar y actualizar las versiones realmente utilizadas para compilar **este repositorio**. No se deben inventar números de versión.

## 3. Mapa de pines y conexiones

| Elemento | Conector / pin de la placa | Pin del MCU o periférico | Función |
|---|---|---|---|
| Potenciómetro — cursor (central) | **A0 / J4-2** | **P1_10 / ADC1_A8** | Entrada analógica de 0 a 3.3 V |
| Potenciómetro — extremo de alimentación | **J3-8** | **LDO_3V3** | Alimentación de 3.3 V |
| Potenciómetro — extremo de tierra | **J3-12** o **J3-14** | **GND** | Referencia común |
| LED rojo de actividad | **D2 / J1-6** | **P3_1 / GPIO3 pin 1** | Salida digital, resistencia de 330 Ω |
| LED blanco de alarma | **D4 / J1-10** | **P3_31 / GPIO3 pin 31** | Salida digital, resistencia de 330 Ω |
| Pulsador integrado SW2 | En la placa | **P1_7 / GPIO1 pin 7** | Entrada digital activa en nivel bajo |
| UART con PC | **USB-C J21, MCU-Link VCOM** | **LPUART0** (pines P0_2 y P0_3) | Comunicación serial virtual |

**Importante:** D2 y D4 en esta tabla son **posiciones del conector Arduino**, no los diodos LED integrados de la placa. Los LED rojo y blanco son componentes externos.

### 3.1 Esquema de conexiones (vista funcional)

```text
                    FRDM-MCXA156
               +-----------------------+
J3-8   3.3 V --+---- extremo 1         |
               |      Potenciómetro    |
J4-2   A0   <--+---- cursor central    |  ADC1_A8 / P1_10
J3-12  GND  ---+---- extremo 2         |
               |                       |
J1-6   D2   ---+--[330 Ω]-->|-- GND    |  LED rojo (actividad)
               |                       |
J1-10  D4   ---+--[330 Ω]-->|-- GND    |  LED blanco (alarma)
               |                       |
               | SW2 integrado (P1_7)  |  RUN / PAUSE
               |                       |
PC <--- USB ---+ J21 MCU-Link / VCOM   |  UART 115200, 8N1
               +-----------------------+
```

- Los dos LED deben llevar **una resistencia limitadora individual**.
- Usar un potenciómetro de **10 kΩ** para ajustarse al material exigido. Las pruebas anteriores se realizaron con uno de **5 kΩ**.
- **No aplicar 5 V** a la entrada A0. Desconectar la alimentación antes de cambiar las conexiones.

## 4. ADC: referencia, resolución y conversión

| Parámetro | Configuración |
|---|---|
| Módulo | **LPADC1 (ADC1)** |
| Entrada | **Canal A8**, pin **P1_10 / A0** |
| Tipo de lectura | Simple (single-ended) |
| Reloj funcional de ADC | **FRO12M / 1 = 12 MHz** |
| Referencia positiva | **VREFH = VDDA_MCU**, aproximadamente **3.3 V** en la placa |
| Referencia negativa | **GND / VREFL** |
| Resolución seleccionada en firmware | **Modo de alta resolución de 16 bits** (`kLPADC_ConversionResolutionHigh`) |
| Rango usado por el programa | **0 a 65535** (`uint16_t`) |
| Intervalo de solicitud de lectura | **100 ms** |

**Nota sobre resolución:** NXP describe el bloque como un ADC SAR de **12 bits**, que admite un **modo de conversión de alta resolución de 16 bits**. El firmware selecciona explícitamente ese modo; por eso interpreta resultados en el rango 0–65535. La cantidad de bits del resultado no garantiza por sí sola una precisión analógica efectiva equivalente.

La referencia nominal es **3.3 V**, pero su valor real depende de la alimentación de la placa; debe medirse con multímetro si se requiere precisión instrumental.

**Conversión del código ADC a porcentaje:**

```text
PCT = (ADC × 100 + 32767) / 65535
```

Se usa aritmética de 32 bits y redondeo entero. Por ejemplo, una lectura próxima a la mitad del rango corresponde aproximadamente a **50 %**.

**Tamaño nominal de un paso de código en modo de 16 bits**, suponiendo VREF = 3.3 V:

```text
3.3 V / 65535 ≈ 0.00005035 V ≈ 50.35 µV por código
```

Este valor expresa el paso numérico ideal, **no** la precisión garantizada de la medición.

## 5. TIMER: configuración y cálculo del período

Se utiliza **CTIMER1** para obtener una base de tiempo de **1 milisegundo (1 ms)** sin bloquear el programa principal.

| Parámetro | Configuración |
|---|---|
| Módulo | CTIMER1 |
| Fuente de reloj | FRO_HF |
| Divisor de reloj del módulo | 1 |
| Preescalador | 0 (división efectiva por 1) |
| Canal de comparación | MATCH0 |
| Reinicio de contador por coincidencia | Habilitado |
| Interrupción | Cada 1 ms, nominalmente |

**Cálculo:**

```text
T = 1 ms = 0.001 s
F_CTIMER = CLOCK_GetCTimerClkFreq(1U)
Cuentas por interrupción = F_CTIMER × T
Cuentas por interrupción = F_CTIMER / 1000
```

Si el reloj efectivo es **96 MHz**:

```text
Cuentas = 96,000,000 / 1000 = 96,000
MATCH0 = 96,000
Período nominal = 96,000 / 96,000,000 = 0.001 s = 1 ms
```

El firmware obtiene **F_CTIMER en ejecución**, por lo que no depende de suponer siempre 96 MHz. El período real debe comprobarse mediante instrumentación si la rúbrica exige tolerancias.

**Tareas programadas con este tick:**

| Tarea | Intervalo |
|---|---|
| Solicitud de conversión ADC | 100 ms |
| Cambio de estado del LED rojo | 500 ms |
| Envío de telemetría UART | 1000 ms |
| Antirrebote del SW2 | 30 ms de estabilidad |

El LED rojo cambia de estado cada 500 ms, de modo que un ciclo completo encendido-apagado dura aproximadamente **1 s**.

## 6. UART: configuración y comandos

| Parámetro | Configuración |
|---|---|
| Periférico | **LPUART0**, conectado a MCU-Link VCOM |
| Velocidad | **115200 baudios** |
| Bits de datos | **8** |
| Paridad | **Ninguna** |
| Bits de parada | **1** |
| Control de flujo | **Ninguno** |
| Recepción y transmisión | Interrupciones, con búferes circulares |
| Terminador para comandos | **LF** o **CRLF** |
| Terminador de respuestas | **CRLF** |
| Puerto en Windows | El asignado al MCU-Link VCOM; durante las pruebas originales fue **COM3** |

### 6.1 Comandos admitidos

| Comando | Respuesta esperada | Acción |
|---|---|---|
| `STATUS` | Una línea de telemetría | Muestra estado y mediciones |
| `START` | `OK RUN` | Activa el monitoreo |
| `STOP` | `OK PAUSE` | Suspende la alarma, sin detener la telemetría |
| `THR 50` | `OK THR=50` | Establece umbral en 50 % |
| `THR 95` | `OK THR=95` | Establece umbral en 95 % |
| `THR 96` | `ERR` | Rechaza umbral fuera del intervalo 10–95 |
| Otro comando desconocido | `ERR` | No modifica la configuración |

**Formato de telemetría:**

```text
t=200;ADC=0;PCT=0;THR=70;MODE=RUN;ALARM=0
```

En cada línea se reportan milisegundos (`t`), lectura ADC, porcentaje, umbral, modo y estado de alarma. El sistema transmite una línea automáticamente cada **1000 ms**.

## 7. Funcionamiento de la alarma

El estado inicial previsto es **RUN**, con umbral **70 %** y LED de alarma apagado hasta disponer de una lectura válida.

- **RUN:** procesa el ADC y permite la activación de la alarma.
- **PAUSE:** mantiene la adquisición y telemetría, pero apaga la alarma.
- **Histéresis:** se enciende al llegar a `PCT >= THR`; se apaga al bajar de `PCT < THR - 5`; dentro de esa banda conserva su estado anterior.
- **SW2:** una pulsación estable de 30 ms alterna RUN/PAUSE; mantener el pulsador oprimido no genera pulsaciones adicionales.

Ejemplo con umbral de 50 %: la alarma se enciende al alcanzar **50 %**; si está encendida y el porcentaje desciende a **47 %**, permanece encendida; se apaga al caer por debajo de **45 %**.

## 8. Estructura del proyecto

```text
thaliaparcial2micro/
├── CMakeLists.txt
├── prj.conf
├── README.md
├── src/
│   ├── lpadc_polling.c       # Aplicación principal
│   ├── hardware_init.c       # Inicialización de hardware
│   ├── pin_mux.c             # Configuración de pines
│   ├── app.h
│   ├── pin_mux.h
│   ├── alarm_core.h          # Estados, comandos, histéresis y SW2
│   ├── adc_control.h        # Módulo auxiliar del desarrollo
│   ├── led_control.h        # Módulo auxiliar del desarrollo
│   ├── timer_control.h      # Módulo auxiliar del desarrollo
│   ├── uart_control.h       # Módulo auxiliar del desarrollo
│   └── periodic_tasks.h     # Módulo auxiliar del desarrollo
└── evidencias/
    └── RESULTADOS_P01_P10.md
```

**Alcance de la integración:** `lpadc_polling.c` contiene la implementación principal de los periféricos y utiliza `alarm_core.h` para la lógica. Los demás encabezados auxiliares fueron creados durante el desarrollo incremental, pero **no están integrados en la aplicación principal** en esta versión.

## 9. Instrucciones de compilación y ejecución

1. Instalar **Visual Studio Code**, la extensión **MCUXpresso for VS Code**, el **MCUXpresso SDK para FRDM-MCXA156**, **Arm GNU Toolchain** y **LinkServer**.
2. Abrir en VS Code la carpeta raíz **`thaliaparcial2micro`** (donde se encuentra `CMakeLists.txt`), no solamente la carpeta `src`.
3. Desde el panel **MCUXpresso**, importar **el proyecto CMake existente**, seleccionando el SDK para FRDM-MCXA156 y la herramienta Arm GNU. No es necesario importar un nuevo ejemplo.
4. Verificar que `CMakeLists.txt` contenga las selecciones `set(board frdmmcxa156)` y `set(board_root examples/_boards)` y que incluya `src/lpadc_polling.c`.
5. Seleccionar el proyecto **`analog_alarm`** y ejecutar **Build Project**. Revisar la salida de compilación; si hay errores, corregirlos antes de programar la placa.
6. Con la alimentación desconectada, revisar las conexiones del potenciómetro y las resistencias de ambos LED. Conectar después el cable USB-C del equipo al conector **J21 (MCU-Link)** de la placa.
7. En la extensión MCUXpresso, iniciar **Debug/Flash** para cargar el programa. Si la ejecución se detiene al entrar en `main`, pulsar **Continue/Resume**.
8. Abrir el monitor serial y seleccionar el puerto **MCU-Link VCOM** correspondiente (puede ser COM3 u otro). Configurar **115200, 8N1** y enviar `STATUS` terminado en LF o CRLF.
9. Girar el potenciómetro y observar los cambios en `ADC` y `PCT`. Verificar el parpadeo del LED rojo, el encendido/apagado del LED blanco, los comandos `THR`, `STOP`, `START` y el funcionamiento de SW2.
10. Registrar capturas, resultados de pruebas y **hash del commit que realmente se compiló y grabó** para documentar la versión evaluada.

**Nota:** las líneas rojas de IntelliSense debajo de `#include` no demuestran por sí solas un error. El criterio para validar la compilación es el resultado real de **Build Project** con el SDK y toolchain configurados.

## 10. Evidencias y estado de verificación

En el proyecto original `examen2microthalia` se confirmó la compilación, carga y funcionamiento de ADC, LED, botón SW2 y comandos UART en la FRDM-MCXA156. El **6 de noviembre de 2026**, una prueba automatizada P10 identificó **150 respuestas de 150 comandos** y **30 intervalos de telemetría de 1000 ms**; la latencia máxima aproximada registrada desde Python/Windows fue de **16 ms**.

Esos resultados **corresponden al firmware original** y **no certifican automáticamente** que `thaliaparcial2micro` compile o funcione en la placa. Este nuevo repositorio tiene **pendiente** la validación de Build, Flash y pruebas físicas. También faltan algunas mediciones instrumentales solicitadas en la rúbrica. Se detallan en [RESULTADOS_P01_P10.md](evidencias/RESULTADOS_P01_P10.md).

Los registros P10 originales están disponibles en el repositorio previamente probado:

- [Registro de prueba P10 (TXT)](https://github.com/thaliaaminishapimentel/examen2microthalia/blob/main/evidencias/P10_log_20261106_210046.txt)
- [Respuestas de los 150 comandos (CSV)](https://github.com/thaliaaminishapimentel/examen2microthalia/blob/main/evidencias/P10_respuestas_20261106_210046.csv)

## 11. Referencias técnicas

- [NXP — Manual de la placa FRDM-MCXA156, UM12121](https://www.nxp.com/docs/en/user-manual/UM12121.pdf)
- [NXP — Guía de inicio FRDM-MCXA156](https://www.nxp.com/document/guide/getting-started-with-frdm-mcxa156:GS-FRDM-MCXA156)
- [NXP — Referencia de API MCXA156 y reloj CTIMER](https://mcuxpresso.nxp.com/mcuxsdk/latest/api/devices/MCXA156/index.html)
- [NXP — Driver LPADC y modos de resolución](https://mcuxpresso.nxp.com/api_doc/dev/4178/a00036.html)

---

**Estado del proyecto al redactar este documento:** firmware organizado en el repositorio, **pendiente de compilación y validación física de esta versión**.
