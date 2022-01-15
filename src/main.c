#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include "cards.h"
#include "events.h"
#include "stack.h"
#include "widgets.h"

void swap(Card *x, Card *y)
{
    Card tmp = *x;
    *x = *y;
    *y = tmp;
}
void random_shuffle(int n, Card t[])
{
    for(int i = 0; i < n; i++)
    {
        int j = rand()%(n-i) + i;
        swap(t+i,t+j);
    }
}

void clear_stack(Stack **stack)
{
    Field *field;
    while(!stack_is_empty(*stack))
    {
        field = stack_pop(stack);
        gtk_widget_destroy(field->widget);
    }
}
void clear_game()
{
    for(int i = 0; i < CARD_COUNT; i++) g_object_ref(cards[i].image);

    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
        for(int j = 1; j < MAIN_GRID_SIZE_Y; j++)
            if(main_grid[i][j] != NULL) gtk_widget_destroy(main_grid[i][j]->widget), free(main_grid[i][j]), main_grid[i][j] = NULL; 
            
    clear_stack(&covered_stack);
    clear_stack(&uncovered_stack);
    for(int i = 0; i < 4; i++) clear_stack(&dest_stack[i]);

    cards_on_dest_stacks = 0;
    current_game_time = 0;
    current_moves = 0;

    gtk_label_set_text(GTK_LABEL(timer),"Czas gry: 00:00:00");
    gtk_label_set_text(GTK_LABEL(move_counter),"Ilość ruchów: 0");
    if(current_timer_id != 0) g_source_remove(current_timer_id);
}

void new_game_init()
{
    clear_game();

    random_shuffle(CARD_COUNT,cards);

    int current_ind = 0;

    //main grid
    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
    {
        main_grid_stack_size[i] = i+1;
        for(int j = 1; j <= i+1; j++)
        {
            main_grid[i][j] = malloc(sizeof(Field));
            main_grid[i][j]->locked = i+1 != j;
            main_grid[i][j]->card = cards+current_ind;
            main_grid[i][j]->widget = gtk_event_box_new();
            
            if(main_grid[i][j]->locked)
            {
                GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("images/zback_blue.svg",CARD_WIDTH,CARD_HEIGHT,TRUE,NULL);
                gtk_container_add(GTK_CONTAINER(main_grid[i][j]->widget),gtk_image_new_from_pixbuf(pixbuf));
            }
            else gtk_container_add(GTK_CONTAINER(main_grid[i][j]->widget),cards[current_ind].image), make_draggable(main_grid[i][j]);
            
            gtk_fixed_put(GTK_FIXED(main_fixed),main_grid[i][j]->widget,i*(GAP_SIZE+CARD_WIDTH),MAIN_GRID_START_Y+(j-1)*GAP_SIZE);
            current_ind++;
        }
    }

    gtk_widget_show_all(main_window);

    //kupka kart do dobierania
    while(current_ind < CARD_COUNT)
    {
        Field *field = malloc(sizeof(Field));
        field->locked = FALSE;
        field->card = cards+current_ind; current_ind++;

        field->widget = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(field->widget),field->card->image);
        gtk_fixed_put(GTK_FIXED(main_fixed),field->widget,GAP_SIZE+CARD_WIDTH,0);

        gtk_drag_source_set(field->widget,GDK_BUTTON1_MASK,targets,1,GDK_ACTION_COPY);
        g_signal_connect(G_OBJECT(field->widget),"drag-begin",G_CALLBACK(drag_begin),NULL);
        
        stack_insert(&covered_stack,field);
    }
}

