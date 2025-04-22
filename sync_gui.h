#ifndef SYNC_GUI_H
#define SYNC_GUI_H

#include <gtk/gtk.h>
#include <string>
#include <vector>

struct AppWidgets {
    GtkWidget *video_folder_entry;
    GtkWidget *srt_folder_entry;
    GtkWidget *regex_entry;
    GtkWidget *episode_regex_entry;
    GtkWidget *output_name_entry;
    GtkWidget *disable_fps_checkbox;
    GtkWidget *split_penalty_scale;
    GtkWidget *video_file_list_box;
    GtkWidget *srt_file_list_box;
    GtkWidget *match_index_video_spin;
    GtkWidget *match_index_srt_spin;
    GtkWidget *video_folder_button;
    GtkWidget *srt_folder_button;
    std::string config_file = "sync_config.json";
};

// Function declarations
void on_select_video_folder(GtkWidget *widget, gpointer data);
void on_select_srt_folder(GtkWidget *widget, gpointer data);
void on_sync_button_clicked(GtkWidget *widget, gpointer data);
void on_value_changed(GtkWidget *widget, gpointer data);
void update_file_list(AppWidgets *app_widgets, const std::string &video_dir, const std::string &srt_dir);
void load_saved_values(AppWidgets *app_widgets);
void save_values(AppWidgets *app_widgets);

#endif // SYNC_GUI_H
