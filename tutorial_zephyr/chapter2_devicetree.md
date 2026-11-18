# Chapter 2: Devicetree

[← Spis treści](README.md)

**Cel rozdziału:** czytać plik DTS swojej płytki, rozumieć aliasy i
makra `DT_*`, pisać własny overlay oraz wiedzieć, gdzie w Zephyrze
„mieszka” konfiguracja pinów i zegarów (zamiast w CubeMX).

---

## 1. Po co drzewo urządzeń

Devicetree to **opis sprzętu** w formie drzewa węzłów z właściwościami.
Nie zawiera kodu ani logiki, tylko fakty: „na pinie PB0 jest dioda”,
„USART3 ma piny PD8/PD9 i prędkość 115200”, „PLL mnoży przez 240”.

Dzięki temu:
- sterownik jest **ogólny**, a konkretne piny pochodzą z DT,
- kod aplikacji odwołuje się do **nazw** (aliasów), a nie numerów pinów,
- jedna płytka różni się od drugiej plikiem `.dts`, a nie kodem.

## 2. Składnia w pigułce

```dts
/ {                                  /* korzeń drzewa */
    leds: leds {                     /* etykieta "leds" i węzeł o nazwie "leds" */
        compatible = "gpio-leds";    /* wybiera "binding" (schemat) i sterownik */

        green_led: led_0 {           /* etykieta "green_led" */
            gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>;   /* odwołanie (phandle) do GPIOB, pin 0 */
            label = "User LD1";
        };
    };

    aliases {
        led0 = &green_led;           /* alias -> węzeł */
    };
};

&usart3 {                            /* modyfikacja istniejącego węzła przez etykietę */
    current-speed = <115200>;
    status = "okay";                 /* włącz węzeł */
};
```

| Pojęcie | Znaczenie |
|---|---|
| **węzeł** | Obiekt (urządzenie, grupa, ...) |
| **właściwość** | Para nazwa–wartość w węźle (`current-speed = <115200>;`) |
| **etykieta** (`label:`) | Nazwa do odwoływania się: `&label` |
| **`compatible`** | Łączy węzeł z *bindingiem* (plik YAML w `zephyr/dts/bindings`) i sterownikiem |
| **`status`** | `"okay"` = urządzenie istnieje i jest włączone; `"disabled"` = pomijane |
| **phandle** (`&gpiob`) | Odwołanie do innego węzła |

> Binding to plik YAML opisujący, jakie właściwości węzeł może/musi mieć.
> Literówka w nazwie właściwości jest wykrywana już w czasie budowania.

## 3. DTS Twojej płytki (najważniejsze fragmenty)

Plik: `zephyr/boards/st/nucleo_h743zi/nucleo_h743zi.dts`.

**Wybrane urządzenia systemowe (`chosen`)** – które urządzenie pełni rolę
konsoli, shella itd.:

```dts
chosen {
    zephyr,console = &usart3;
    zephyr,shell-uart = &usart3;
    zephyr,sram = &sram0;
    zephyr,flash = &flash0;
};
```

**Diody, przycisk i aliasy:**

```dts
leds: leds {
    compatible = "gpio-leds";
    green_led:  led_0 { gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>; label = "User LD1"; };
    yellow_led: led_1 { gpios = <&gpioe 1 GPIO_ACTIVE_HIGH>; label = "User LD2"; };
};

pwmleds {
    compatible = "pwm-leds";
    red_pwm_led: red_pwm_led {
        pwms = <&pwm12 1 PWM_MSEC(20) PWM_POLARITY_NORMAL>;   /* LD3, okres 20 ms */
    };
};

gpio_keys {
    compatible = "gpio-keys";
    user_button: button_0 {
        label = "User";
        gpios = <&gpioc 13 GPIO_ACTIVE_HIGH>;                 /* przycisk użytkownika */
        zephyr,code = <INPUT_KEY_0>;
    };
};

aliases {
    led0 = &green_led;       /* LD1 zielona, PB0 */
    led1 = &yellow_led;      /* LD2 żółta,  PE1 */
    pwm-led0 = &red_pwm_led; /* LD3 czerwona, PB14 przez TIM12 */
    sw0 = &user_button;      /* przycisk, PC13 */
    die-temp0 = &die_temp;   /* czujnik temperatury struktury */
};
```

Dlatego Twój blinky używa `led0` i `led1`: to nazwy, które każda płytka z
diodami „obiecuje” udostępniać.

**Piny w DT (zamiast CubeMX):**

