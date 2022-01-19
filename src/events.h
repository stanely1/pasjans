#ifndef EVENTS_INCLUDE
#define EVENTS_INCLUDE

#include <gtk/gtk.h>
#include "cards.h"
#include "widgets.h"
#include "stack.h"

GtkTargetEntry targets[1] = {"dummy",GTK_TARGET_SAME_APP,1};

int current_game_time = 0;
int current_timer_id = 0;
int current_moves = 0;

enum Stat_type {GAMES_PLAYED_STAT,GAMES_WON_STAT,FEWEST_MOVES_STAT,BEST_TIME_SECONDS_STAT};
int stat_value[4] = {0};

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

gboolean timer_update(int *seconds);
void moves_add();

void preferences_dialog_init(GtkWidget *widget, gpointer data);
void stats_dialog_init(GtkWidget *widget, gpointer data);
void save_stats();

//*******************************************************************************//

GtkWidget *drag_source;

// general functions definition
void drag_begin(GtkWidget *source, gpointer data)
{
    drag_source = source == drag_source ? NULL : source;
}

void drag_drop(GtkWidget *target, gpointer data)
{
    if(drag_source == NULL) return;

    GtkWidget *fixed = gtk_widget_get_parent(target);

    GValue gvy = G_VALUE_INIT;
    g_value_init(&gvy,G_TYPE_INT);

    gtk_container_child_get_property(GTK_CONTAINER(fixed),drag_source,"y",&gvy);
    int src_y = g_value_get_int(&gvy);

    gtk_container_child_get_property(GTK_CONTAINER(fixed),target,"y",&gvy);
    int targ_y = g_value_get_int(&gvy);

    if(     src_y == 0 && targ_y == 0) drag_on_stacks(target,fixed);
    else if(src_y == 0 && targ_y  > 0) drag_from_stack_to_main(target,fixed);
    else if(src_y  > 0 && targ_y == 0) drag_from_main_to_dest(target,fixed,src_y);
    else if(src_y  > 0 && targ_y  > 0) drag_on_main(target,fixed,src_y);
}

void drag_end(GtkWidget *source, gpointer data)
{
    drag_source = NULL;
}

//win game
void win_game_block()
{
    Field *field;
    for(int i = 0; i < 4; i++) field = stack_top(dest_stack[i]), gtk_drag_source_unset(field->widget);
}

void win_game_dialog_response(GtkWidget *dialog, gint response_id, gpointer data)
{
    gtk_widget_destroy(dialog);
    response_id == GTK_RESPONSE_ACCEPT ? new_game_init() : win_game_block();
}

void win_game()
{
    g_source_remove(current_timer_id);
    current_timer_id = 0;

    stat_value[GAMES_WON_STAT]++;
    if(stat_value[FEWEST_MOVES_STAT] == 0 || current_moves < stat_value[FEWEST_MOVES_STAT]) 
        stat_value[FEWEST_MOVES_STAT] = current_moves;
    if(stat_value[BEST_TIME_SECONDS_STAT] == 0 || current_game_time < stat_value[BEST_TIME_SECONDS_STAT])
        stat_value[BEST_TIME_SECONDS_STAT] = current_game_time;

    save_stats();

    GtkWidget *win_dialog = gtk_dialog_new_with_buttons("Wygrana!",GTK_WINDOW(main_window),
    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,"Nowa gra",GTK_RESPONSE_ACCEPT,"Zamknij",GTK_RESPONSE_REJECT,NULL);
    g_signal_connect(G_OBJECT(win_dialog),"response",G_CALLBACK(win_game_dialog_response),NULL);

    gtk_dialog_run(GTK_DIALOG(win_dialog));
}

// check if move can be made
gboolean check_on_main(Field *source, Field *target)
{
    if(target->card == NULL) return !strcmp(source->card->type,"king");
    return strcmp(source->card->color, target->card->color) && (source->card->type_hierarchy == target->card->type_hierarchy+1);
    //rozny color , roznica hierarchii typow = 1
}

