#ifndef CARDS_INCLUDE
#define CARDS_INCLUDE

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "stack.h"

#define CARD_WIDTH 114
#define CARD_HEIGHT 166
#define CARD_COUNT 52
#define MAIN_GRID_SIZE_X 7
#define MAIN_GRID_SIZE_Y 22
#define GAP_SIZE 30
#define MAIN_GRID_START_Y 200

typedef struct {
    char color[7]; // red / black
    char sign[10]; //pik / kier itp.
    char type[7]; //king / queen / 10 itp.
    int type_hierarchy;
    GtkWidget *image;
} Card;

typedef struct {
    GtkWidget *widget;
    Card *card;
    gboolean locked;
} Field;

extern Field *main_grid[MAIN_GRID_SIZE_X][MAIN_GRID_SIZE_Y];
extern int main_grid_stack_size[MAIN_GRID_SIZE_X];

extern Card cards[CARD_COUNT];

extern Stack *covered_stack, *uncovered_stack, *dest_stack[4];

extern int cards_on_dest_stacks;

extern char *card_type_hierarchy[13];

void cards_init();

#endif