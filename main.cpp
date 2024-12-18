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

// Define the AppWidgets structure
typedef struct {
    GtkWidget *window;
    GtkWidget *vbox;

    // Labels
    GtkWidget *video_folder_label;
    GtkWidget *srt_folder_label;
    GtkWidget *video_regex_label;
    GtkWidget *subtitle_regex_label;
    GtkWidget *video_match_index_label;
    GtkWidget *subtitle_match_index_label;

    // Entry fields
    GtkWidget *video_folder_entry;
    GtkWidget *srt_folder_entry;
    GtkWidget *video_regex_entry;
    GtkWidget *subtitle_regex_entry;

    // Spin buttons
    GtkWidget *video_match_index_input;
    GtkWidget *subtitle_match_index_input;

    // Buttons
    GtkWidget *refresh_button;
    GtkWidget *sync_button;
    GtkWidget *show_video_dir_button;
    GtkWidget *show_srt_dir_button;

    // File list boxes
    GtkWidget *video_file_list_box;
    GtkWidget *srt_file_list_box;
    std::vector<std::string> video_files;
    std::vector<std::string> subtitle_files;
    std::string config_file = "sync_config.json";
    bool video_dir_visible = true;
    bool srt_dir_visible = true;
} AppWidgets;

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

// Main function
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

    // Labels
    app_widgets.video_folder_label = gtk_label_new("Video Folder:");
    app_widgets.srt_folder_label = gtk_label_new("Subtitle Folder:");
    app_widgets.video_regex_label = gtk_label_new("Video Matching Regex:");
    app_widgets.subtitle_regex_label = gtk_label_new("Subtitle Matching Regex:");
    app_widgets.video_match_index_label = gtk_label_new("Video Regex Match Index:");
    app_widgets.subtitle_match_index_label = gtk_label_new("Subtitle Regex Match Index:");

    // Entry fields
    app_widgets.video_folder_entry = gtk_entry_new();
    app_widgets.srt_folder_entry = gtk_entry_new();
    app_widgets.video_regex_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app_widgets.video_regex_entry), R"(\d+)");  // Default regex for videos
    app_widgets.subtitle_regex_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app_widgets.subtitle_regex_entry), R"(\d+)");  // Default regex for subtitles

    // Spin buttons for match index
    app_widgets.video_match_index_input = gtk_spin_button_new_with_range(1, 10, 1);
    app_widgets.subtitle_match_index_input = gtk_spin_button_new_with_range(1, 10, 1);

    // Buttons
    app_widgets.refresh_button = gtk_button_new_with_label("Refresh Directories");
    app_widgets.sync_button = gtk_button_new_with_label("Sync Subtitles");
    app_widgets.show_video_dir_button = gtk_button_new_with_label("Show Video Files");
    app_widgets.show_srt_dir_button = gtk_button_new_with_label("Show Subtitle Files");

    // File list boxes
    app_widgets.video_file_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    app_widgets.srt_file_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    // Add widgets to the vertical box container
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_folder_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_folder_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_regex_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_regex_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.subtitle_regex_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.subtitle_regex_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_match_index_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_match_index_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.subtitle_match_index_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.subtitle_match_index_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.refresh_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.sync_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.show_video_dir_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.show_srt_dir_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.video_file_list_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(app_widgets.vbox), app_widgets.srt_file_list_box, TRUE, TRUE, 0);

    // Load saved configurations (if any)
    // load_saved_values(&app_widgets); // You would need to implement this function

    // Set up signal handlers for widgets
    g_signal_connect(app_widgets.refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), &app_widgets);
    g_signal_connect(app_widgets.sync_button, "clicked", G_CALLBACK(on_sync_button_clicked), &app_widgets);
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

    log_message("Starting subtitle synchronization...");
    std::string video_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_regex_entry));
    std::string subtitle_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->subtitle_regex_entry));

    if (!is_valid_regex(video_regex) || !is_valid_regex(subtitle_regex)) {
        log_error("One or both regex patterns are invalid. Please correct them.");
        return;
    }

    // Extract episode numbers for videos and subtitles
    std::vector<std::string> video_matches = extract_episode_numbers(app_widgets->video_files, video_regex);
    std::vector<std::string> subtitle_matches = extract_episode_numbers(app_widgets->subtitle_files, subtitle_regex);

    if (video_matches.empty()) {
        log_error("No video matches found. Check the video regex pattern.");
        return;
    }
    if (subtitle_matches.empty()) {
        log_error("No subtitle matches found. Check the subtitle regex pattern.");
        return;
    }

    log_message("Found " + std::to_string(video_matches.size()) + " video matches.");
    log_message("Found " + std::to_string(subtitle_matches.size()) + " subtitle matches.");

    // Match and sync
    for (size_t i = 0; i < video_matches.size(); ++i) {
        for (size_t j = 0; j < subtitle_matches.size(); ++j) {
            if (video_matches[i] == subtitle_matches[j]) {
                std::string video_file = app_widgets->video_files[i];
                std::string subtitle_file = app_widgets->subtitle_files[j];
                std::string output_file = "synced_" + video_matches[i] + ".mkv";

                std::string command = "alass \"" + video_file + "\" \"" + subtitle_file + "\" \"" + output_file + "\"";
                log_message("Executing: " + command);

                int result = system(command.c_str());
                if (result != 0) {
                    log_error("Failed to sync subtitles for " + video_file);
                } else {
                    log_message("Successfully synced subtitles for " + video_file);
                }
            }
        }
    }

    log_message("Subtitle synchronization completed.");
}