gboolean check_on_stack(Field *source, Field *target)
{
    if(target == NULL) return !strcmp(source->card->type,"ace"); // na dol idzie as
    return !strcmp(source->card->sign, target->card->sign) && (source->card->type_hierarchy+1 == target->card->type_hierarchy);
    // ten sam znak i roznica hierarchii typow = 1
}

//make card draggable
void make_draggable(Field *field)
{
    gtk_drag_source_set(field->widget,GDK_BUTTON1_MASK,targets,1,GDK_ACTION_COPY);
    gtk_drag_dest_set(field->widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY);

    g_signal_connect(G_OBJECT(field->widget),"drag-begin",G_CALLBACK(drag_begin),NULL);
    g_signal_connect(G_OBJECT(field->widget),"drag-drop",G_CALLBACK(drag_drop),NULL);

    g_signal_connect(G_OBJECT(field->widget),"drag-end",G_CALLBACK(drag_end),NULL);

    g_signal_connect(G_OBJECT(field->widget),"button-press-event",G_CALLBACK(card_double_click),field);
    g_signal_connect(G_OBJECT(field->widget),"button-release-event",G_CALLBACK(card_release),field);
}

//unlock card
void unlock(Field *field, int x, int y)
{
    field->locked = FALSE;

    gtk_container_foreach(GTK_CONTAINER(field->widget),(void*)gtk_widget_destroy,NULL);
    gtk_container_add(GTK_CONTAINER(field->widget),field->card->image);

    make_draggable(field);

    GtkWidget *tmp_widget = g_object_ref(field->widget);
    gtk_container_remove(GTK_CONTAINER(main_fixed),field->widget);
    gtk_fixed_put(GTK_FIXED(main_fixed),tmp_widget,x,y);
    gtk_widget_show_all(tmp_widget);
}

// spcefific functions definition:}
#define get_src_targ_x \
    GValue gvy = G_VALUE_INIT; \
    g_value_init(&gvy,G_TYPE_INT);\
    gtk_container_child_get_property(GTK_CONTAINER(fixed),drag_source,"x",&gvy);\
    int src_x = g_value_get_int(&gvy);\
    gtk_container_child_get_property(GTK_CONTAINER(fixed),target,"x",&gvy);\
    int targ_x = g_value_get_int(&gvy);

void drag_on_main(GtkWidget *target, GtkWidget *fixed, int src_y)
{
    get_src_targ_x

    int src_ind_x  = src_x/(GAP_SIZE+CARD_WIDTH);
    int src_ind_y  = (src_y-MAIN_GRID_START_Y)/GAP_SIZE + 1;
    int targ_ind_x = targ_x/(GAP_SIZE+CARD_WIDTH);

    Field *src_field  = main_grid[src_ind_x][src_ind_y];
    Field *targ_field = main_grid[targ_ind_x][main_grid_stack_size[targ_ind_x]];

    if(!check_on_main(src_field,targ_field)) return;
    
    //wlasciwa akcja
    for(int i = 0; src_ind_y+i <= main_grid_stack_size[src_ind_x]; i++)
    {
        src_field = main_grid[src_ind_x][src_ind_y+i];

        main_grid[src_ind_x][src_ind_y+i] = NULL;
        main_grid[targ_ind_x][++main_grid_stack_size[targ_ind_x]] = src_field;

        GtkWidget *tmp_widget = g_object_ref(src_field->widget);
        gtk_container_remove(GTK_CONTAINER(fixed),src_field->widget);
        gtk_fixed_put(GTK_FIXED(fixed),tmp_widget,
        targ_ind_x*(GAP_SIZE+CARD_WIDTH), MAIN_GRID_START_Y+(main_grid_stack_size[targ_ind_x]-1)*GAP_SIZE);
    }
    main_grid_stack_size[src_ind_x] = src_ind_y-1;

    if(main_grid[src_ind_x][main_grid_stack_size[src_ind_x]]->locked) 
        unlock(main_grid[src_ind_x][main_grid_stack_size[src_ind_x]],src_x,src_y-GAP_SIZE);

    moves_add();
}

