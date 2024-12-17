#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "nlohmann/json.hpp" // Include the JSON library

namespace fs = std::filesystem;
using json = nlohmann::json;

// Function declarations
void on_select_video_folder(GtkWidget *widget, gpointer data);
void on_select_srt_folder(GtkWidget *widget, gpointer data);
void on_sync_button_clicked(GtkWidget *widget, gpointer data);
void on_value_changed(GtkWidget *widget, gpointer data);
int match_index = 1;
std::string episode_regex = "(\\d+)";
std::string extract_episode_number(const std::string &filename, const std::string &pattern);
bool matches_pattern(const std::string &filename, const std::string &pattern);
void sync_subtitles(const std::string &video_file, const std::string &srt_file, const std::string &output_file);

// Struct to hold widget pointers
struct AppWidgets {
    GtkWidget *video_folder_entry;
    GtkWidget *srt_folder_entry;
    GtkWidget *regex_entry;
    GtkWidget *output_name_entry;
    GtkWidget *disable_fps_checkbox;
    GtkWidget *split_penalty_scale;
    std::string config_file = "sync_config.json";
};

// Utility function to log messages
void log_message(const std::string &message) {
    std::cout << "[INFO]: " << message << std::endl;
}

void log_warning(const std::string &message) {
    std::cerr << "[WARNING]: " << message << std::endl;
}

void log_error(const std::string &message) {
    std::cerr << "[ERROR]: " << message << std::endl;
}
void save_values(AppWidgets *app_widgets) {
    json config;
    config["video_folder"] = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_folder_entry));
    config["srt_folder"] = gtk_entry_get_text(GTK_ENTRY(app_widgets->srt_folder_entry));
    config["regex"] = gtk_entry_get_text(GTK_ENTRY(app_widgets->regex_entry));
    config["output_name"] = gtk_entry_get_text(GTK_ENTRY(app_widgets->output_name_entry));
    config["disable_fps"] = static_cast<bool>(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app_widgets->disable_fps_checkbox)));
    config["split_penalty"] = static_cast<int>(gtk_range_get_value(GTK_RANGE(app_widgets->split_penalty_scale)));

    std::ofstream file(app_widgets->config_file);
    if (!file) {
        log_error("Failed to save configurations to file.");
        return;
    }

    file << config.dump(4);
    log_message("Configurations saved to file.");
}
void load_saved_values(AppWidgets *app_widgets) {
    std::ifstream file(app_widgets->config_file);
    if (!file) {
        log_warning("No configuration file found. Using defaults.");
        return;
    }

    json config;
    file >> config;
    log_message("Configurations loaded from file.");

    gtk_entry_set_text(GTK_ENTRY(app_widgets->video_folder_entry), config.value("video_folder", "").c_str());
    gtk_entry_set_text(GTK_ENTRY(app_widgets->srt_folder_entry), config.value("srt_folder", "").c_str());
    gtk_entry_set_text(GTK_ENTRY(app_widgets->regex_entry), config.value("regex", "").c_str());
    gtk_entry_set_text(GTK_ENTRY(app_widgets->output_name_entry), config.value("output_name", "").c_str());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app_widgets->disable_fps_checkbox), config.value("disable_fps", false));
    gtk_range_set_value(GTK_RANGE(app_widgets->split_penalty_scale), config.value("split_penalty", 60));
}

int main(int argc, char *argv[]) {
    log_message("Initializing GTK application...");
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Subtitle Sync Tool");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    log_message("Creating main window and UI components...");
    auto *app_widgets = new AppWidgets();
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Video folder selection
    app_widgets->video_folder_entry = gtk_entry_new();
    GtkWidget *video_folder_button = gtk_button_new_with_label("Select Video Folder");
    g_signal_connect(video_folder_button, "clicked", G_CALLBACK(on_select_video_folder), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->video_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), video_folder_button, FALSE, FALSE, 0);

    // SRT folder selection
    app_widgets->srt_folder_entry = gtk_entry_new();
    GtkWidget *srt_folder_button = gtk_button_new_with_label("Select SRT Folder");
    g_signal_connect(srt_folder_button, "clicked", G_CALLBACK(on_select_srt_folder), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->srt_folder_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), srt_folder_button, FALSE, FALSE, 0);

    // Regex entry
    app_widgets->regex_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_widgets->regex_entry), "Enter Regex Pattern");
    g_signal_connect(app_widgets->regex_entry, "changed", G_CALLBACK(on_value_changed), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->regex_entry, FALSE, FALSE, 0);

    // Output name entry
    app_widgets->output_name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_widgets->output_name_entry), "Enter Output Name");
    g_signal_connect(app_widgets->output_name_entry, "changed", G_CALLBACK(on_value_changed), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->output_name_entry, FALSE, FALSE, 0);

    // Disable FPS checkbox
    app_widgets->disable_fps_checkbox = gtk_check_button_new_with_label("Disable FPS Guessing");
    g_signal_connect(app_widgets->disable_fps_checkbox, "toggled", G_CALLBACK(on_value_changed), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->disable_fps_checkbox, FALSE, FALSE, 0);

    // Split penalty slider
    app_widgets->split_penalty_scale = gtk_hscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(app_widgets->split_penalty_scale), 60); // Default value
    GtkWidget *split_penalty_label = gtk_label_new("Split Penalty (0-100)");
    g_signal_connect(app_widgets->split_penalty_scale, "value-changed", G_CALLBACK(on_value_changed), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), split_penalty_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_widgets->split_penalty_scale, FALSE, FALSE, 0);

    // Sync button
    GtkWidget *sync_button = gtk_button_new_with_label("Sync Subtitles");
    g_signal_connect(sync_button, "clicked", G_CALLBACK(on_sync_button_clicked), app_widgets);
    gtk_box_pack_start(GTK_BOX(vbox), sync_button, FALSE, FALSE, 0);

    log_message("Loading saved configurations...");
    load_saved_values(app_widgets);

    log_message("Displaying the main application window.");
    gtk_widget_show_all(window);
    gtk_main();
    log_message("Application exited cleanly.");

    delete app_widgets;
    return 0;
}