void on_episode_regex_value_changed(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);

    // Get the regex from the input and escape backslashes
    std::string video_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_regex_entry));
    video_regex = escape_backslashes(video_regex);  // Escape backslashes

    // Validate the regex before proceeding
    if (is_valid_regex(video_regex)) {
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
    const std::string video_episode_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_regex_entry));
    const std::string subtitle_episode_regex = gtk_entry_get_text(GTK_ENTRY(app_widgets->subtitle_regex_entry));

    // Extract episode numbers using regex for video and subtitle files
    std::vector<std::string> video_matches = extract_episode_numbers(app_widgets->video_files, video_episode_regex);
    std::vector<std::string> subtitle_matches = extract_episode_numbers(app_widgets->subtitle_files, subtitle_episode_regex);
  
    // Display matches for video files
    for (const auto &match : video_matches) {
        GtkWidget *label = gtk_label_new(("Video Match: " + match).c_str());
        gtk_box_pack_start(GTK_BOX(video_file_list_box), label, FALSE, FALSE, 0);
    }

    // Display matches for subtitle files
    for (const auto &match : subtitle_matches) {
        GtkWidget *label = gtk_label_new(("Subtitle Match: " + match).c_str());
        gtk_box_pack_start(GTK_BOX(srt_file_list_box), label, FALSE, FALSE, 0);
    }

    gtk_widget_show_all(video_file_list_box);
    gtk_widget_show_all(srt_file_list_box);
}

void save_values(AppWidgets *app_widgets) {
    json config = {
        {"video_folder", gtk_entry_get_text(GTK_ENTRY(app_widgets->video_folder_entry))},
        {"srt_folder", gtk_entry_get_text(GTK_ENTRY(app_widgets->srt_folder_entry))},
        {"video_regex", gtk_entry_get_text(GTK_ENTRY(app_widgets->video_regex_entry))},
        {"subtitle_regex", gtk_entry_get_text(GTK_ENTRY(app_widgets->subtitle_regex_entry))},
        {"video_match_index", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_widgets->video_match_index_input))},
        {"subtitle_match_index", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_widgets->subtitle_match_index_input))}
    };

    std::ofstream config_file(app_widgets->config_file);
    config_file << config.dump(4);
    config_file.close();
}

void load_saved_values(AppWidgets *app_widgets) {
    if (fs::exists(app_widgets->config_file)) {
        std::ifstream config_file(app_widgets->config_file);
        json config;
        try {
            config_file >> config;
        } catch (const json::parse_error &e) {
            log_error("Error parsing config file: " + std::string(e.what()));
            return;
        }

        // Load and validate the values
        if (config.contains("video_folder") && config["video_folder"].is_string()) {
            gtk_entry_set_text(GTK_ENTRY(app_widgets->video_folder_entry), config["video_folder"].get<std::string>().c_str());
        } else {
            log_error("Missing or invalid 'video_folder' in config.");
        }

        if (config.contains("srt_folder") && config["srt_folder"].is_string()) {
            gtk_entry_set_text(GTK_ENTRY(app_widgets->srt_folder_entry), config["srt_folder"].get<std::string>().c_str());
        } else {
            log_error("Missing or invalid 'srt_folder' in config.");
        }

        if (config.contains("video_regex") && config["video_regex"].is_string()) {
            gtk_entry_set_text(GTK_ENTRY(app_widgets->video_regex_entry), config["video_regex"].get<std::string>().c_str());
        } else {
            log_error("Missing or invalid 'video_regex' in config.");
        }

        if (config.contains("subtitle_regex") && config["subtitle_regex"].is_string()) {
            gtk_entry_set_text(GTK_ENTRY(app_widgets->subtitle_regex_entry), config["subtitle_regex"].get<std::string>().c_str());
        } else {
            log_error("Missing or invalid 'subtitle_regex' in config.");
        }

        if (config.contains("video_match_index") && config["video_match_index"].is_number_integer()) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_widgets->video_match_index_input), config["video_match_index"].get<int>());
        } else {
            log_error("Missing or invalid 'video_match_index' in config.");
        }

        if (config.contains("subtitle_match_index") && config["subtitle_match_index"].is_number_integer()) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_widgets->subtitle_match_index_input), config["subtitle_match_index"].get<int>());
        } else {
            log_error("Missing or invalid 'subtitle_match_index' in config.");
        }
    } else {
        log_error("Config file does not exist.");
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
