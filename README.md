## Kompilacja
- make
- w terminalu: `` gcc -std=c11 -xc -Wno-deprecated-declarations -o pasjans src/stack.c src/cards.c src/events.c src/main.c `pkg-config gtk+-3.0 --cflags --libs` ``

## Struktura programu 
- `main.c`: inicjalizacja interfejsu graficznego, inicjalizacja i resetowanie gry.
- `events.h`,`events.c`: obsługa zdarzeń występujących podczas działania programu.
- `cards.h`,`cards.c`: struktury przechowujące informacje o planszy i kartach występujących w grze.
- `stack.h`,`stack.c`: obsługa struktury stosu.
