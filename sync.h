#ifndef SYNC_ALASS_HPP
#define SYNC_ALASS_HPP

#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <regex>
#include <vector>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

// Struct to hold widget pointers and app data
struct AppWidgets {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *video_folder_entry;
    GtkWidget *srt_folder_entry;
    GtkWidget *regex_entry;
    GtkWidget *episode_regex_entry;
    GtkWidget *episode_match_index_input_video;
    GtkWidget *episode_match_index_input_srt;
    GtkWidget *refresh_button;
    GtkWidget *sync_button;
    GtkWidget *show_video_dir_button;
    GtkWidget *show_srt_dir_button;
    GtkWidget *video_folder_label;
    GtkWidget *srt_folder_label;
    GtkWidget *regex_label;
    GtkWidget *episode_regex_label;
    GtkWidget *episode_match_index_label_video;
    GtkWidget *episode_match_index_label_srt;
    GtkWidget *file_list_box;
    GtkWidget *video_file_list_box;
    GtkWidget *srt_file_list_box;
    std::vector<std::string> video_files;
    std::vector<std::string> subtitle_files;
    std::string config_file = "sync_config.json";
    bool video_dir_visible = true;
    bool srt_dir_visible = true;
};

// Function declarations
void on_refresh_button_clicked(GtkWidget *widget, gpointer data);
void on_sync_button_clicked(GtkWidget *widget, gpointer data);
void on_episode_regex_value_changed(GtkWidget *widget, gpointer data);
void on_show_video_dir_button_clicked(GtkWidget *widget, gpointer data);
void on_show_srt_dir_button_clicked(GtkWidget *widget, gpointer data);
void show_file_matches(AppWidgets *app_widgets);
void save_values(AppWidgets *app_widgets);
void load_saved_values(AppWidgets *app_widgets);
void log_message(const std::string &message);
void log_error(const std::string &message);
std::vector<std::string> get_files_in_directory(const std::string &directory);
std::vector<std::string> extract_episode_numbers(const std::vector<std::string> &files, const std::string &regex, int match_index);

#endif // SYNC_ALASS_HPP