void drag_from_main_to_dest(GtkWidget *target, GtkWidget *fixed, int src_y)
{
    get_src_targ_x

    int src_ind_x  = src_x/(GAP_SIZE+CARD_WIDTH);
    int src_ind_y  = (src_y-MAIN_GRID_START_Y)/GAP_SIZE + 1;

    if(src_ind_y != main_grid_stack_size[src_ind_x]) return; // src nie jest gornym elementem

    int targ_stack_ind = targ_x/(GAP_SIZE+CARD_WIDTH)-3;

    Field *src_field = main_grid[src_ind_x][src_ind_y];
    Field *targ_field = stack_top(dest_stack[targ_stack_ind]);

    if(!check_on_stack(src_field,targ_field)) return;

    //wlasciwa akcja
    if(targ_field != NULL) gtk_widget_hide(targ_field->widget);

    stack_insert(&dest_stack[targ_stack_ind],src_field);
    gtk_fixed_move(GTK_FIXED(fixed),src_field->widget,targ_x,0);

    main_grid[src_ind_x][src_ind_y] = NULL;
    main_grid_stack_size[src_ind_x]--;
    
    if(main_grid[src_ind_x][src_ind_y-1]->locked) 
        unlock(main_grid[src_ind_x][src_ind_y-1],src_x,src_y-GAP_SIZE);

    moves_add();
    if(++cards_on_dest_stacks == CARD_COUNT) win_game();
}

void drag_from_stack_to_main(GtkWidget *target, GtkWidget *fixed)
{
    get_src_targ_x

    int src_ind_x  = src_x/(GAP_SIZE+CARD_WIDTH);
    int targ_ind_x = targ_x/(GAP_SIZE+CARD_WIDTH);

    Stack **src_stack = src_ind_x == 1 ? &uncovered_stack : &dest_stack[src_ind_x-3];

    Field *src_field = stack_top(*src_stack);
    Field *targ_field = main_grid[targ_ind_x][main_grid_stack_size[targ_ind_x]];

    if(!check_on_main(src_field,targ_field)) return;

    //wlasciwa akcja
    stack_pop(src_stack);
    if(!stack_is_empty(*src_stack))
    {
        Field *top = stack_top(*src_stack);
        gtk_widget_show_all(top->widget);
    }

    GtkWidget *tmp_widget = g_object_ref(src_field->widget);
    gtk_container_remove(GTK_CONTAINER(fixed),src_field->widget);
    gtk_fixed_put(GTK_FIXED(fixed),tmp_widget,targ_x,MAIN_GRID_START_Y+(main_grid_stack_size[targ_ind_x])*GAP_SIZE);
    
    if(src_ind_x == 1)
        gtk_drag_dest_set(tmp_widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY),
        g_signal_connect(G_OBJECT(tmp_widget),"drag-drop",G_CALLBACK(drag_drop),NULL);

    main_grid[targ_ind_x][++main_grid_stack_size[targ_ind_x]] = src_field;

    if(src_ind_x != 1) cards_on_dest_stacks--;
    moves_add();
}

void drag_on_stacks(GtkWidget *target, GtkWidget *fixed)
{
    get_src_targ_x

    int src_ind_x  = src_x/(GAP_SIZE+CARD_WIDTH);
    int targ_ind_x = targ_x/(GAP_SIZE+CARD_WIDTH);

    Stack **src_stack = src_ind_x == 1 ? &uncovered_stack : &dest_stack[src_ind_x-3];
    Stack **targ_stack = &dest_stack[targ_ind_x-3];

    Field *src_field = stack_top(*src_stack);
    Field *targ_field = stack_top(*targ_stack);

    if(!check_on_stack(src_field,targ_field)) return;

    //wlasciwa akcja
    stack_pop(src_stack);
    if(!stack_is_empty(*src_stack))
    {
        Field *top = stack_top(*src_stack);
        gtk_widget_show_all(top->widget);
    }

    if(targ_field != NULL) gtk_widget_hide(targ_field->widget);

    stack_insert(targ_stack,src_field);
    gtk_fixed_move(GTK_FIXED(fixed),src_field->widget,targ_x,0);

    if(src_ind_x == 1)
        gtk_drag_dest_set(src_field->widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY),
        g_signal_connect(G_OBJECT(src_field->widget),"drag-drop",G_CALLBACK(drag_drop),NULL);

    moves_add();
    if(src_ind_x == 1 && ++cards_on_dest_stacks == CARD_COUNT) win_game();
}

