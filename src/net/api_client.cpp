#include <net/api_client.h>

#include <HTTPClient.h>
#include <app/app_config.h>

namespace {
String buildUrl(const char *path) {
  String base(appcfg::API_BASE_URL);
  while (base.endsWith("/")) {
    base.remove(base.length() - 1);
  }

  String normalizedPath(path ? path : "");
  if (!normalizedPath.startsWith("/")) {
    normalizedPath = "/" + normalizedPath;
  }

  return base + normalizedPath;
}

void addBearerHeader(HTTPClient &http, const char *bearerToken) {
  if (!bearerToken || bearerToken[0] == '\0')
    return;

  String value("Bearer ");
  value += bearerToken;
  http.addHeader("Authorization", value);
}

ApiResponse finishRequest(HTTPClient &http, int statusCode) {
  ApiResponse response;
  response.statusCode = statusCode;

  if (statusCode <= 0) {
    response.error = http.errorToString(statusCode);
  } else {
    response.body = http.getString();
  }

  http.end();
  return response;
}
} // namespace

ApiResponse apiClientGet(const char *path, const char *bearerToken) {
  HTTPClient http;
  http.setTimeout(appcfg::CLOUD_HTTP_TIMEOUT_MS);

  if (!http.begin(buildUrl(path))) {
    return {.statusCode = 0, .body = "", .error = "HTTP begin failed"};
  }

  addBearerHeader(http, bearerToken);
  return finishRequest(http, http.GET());
}

ApiResponse apiClientPostJson(const char *path, const String &body,
                              const char *bearerToken) {
  HTTPClient http;
  http.setTimeout(appcfg::CLOUD_HTTP_TIMEOUT_MS);

  if (!http.begin(buildUrl(path))) {
    return {.statusCode = 0, .body = "", .error = "HTTP begin failed"};
  }

  http.addHeader("Content-Type", "application/json");
  addBearerHeader(http, bearerToken);
  return finishRequest(http, http.POST(body));
}