```dts
&usart3 {
    pinctrl-0 = <&usart3_tx_pd8 &usart3_rx_pd9>;   /* wybór pinów (AF) */
    pinctrl-names = "default";
    current-speed = <115200>;
    status = "okay";
};
```

**Zegary w DT:**

```dts
&clk_hse { hse-bypass; clock-frequency = <DT_FREQ_M(8)>; status = "okay"; };  /* 8 MHz z ST-LINK */

&pll {
    div-m = <2>;  mul-n = <240>;  div-p = <2>;
    clocks = <&clk_hse>;
    status = "okay";
};

&rcc { clocks = <&pll>; clock-frequency = <DT_FREQ_M(480)>; ... };
```

Rachunek: 8 MHz ÷ 2 × 240 ÷ 2 = **480 MHz**. To odpowiednik drzewa zegarów z
CubeMX, tylko w tekście, który można przeglądać i różnicować w git.

## 4. Dostęp z kodu: makra `DT_*`

| Makro | Znaczenie | Przykład |
|---|---|---|
| `DT_ALIAS(x)` | Węzeł wskazywany przez alias `x` | `DT_ALIAS(led0)` |
| `DT_NODELABEL(x)` | Węzeł o etykiecie `x` | `DT_NODELABEL(usart3)` |
| `DT_CHOSEN(x)` | Węzeł z sekcji `chosen` (`,` → `_`) | `DT_CHOSEN(zephyr_console)` |
| `DT_PATH(...)` | Węzeł po ścieżce | `DT_PATH(zephyr_user)` |
| `DT_PROP(node, p)` | Wartość właściwości (`-` → `_`) | `DT_PROP(n, current_speed)` |
| `DT_NODE_EXISTS(n)` | Czy węzeł istnieje | `DT_NODE_EXISTS(DT_ALIAS(sw0))` |
| `DT_NODE_HAS_STATUS(n, okay)` | Czy włączony | |
| `GPIO_DT_SPEC_GET(n, gpios)` | Struktura (port, pin, flagi) | |
| `DEVICE_DT_GET(n)` | Wskaźnik do urządzenia | |

Zasada: nazwy w DT zapisane z myślnikami i przecinkami (`pwm-led0`,
`zephyr,console`) w kodzie mają **podkreślniki** (`pwm_led0`, `zephyr_console`).

Wszystko to makra rozwijane **w czasie kompilacji**, więc nie ma narzutu
w czasie działania.

## 5. Overlay: modyfikacja bez dotykania płytki

Nie edytujesz pliku płytki w katalogu Zephyra. Piszesz **overlay**, który
jest nakładany na DTS płytki. Zgodnie z dokumentacją Zephyra overlay możesz
umieścić w `boards/<płytka>.overlay` (użyty automatycznie dla tej płytki) albo
przekazać jawnie: `-DEXTRA_DTC_OVERLAY_FILE=ścieżka`.

Przykład (sprawdzony, build dla `nucleo_h743zi`):

```dts
/ {
	aliases {
		status-led = &yellow_led;     /* nowy alias na istniejący węzeł płytki (LD2) */
	};

	zephyr,user {                     /* węzeł na własne dane aplikacji */
		blink-period-ms = <250>;
	};
};

&usart3 {
	current-speed = <921600>;         /* zmiana prędkości konsoli */
};
```

Kod, który z tego korzysta:

```c
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define USER_NODE DT_PATH(zephyr_user)
#define BLINK_MS  DT_PROP(USER_NODE, blink_period_ms)

#define STATUS_LED DT_ALIAS(status_led)
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED, gpios);

/* Sprawdzenia w czasie kompilacji. */
BUILD_ASSERT(DT_NODE_HAS_STATUS(DT_NODELABEL(usart3), okay), "usart3 musi byc wlaczony");
BUILD_ASSERT(BLINK_MS >= 50, "za szybkie miganie");

int main(void)
{
	printk("USART3 z devicetree: %d bps, blink co %d ms\n",
	       DT_PROP(DT_NODELABEL(usart3), current_speed), BLINK_MS);
	printk("przycisk sw0 istnieje: %d\n", DT_NODE_EXISTS(DT_ALIAS(sw0)));

	if (!gpio_is_ready_dt(&status_led)) {
		return 0;
	}
	gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&status_led);
		k_msleep(BLINK_MS);
	}
}
```

Efekt możesz sprawdzić w pliku `build\...\zephyr\zephyr.dts`. Każda właściwość
ma tam komentarz z plikiem i numerem linii, z której pochodzi. Dla `usart3`
zobaczysz:

