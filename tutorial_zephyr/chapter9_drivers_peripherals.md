# Chapter 9: Sterowniki i peryferia płytki

[← Spis treści](README.md)

**Cel rozdziału:** zrozumieć model urządzeń Zephyra (device model),
korzystać z peryferiów NUCLEO-H743ZI przez jednolite API (PWM, czujnik
temperatury, UART) i wiedzieć, jak dodać nowy czujnik (np. I2C) bez pisania
sterownika.

---

## 1. Device model

Każde urządzenie z drzewa urządzeń (`status = "okay"`) ma w programie
instancję `struct device`, utworzoną przez sterownik podczas startu. Kod
aplikacji dostaje do niej wskaźnik **w czasie kompilacji**:

```c
static const struct device *const die_temp = DEVICE_DT_GET(DT_ALIAS(die_temp0));

if (!device_is_ready(die_temp)) {
	return;                          // sterownik się nie zainicjował
}
```

Zasady, które powtarzają się w całym Zephyrze:

1. **Znajdź węzeł** w DT (alias, etykieta) → makro `DT_*`.
2. **Pobierz urządzenie**: `DEVICE_DT_GET(node)` lub struktura `*_dt_spec`.
3. **Sprawdź gotowość**: `device_is_ready()` / `gpio_is_ready_dt()` / `pwm_is_ready_dt()`.
4. **Wołaj API klasy** (`gpio_`, `pwm_`, `sensor_`, `uart_`, `i2c_`, `spi_`, `adc_`...).
5. **Sprawdzaj wynik**: funkcje zwracają `0` albo **ujemny kod errno** (`-EINVAL`, `-ENODEV`...).

Struktury `*_dt_spec` (`gpio_dt_spec`, `pwm_dt_spec`, `i2c_dt_spec`, `spi_dt_spec`,
`adc_dt_spec`) łączą **urządzenie + parametry z DT** (pin, kanał, okres) w jednym
obiekcie, więc nie przekazujesz ich ręcznie.

## 2. Co ma Twoja płytka w DTS

Peryferia włączone (`status = "okay"`) w `nucleo_h743zi.dts`:

| Peryferium | Szczegóły |
|---|---|
| **USART3** | Konsola i shell, PD8/PD9, 115200, VCP ST-LINK |
| **LPUART1** | PB6/PB7, 115200 (`arduino_serial`) |
| **I2C1** | PB8/PB9, 400 kHz (*fast*) |
| **SPI1** | PA5/PA6/PB5, CS na PD14 |
| **TIM12 PWM** | CH1 na PB14: czerwona dioda LD3 (alias `pwm-led0`, okres 20 ms) |
| **ADC1 / ADC3** | PA3 (ADC1 IN15), PF3 (ADC3 IN5) |
| **DAC1** | PA4 |
| **Czujniki wewnętrzne** | `die-temp0` (temperatura struktury), `volt-sensor0/1` (VREF, VBAT) |
| **FDCAN1** | PD0/PD1 |
| **Ethernet (RMII)** | MAC + PHY |
| **USB OTG FS** | PA11/PA12 |
| **RNG, RTC, IWDG1, Backup SRAM** | |

Włączenie peryferium w DT **nie wystarczy**: w Kconfig musi być włączona
**klasa sterowników** (np. `CONFIG_PWM=y`, `CONFIG_SENSOR=y`). Konkretny
sterownik (np. `CONFIG_I2C_STM32`) włącza się zwykle **sam**, gdy w DT jest
odpowiedni włączony węzeł (patrz sekcja 5).

## 3. Przykład: PWM na LD3, temperatura struktury i UART

```
# prj.conf
CONFIG_PWM=y
CONFIG_SENSOR=y
CONFIG_ADC=y
```

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>

/* Czerwona dioda LD3 podpięta do PWM (alias pwm-led0 z dts płytki). */
static const struct pwm_dt_spec red = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

/* Czujnik temperatury struktury (alias die-temp0). */
static const struct device *const die_temp = DEVICE_DT_GET(DT_ALIAS(die_temp0));

