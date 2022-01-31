## Kompilacja
- make
- w terminalu: `` gcc -std=c11 -xc -Wno-deprecated-declarations -o pasjans src/main.c src/stack.c `pkg-config gtk+-3.0 --cflags --libs` ``

## Struktura programu 
- `main.c`: inicjalizacja interfejsu graficznego, inicjalizacja i resetowanie gry.
- `events.h`: obsługa zdarzeń występujących podczas działania programu.
- `widgets.h`: deklaracje obiektów interfejsu graficznego.
- `cards.h`: definicje struktur przechowujących informacje o planszy i kartach występujących w grze.
- `stack.h`: deklaracje fuckcji obsługujących strukturę stosu.
- `stack.c`: definicje fuckcji obsługujących strukturę stosu.