void main_window_init()
{
    //main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window),"Pasjans");
    gtk_window_maximize(GTK_WINDOW(main_window));
    gtk_container_set_border_width(GTK_CONTAINER(main_window),0);
    
    GdkColor bg_color = {.red = 120, .green = 20125, .blue = 812};
    GdkColor frame_color = {.red = 120, .green = 25565, .blue = 812};
    gtk_widget_modify_bg(main_window,GTK_STATE_NORMAL,&bg_color);

    g_signal_connect(G_OBJECT(main_window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
    // *******

    GtkWidget *main_vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(main_window),main_vbox);

    //menubar
    menubar = gtk_menu_bar_new();

    GtkWidget *new_game_menu_item = gtk_menu_item_new_with_label("Nowa gra");
    g_signal_connect(G_OBJECT(new_game_menu_item),"activate",G_CALLBACK(new_game_init),NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),new_game_menu_item);

    GtkWidget *rules_menu_item = gtk_menu_item_new_with_label("Zasady");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rules_menu_item);

    GtkWidget *preferences_menu_item = gtk_menu_item_new_with_label("Preferencje");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),preferences_menu_item);

    GtkWidget *stats_menu_item = gtk_menu_item_new_with_label("Statystyki");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),stats_menu_item);

    gtk_box_pack_start(GTK_BOX(main_vbox),menubar,FALSE,TRUE,0);
    //*******************

    main_fixed = gtk_fixed_new();

    GtkWidget *main_overlay = gtk_overlay_new();

    //fixed
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay),main_fixed);
    g_object_set(main_fixed,"halign",GTK_ALIGN_CENTER,"valign",GTK_ALIGN_START,NULL);
    gtk_box_pack_start(GTK_BOX(main_vbox),main_overlay,TRUE,TRUE,40);
    //***

    cards_init();

    // puste pola pod kupkami kart
    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
    {
        main_grid[i][0] = malloc(sizeof(Field));

        main_grid[i][0]->locked = FALSE;
        main_grid[i][0]->card = NULL;
        main_grid[i][0]->widget = gtk_label_new(NULL);

        gtk_widget_set_size_request(main_grid[i][0]->widget,CARD_WIDTH,CARD_HEIGHT);
        gtk_drag_dest_set(main_grid[i][0]->widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY);
        g_signal_connect(G_OBJECT(main_grid[i][0]->widget),"drag-drop",G_CALLBACK(drag_drop),NULL);
        
        gtk_fixed_put(GTK_FIXED(main_fixed),main_grid[i][0]->widget,i*(GAP_SIZE+CARD_WIDTH),MAIN_GRID_START_Y);
        gtk_widget_modify_bg(main_grid[i][0]->widget,GTK_STATE_NORMAL,&frame_color);
    }
    //***

    //kupka do dobierania
    covered_stack_base = gtk_event_box_new();
    //gtk_container_add(GTK_CONTAINER(covered_stack_base),gtk_frame_new(NULL));
    gtk_widget_set_size_request(covered_stack_base,CARD_WIDTH,CARD_HEIGHT);
    g_signal_connect(G_OBJECT(covered_stack_base),"button-press-event",G_CALLBACK(covered_base_click),NULL);
    gtk_fixed_put(GTK_FIXED(main_fixed),covered_stack_base,0,0);
    gtk_widget_modify_bg(covered_stack_base,GTK_STATE_NORMAL,&frame_color);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("images/zback_blue.svg",CARD_WIDTH,CARD_HEIGHT,TRUE,NULL);
    covered_stack_card = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(covered_stack_card),gtk_image_new_from_pixbuf(pixbuf));
    g_signal_connect(G_OBJECT(covered_stack_card),"button-press-event",G_CALLBACK(covered_card_click),NULL);
    gtk_fixed_put(GTK_FIXED(main_fixed),covered_stack_card,0,0);

    uncovered_stack_base = gtk_label_new(NULL);
    gtk_widget_set_size_request(uncovered_stack_base,CARD_WIDTH,CARD_HEIGHT);
    gtk_fixed_put(GTK_FIXED(main_fixed),uncovered_stack_base,CARD_WIDTH+GAP_SIZE,0);
    gtk_widget_modify_bg(uncovered_stack_base,GTK_STATE_NORMAL,&frame_color);
    //***

    //dest
    for(int i = 0; i < 4; i++)
    {
        dest_stack_base[i] = gtk_label_new(NULL);
        gtk_widget_set_size_request(dest_stack_base[i],CARD_WIDTH,CARD_HEIGHT);
        gtk_drag_dest_set(dest_stack_base[i],GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY);
        g_signal_connect(G_OBJECT(dest_stack_base[i]),"drag-drop",G_CALLBACK(drag_drop),NULL);
        gtk_fixed_put(GTK_FIXED(main_fixed),dest_stack_base[i],(MAIN_GRID_SIZE_X-4+i)*(GAP_SIZE+CARD_WIDTH),0);
        gtk_widget_modify_bg(dest_stack_base[i],GTK_STATE_NORMAL,&frame_color);
    }
    //****

    //stacks
    covered_stack = stack_new();
    uncovered_stack = stack_new();
    for(int i = 0; i < 4; i++) dest_stack[i] = stack_new();
    //***

    //timer
    timer = gtk_label_new(NULL);
    g_object_set(timer,"halign",GTK_ALIGN_START,"valign",GTK_ALIGN_END,"margin-start",40,NULL);
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay),timer);
    //***

    //move counter
    move_counter = gtk_label_new(NULL);
    g_object_set(move_counter,"halign",GTK_ALIGN_START,"valign",GTK_ALIGN_END,"margin-start",40,"margin-bottom",20,NULL);
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay),move_counter);
    //***

    new_game_init();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    gtk_init(&argc,&argv);

    main_window_init();

    gtk_main();
}