/* UART z devicetree: ten sam, co konsola (VCP ST-LINK). */
static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int main(void)
{
	if (!pwm_is_ready_dt(&red) || !device_is_ready(die_temp) || !device_is_ready(console)) {
		printk("urzadzenie niegotowe\n");
		return 0;
	}

	uart_poll_out(console, '>');		/* niskopoziomowy zapis jednego znaku */
	uart_poll_out(console, '\n');

	for (int pct = 0;; pct = (pct + 10) % 110) {
		int ret = pwm_set_dt(&red, red.period, red.period / 100 * pct);

		if (ret < 0) {
			printk("pwm_set_dt: %d\n", ret);
		}

		struct sensor_value temp;

		if (sensor_sample_fetch(die_temp) == 0 &&
		    sensor_channel_get(die_temp, SENSOR_CHAN_DIE_TEMP, &temp) == 0) {
			printk("jasnosc %3d%%, temperatura struktury: %d.%02d C\n",
			       pct, temp.val1, temp.val2 / 10000);
		}

		k_msleep(200);
	}
}
```

> Ten przykład został **zbudowany** dla `nucleo_h743zi` (Flash 32,5 KB, RAM 5,2 KB),
> ale nie uruchomiony, bo środowisko pracy nie miało podłączonej płytki. Oczekiwane
> zachowanie: dioda LD3 stopniowo rozjaśnia się co 200 ms, a konsola wypisuje
> temperaturę struktury. Sprawdź na sprzęcie.

Co tu się dzieje:

| Element | Wyjaśnienie |
|---|---|
| `PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0))` | Struktura z kontrolerem PWM (TIM12), kanałem 1, **okresem 20 ms** (z `PWM_MSEC(20)` w DTS) i polaryzacją |
| `pwm_set_dt(&red, period, pulse)` | Ustawia okres i szerokość impulsu w **nanosekundach** (typ `uint32_t`) |
| `struct sensor_value` | Liczba stałoprzecinkowa: `val1` = część całkowita, `val2` = część ułamkowa w **milionowych**. Stąd `val2 / 10000` daje setne |
| `sensor_sample_fetch` + `sensor_channel_get` | Model dwuetapowy: najpierw **odczyt z sprzętu**, potem **pobranie kanału** z bufora sterownika |
| `DT_CHOSEN(zephyr_console)` | Urządzenie wskazane w `chosen` (USART3), bez wpisywania jego nazwy |
| `uart_poll_out` | Najprostszy, **blokujący** zapis znaku. Tu tylko jako ilustracja |

> Uwaga: `uart_poll_out` na konsoli miesza się z wyjściem `printk`. W prawdziwym
> kodzie dla własnej komunikacji użyj **drugiego UART-a** (np. `lpuart1`).

### Trzy style API UART

| Styl | Funkcje | Kiedy |
|---|---|---|
| **Polling** | `uart_poll_in`, `uart_poll_out` | Najprostsze, blokujące, krótkie komunikaty |
| **Sterowane przerwaniami** | `uart_irq_callback_user_data_set`, `uart_irq_rx_enable` | Odbiór bajt po bajcie z callbackiem |
| **Asynchroniczne (DMA)** | `uart_callback_set`, `uart_rx_enable`, `uart_tx` | Ramki/bufory, najmniejsze obciążenie CPU |

## 4. Dostęp do pól struktury `sensor_value`

```c
struct sensor_value temp;                  // np. val1 = 41, val2 = 270000
double celsius = sensor_value_to_double(&temp);   // 41.27
```

Na MCU trzymaj się wartości całkowitych jak w przykładzie. Wypisywanie `float`
przez `printk`/logi wymaga osobnej opcji (`CONFIG_CBPRINTF_FP_SUPPORT=y`) i
zwiększa rozmiar programu, więc włączaj ją świadomie.

## 5. Dodawanie czujnika I2C bez pisania sterownika

Załóżmy, że podłączasz czujnik BME280 do **I2C1** (PB8/PB9, już włączony w
DTS płytki). Wystarczą trzy rzeczy.

**1. Overlay** (`boards/nucleo_h743zi.overlay`): węzeł potomny magistrali, adres
na I2C:

```dts
&i2c1 {
	bme280: bme280@76 {
		compatible = "bosch,bme280";
		reg = <0x76>;
	};
};
```

**2. `prj.conf`**: klasy sterowników:

```
CONFIG_I2C=y
CONFIG_SENSOR=y
```

**3. Kod**: urządzenie pobierasz po etykiecie:

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/* Węzeł potomny magistrali I2C1 zdefiniowany w overlayu. */
static const struct device *const bme = DEVICE_DT_GET(DT_NODELABEL(bme280));

int main(void)
{
	if (!device_is_ready(bme)) {
		printk("BME280 niegotowy (brak czujnika na magistrali?)\n");
		return 0;
	}

	while (1) {
		struct sensor_value temp;

		if (sensor_sample_fetch(bme) == 0 &&
		    sensor_channel_get(bme, SENSOR_CHAN_AMBIENT_TEMP, &temp) == 0) {
			printk("T = %d.%02d C\n", temp.val1, temp.val2 / 10000);
		}
		k_msleep(1000);
	}
}
```

