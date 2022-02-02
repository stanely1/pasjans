#ifndef EVENTS_INCLUDE
#define EVENTS_INCLUDE

#include <gtk/gtk.h>
#include "cards.h"

extern GtkWidget *main_window;
extern GtkWidget *main_fixed;
extern GtkWidget *covered_stack_card;
extern GtkWidget *covered_stack_base;
extern GtkWidget *uncovered_stack_base;
extern GtkWidget *dest_stack_base[4];
extern GtkWidget *menubar;
extern GtkWidget *timer;
extern GtkWidget *move_counter;

extern GdkPixbuf *card_back_pixbuf[2];

extern GdkColor default_bg_color;
extern GdkColor default_frame_color;

extern GdkColor current_bg_color;
extern GdkColor current_frame_color;

extern GtkTargetEntry targets[1];

extern int current_game_time;
extern int current_timer_id;
extern int current_moves;

extern int current_card_back_state;
extern int tmp_card_back_state;

enum Stat_type {GAMES_PLAYED_STAT,GAMES_WON_STAT,FEWEST_MOVES_STAT,BEST_TIME_SECONDS_STAT};
extern int stat_value[4];

void new_game_init();

//general
void drag_begin(GtkWidget *target, gpointer data);
void drag_drop(GtkWidget *source, gpointer data);
void drag_end(GtkWidget *source, gpointer data);

//specific
void drag_on_main(GtkWidget *target, GtkWidget *fixed, int src_y);
void drag_from_main_to_dest(GtkWidget *target, GtkWidget *fixed, int src_y);
void drag_from_stack_to_main(GtkWidget *target, GtkWidget *fixed);
void drag_on_stacks(GtkWidget *target, GtkWidget *fixed);

void covered_card_click(GtkWidget *widget, gpointer data);
void covered_base_click(GtkWidget *widget, gpointer data);

void card_double_click(GtkWidget *widget, GdkEventButton *event, Field *field);
void card_release(GtkWidget *widget, GdkEventButton *event, gpointer data);

void make_draggable(Field *field);

gboolean timer_update(int *seconds);
void moves_add();

void rules_dialog_init(GtkWidget *widget, gpointer data);
void preferences_dialog_init(GtkWidget *widget, gpointer data);
void stats_dialog_init(GtkWidget *widget, gpointer data);
void save_stats();

#endif