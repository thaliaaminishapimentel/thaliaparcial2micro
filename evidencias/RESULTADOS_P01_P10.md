# Evidencias P01-P10 — Monitor analogico FRDM-MCXA156

**Origen de estas observaciones:** firmware original del proyecto [`examen2microthalia`](https://github.com/thaliaaminishapimentel/examen2microthalia), probado el **6 de noviembre de 2026**.  
**Configuracion:** FRDM-MCXA156, MCU-Link VCOM COM3, UART 115200 8N1, potenciometro de **5 kOhm** durante la prueba.  
**Advertencia:** este documento no representa una validacion de la compilacion ni de la ejecucion del nuevo repositorio `thaliaparcial2micro`. El montaje final requiere revisar el potenciometro de 10 kOhm indicado en el enunciado.

## Criterios de evaluacion

- **PASS (funcional):** comportamiento observado en la practica, aunque pueden faltar capturas independientes.
- **PARCIAL:** hay observaciones de funcionamiento, pero falta al menos una condicion exigida por la prueba.
- **PENDIENTE:** aun no se cuenta con evidencia suficiente.

## Resumen por prueba

| Prueba | Estado en firmware original | Observacion | Falta para la rubrica completa |
|---|---|---|---|
| **P01** | PARCIAL | Se observo RUN, THR=70, ALARM=0 y ejecucion fisica | Reinicio documentado y STATUS a 200 ms |
| **P02** | PARCIAL | LED rojo parpadeando; 30 intervalos UART de 1000 ms segun tiempo del MCU | Medir ADC a 100 ms y LED a 500 ms con tolerancia exigida |
| **P03** | PARCIAL | ADC=0/PCT=0, lecturas intermedias y ADC=65535/PCT=100 | Comprobar voltajes de 0/50/100 % con multimetro |
| **P04** | PARCIAL | Histeresis observada con THR=50: 50 % enciende, 47 % retiene, 42 % apaga | Registrar secuencia exigida con THR=70: 60/72/67/62 % |
| **P05** | PARCIAL | SW2 alterno RUN/PAUSE; pulsacion mantenida 2 s no repitio | Documentar cinco pulsaciones y rebotes menores de 30 ms |
| **P06** | PARCIAL | STOP apago alarma sin detener lecturas; START reanudo | Ensayar secuencia exacta a PCT aproximadamente 80 % |
| **P07** | PARCIAL | Se aceptaron THR 50, 80 y 95; THR 95 apago alarma a PCT=52 % | Documentar THR 10 y todos los cambios de estado |
| **P08** | PARCIAL | THR 96, THR 50x y MODE=RUN respondieron ERR | Ensayar THR 9, THR abc y THR 50 extra |
| **P09** | PASS (funcional) | Fragmentacion STA+TUS, CRLF, y linea de 33 caracteres; hubo recuperacion posterior | Guardar captura o traza independiente |
| **P10** | PASS automatizado / PARCIAL de rubrica | 150/150 respuestas; 30 intervalos de 1 s; 16 ms maximo aprox. desde Python | Variar potenciometro durante 30 s; confirmar tiempos con instrumentacion |

## Detalle de la prueba automatizada P10

- **Fecha:** 6 de noviembre de 2026.
- **Duracion nominal:** 30 segundos.
- **Comandos enviados:** 150, unos 5 por segundo.
- **Distribucion:** 90 STATUS, 15 STOP, 15 START, 15 THR 50 y 15 THR 95.
- **Respuestas reconocidas:** 150/150.
- **Lineas inesperadas:** 0.
- **Latencia maxima observada desde Python en Windows:** aproximadamente 16 ms.
- **Latencia media estimada:** aproximadamente 3.55 ms.
- **Telemetria:** 30 intervalos consecutivos de 1000 ms segun las marcas de tiempo del microcontrolador.
- **ADC durante P10:** alrededor de 42 %; no hay evidencia de que se girara el potenciometro durante esos 30 segundos.

La latencia calculada en Python incluye posibles demoras del puerto USB, del sistema operativo y del programa de medicion. No debe describirse como una medicion certificada mediante osciloscopio.

## Archivos de evidencia originales

Los siguientes archivos estan en el **repositorio original**; no se modificaron ni se fabricaron registros nuevos para este commit:

- [P10_log_20261106_210046.txt](https://github.com/thaliaaminishapimentel/examen2microthalia/blob/main/evidencias/P10_log_20261106_210046.txt)
- [P10_respuestas_20261106_210046.csv](https://github.com/thaliaaminishapimentel/examen2microthalia/blob/main/evidencias/P10_respuestas_20261106_210046.csv)
- [Informe original detallado](https://github.com/thaliaaminishapimentel/examen2microthalia/blob/main/evidencias/RESULTADOS_P01_P10.md)

## Verificacion pendiente en thaliaparcial2micro

- [ ] Importar el proyecto CMake en MCUXpresso for VS Code y compilar sin errores.
- [ ] Grabar el nuevo firmware mediante MCU-Link.
- [ ] Confirmar ADC, LED rojo, LED blanco, SW2 y comandos UART.
- [ ] Repetir las pruebas P01-P10 exigidas por la evaluacion, con las medidas instrumentales pendientes.
- [ ] Registrar el hash del commit que efectivamente se compilo y grabo.
- [ ] Incorporar capturas, videos o registros nuevos sin mezclar sus resultados con los del firmware original.

**No declarar como completadas las verificaciones que aun no se han realizado.**