Ten przykład **zbudował się** dla `nucleo_h743zi` (Flash 34 KB). Zwróć uwagę na
`.config`: nie wpisywałeś `CONFIG_BME280` ani `CONFIG_I2C_STM32`, a mimo to:

```
CONFIG_I2C_STM32=y
CONFIG_BME280=y
```

To **Devicetree steruje Kconfig**: sterownik ma domyślnie `y`, jeśli w DT
istnieje włączony węzeł o pasującym `compatible`. Dlatego brak czujnika na
magistrali nie powoduje błędu budowania, tylko `device_is_ready()` zwróci
`false` w czasie działania (dlatego **zawsze** je sprawdzaj).

### Błąd: węzeł istnieje, ale urządzenia nie ma

Jeśli ten sam węzeł oznaczysz `status = "disabled"` (albo nie będzie dla niego
sterownika), `DEVICE_DT_GET` przestaje się kompilować:

```
error: '__device_dts_ord_157' undeclared here (not in a function); did you mean '__device_dts_ord_57'?
```

Rozpoznasz go po `__device_dts_ord_<liczba>`. Porównaj z błędem z Chapter 2:

| Komunikat | Znaczenie |
|---|---|
| `DT_N_ALIAS_led0_...` undeclared | **Nie ma** węzła/aliasu w drzewie |
| `__device_dts_ord_<N>` undeclared | Węzeł **jest**, ale **nie ma jego urządzenia** (wyłączony lub brak sterownika) |

> Wartości wielu czujników i adresy I2C zależą od modułu (np. BME280 ma
> adres `0x76` lub `0x77`). Sprawdź dokumentację swojego modułu.

## 6. Własny sterownik lub binding

Gdy brak gotowego sterownika, piszesz:
1. **binding** (plik YAML opisujący `compatible` i właściwości),
2. **sterownik** (C) rejestrowany makrem `DEVICE_DT_INST_DEFINE`.

To temat poza zakresem tego kursu. Zacznij od przeczytania małego,
gotowego sterownika (np. prostego czujnika w `zephyr/drivers/sensor`).

## 🔧 Różnica względem STM32 HAL

| STM32 HAL (CubeMX) | Zephyr |
|---|---|
| `HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1)` | `pwm_set_dt(&red, period, pulse)` |
| Uchwyty `htimX`, `huartX` globalne | Wskaźnik `const struct device *` z `DEVICE_DT_GET` |
| Piny w `.ioc` / `MX_*_Init` | Piny w DT (`pinctrl-0`) |
| Sterownik czujnika pisany ręcznie | Węzeł w DT + `CONFIG_*` (często automatycznie) |
| Zwraca `HAL_StatusTypeDef` | Zwraca `0` lub ujemne errno |
| Kod zależny od układu (`stm32h7xx_hal_*.h`) | Kod przenośny między płytkami (`gpio_`, `pwm_`, `sensor_`) |

---

## ✅ Zadania

1. Zbuduj i wgraj przykład z PWM. Zmień okres w DTS (przez overlay, węzeł
   `red_pwm_led`, `pwms = <&pwm12 1 PWM_MSEC(10) PWM_POLARITY_NORMAL>;`) i
   sprawdź, czy kod nadal działa. Dlaczego nie trzeba go zmieniać?
2. Zmodyfikuj przykład tak, aby jasność zmieniała się płynnie
   (rozjaśnianie i przyciemnianie) w osobnym wątku.
3. Dodaj odczyt `volt-sensor0` (`VREF`) przez API `sensor_*`: alias
   `volt-sensor0`, kanał `SENSOR_CHAN_VOLTAGE` (taki kod się buduje dla tej
   płytki; w `prj.conf` użyto `CONFIG_SENSOR=y` i `CONFIG_ADC=y`).
4. Napisz komendę shella (Chapter 8) `temp`, która wypisuje temperaturę
   struktury na żądanie.
5. Zbuduj przykład z BME280 **bez** czujnika na magistrali. Co wypisze program?
   Następnie sprawdź w `.config`, które symbole zostały włączone
   automatycznie i dlaczego.
6. Zmień węzeł w overlayu na `status = "disabled"` i zbuduj. Zapisz komunikat
   błędu i wyjaśnij, czym różni się od błędu `DT_N_ALIAS_...` z Chapter 2.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 10 – Debugowanie i typowe błędy](chapter10_debugging_pitfalls.md)
