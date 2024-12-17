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

// Struct to hold widget pointers
struct AppWidgets {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *video_folder_entry;
    GtkWidget *srt_folder_entry;
    GtkWidget *regex_entry;
    GtkWidget *episode_regex_entry;
    GtkWidget *episode_match_index_input;
    GtkWidget *refresh_button;
    GtkWidget *sync_button;
    GtkWidget *show_video_dir_button;
    GtkWidget *show_srt_dir_button;
    GtkWidget *video_folder_label;
    GtkWidget *srt_folder_label;
    GtkWidget *regex_label;
    GtkWidget *episode_regex_label;
    GtkWidget *episode_match_index_label;
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
std::vector<std::string> extract_episode_numbers(const std::vector<std::string> &files, const std::string &regex);

// Function to escape backslashes in the user input for regex
std::string escape_backslashes(const std::string &input) {
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find("\\", pos)) != std::string::npos) {
        result.replace(pos, 1, "\\\\");
        pos += 2;
    }
    return result;
}

// Function to validate the regex
bool is_valid_regex(const std::string &regex_str) {
    try {
        std::regex re(regex_str);
        return true;
    } catch (const std::regex_error &) {
        return false;
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWidgets app_widgets = {};

    // Create the main window
    app_widgets.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_widgets.window), "Subtitle Sync");
    gtk_window_set_default_size(GTK_WINDOW(app_widgets.window), 800, 600);

    // Create a vertical box container for the UI
    app_widgets.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app_widgets.window), app_widgets.vbox);

    // Create labels for each widget
    app_widgets.video_folder_label = gtk_label_new("Video Folder:");
    app_widgets.srt_folder_label = gtk_label_new("SRT Folder:");
    app_widgets.regex_label = gtk_label_new("Regex Pattern:");
    app_widgets.episode_regex_label = gtk_label_new("Episode Regex:");
    app_widgets.episode_match_index_label = gtk_label_new("Match Index:");

    // Create widgets
    app_widgets.video_folder_entry = gtk_entry_new();
    app_widgets.srt_folder_entry = gtk_entry_new();
    app_widgets.regex_entry = gtk_entry_new();
    app_widgets.episode_regex_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app_widgets.episode_regex_entry), R"(\d+)"); // Default to match numbers

    app_widgets.episode_match_index_input = gtk_spin_button_new_with_range(1, 10, 1);
    app_widgets.refresh_button = gtk_button_new_with_label("Refresh Directories");
    app_widgets.sync_button = gtk_button_new_with_label("Sync Subtitles");
    app_widgets.show_video_dir_button = gtk_button_new_with_label("Show Video Files");
    app_widgets.show_srt_dir_button = gtk_button_new_with_label("Show SRT Files");

    // File list boxes to display video and subtitle file matches
    app_widgets.video_file_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    app_widgets.srt_file_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    // Add widgets to the vertical box container
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_folder_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_folder_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.regex_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.regex_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.episode_regex_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.episode_regex_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.episode_match_index_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.episode_match_index_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.refresh_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.sync_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.show_video_dir_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.show_srt_dir_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_file_list_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_file_list_box, TRUE, TRUE, 0);

    // Load saved configurations (if any)
    load_saved_values(&app_widgets);

    // Set up signal handlers for widgets
    g_signal_connect(app_widgets.refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), &app_widgets);
    g_signal_connect(app_widgets.sync_button, "clicked", G_CALLBACK(on_sync_button_clicked), &app_widgets);
    g_signal_connect(app_widgets.episode_regex_entry, "changed", G_CALLBACK(on_episode_regex_value_changed), &app_widgets);
    g_signal_connect(app_widgets.show_video_dir_button, "clicked", G_CALLBACK(on_show_video_dir_button_clicked), &app_widgets);
    g_signal_connect(app_widgets.show_srt_dir_button, "clicked", G_CALLBACK(on_show_srt_dir_button_clicked), &app_widgets);

    // Ensure the widgets are properly displayed
    gtk_widget_show_all(app_widgets.window);

    // Main event loop
    g_signal_connect(app_widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();

    return 0;
}

void on_refresh_button_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    // Get the directories and update the file lists
    const std::string video_folder = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_folder_entry));
    const std::string srt_folder = gtk_entry_get_text(GTK_ENTRY(app_widgets->srt_folder_entry));
    
    // Fetch files from directories
    app_widgets->video_files = get_files_in_directory(video_folder);
    app_widgets->subtitle_files = get_files_in_directory(srt_folder);

    // Show matches of episode numbers from both directories
    show_file_matches(app_widgets);
}

void on_sync_button_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    log_message("Syncing subtitles...");

    // Check if episode numbers match
    std::string episode_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->episode_regex_entry));
    std::vector<std::string> video_matches = extract_episode_numbers(app_widgets->video_files, episode_regex);
    std::vector<std::string> subtitle_matches = extract_episode_numbers(app_widgets->subtitle_files, episode_regex);
    
    // Implement logic to sync subtitles if episode numbers match
    // This should include checking if the selected match index from the regex matches
}

