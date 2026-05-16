#pragma once

#include <Arduino.h>

struct ApiResponse {
  int statusCode = 0;
  String body;
  String error;

  bool ok() const { return statusCode >= 200 && statusCode < 300; }
};

ApiResponse apiClientGet(const char *path, const char *bearerToken = nullptr);
ApiResponse apiClientPostJson(const char *path, const String &body,
                              const char *bearerToken = nullptr);
