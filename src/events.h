#ifndef EVENTS_INCLUDE
#define EVENTS_INCLUDE

#include <gtk/gtk.h>
#include "cards.h"

GtkTargetEntry targets[1] = {"dummy",GTK_TARGET_SAME_APP,1};

//general
void drag_begin(GtkWidget *target, gpointer data);
void drag_drop(GtkWidget *source, gpointer data);

//specific
void drag_on_main(GtkWidget *target, GtkWidget *fixed, int src_y);
void drag_from_main_to_dest(GtkWidget *target, GtkWidget *fixed);
void drag_from_stack_to_main(GtkWidget *target, GtkWidget *fixed);
void drag_on_stacks(GtkWidget *target, GtkWidget *fixed);

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
    else if(src_y  > 0 && targ_y == 0) drag_from_main_to_dest(target,fixed);
    else if(src_y  > 0 && targ_y  > 0) drag_on_main(target,fixed,src_y);
}

// check if move can be made
gboolean check_on_main(Field *source, Field *target)
{
    if(target->card == NULL) return !strcmp(source->card->type,"king");
    return strcmp(source->card->color, target->card->color) && (source->card->type_hierarchy == target->card->type_hierarchy+1);
    //różny color , roznica hierarchii typów = 1
}

gboolean check_on_stack(Field *source, Field *target)
{
    if(target->card == NULL) return !strcmp(source->card->type,"ace"); // na dol idzie as
    return !strcmp(source->card->sign, target->card->sign) && (source->card->type_hierarchy+1 == target->card->type_hierarchy);
    // ten sam znak i roznica hierarchii typów = 1
}

//make card draggable
void make_draggable(Field *field)
{
    GtkWidget *drag_event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(drag_event_box),field->widget);

    field->widget = drag_event_box;

    gtk_drag_source_set(field->widget,GDK_BUTTON1_MASK,targets,1,GDK_ACTION_COPY);
    gtk_drag_dest_set(field->widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY);

    g_signal_connect(G_OBJECT(field->widget),"drag-begin",G_CALLBACK(drag_begin),NULL);
    g_signal_connect(G_OBJECT(field->widget),"drag-drop",G_CALLBACK(drag_drop),NULL);
}

// spcefific functions definition:}
void drag_on_main(GtkWidget *target, GtkWidget *fixed, int src_y)
{
    GValue gvy = G_VALUE_INIT;
    g_value_init(&gvy,G_TYPE_INT);

    gtk_container_child_get_property(GTK_CONTAINER(fixed),drag_source,"x",&gvy);
    int src_x = g_value_get_int(&gvy);

    gtk_container_child_get_property(GTK_CONTAINER(fixed),target,"x",&gvy);
    int targ_x = g_value_get_int(&gvy);

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
       // gtk_widget_show(src_field->widget);

    }
    main_grid_stack_size[src_ind_x] = src_ind_y-1;

    if(main_grid[src_ind_x][main_grid_stack_size[src_ind_x]]->locked)
    {
        Field *unlocked = main_grid[src_ind_x][main_grid_stack_size[src_ind_x]];
        unlocked->locked = FALSE;

        gtk_widget_destroy(unlocked->widget);
        unlocked->widget = unlocked->card->image;

        make_draggable(unlocked);

        gtk_container_remove(GTK_CONTAINER(fixed),unlocked->widget);
        gtk_fixed_put(GTK_FIXED(fixed),unlocked->widget,src_x,src_y-GAP_SIZE);
        gtk_widget_show_all(unlocked->widget);
    }
}

void drag_from_main_to_dest(GtkWidget *target, GtkWidget *fixed)
{
    return;
}

void drag_from_stack_to_main(GtkWidget *target, GtkWidget *fixed)
{
    return;
}

void drag_on_stacks(GtkWidget *target, GtkWidget *fixed)
{
    return;
}

#endif