void on_episode_regex_value_changed(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    // Get the regex from the input and escape backslashes
    std::string episode_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->episode_regex_entry));
    episode_regex = escape_backslashes(episode_regex);  // Escape backslashes

    // Validate the regex before proceeding
    if (is_valid_regex(episode_regex)) {
        // Refresh file matches if the regex is valid
        show_file_matches(app_widgets);
    } else {
        log_error("Invalid regex. Please fix the pattern.");
    }
}

void on_show_video_dir_button_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    app_widgets->video_dir_visible = !app_widgets->video_dir_visible;
    if (app_widgets->video_dir_visible) {
        gtk_widget_show(app_widgets->video_file_list_box);
    } else {
        gtk_widget_hide(app_widgets->video_file_list_box);
    }
}

void on_show_srt_dir_button_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    app_widgets->srt_dir_visible = !app_widgets->srt_dir_visible;
    if (app_widgets->srt_dir_visible) {
        gtk_widget_show(app_widgets->srt_file_list_box);
    } else {
        gtk_widget_hide(app_widgets->srt_file_list_box);
    }
}

void show_file_matches(AppWidgets *app_widgets) {
    // Clear previous list
    GtkWidget *video_file_list_box = app_widgets->video_file_list_box;
    GList *children = gtk_container_get_children(GTK_CONTAINER(video_file_list_box));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    GtkWidget *srt_file_list_box = app_widgets->srt_file_list_box;
    children = gtk_container_get_children(GTK_CONTAINER(srt_file_list_box));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Show file entries from both video and subtitle directories
    for (const auto &file : app_widgets->video_files) {
        GtkWidget *label = gtk_label_new(file.c_str());
        gtk_box_pack_start(GTK_BOX(video_file_list_box), label, FALSE, FALSE, 0);
    }

    for (const auto &file : app_widgets->subtitle_files) {
        GtkWidget *label = gtk_label_new(file.c_str());
        gtk_box_pack_start(GTK_BOX(srt_file_list_box), label, FALSE, FALSE, 0);
    }

    // Show episode number matches for video and subtitle files
    const std::string episode_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->episode_regex_entry));
    std::vector<std::string> video_matches = extract_episode_numbers(app_widgets->video_files, episode_regex);
    std::vector<std::string> subtitle_matches = extract_episode_numbers(app_widgets->subtitle_files, episode_regex);

    // Display matches for video files
    for (const auto &match : video_matches) {
        GtkWidget *label = gtk_label_new(match.c_str());
        gtk_box_pack_start(GTK_BOX(video_file_list_box), label, FALSE, FALSE, 0);
    }

    // Display matches for subtitle files
    for (const auto &match : subtitle_matches) {
        GtkWidget *label = gtk_label_new(match.c_str());
        gtk_box_pack_start(GTK_BOX(srt_file_list_box), label, FALSE, FALSE, 0);
    }

    gtk_widget_show_all(video_file_list_box);
    gtk_widget_show_all(srt_file_list_box);
}

void save_values(AppWidgets *app_widgets) {
    // Save values to config file
    json config = {
        {"video_folder", gtk_entry_get_text(GTK_ENTRY(app_widgets->video_folder_entry))},
        {"srt_folder", gtk_entry_get_text(GTK_ENTRY(app_widgets->srt_folder_entry))},
        {"episode_regex", gtk_entry_get_text(GTK_ENTRY(app_widgets->episode_regex_entry))},
        {"episode_match_index", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_widgets->episode_match_index_input))}
    };

    std::ofstream config_file(app_widgets->config_file);
    config_file << config.dump(4);
    config_file.close();
}

void load_saved_values(AppWidgets *app_widgets) {
    if (fs::exists(app_widgets->config_file)) {
        std::ifstream config_file(app_widgets->config_file);
        json config;
        config_file >> config;

        gtk_entry_set_text(GTK_ENTRY(app_widgets->video_folder_entry), config["video_folder"].get<std::string>().c_str());
        gtk_entry_set_text(GTK_ENTRY(app_widgets->srt_folder_entry), config["srt_folder"].get<std::string>().c_str());
        gtk_entry_set_text(GTK_ENTRY(app_widgets->episode_regex_entry), config["episode_regex"].get<std::string>().c_str());
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_widgets->episode_match_index_input), config["episode_match_index"].get<int>());
    }
}

void log_message(const std::string &message) {
    std::cout << "[INFO]: " << message << std::endl;
}

void log_error(const std::string &message) {
    std::cerr << "[ERROR]: " << message << std::endl;
}

std::vector<std::string> get_files_in_directory(const std::string &directory) {
    std::vector<std::string> files;
    for (const auto &entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

std::vector<std::string> extract_episode_numbers(const std::vector<std::string> &files, const std::string &regex_str) {
    std::vector<std::string> matches;
    try {
        std::regex re(regex_str);
        for (const auto &file : files) {
            std::smatch match;
            if (std::regex_search(file, match, re)) {
                matches.push_back(match.str());
            }
        }
    } catch (const std::regex_error &) {
        log_error("Invalid regex for episode extraction.");
    }
    return matches;
}
