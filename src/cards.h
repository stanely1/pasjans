#ifndef CARDS_INCLUDE
#define CARDS_INCLUDE

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <string.h>
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

Field *main_grid[MAIN_GRID_SIZE_X][MAIN_GRID_SIZE_Y];
int main_grid_stack_size[MAIN_GRID_SIZE_X];

Card cards[CARD_COUNT];

Stack *covered_stack, *uncovered_stack, *dest_stack[4];

int cards_on_dest_stacks;

char *card_type_hierarchy[13] = {"king","queen","jack","10","9","8","7","6","5","4","3","2","ace"};

void cards_init()
{
    FILE *card_inp = fopen("card_info","r");
    for(int i = 0; i < CARD_COUNT; i++)
    {
        fscanf(card_inp,"%s",cards[i].color);
        fscanf(card_inp,"%s",cards[i].sign);
        fscanf(card_inp,"%s",cards[i].type);

        char filename[30];
        fscanf(card_inp,"%s",filename);

        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename,CARD_WIDTH,CARD_HEIGHT,TRUE,NULL);
        cards[i].image = gtk_image_new_from_pixbuf(pixbuf);

        for(int j = 0; j < 13; j++)
            if(!strcmp(cards[i].type,card_type_hierarchy[j])) cards[i].type_hierarchy = j;
    }
}
#endif