#include <ui/settings/cloud.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <flow/device_pairing_flow.h>
#include <flow/device_sync_flow.h>
#include <lib/tea_qrcode.h>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

#include <cstdio>
#include <cstring>

namespace {
unsigned long cloudLastDrawMs = 0;
constexpr unsigned long CLOUD_DRAW_INTERVAL_MS = 500;

const char *pairingBadge(const DevicePairingSnapshot &pairing,
                         const DeviceSyncSnapshot &sync) {
  if (!pairing.paired) {
    if (pairing.state == DevicePairingFlowState::Pending)
      return "CODE";
    if (pairing.state == DevicePairingFlowState::Failed ||
        pairing.state == DevicePairingFlowState::Expired ||
        pairing.state == DevicePairingFlowState::Consumed)
      return "FAIL";
    return "PAIR";
  }

  if (sync.state == DeviceSyncFlowState::Failed)
    return "FAIL";
  if (sync.state == DeviceSyncFlowState::Syncing)
    return "SYNC";
  return "OK";
}

const char *pairingStateText(DevicePairingFlowState state, bool paired) {
  if (paired)
    return "PAIRED";

  switch (state) {
  case DevicePairingFlowState::Creating:
    return "CREATING";
  case DevicePairingFlowState::Pending:
    return "PENDING";
  case DevicePairingFlowState::Expired:
    return "EXPIRED";
  case DevicePairingFlowState::Consumed:
    return "CONSUMED";
  case DevicePairingFlowState::Failed:
    return "FAILED";
  case DevicePairingFlowState::Authorized:
    return "PAIRED";
  case DevicePairingFlowState::Idle:
  default:
    return "UNPAIRED";
  }
}

const char *syncStatusText(SessionJournalSyncStatus status) {
  switch (status) {
  case SessionJournalSyncStatus::Synced:
    return "synced";
  case SessionJournalSyncStatus::Failed:
    return "failed";
  case SessionJournalSyncStatus::Pending:
  default:
    return "pending";
  }
}

const char *tail(const char *value, size_t maxLen) {
  if (!value)
    return "";

  size_t len = std::strlen(value);
  if (len <= maxLen)
    return value;
  return value + len - maxLen;
}

bool drawQrCode(const char *text) {
  if (!text || text[0] == '\0')
    return false;

  TeaQRCode qrCode;
  uint8_t qrData[tea_qrcode_getBufferSize(6)];
  if (tea_qrcode_initText(&qrCode, qrData, 6, TEA_QR_ECC_LOW, text) != 0)
    return false;

  const int size = qrCode.size;
  const int quietZone = 2;
  const int qrX = 0;
  const int qrY = ui::layout::UI_HEADER_LINE_Y + 1;

  display.fillRect(qrX, qrY, size + quietZone * 2, size + quietZone * 2,
                   SSD1306_WHITE);

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (tea_qrcode_getModule(&qrCode, x, y)) {
        display.drawPixel(qrX + quietZone + x, qrY + quietZone + y,
                          SSD1306_BLACK);
      }
    }
  }

  return true;
}

void drawCloudScreen() {
  DevicePairingSnapshot pairing;
  DeviceSyncSnapshot sync;
  devicePairingSnapshot(pairing);
  deviceSyncSnapshot(sync);

  display.clearDisplay();
  drawHeader("CLOUD", pairingBadge(pairing, sync));
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (!pairing.paired && pairing.state == DevicePairingFlowState::Pending &&
      pairing.verificationUri[0] != '\0' && drawQrCode(pairing.verificationUri)) {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(56, ui::layout::INFO_ROW1_Y);
    display.print("Scan");

    display.setCursor(56,
                      ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
    display.print(pairing.userCode);

    display.setCursor(56, ui::layout::INFO_ROW1_Y +
                              ui::layout::INFO_ROW_STEP_Y * 2);
    display.print("Sel:cancel");

    display.display();
    return;
  }

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print("UID: ");
  display.print(tail(pairing.deviceUid, 15));

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  display.print("State: ");
  display.print(pairingStateText(pairing.state, pairing.paired));

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  if (pairing.state == DevicePairingFlowState::Pending &&
      pairing.userCode[0] != '\0') {
    display.print("Code: ");
    display.print(pairing.userCode);
  } else if (pairing.paired) {
    display.print("Dev: ");
    display.print(tail(pairing.deviceId, 16));
  } else {
    display.print("API: ");
    display.print("ready");
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  if (pairing.paired) {
    char syncBuf[24];
    std::snprintf(syncBuf, sizeof(syncBuf), "%s %d rec",
                  syncStatusText(sync.journalStatus), sync.recordCount);
    display.print("Sync: ");
    display.print(syncBuf);
  } else {
    display.print("Msg: ");
    display.print(pairing.message[0] ? pairing.message : "-");
  }

  display.setCursor(0, 56);
  if (pairing.paired) {
    display.print("Sel:sync Hold:unpair");
  } else if (pairing.state == DevicePairingFlowState::Pending ||
             pairing.state == DevicePairingFlowState::Creating) {
    display.print("Sel: cancel");
  } else {
    display.print("Sel: pair");
  }

  if (app.cloud.unpairConfirm.active) {
    drawConfirmOverlay("Unpair device?", app.cloud.unpairConfirm);
  }

  display.display();
}
} // namespace

void drawCloud() {
  cloudLastDrawMs = 0;
  drawCloudScreen();
}

void updateCloudScreen() {
  unsigned long now = millis();
  if (now - cloudLastDrawMs < CLOUD_DRAW_INTERVAL_MS)
    return;

  drawCloudScreen();
  cloudLastDrawMs = now;
}