void covered_card_click(GtkWidget *widget, gpointer data)
{
    if(stack_is_empty(covered_stack)) return;

    Field *covered_top = stack_pop(&covered_stack);
    Field *uncovered_top = stack_top(uncovered_stack);

    if(uncovered_top != NULL) gtk_widget_hide(uncovered_top->widget);
    
    stack_insert(&uncovered_stack,covered_top);
    gtk_widget_show_all(covered_top->widget);

    if(stack_is_empty(covered_stack)) gtk_widget_hide(covered_stack_card);
    moves_add();
}

void covered_base_click(GtkWidget *widget, gpointer data)
{
    if(stack_is_empty(uncovered_stack)) return;

    Field *top = stack_top(uncovered_stack);
    gtk_widget_hide(top->widget);

    while(!stack_is_empty(uncovered_stack)) stack_insert(&covered_stack,stack_pop(&uncovered_stack));

    if(!stack_is_empty(covered_stack)) gtk_widget_show_all(covered_stack_card);
    moves_add();
}

void card_double_click(GtkWidget *widget, GdkEventButton *event, Field *field)
{
    if(event->type != GDK_2BUTTON_PRESS) return;
    
    for(int i = 0; i < 4; i++) 
        if(field == stack_top(dest_stack[i])) return; // field on dest

    drag_source = field->widget;

    for(int i = 0; i < 4; i++)
    {
        Field *targ_field = stack_top(dest_stack[i]);
        if(check_on_stack(field,targ_field)) 
        {
            drag_drop(targ_field == NULL ? dest_stack_base[i] : targ_field->widget, NULL); 
            break;
        }
    }
}
void card_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    drag_source = NULL;
}

//timer + moves
void moves_add()
{
    if(++current_moves == 1) 
    {
        current_timer_id = g_timeout_add_seconds(1,(GSourceFunc)timer_update,&current_game_time); // start timer

        stat_value[GAMES_PLAYED_STAT]++; // zwiekszamy liczbe rozegranych gier
        save_stats();
    }

    char label[25];
    sprintf(label,"Liczba ruchów: %d",current_moves);
    gtk_label_set_text(GTK_LABEL(move_counter),label);
}

void secs_to_time_string(int seconds, char dststr[])
{
    int s = seconds%60;
    int m = (seconds/60)%60;
    int h = seconds/3600;

    sprintf(dststr,"%s%d:%s%d:%s%d",h<10 ? "0":"",h, m<10 ? "0":"",m, s<10 ? "0":"",s);
}
gboolean timer_update(int *seconds)
{
    (*seconds)++;

    char label[25];
    sprintf(label,"Czas gry: ");
    secs_to_time_string(*seconds,label+10);
    gtk_label_set_text(GTK_LABEL(timer),label);

    return TRUE;
}

//preferences
int current_card_back_state;
int tmp_card_back_state;

void card_back_toggle(GtkWidget *widget, gpointer data)
{
    tmp_card_back_state ^= 1;
}

void preferences_reset(GtkWidget *widget, GtkWidget *color_button[])
{
    gtk_color_button_set_color(GTK_COLOR_BUTTON(color_button[0]),&default_bg_color);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(color_button[1]),&default_frame_color);
}

