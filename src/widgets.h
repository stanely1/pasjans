#ifndef WIDGETS_INCLUDE
#define WIDGETS_INCLUDE

#include <gtk/gtk.h>

GtkWidget *main_window;
GtkWidget *main_fixed;
GtkWidget *covered_stack_card;
GtkWidget *covered_stack_base;
GtkWidget *uncovered_stack_base;
GtkWidget *dest_stack_base[4];
GtkWidget *menubar;
GtkWidget *timer;
GtkWidget *move_counter;

GdkPixbuf *card_back_pixbuf[2];

GdkColor default_bg_color = {.red = 120, .green = 20125, .blue = 812};
GdkColor default_frame_color = {.red = 120, .green = 25565, .blue = 812};

GdkColor current_bg_color;
GdkColor current_frame_color;

#endif