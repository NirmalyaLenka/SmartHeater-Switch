/*
 * config.example.h
 * ─────────────────────────────────────────────────────────────
 *  Copy this file to config.h and fill in your real values.
 *  config.h is listed in .gitignore — this example file is safe
 *  to commit and acts as documentation for collaborators.
 * ─────────────────────────────────────────────────────────────
 */

#pragma once

#define WIFI_SSID               "YourWiFiName"
#define WIFI_PASS               "YourWiFiPassword"

#define SINRICPRO_APP_KEY       "APP_KEY_FROM_SINRIC_DASHBOARD"
#define SINRICPRO_APP_SECRET    "APP_SECRET_FROM_SINRIC_DASHBOARD"
#define SINRICPRO_SWITCH_ID     "DEVICE_ID_FROM_SINRIC_DASHBOARD"

// UTC offset in seconds. India = 19800, UK = 0, US EST = -18000
#define NTP_UTC_OFFSET_SEC      19800L