```dts
pinctrl-0 = < &usart3_tx_pd8 &usart3_rx_pd9 >;  /* in ...\nucleo_h743zi.dts:129 */
status = "okay";                                 /* in ...\nucleo_h743zi.dts:132 */
current-speed = < 0xe1000 >;                     /* in ...\ex02_devicetree.overlay:12 */
```

`0xe1000` to 921600, czyli wartość z overlaya **zastąpiła** 115200 z pliku
płytki (kolejny `current-speed = < 0x1c200 >` w tym pliku należy do `lpuart1`,
którego overlay nie zmienia). Nowa właściwość `blink-period-ms = < 0xfa >`
(250) jest w węźle `zephyr,user`.

> Uwaga: po zmianie prędkości USART3 na 921600 terminal na PC też musi
> działać z 921600, inaczej zobaczysz śmieci.

## 6. Typowe błędy

### Brakujący alias

Zbuduj blinky dla płytki, która nie ma `led0` (np. `qemu_cortex_m3`):

```
error: '__device_dts_ord_DT_N_ALIAS_led0_P_gpios_IDX_0_PH_ORD' undeclared here (not in a function)
error: 'DT_N_ALIAS_led0_P_gpios_IDX_0_VAL_pin' undeclared here (not in a function)
```

Rozpoznajesz go po przedrostku `DT_N_...`. Znaczy: **kod odwołuje się do
węzła/aliasu, którego nie ma w drzewie tej płytki**. Rozwiązania:
- dodaj alias w overlayu dla tej płytki,
- użyj `DT_NODE_EXISTS(...)`/`#if` i obsłuż brak,
- zbuduj dla płytki, która ma takie urządzenie.

### Literówka w nazwie właściwości

Overlay z `current-spedd = <921600>;` zamiast `current-speed` kończy się
błędem już na etapie konfiguracji (jeszcze przed kompilacją kodu):

```
devicetree error: 'current-spedd' appears in /soc/serial@40004800 in ...zephyr.dts.pre,
but is not declared in 'properties:' in ...\dts\bindings\serial\st,stm32-usart.yaml
```

To zaleta bindingów: nazwy i typy właściwości są sprawdzane względem
schematu YAML, a komunikat wskazuje plik bindingu.

## 7. Jak debugować Devicetree

1. Zajrzyj do **`build\zephyr\zephyr.dts`** – to finalne drzewo po scaleniu
   (z komentarzami, z którego pliku pochodzi każda linia).
2. Zajrzyj do **`build\zephyr\include\generated\zephyr\devicetree_generated.h`**
   – tu widać dokładnie, jakie makra istnieją.
3. Sprawdź `status`: węzeł z `status = "disabled"` nie istnieje dla sterownika.
4. Sprawdź, czy włączyłeś odpowiedni sterownik w Kconfig (np. `CONFIG_PWM=y`).
   DT mówi, że sprzęt **jest**, Kconfig mówi, że sterownik ma **być w programie**.

## 🔧 Różnica względem CubeMX

| CubeMX | Devicetree |
|---|---|
| Wizualna konfiguracja w `.ioc` | Tekstowy DTS / overlay (łatwy do diffowania w git) |
| Generuje kod `MX_*_Init()` | Nie generuje kodu, generuje makra `DT_*` |
| Zmiana pinu = klikanie + regeneracja | Zmiana pinu = edycja `pinctrl-0` w overlayu |
| Zegary na zakładce Clock Configuration | Węzły `&pll`, `&rcc` w DTS |

---

## ✅ Zadania

1. Znajdź w `zephyr.dts` węzły `led_0`, `led_1` i `button_0` oraz ich
   piny. Który port i pin obsługuje przycisk i jaka jest jego polaryzacja?
2. Napisz overlay, który dodaje alias `status-led` na zieloną diodę i
   własną wartość `blink-period-ms`. Zbuduj przykład z tego rozdziału.
3. Zmień `blink-period-ms` na `10`. Co się stanie z `BUILD_ASSERT`?
4. Zmień prędkość `usart3` na `921600` i sprawdź w `zephyr.dts`, czy `current-speed`
   w węźle `usart3` ma nową wartość i z którego pliku pochodzi. Czy konsola
   nadal działa z terminalem ustawionym na 115200?
5. Oblicz częstotliwość rdzenia z parametrów PLL w DTS (`div-m`, `mul-n`,
   `div-p`) i sprawdź, czy zgadza się z `clock-frequency` w `&rcc`.
6. Odtwórz błąd z `DT_N_ALIAS_led0` budując blinky dla `qemu_cortex_m3`, a
   potem błąd literówki, pisząc w overlayu `current-spedd`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 3 – Kconfig i `prj.conf`](chapter3_kconfig.md)