void preferences_dialog_response(GtkWidget *dialog, gint response_id, GtkWidget *color_button[])
{
    if(response_id != GTK_RESPONSE_APPLY) {gtk_widget_destroy(dialog); tmp_card_back_state = current_card_back_state; return;}

    //card back color
    current_card_back_state = tmp_card_back_state;

    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
        for(int j = 1; j < MAIN_GRID_SIZE_Y; j++)
            if(main_grid[i][j] != NULL && main_grid[i][j]->locked)
                gtk_container_foreach(GTK_CONTAINER(main_grid[i][j]->widget),(void*)gtk_widget_destroy,NULL),
                gtk_container_add(GTK_CONTAINER(main_grid[i][j]->widget),gtk_image_new_from_pixbuf(card_back_pixbuf[current_card_back_state])),
                gtk_widget_show_all(main_grid[i][j]->widget);

    gtk_container_foreach(GTK_CONTAINER(covered_stack_card),(void*)gtk_widget_destroy,NULL);
    gtk_container_add(GTK_CONTAINER(covered_stack_card),gtk_image_new_from_pixbuf(card_back_pixbuf[current_card_back_state]));
    if(!stack_is_empty(covered_stack)) gtk_widget_show_all(covered_stack_card);

    FILE *card_back_tmp_file = fopen("data/card_back_state","w");
    fprintf(card_back_tmp_file,"%d",current_card_back_state);
    fclose(card_back_tmp_file);

    //bg and frame color
    gtk_color_button_get_color(GTK_COLOR_BUTTON(color_button[0]),&current_bg_color);
    gtk_color_button_get_color(GTK_COLOR_BUTTON(color_button[1]),&current_frame_color);

    gtk_widget_modify_bg(main_window,GTK_STATE_NORMAL,&current_bg_color);

    for(int i = 0; i < MAIN_GRID_SIZE_X; i++) gtk_widget_modify_bg(main_grid[i][0]->widget,GTK_STATE_NORMAL,&current_frame_color);
    gtk_widget_modify_bg(covered_stack_base,GTK_STATE_NORMAL,&current_frame_color);
    gtk_widget_modify_bg(uncovered_stack_base,GTK_STATE_NORMAL,&current_frame_color);
    for(int i = 0; i < 4; i++) gtk_widget_modify_bg(dest_stack_base[i],GTK_STATE_NORMAL,&current_frame_color);

    FILE *color_tmp_file = fopen("data/color_preferences","wb");
    fwrite(&current_bg_color,sizeof(current_bg_color),1,color_tmp_file);
    fwrite(&current_frame_color,sizeof(current_frame_color),1,color_tmp_file);
    fclose(color_tmp_file);

    gtk_dialog_run(GTK_DIALOG(dialog));
}