void on_select_video_folder(GtkWidget *widget, gpointer data) {
    log_message("Opening file chooser dialog for video folder...");
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Select Video Folder", NULL,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Select", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app_widgets->video_folder_entry), folder);
        log_message("Video folder selected: " + std::string(folder));
        g_free(folder);
        save_values(app_widgets);
    } else {
        log_warning("Video folder selection was cancelled.");
    }
    gtk_widget_destroy(dialog);
}

void on_select_srt_folder(GtkWidget *widget, gpointer data) {
    log_message("Opening file chooser dialog for SRT folder...");
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Select SRT Folder", NULL,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Select", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app_widgets->srt_folder_entry), folder);
        log_message("SRT folder selected: " + std::string(folder));
        g_free(folder);
        save_values(app_widgets);
    } else {
        log_warning("SRT folder selection was cancelled.");
    }
    gtk_widget_destroy(dialog);
}

void on_value_changed(GtkWidget *widget, gpointer data) {
    log_message("A value was changed. Saving configurations...");
    save_values(static_cast<AppWidgets *>(data));
}

void on_sync_button_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app_widgets = static_cast<AppWidgets *>(data);
    log_message("Sync button clicked. Starting subtitle synchronization process...");

    std::string video_dir = gtk_entry_get_text(GTK_ENTRY(app_widgets->video_folder_entry));
    std::string srt_dir = gtk_entry_get_text(GTK_ENTRY(app_widgets->srt_folder_entry));
    std::string regex_pattern = gtk_entry_get_text(GTK_ENTRY(app_widgets->regex_entry));
    std::string output_name = gtk_entry_get_text(GTK_ENTRY(app_widgets->output_name_entry));

    log_message(video_dir);
    if (video_dir.empty() || srt_dir.empty() || regex_pattern.empty() || output_name.empty()) {
        log_error("One or more required inputs are empty. Aborting sync process.");
        return;
    }
    for (const auto &entry : fs::directory_iterator(video_dir)) {
        if (entry.is_regular_file()) {
            std::string video_file = entry.path().string();
            log_message(video_file);
            if (matches_pattern(video_file, regex_pattern)) {
                std::string episode = extract_episode_number(video_file, episode_regex);
                log_message(episode);
                if (!episode.empty()) {
                    std::string srt_file = srt_dir + "/" + episode + ".srt";
                    std::string output_file = video_dir + "/" + output_name + "_" + episode + ".srt";

                    if (!fs::exists(srt_file)) {
                        log_warning("Subtitle file not found for episode: " + episode);
                        continue;
                    }

                    log_message("Syncing video: " + video_file + " with subtitle: " + srt_file);
                    sync_subtitles(video_file, srt_file, output_file);
                }
            }
        }
    }

    log_message("Subtitle synchronization process completed.");
}

std::string extract_episode_number(const std::string &filename, const std::string &pattern) {
    std::regex regex(pattern);
    std::smatch match;
    if (std::regex_search(filename, match, regex) && match.size() > 1) {
        return match.str(match_index);
    }
    return "";
}

bool matches_pattern(const std::string &filename, const std::string &pattern) {
    log_message(pattern);
    return std::regex_search(filename, std::regex(pattern));
}

void sync_subtitles(const std::string &video_file, const std::string &srt_file, const std::string &output_file) {
    std::string command = "alass \"" + video_file + "\" \"" + srt_file + "\" \"" + output_file + "\"";
    log_message("Executing command: " + command);
    int result = system(command.c_str());
    if (result != 0) {
        log_error("Failed to sync subtitles for: " + video_file);
    } else {
        log_message("Successfully synced subtitles for: " + video_file);
    }
}
