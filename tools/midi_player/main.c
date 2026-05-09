#include "r2d/r2d.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIDI_PLAYER_MAX_FILES 128
#define MIDI_PLAYER_NAME_SIZE 64
#define MIDI_PLAYER_PATH_SIZE 256

typedef struct MidiPlayerFile {
    char name[MIDI_PLAYER_NAME_SIZE];
    char path[MIDI_PLAYER_PATH_SIZE];
} MidiPlayerFile;

typedef struct MidiPlayer {
    MidiPlayerFile midis[MIDI_PLAYER_MAX_FILES];
    MidiPlayerFile soundfonts[MIDI_PLAYER_MAX_FILES];
    R2D_Music music;
    R2D_Context *context;
    R2D_Crt *crt;
    int midi_count;
    int soundfont_count;
    int selected_midi;
    int selected_soundfont;
    int selected_channel;
    int active_column;
    float message_timer;
    float volume;
    const char *message;
    bool loop;
    bool auto_play;
    bool channel_view;
    bool music_loaded;
    char last_error[128];
    char loaded_midi[MIDI_PLAYER_NAME_SIZE];
    char loaded_soundfont[MIDI_PLAYER_NAME_SIZE];
} MidiPlayer;

static void MidiPlayer_SetMessage(MidiPlayer *player, const char *message)
{
    player->message = message;
    player->message_timer = 1.5f;
}

static void MidiPlayer_CopyText(char *destination, int destination_size, const char *source)
{
    if (destination_size <= 0) {
        return;
    }

    if (source == 0) {
        source = "";
    }

    snprintf(destination, (size_t)destination_size, "%s", source);
}

static int MidiPlayer_CompareNames(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        const char a = *left >= 'A' && *left <= 'Z' ? (char)(*left + 32) : *left;
        const char b = *right >= 'A' && *right <= 'Z' ? (char)(*right + 32) : *right;

        if (a != b) {
            return (int)a - (int)b;
        }

        ++left;
        ++right;
    }

    return (int)*left - (int)*right;
}

static bool MidiPlayer_HasExtension(const char *file_name, const char *extension)
{
    const char *dot = strrchr(file_name, '.');

    if (dot == 0) {
        return false;
    }

    return MidiPlayer_CompareNames(dot, extension) == 0;
}

static void MidiPlayer_SortFiles(MidiPlayerFile *files, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (MidiPlayer_CompareNames(files[i].name, files[j].name) > 0) {
                const MidiPlayerFile temp = files[i];
                files[i] = files[j];
                files[j] = temp;
            }
        }
    }
}

static void MidiPlayer_AddFile(
    MidiPlayerFile *files,
    int *count,
    const char *directory,
    const char *file_name
)
{
    MidiPlayerFile *file;

    if (*count >= MIDI_PLAYER_MAX_FILES || file_name == 0 || file_name[0] == '\0') {
        return;
    }

    file = &files[*count];
    MidiPlayer_CopyText(file->name, sizeof(file->name), file_name);
    snprintf(file->path, sizeof(file->path), "%s/%s", directory, file_name);
    *count += 1;
}

static void MidiPlayer_DiscoverFiles(MidiPlayer *player)
{
    FilePathList midi_files;
    FilePathList soundfont_files;

    player->midi_count = 0;
    player->soundfont_count = 0;

    midi_files = LoadDirectoryFiles(R2D_AssetPath("audio/music"));
    for (unsigned int i = 0; i < midi_files.count; ++i) {
        const char *file_name = GetFileName(midi_files.paths[i]);

        if (MidiPlayer_HasExtension(file_name, ".mid") || MidiPlayer_HasExtension(file_name, ".midi")) {
            MidiPlayer_AddFile(player->midis, &player->midi_count, "audio/music", file_name);
        }
    }
    UnloadDirectoryFiles(midi_files);

    soundfont_files = LoadDirectoryFilesEx(R2D_AssetPath("audio/soundfonts"), ".sf2", false);
    for (unsigned int i = 0; i < soundfont_files.count; ++i) {
        MidiPlayer_AddFile(player->soundfonts, &player->soundfont_count, "audio/soundfonts", GetFileName(soundfont_files.paths[i]));
    }
    UnloadDirectoryFiles(soundfont_files);

    MidiPlayer_SortFiles(player->midis, player->midi_count);
    MidiPlayer_SortFiles(player->soundfonts, player->soundfont_count);

    if (player->selected_midi >= player->midi_count) {
        player->selected_midi = player->midi_count > 0 ? player->midi_count - 1 : 0;
    }

    if (player->selected_soundfont >= player->soundfont_count) {
        player->selected_soundfont = player->soundfont_count > 0 ? player->soundfont_count - 1 : 0;
    }
}

