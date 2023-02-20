#include <8051.h>

#define FALSE 0
#define TRUE 1

#define T0_DAT 65536 - 921
#define TL_0 T0_DAT % 256
#define TH_0 T0_DAT / 256

__bit T0_FLAG; // FLAGA PRZERWANIA LICZNIKA T0
unsigned int timer = 0;

__xdata unsigned char *led_wyb = (__xdata unsigned char *)0xFF30; // wybrany wyświetlacz/wskaźnik
__xdata unsigned char *led_led = (__xdata unsigned char *)0xFF38; // wybrane segmenty wyświetlacza

__bit __at(0x96) SEG_OFF;

unsigned char led_p = 0; // indeks aktywnego wyświetlacza
unsigned char led_b = 1; // aktywny wyświetlacz (bitowo)

__code unsigned char WZOR[10] = {0b0111111, 0b0000110, 0b1011011,
                                 0b1001111, 0b1100110, 0b1101101,
                                 0b1111101, 0b0000111, 0b1111111, 0b1101111};

unsigned char HH = 22;
unsigned char MM = 0;
unsigned char SS = 0;
unsigned char ZEGAR[6] = {0, 0, 0, 0, 0, 0}; // tablica przechowująca 6 cyfr zegara

unsigned char KEYS[4];

#define GORA 0b001000
#define DOL 0b010000
#define LEWO 0b100000
#define PRAWO 0b000100
#define ENTER 0b000001
#define ESC 0b000010
void aktualizujZegar()
{
    ZEGAR[5] = HH / 10; // pierwsza cyfra godzin (dziesiątki)
    ZEGAR[4] = HH % 10; // druga cyfra godzin (jedności)
    ZEGAR[3] = MM / 10; // pierwsza cyfra minut (dziesiątki)
    ZEGAR[2] = MM % 10; // druga cyfra minut (jedności)
    ZEGAR[1] = SS / 10; // pierwsza cyfra sekund (dziesiątki)
    ZEGAR[0] = SS % 10; // druga cyfra sekund (jedności)
}

void aktualizujCzas()
{
    SS++;
    if (SS >= 60)
    {
        SS = 0;
        MM++;
    }
    if (MM >= 60)
    {
        MM = 0;
        HH++;
    }
    if (HH >= 24)
    {
        HH = 0;
    }

    aktualizujZegar();
}

void wyswietlacz()
{
    SEG_OFF = TRUE;
    *led_wyb = led_b;
    *led_led = WZOR[ZEGAR[led_p]];
    SEG_OFF = FALSE;
}

void obslugaKlawiatury()
{
    if (KEYS[0] != 0)
    {
        if (KEYS[0] != KEYS[1] && KEYS[0] != KEYS[2] && KEYS[3] != KEYS[0]) // sprawdza czy klawisz jest już odpowiednio długo wciśnięty
        {

            if (KEYS[0] == (ENTER | LEWO))
            {
                if (HH != 23)
                    HH++;
                else
                    HH = 0;
                aktualizujZegar();
            }

            if (KEYS[0] == (ESC | LEWO))
            {
                if (HH != 0)
                    HH--;
                else
                    HH = 23;
                aktualizujZegar();
            }

            if (KEYS[0] == (ENTER | DOL))
            {
                if (MM != 59)
                    MM++;
                else
                    MM = 0;
                aktualizujZegar();
            }

            if (KEYS[0] == (ESC | DOL))
            {
                if (MM != 0)
                    MM--;
                else
                    MM = 59;
                aktualizujZegar();
            }
            if (KEYS[0] == (ENTER | PRAWO))
            {
                if (SS != 59)
                    SS++;
                else
                    SS = 0;
                aktualizujZegar();
            }
            if (KEYS[0] == (ESC | PRAWO))
            {
                if (SS != 0)
                    SS--;
                else
                    SS = 59;
                aktualizujZegar();
            }
        }

        KEYS[3] = KEYS[2];
        KEYS[2] = KEYS[1];
        KEYS[1] = KEYS[0];
        KEYS[0] = 0;
    }
}

void checkTimer()
{
    if (timer == 1024)
    {
        timer = 0;
        aktualizujCzas();
    }
}

void main()
{
    T0_FLAG = FALSE;
    TL0 = TL_0; // ustawienie młodszego i starszego
    TH0 = TH_0; // bajtu T0 przerwanie co 1 milisekundę
    ET0 = TRUE; // aktywuj przerwanie od licznika T0
    ES = TRUE;  // aktywuj przerwanie od UART
    EA = TRUE;  // aktywuj wszystkie przerwania
    TR0 = TRUE; // uruchom licznik T0

    aktualizujZegar();

    while (TRUE)
    {

        if (T0_FLAG)
        {
            T0_FLAG = FALSE;
            wyswietlacz();
            if (P3_5)
            {
                KEYS[0] = (KEYS[0] | led_b); // jeśli led_b ma jakieś bity ustawione na 1, to OR spowoduje, że te same bity zostaną ustawione na 1 również w KEYS[0].
            }
            if (led_p < 5)
            {
                led_p++;
                led_b += led_b;
            }
            else
            {
                led_p = 0;
                led_b = 1;
                obslugaKlawiatury();
            }

            checkTimer();
        }
    }
}

void t0_int(void) __interrupt(1)
{
    TL0 = 252;
    TH0 = 227;
    T0_FLAG = TRUE;
    timer++;
}