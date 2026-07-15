#pragma once
#include <QString>

namespace Theme {

inline const char* SURFACE       = "#FFFFFF";
inline const char* SURFACE_ALT   = "#FAF5FF";
inline const char* BORDER        = "#E9D5FF";
inline const char* TEXT_PRIMARY  = "#1F1233";
inline const char* TEXT_SECONDARY= "#9333EA";
inline const char* NAV_TEXT      = "#7E22CE";
inline const char* ACCENT        = "#A855F7";
inline const char* ACCENT2       = "#EC4899";
inline const char* DANGER        = "#DC2626";

inline QString primaryButton() {
    return QString(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2);"
        "  color: white; border: none;"
        "  border-radius: 8px; padding: 10px 20px;"
        "  font-size: 13px; font-weight: 500;"
        "}"
        "QPushButton:hover { background: %1; }"
    ).arg(ACCENT, ACCENT2);
}

inline QString secondaryButton() {
    return QString(
        "QPushButton {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 8px; padding: 10px 20px;"
        "  font-size: 13px; font-weight: 500;"
        "}"
        "QPushButton:hover { background: %4; }"
    ).arg(SURFACE, TEXT_PRIMARY, BORDER, SURFACE_ALT);
}

inline QString textInput() {
    return QString(
        "QLineEdit {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 8px; padding: 8px 12px; font-size: 13px;"
        "}"
        "QLineEdit:focus { border: 1px solid %4; }"
    ).arg(SURFACE_ALT, TEXT_PRIMARY, BORDER, ACCENT);
}

inline QString textArea() {
    return QString(
        "QTextEdit {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 8px; padding: 8px 12px; font-size: 13px;"
        "}"
        "QTextEdit:focus { border: 1px solid %4; }"
    ).arg(SURFACE_ALT, TEXT_PRIMARY, BORDER, ACCENT);
}

inline QString card() {
    return QString(
        "background: %1; border: 1px solid %2; border-radius: 12px;"
    ).arg(SURFACE, BORDER);
}

inline QString pageBackground() {
    return QString("background: %1;").arg(SURFACE_ALT);
}

inline QString heading() {
    return QString(
        "color: %1; font-size: 20px; font-weight: 500;"
    ).arg(TEXT_PRIMARY);
}

inline QString bodyText() {
    return QString(
        "color: %1; font-size: 13px;"
    ).arg(TEXT_PRIMARY);
}

inline QString mutedText() {
    return QString(
        "color: %1; font-size: 12px;"
    ).arg(TEXT_SECONDARY);
}

} // namespace Theme