static MidiPlayerFile *MidiPlayer_CurrentMidi(MidiPlayer *player)
{
    return player->midi_count > 0 ? &player->midis[player->selected_midi] : 0;
}

static MidiPlayerFile *MidiPlayer_CurrentSoundFont(MidiPlayer *player)
{
    return player->soundfont_count > 0 ? &player->soundfonts[player->selected_soundfont] : 0;
}

static const char *MidiPlayer_FormatTime(float seconds)
{
    const int total = seconds > 0.0f ? (int)(seconds + 0.5f) : 0;
    const int minutes = total / 60;
    const int remaining_seconds = total % 60;

    return TextFormat("%02d:%02d", minutes, remaining_seconds);
}

static const char *MidiPlayer_FitText(const char *text, int font_size, int max_width)
{
    static char buffers[4][MIDI_PLAYER_PATH_SIZE];
    static int buffer_index = 0;
    char *buffer;
    int length;

    if (text == 0) {
        text = "";
    }

    if (MeasureText(text, font_size) <= max_width) {
        return text;
    }

    buffer = buffers[buffer_index];
    buffer_index = (buffer_index + 1) % 4;
    snprintf(buffer, MIDI_PLAYER_PATH_SIZE, "%s", text);
    length = (int)strlen(buffer);

    while (length > 3) {
        buffer[length - 3] = '.';
        buffer[length - 2] = '.';
        buffer[length - 1] = '.';
        buffer[length] = '\0';

        if (MeasureText(buffer, font_size) <= max_width) {
            return buffer;
        }

        length -= 1;
        buffer[length] = '\0';
    }

    snprintf(buffer, MIDI_PLAYER_PATH_SIZE, "...");
    return buffer;
}

static bool MidiPlayer_OpenFile(FILE **file, const char *path, const char *mode)
{
    if (file == 0 || path == 0 || mode == 0) {
        return false;
    }

#if defined(_MSC_VER)
    return fopen_s(file, path, mode) == 0 && *file != 0;
#else
    *file = fopen(path, mode);
    return *file != 0;
#endif
}

static char *MidiPlayer_Trim(char *text)
{
    char *end;

    while (*text != '\0' && isspace((unsigned char)*text)) {
        ++text;
    }

    end = text + strlen(text);

    while (end > text && isspace((unsigned char)*(end - 1))) {
        --end;
    }

    *end = '\0';
    return text;
}

static bool MidiPlayer_ParseBool(const char *text)
{
    return MidiPlayer_CompareNames(text, "true") == 0 ||
        MidiPlayer_CompareNames(text, "yes") == 0 ||
        MidiPlayer_CompareNames(text, "on") == 0 ||
        MidiPlayer_CompareNames(text, "1") == 0;
}