void preferences_dialog_init(GtkWidget *widget, gpointer data)
{
    //main dialog window
    GtkWidget *preferences_dialog = gtk_dialog_new_with_buttons("Preferencje",GTK_WINDOW(main_window),
    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,"Zapisz",GTK_RESPONSE_APPLY,"Anuluj",GTK_RESPONSE_CANCEL,NULL);

    gtk_window_set_resizable(GTK_WINDOW(preferences_dialog),FALSE);
    g_object_set(gtk_dialog_get_action_area(GTK_DIALOG(preferences_dialog)),"halign",GTK_ALIGN_CENTER,NULL);

    GtkWidget *preferences_fixed = gtk_fixed_new();
    gtk_container_set_border_width(GTK_CONTAINER(preferences_fixed),10);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(preferences_dialog))),preferences_fixed,TRUE,TRUE,0);

    //card back color
    gtk_fixed_put(GTK_FIXED(preferences_fixed),gtk_label_new("Tył kraty:"),0,4);

    GtkWidget *card_back_button[2] = {gtk_radio_button_new_with_label(NULL,"Czerwony"),gtk_radio_button_new_with_label(NULL,"Niebieski")};
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(card_back_button[0]),GTK_RADIO_BUTTON(card_back_button[1]));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(card_back_button[current_card_back_state]),TRUE);

    g_signal_connect(G_OBJECT(card_back_button[0]),"toggled",G_CALLBACK(card_back_toggle),NULL);

    gtk_fixed_put(GTK_FIXED(preferences_fixed),card_back_button[0],0,36);
    gtk_fixed_put(GTK_FIXED(preferences_fixed),card_back_button[1],0,61);
    
    //color buttons
    gtk_fixed_put(GTK_FIXED(preferences_fixed),gtk_label_new("Kolor tła:"),150,4);
    gtk_fixed_put(GTK_FIXED(preferences_fixed),gtk_label_new("Kolor pod kartami:"),150,36);

    GtkWidget *color_button[2] = {gtk_color_button_new_with_color(&current_bg_color),gtk_color_button_new_with_color(&current_frame_color)};
    gtk_fixed_put(GTK_FIXED(preferences_fixed),color_button[0],300,0);
    gtk_fixed_put(GTK_FIXED(preferences_fixed),color_button[1],300,32);

    //reset to default button
    GtkWidget *reset_button = gtk_button_new_with_label("Domyślne");
    g_signal_connect(G_OBJECT(reset_button),"clicked",G_CALLBACK(preferences_reset),color_button);
    gtk_fixed_put(GTK_FIXED(preferences_fixed),reset_button,150,70);
    //****
    g_signal_connect(G_OBJECT(preferences_dialog),"response",G_CALLBACK(preferences_dialog_response),color_button);

    gtk_widget_show_all(preferences_dialog);
    gtk_dialog_run(GTK_DIALOG(preferences_dialog));
}

//stats
void stats_dialog_response(GtkWidget *dialog, gint response_id, gpointer data)
{
    if(response_id == GTK_RESPONSE_CLOSE) gtk_widget_destroy(dialog);
}

void stats_dialog_init(GtkWidget *widget, gpointer data)
{
    GtkWidget *stats_dialog = gtk_dialog_new_with_buttons("Statystyki",GTK_WINDOW(main_window),
    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,"Zamknij",GTK_RESPONSE_CLOSE,NULL);
    g_signal_connect(G_OBJECT(stats_dialog),"response",G_CALLBACK(stats_dialog_response),NULL);

    gtk_window_set_resizable(GTK_WINDOW(stats_dialog),FALSE);
    g_object_set(gtk_dialog_get_action_area(GTK_DIALOG(stats_dialog)),"halign",GTK_ALIGN_CENTER,NULL);

    char stats_label_text[250];
    char time_stat_text[12];
    secs_to_time_string(stat_value[BEST_TIME_SECONDS_STAT],time_stat_text);
    sprintf(stats_label_text,"Gry rozegrane: %d\n"
                        "Gry wygrane: %d\n"
                        "Procent gier wygranych: %.2lf%%\n"
                        "Najmniejsza liczba ruchów w wygranej grze: %d\n"
                        "Najkrótszy czas wygranej gry: %s",
                        stat_value[GAMES_PLAYED_STAT],stat_value[GAMES_WON_STAT],
                        stat_value[GAMES_PLAYED_STAT] != 0 ? (double)(stat_value[GAMES_WON_STAT]*100)/stat_value[GAMES_PLAYED_STAT] : 0,
                        stat_value[FEWEST_MOVES_STAT],time_stat_text);

    GtkWidget *stats_label_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,20);
    gtk_box_pack_start(GTK_BOX(stats_label_box),gtk_label_new(stats_label_text),TRUE,TRUE,20);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(stats_dialog))),stats_label_box,TRUE,TRUE,20);

    gtk_widget_show_all(stats_dialog);
    gtk_dialog_run(GTK_DIALOG(stats_dialog));
}

void save_stats()
{
        FILE *stats_tmp_file = fopen("data/stats","wb");
        fwrite(stat_value,sizeof(stat_value),1,stats_tmp_file);
        fclose(stats_tmp_file);
}
#endif