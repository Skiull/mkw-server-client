#include "Settings.hh"

#include "sp/storage/Storage.hh"

#include <cstdio>
#include <cstring>

namespace Settings {

#define SETTINGS_DIR L"/mkw-selfhost"
#define SETTINGS_PATH SETTINGS_DIR L"/settings.ini"

// Values as last loaded or saved, to skip writing when nothing changed.
static u32 savedValues[settingCount];
static bool savedValid = false;

static bool Equals(std::string_view a, const char *b, size_t bLen) {
    return a.size() == bLen && !memcmp(a.data(), b, bLen);
}

static void ParseLine(const char *line, size_t len) {
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == ' ' || line[len - 1] == '\t')) {
        len--;
    }
    while (len > 0 && (*line == ' ' || *line == '\t')) {
        line++;
        len--;
    }
    if (len == 0 || *line == '#' || *line == ';' || *line == '[') {
        return;
    }
    const char *eq = static_cast<const char *>(memchr(line, '=', len));
    if (!eq) {
        return;
    }
    size_t keyLen = eq - line;
    while (keyLen > 0 && (line[keyLen - 1] == ' ' || line[keyLen - 1] == '\t')) {
        keyLen--;
    }
    const char *value = eq + 1;
    size_t valueLen = len - (value - line);
    while (valueLen > 0 && (*value == ' ' || *value == '\t')) {
        value++;
        valueLen--;
    }
    if (valueLen == 0) {
        return;
    }

    for (u32 i = 0; i < settingCount; i++) {
        SettingEntry &entry = settings[i];
        if (!Equals(entry.name, line, keyLen)) {
            continue;
        }
        if (entry.valueNames) {
            for (u32 j = 0; j < entry.valueCount; j++) {
                if (Equals(entry.valueNames[j], value, valueLen)) {
                    entry.selectedValue = entry.valueOffset + j;
                    return;
                }
            }
        } else {
            u32 n = 0;
            for (size_t k = 0; k < valueLen; k++) {
                if (value[k] < '0' || value[k] > '9' || k >= 6) {
                    return;
                }
                n = n * 10 + (value[k] - '0');
            }
            if (n >= entry.valueOffset && n < entry.valueOffset + entry.valueCount) {
                entry.selectedValue = n;
            }
        }
        return;
    }
}

static void Snapshot() {
    for (u32 i = 0; i < settingCount; i++) {
        savedValues[i] = settings[i].selectedValue;
    }
    savedValid = true;
}

void Load() {
    if (!SP::Storage::HasWritable()) {
        return;
    }
    static char buffer[4096];
    auto size = SP::Storage::ReadFile(SETTINGS_PATH, buffer, sizeof(buffer) - 1);
    if (!size) {
        return;
    }
    buffer[*size] = '\0';
    const char *p = buffer;
    const char *end = buffer + *size;
    while (p < end) {
        const char *nl = static_cast<const char *>(memchr(p, '\n', end - p));
        const char *lineEnd = nl ? nl : end;
        ParseLine(p, lineEnd - p);
        p = lineEnd + 1;
    }
    Snapshot();
}

void Save() {
    if (!SP::Storage::HasWritable()) {
        return;
    }
    if (savedValid) {
        bool changed = false;
        for (u32 i = 0; i < settingCount; i++) {
            changed |= savedValues[i] != settings[i].selectedValue;
        }
        if (!changed) {
            return;
        }
    }

    static char buffer[4096];
    size_t len = 0;
    len += snprintf(buffer + len, sizeof(buffer) - len, "[mkw-selfhost settings]\n");
    for (u32 i = 0; i < settingCount && len < sizeof(buffer); i++) {
        const SettingEntry &entry = settings[i];
        u32 value = entry.selectedValue;
        if (entry.valueNames) {
            u32 j = value - entry.valueOffset;
            if (j >= entry.valueCount) {
                continue;
            }
            len += snprintf(buffer + len, sizeof(buffer) - len, "%.*s=%.*s\n",
                    static_cast<int>(entry.name.size()), entry.name.data(),
                    static_cast<int>(entry.valueNames[j].size()), entry.valueNames[j].data());
        } else {
            len += snprintf(buffer + len, sizeof(buffer) - len, "%.*s=%u\n",
                    static_cast<int>(entry.name.size()), entry.name.data(), value);
        }
    }
    if (len >= sizeof(buffer)) {
        return;
    }

    SP::Storage::CreateDir(SETTINGS_DIR, true);
    if (SP::Storage::WriteFile(SETTINGS_PATH, buffer, len, true)) {
        Snapshot();
    }
}

} // namespace Settings