static int MidiPlayer_FindFile(const MidiPlayerFile *files, int count, const char *name)
{
    for (int i = 0; i < count; ++i) {
        if (MidiPlayer_CompareNames(files[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

static void MidiPlayer_SongPathForMidi(char *path, int path_size, const char *midi_name)
{
    char directory[1024];
    char base[MIDI_PLAYER_NAME_SIZE];
    char *extension;

    snprintf(directory, sizeof(directory), "%s", R2D_AssetPath("audio/music"));
    MidiPlayer_CopyText(base, sizeof(base), midi_name);
    extension = strrchr(base, '.');

    if (extension != 0) {
        *extension = '\0';
    }

    snprintf(path, (size_t)path_size, "%s/%s.r2song", directory, base);
}

static void MidiPlayer_Stop(MidiPlayer *player)
{
    if (player->music_loaded) {
        R2D_MusicUnload(&player->music);
        player->music_loaded = false;
    }
}

static void MidiPlayer_PlaySelection(MidiPlayer *player)
{
    MidiPlayerFile *midi = MidiPlayer_CurrentMidi(player);
    MidiPlayerFile *soundfont = MidiPlayer_CurrentSoundFont(player);
    char midi_path[1024];
    char soundfont_path[1024];

    if (midi == 0) {
        MidiPlayer_SetMessage(player, "no midi files");
        return;
    }

    if (soundfont == 0) {
        MidiPlayer_SetMessage(player, "no soundfonts");
        return;
    }

    MidiPlayer_Stop(player);

    snprintf(midi_path, sizeof(midi_path), "%s", R2D_AssetPath(midi->path));
    snprintf(soundfont_path, sizeof(soundfont_path), "%s", R2D_AssetPath(soundfont->path));

    if (R2D_MusicLoad(&player->music, midi_path, soundfont_path)) {
        R2D_MusicSetVolume(&player->music, player->volume);
        R2D_MusicPlay(&player->music, player->loop);
        player->music_loaded = true;
        MidiPlayer_CopyText(player->loaded_midi, sizeof(player->loaded_midi), midi->name);
        MidiPlayer_CopyText(player->loaded_soundfont, sizeof(player->loaded_soundfont), soundfont->name);
        player->last_error[0] = '\0';
        MidiPlayer_SetMessage(player, "playing");
    } else {
        player->loaded_midi[0] = '\0';
        player->loaded_soundfont[0] = '\0';
        MidiPlayer_CopyText(player->last_error, sizeof(player->last_error), R2D_MusicLastError());
        MidiPlayer_SetMessage(player, "load failed");
    }
}

static void MidiPlayer_SaveSong(MidiPlayer *player)
{
    FILE *file = 0;
    char path[1200];

    if (!player->music_loaded) {
        MidiPlayer_SetMessage(player, "play first");
        return;
    }

    MidiPlayer_SongPathForMidi(path, sizeof(path), player->loaded_midi);

    if (!MidiPlayer_OpenFile(&file, path, "wb")) {
        MidiPlayer_SetMessage(player, "save failed");
        return;
    }

    fprintf(file, "version=1\n");
    fprintf(file, "midi=%s\n", player->loaded_midi);
    fprintf(file, "soundfont=%s\n", player->loaded_soundfont);
    fprintf(file, "loop=%s\n", player->loop ? "true" : "false");
    fprintf(file, "volume=%.6g\n", player->volume);

    for (int channel = 0; channel < 16; ++channel) {
        fprintf(
            file,
            "channel%d_muted=%s\n",
            channel,
            R2D_MusicChannelMuted(&player->music, channel) ? "true" : "false"
        );
        fprintf(file, "channel%d_volume=%.6g\n", channel, R2D_MusicChannelVolume(&player->music, channel));
        fprintf(file, "channel%d_program=%d\n", channel, R2D_MusicChannelProgram(&player->music, channel));
    }

    if (fclose(file) == 0) {
        MidiPlayer_SetMessage(player, "song saved");
    } else {
        MidiPlayer_SetMessage(player, "save failed");
    }
}

static void MidiPlayer_LoadSong(MidiPlayer *player)
{
    FILE *file = 0;
    MidiPlayerFile *selected_midi = MidiPlayer_CurrentMidi(player);
    char path[1200];
    char line[256];
    char midi_name[MIDI_PLAYER_NAME_SIZE] = "";
    char soundfont_name[MIDI_PLAYER_NAME_SIZE] = "";
    bool loop = player->loop;
    float volume = player->volume;
    bool channel_muted[16] = { 0 };
    float channel_volume[16];
    int channel_program[16];

    for (int channel = 0; channel < 16; ++channel) {
        channel_volume[channel] = 1.0f;
        channel_program[channel] = -1;
    }

    if (selected_midi == 0) {
        MidiPlayer_SetMessage(player, "no midi files");
        return;
    }

    MidiPlayer_SongPathForMidi(path, sizeof(path), selected_midi->name);

    if (!MidiPlayer_OpenFile(&file, path, "rb")) {
        MidiPlayer_SetMessage(player, "no song config");
        return;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        char *text = MidiPlayer_Trim(line);
        char *separator = strchr(text, '=');
        char *key;
        char *value;
        int channel = -1;
        char field[32];

        if (*text == '\0' || *text == '#' || *text == ';' || separator == 0) {
            continue;
        }

        *separator = '\0';
        key = MidiPlayer_Trim(text);
        value = MidiPlayer_Trim(separator + 1);

        if (MidiPlayer_CompareNames(key, "midi") == 0) {
            MidiPlayer_CopyText(midi_name, sizeof(midi_name), value);
        } else if (MidiPlayer_CompareNames(key, "soundfont") == 0) {
            MidiPlayer_CopyText(soundfont_name, sizeof(soundfont_name), value);
        } else if (MidiPlayer_CompareNames(key, "loop") == 0) {
            loop = MidiPlayer_ParseBool(value);
        } else if (MidiPlayer_CompareNames(key, "volume") == 0) {
            volume = (float)atof(value);
        } else if (sscanf(key, "channel%d_%31s", &channel, field) == 2 && channel >= 0 && channel < 16) {
            if (MidiPlayer_CompareNames(field, "muted") == 0) {
                channel_muted[channel] = MidiPlayer_ParseBool(value);
            } else if (MidiPlayer_CompareNames(field, "volume") == 0) {
                channel_volume[channel] = (float)atof(value);
            } else if (MidiPlayer_CompareNames(field, "program") == 0) {
                channel_program[channel] = atoi(value);
            }
        }
    }

    fclose(file);

    if (midi_name[0] != '\0') {
        const int index = MidiPlayer_FindFile(player->midis, player->midi_count, midi_name);

        if (index >= 0) {
            player->selected_midi = index;
        }
    }

    if (soundfont_name[0] != '\0') {
        const int index = MidiPlayer_FindFile(player->soundfonts, player->soundfont_count, soundfont_name);

        if (index >= 0) {
            player->selected_soundfont = index;
        }
    }

    player->loop = loop;
    player->volume = volume;
    MidiPlayer_PlaySelection(player);

    if (!player->music_loaded) {
        return;
    }

    R2D_MusicSetVolume(&player->music, player->volume);
    R2D_MusicSetLoop(&player->music, player->loop);

    for (int channel = 0; channel < 16; ++channel) {
        if (channel_program[channel] >= 0) {
            R2D_MusicSetChannelProgram(&player->music, channel, channel_program[channel]);
        }

        R2D_MusicSetChannelVolume(&player->music, channel, channel_volume[channel]);
        R2D_MusicSetChannelMuted(&player->music, channel, channel_muted[channel]);
    }

    MidiPlayer_SetMessage(player, "song loaded");
}

static void MidiPlayer_Select(MidiPlayer *player, int direction)
{
    int *selected = player->active_column == 0 ? &player->selected_midi : &player->selected_soundfont;
    const int count = player->active_column == 0 ? player->midi_count : player->soundfont_count;

    if (count <= 0) {
        return;
    }

    *selected = (*selected + direction + count) % count;

    if (player->auto_play) {
        MidiPlayer_PlaySelection(player);
    }
}

static void MidiPlayer_Init(void *user_data)
{
    MidiPlayer *player = (MidiPlayer *)user_data;

    player->active_column = 0;
    player->selected_channel = 0;
    player->loop = true;
    player->volume = 0.65f;
    player->message = "";
    player->message_timer = 0.0f;
    MidiPlayer_DiscoverFiles(player);
}

static void MidiPlayer_Update(float dt, void *user_data)
{
    MidiPlayer *player = (MidiPlayer *)user_data;

    R2D_MusicUpdate(&player->music);

    if (IsKeyPressed(KEY_TAB)) {
        player->channel_view = !player->channel_view;
    }

    if (player->channel_view) {
        const bool fast = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (IsKeyPressed(KEY_UP)) {
            player->selected_channel = (player->selected_channel + 15) % 16;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            player->selected_channel = (player->selected_channel + 1) % 16;
        }

        if (IsKeyPressed(KEY_M) && player->music_loaded) {
            const bool muted = R2D_MusicChannelMuted(&player->music, player->selected_channel);
            R2D_MusicSetChannelMuted(&player->music, player->selected_channel, !muted);
            MidiPlayer_SetMessage(player, muted ? "channel on" : "channel muted");
        }

        if (IsKeyPressed(KEY_LEFT) && player->music_loaded) {
            const float volume = R2D_MusicChannelVolume(&player->music, player->selected_channel);
            R2D_MusicSetChannelVolume(&player->music, player->selected_channel, volume - (fast ? 0.20f : 0.05f));
            MidiPlayer_SetMessage(player, "channel volume");
        }

        if (IsKeyPressed(KEY_RIGHT) && player->music_loaded) {
            const float volume = R2D_MusicChannelVolume(&player->music, player->selected_channel);
            R2D_MusicSetChannelVolume(&player->music, player->selected_channel, volume + (fast ? 0.20f : 0.05f));
            MidiPlayer_SetMessage(player, "channel volume");
        }

        if (IsKeyPressed(KEY_Q) && player->music_loaded) {
            const int program = R2D_MusicChannelProgram(&player->music, player->selected_channel);
            R2D_MusicSetChannelProgram(&player->music, player->selected_channel, program - (fast ? 8 : 1));
            MidiPlayer_SetMessage(player, "channel program");
        }

        if (IsKeyPressed(KEY_E) && player->music_loaded) {
            const int program = R2D_MusicChannelProgram(&player->music, player->selected_channel);
            R2D_MusicSetChannelProgram(&player->music, player->selected_channel, program + (fast ? 8 : 1));
            MidiPlayer_SetMessage(player, "channel program");
        }
    } else {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        player->active_column = 1 - player->active_column;
    }

    if (IsKeyPressed(KEY_UP)) {
        MidiPlayer_Select(player, -1);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        MidiPlayer_Select(player, 1);
    }

    if (IsKeyPressed(KEY_Q)) {
        player->active_column = 0;
        MidiPlayer_Select(player, -1);
    }

    if (IsKeyPressed(KEY_E)) {
        player->active_column = 1;
        MidiPlayer_Select(player, 1);
    }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (player->music_loaded && R2D_MusicIsPlaying(&player->music)) {
            R2D_MusicPause(&player->music);
            MidiPlayer_SetMessage(player, "paused");
        } else if (player->music_loaded && R2D_MusicIsPaused(&player->music)) {
            R2D_MusicResume(&player->music);
            MidiPlayer_SetMessage(player, "resumed");
        } else {
            MidiPlayer_PlaySelection(player);
        }
    }

    if (IsKeyPressed(KEY_S)) {
        MidiPlayer_Stop(player);
        MidiPlayer_SetMessage(player, "stopped");
    }

    if (IsKeyPressed(KEY_L)) {
        player->loop = !player->loop;
        R2D_MusicSetLoop(&player->music, player->loop);
        MidiPlayer_SetMessage(player, player->loop ? "loop on" : "loop off");
    }

    if (IsKeyPressed(KEY_A)) {
        player->auto_play = !player->auto_play;
        MidiPlayer_SetMessage(player, player->auto_play ? "auto on" : "auto off");
    }

    if (IsKeyPressed(KEY_R) && player->music_loaded) {
        R2D_MusicPlay(&player->music, player->loop);
        MidiPlayer_SetMessage(player, "restarted");
    }

    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
        player->volume -= 0.05f;
        if (player->volume < 0.0f) {
            player->volume = 0.0f;
        }
        R2D_MusicSetVolume(&player->music, player->volume);
        MidiPlayer_SetMessage(player, "volume");
    }

    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
        player->volume += 0.05f;
        if (player->volume > 1.0f) {
            player->volume = 1.0f;
        }
        R2D_MusicSetVolume(&player->music, player->volume);
        MidiPlayer_SetMessage(player, "volume");
    }

    if (IsKeyPressed(KEY_C) && player->crt != 0) {
        R2D_CrtSetEnabled(player->crt, !player->crt->enabled);
    }

    if (IsKeyPressed(KEY_F5)) {
        MidiPlayer_Stop(player);
        MidiPlayer_DiscoverFiles(player);
        player->last_error[0] = '\0';
        MidiPlayer_SetMessage(player, "refreshed");
    }

    if (IsKeyPressed(KEY_F6)) {
        MidiPlayer_SaveSong(player);
    }

    if (IsKeyPressed(KEY_F7)) {
        MidiPlayer_LoadSong(player);
    }

    if (player->message_timer > 0.0f) {
        player->message_timer -= dt;
    }
}

static void MidiPlayer_DrawChannels(const MidiPlayer *player)
{
    const Rectangle area = { 8.0f, 64.0f, 304.0f, 94.0f };

    DrawRectangleLinesEx(area, 1.0f, R2D_ColorFromHex(0x303040ff));

    for (int channel = 0; channel < 16; ++channel) {
        const int column = channel / 8;
        const int row = channel % 8;
        const int x = 12 + column * 152;
        const int y = 69 + row * 10;
        const bool selected = channel == player->selected_channel;
        const bool used = R2D_MusicChannelUsed(&player->music, channel);
        const bool muted = R2D_MusicChannelMuted(&player->music, channel);
        const int program = R2D_MusicChannelProgram(&player->music, channel);
        const int volume = (int)(R2D_MusicChannelVolume(&player->music, channel) * 100.0f + 0.5f);
        const float activity = R2D_MusicChannelActivity(&player->music, channel);
        const Color color = selected ? R2D_ColorFromHex(0xf1fa8cff) : (used ? R2D_ColorFromHex(0xd7d7e0ff) : R2D_ColorFromHex(0x6272a4ff));
        const Color bar_color = muted ? R2D_ColorFromHex(0xff5555ff) : R2D_ColorFromHex(0x50fa7bff);

        DrawText(selected ? ">" : " ", x, y, 8, color);
        DrawText(TextFormat("%02d", channel + 1), x + 10, y, 8, color);
        DrawText(channel == 9 ? "dr" : "ch", x + 28, y, 8, color);
        DrawText(muted ? "off" : "on ", x + 44, y, 8, muted ? R2D_ColorFromHex(0xff5555ff) : color);
        DrawText(TextFormat("p%03d", program), x + 66, y, 8, color);
        DrawText(TextFormat("%03d", volume), x + 96, y, 8, color);
        DrawRectangle((int)x + 124, y + 3, 16, 3, R2D_ColorFromHex(0x303040ff));
        DrawRectangle((int)x + 124, y + 3, (int)(16.0f * activity), 3, bar_color);
    }
}

static void MidiPlayer_DrawList(
    const MidiPlayerFile *files,
    int count,
    int selected,
    Rectangle area,
    const char *empty_text,
    bool active
)
{
    const int visible_rows = (int)(area.height / 10.0f);
    int start = selected - visible_rows / 2;

    if (start < 0) {
        start = 0;
    }

    if (start > count - visible_rows) {
        start = count - visible_rows;
    }

    if (start < 0) {
        start = 0;
    }

    DrawRectangleLinesEx(area, 1.0f, active ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0x303040ff));

    if (count <= 0) {
        DrawText(empty_text, (int)area.x + 6, (int)area.y + 8, 8, R2D_ColorFromHex(0x6272a4ff));
        return;
    }

    for (int row = 0; row < visible_rows && start + row < count; ++row) {
        const int index = start + row;
        const int y = (int)area.y + 5 + row * 10;
        const Color color = index == selected ? R2D_ColorFromHex(0xf1fa8cff) : R2D_ColorFromHex(0xd7d7e0ff);

        DrawText(index == selected ? ">" : " ", (int)area.x + 4, y, 8, color);
        DrawText(
            MidiPlayer_FitText(files[index].name, 8, (int)area.width - 18),
            (int)area.x + 14,
            y,
            8,
            color
        );
    }
}

static void MidiPlayer_Draw(void *user_data)
{
    MidiPlayer *player = (MidiPlayer *)user_data;
    const MidiPlayerFile *midi = MidiPlayer_CurrentMidi(player);
    const MidiPlayerFile *soundfont = MidiPlayer_CurrentSoundFont(player);
    const float position = R2D_MusicPosition(&player->music);
    const float length = R2D_MusicLength(&player->music);
    const float progress = length > 0.0f ? position / length : 0.0f;
    const Rectangle progress_track = { 8.0f, 160.0f, 304.0f, 4.0f };
    const char *status = R2D_MusicIsPlaying(&player->music)
        ? "playing"
        : (R2D_MusicIsPaused(&player->music) ? "paused" : "stopped");

    DrawText("Retro2D MIDI Player", 8, 8, 10, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText(player->channel_view ? "TAB files  UP/DOWN channel  LEFT/RIGHT vol  Q/E program" : "TAB channels  LEFT/RIGHT column  UP/DOWN select  SPACE play/pause", 8, 23, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText(TextFormat("L loop:%s  A auto:%s  F6 save  F7 load", player->loop ? "on" : "off", player->auto_play ? "on" : "off"), 8, 35, 8, R2D_ColorFromHex(0xffb86cff));

    if (player->channel_view) {
        DrawText("CHANNELS", 8, 52, 8, R2D_ColorFromHex(0x8be9fdff));
        DrawText("M mute  SHIFT faster edits", 138, 52, 8, R2D_ColorFromHex(0xffb86cff));
        MidiPlayer_DrawChannels(player);
    } else {
        DrawText(TextFormat("MIDIS %02d", player->midi_count), 8, 52, 8, R2D_ColorFromHex(0x8be9fdff));
        DrawText(TextFormat("SOUNDFONTS %02d", player->soundfont_count), 166, 52, 8, R2D_ColorFromHex(0xffb86cff));

    MidiPlayer_DrawList(
        player->midis,
        player->midi_count,
        player->selected_midi,
        (Rectangle) { 8.0f, 64.0f, 146.0f, 94.0f },
        "add .mid files",
        player->active_column == 0
    );

    MidiPlayer_DrawList(
        player->soundfonts,
        player->soundfont_count,
        player->selected_soundfont,
        (Rectangle) { 166.0f, 64.0f, 146.0f, 94.0f },
        "add .sf2 files",
        player->active_column == 1
    );
    }

    DrawRectangleRec(progress_track, R2D_ColorFromHex(0x303040ff));
    DrawRectangleRec(
        (Rectangle) {
            progress_track.x,
            progress_track.y,
            progress_track.width * (progress < 0.0f ? 0.0f : (progress > 1.0f ? 1.0f : progress)),
            progress_track.height
        },
        R2D_ColorFromHex(0x50fa7bff)
    );

    DrawText(TextFormat("status: %s  volume:%03d%%", status, (int)(player->volume * 100.0f + 0.5f)), 8, 168, 8, R2D_ColorFromHex(0x50fa7bff));
    DrawText(TextFormat("%s / %s", MidiPlayer_FormatTime(position), MidiPlayer_FormatTime(length)), 236, 168, 8, R2D_ColorFromHex(0xf1fa8cff));

    if (midi != 0) {
        DrawText(
            MidiPlayer_FitText(TextFormat("midi: assets/%s", midi->path), 7, 304),
            8,
            180,
            7,
            R2D_ColorFromHex(0x6272a4ff)
        );
    }

    if (soundfont != 0) {
        DrawText(
            MidiPlayer_FitText(TextFormat("sf2:  assets/%s", soundfont->path), 7, 304),
            8,
            190,
            7,
            R2D_ColorFromHex(0x6272a4ff)
        );
    }

    if (player->music_loaded) {
        DrawText(
            MidiPlayer_FitText(TextFormat("now: %s / %s", player->loaded_midi, player->loaded_soundfont), 7, 304),
            8,
            158,
            7,
            R2D_ColorFromHex(0xf1fa8cff)
        );
    } else if (player->last_error[0] != '\0') {
        DrawText(
            MidiPlayer_FitText(TextFormat("error: %s", player->last_error), 7, 304),
            8,
            158,
            7,
            R2D_ColorFromHex(0xff5555ff)
        );
    }

    if (player->message_timer > 0.0f) {
        DrawText(player->message, 236, 168, 8, R2D_ColorFromHex(0xf1fa8cff));
    }
}

static void MidiPlayer_Shutdown(void *user_data)
{
    MidiPlayer *player = (MidiPlayer *)user_data;

    MidiPlayer_Stop(player);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    MidiPlayer player = { 0 };

    config.title = "Retro2DFramework MIDI Player";
    config.clear_color = R2D_ColorFromHex(0x15151fff);
    player.context = &context;
    player.crt = &crt;

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);

    R2D_Run(&context, (R2D_App) {
        MidiPlayer_Init,
        MidiPlayer_Update,
        MidiPlayer_Draw,
        MidiPlayer_Shutdown,
        &player
    });

    R2D_CrtClose(&crt);
    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
