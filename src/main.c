#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include "cards.h"
#include "events.h"

GtkWidget *main_window;
GtkWidget *main_fixed;

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

void clear_game()
{
    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
        for(int j = 0; j < MAIN_GRID_SIZE_Y; j++)
            main_grid[i][j] = NULL;
}

void new_game_init()
{
    clear_game();

    random_shuffle(CARD_COUNT,cards);

    int current_ind = 0;

    //kupka do dobierania
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("images/zback_blue.svg",CARD_WIDTH,CARD_HEIGHT,TRUE,NULL);
    GtkWidget *covered_stack_top = gtk_image_new_from_pixbuf(pixbuf);
    gtk_fixed_put(GTK_FIXED(main_fixed),covered_stack_top,0,0);

    GtkWidget *stack_frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(stack_frame,CARD_WIDTH,CARD_HEIGHT);
    gtk_fixed_put(GTK_FIXED(main_fixed),stack_frame,CARD_WIDTH+GAP_SIZE,0);
    //***

    //dest
    for(int i = 0; i < 4; i++)
    {
        stack_frame = gtk_frame_new(NULL);
        gtk_widget_set_size_request(stack_frame,CARD_WIDTH,CARD_HEIGHT);
        gtk_fixed_put(GTK_FIXED(main_fixed),stack_frame,(MAIN_GRID_SIZE_X-1-i)*(CARD_WIDTH+GAP_SIZE),0);
    }
    //****

    // puste pola pod kupkami kart
    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
    {
        main_grid_stack_size[i] = i+1;

        main_grid[i][0] = malloc(sizeof(Field));

        main_grid[i][0]->locked = FALSE;
        main_grid[i][0]->card = NULL;
        main_grid[i][0]->widget = gtk_frame_new(NULL);

        gtk_widget_set_size_request(main_grid[i][0]->widget,CARD_WIDTH,CARD_HEIGHT);
        gtk_drag_dest_set(main_grid[i][0]->widget,GTK_DEST_DEFAULT_ALL,targets,1,GDK_ACTION_COPY);
        g_signal_connect(G_OBJECT(main_grid[i][0]->widget),"drag-drop",G_CALLBACK(drag_drop),main_fixed);
        
        //gtk_container_add(GTK_CONTAINER(main_grid[i][0]->widget),gtk_frame_new("dupa"));
        gtk_fixed_put(GTK_FIXED(main_fixed),main_grid[i][0]->widget,i*(GAP_SIZE+CARD_WIDTH),MAIN_GRID_START_Y);
    }
    //***

    //main grid
    for(int i = 0; i < MAIN_GRID_SIZE_X; i++)
    {
        for(int j = 1; j <= i+1; j++)
        {
            main_grid[i][j] = malloc(sizeof(Field));
            main_grid[i][j]->locked = i+1 != j;
            main_grid[i][j]->card = cards+current_ind;

            if(main_grid[i][j]->locked)
            {
                pixbuf = gdk_pixbuf_new_from_file_at_scale("images/zback_blue.svg",CARD_WIDTH,CARD_HEIGHT,TRUE,NULL);
                main_grid[i][j]->widget = gtk_image_new_from_pixbuf(pixbuf);
            }
            else main_grid[i][j]->widget = cards[current_ind].image, make_draggable(main_grid[i][j]);
            
            gtk_fixed_put(GTK_FIXED(main_fixed),main_grid[i][j]->widget,i*(GAP_SIZE+CARD_WIDTH),MAIN_GRID_START_Y+(j-1)*GAP_SIZE);
            current_ind++;
        }
    }
}

void main_window_init()
{
    //main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window),"Pasjans");
    gtk_window_maximize(GTK_WINDOW(main_window));
   // gtk_window_set_resizable(GTK_WINDOW(main_window),FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(main_window),GAP_SIZE);
    
    GdkColor color;
    color.red = 120, color.green = 20125, color.blue = 812;
    gtk_widget_modify_bg(main_window,GTK_STATE_NORMAL,&color);

    g_signal_connect(G_OBJECT(main_window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
    // *******

    main_fixed = gtk_fixed_new();

    GtkWidget *main_overlay = gtk_overlay_new();

    //fixed
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay),main_fixed);
    g_object_set(main_fixed,"halign",GTK_ALIGN_CENTER,"valign",GTK_ALIGN_START,NULL);
    //***

    gtk_container_add(GTK_CONTAINER(main_window),main_overlay);

    cards_init();
    new_game_init();
   
    gtk_widget_show_all(main_window);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    gtk_init(&argc,&argv);

    main_window_init();

    gtk_main();
}