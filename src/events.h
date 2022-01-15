#ifndef EVENTS_INCLUDE
#define EVENTS_INCLUDE

#include <gtk/gtk.h>
#include "cards.h"
#include "widgets.h"
#include "stack.h"

GtkTargetEntry targets[1] = {"dummy",GTK_TARGET_SAME_APP,1};

void new_game_init();

//general
void drag_begin(GtkWidget *target, gpointer data);
void drag_drop(GtkWidget *source, gpointer data);

//specific
void drag_on_main(GtkWidget *target, GtkWidget *fixed, int src_y);
void drag_from_main_to_dest(GtkWidget *target, GtkWidget *fixed, int src_y);
void drag_from_stack_to_main(GtkWidget *target, GtkWidget *fixed);
void drag_on_stacks(GtkWidget *target, GtkWidget *fixed);

void covered_card_click(GtkWidget *widget, gpointer data);
void covered_base_click(GtkWidget *widget, gpointer data);

//*******************************************************************************//

GtkWidget *drag_source;

// general functions definition
void drag_begin(GtkWidget *source, gpointer data)
{
    drag_source = source;
}

void drag_drop(GtkWidget *target, gpointer data)
{
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

//win game
void win_game_block()
{
    Field *field;
    for(int i = 0; i < 4; i++) field = stack_top(dest_stack[i]), gtk_drag_source_unset(field->widget);
}

void win_game_dialog_response(GtkWidget *dialog, gint response_id, gpointer data)
{
    switch(response_id)
    {
        case GTK_RESPONSE_REJECT: gtk_widget_destroy(dialog);
                                  win_game_block();
                                  break;

        case GTK_RESPONSE_ACCEPT: gtk_widget_destroy(dialog);
                                  new_game_init();
                                  break;
    }
}

void win_game()
{
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
    get_src_targ_x;

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
}

void covered_base_click(GtkWidget *widget, gpointer data)
{
    if(stack_is_empty(uncovered_stack)) return;

    Field *top = stack_top(uncovered_stack);
    gtk_widget_hide(top->widget);

    while(!stack_is_empty(uncovered_stack)) stack_insert(&covered_stack,stack_pop(&uncovered_stack));

    if(!stack_is_empty(covered_stack)) gtk_widget_show_all(covered_stack_card);
}

#endif