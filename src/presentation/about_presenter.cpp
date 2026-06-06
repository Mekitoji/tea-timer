#include <presentation/about_presenter.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <storage/session_journal_store.h>
#include <ui/settings/about.h>

void aboutRender(unsigned long freeHeap) {
  size_t storageTotalBytes = 0;
  size_t storageUsedBytes = 0;

  AboutView view;
  view.storageAvailable =
      sessionJournalStoreSpace(storageTotalBytes, storageUsedBytes);
  if (view.storageAvailable && storageTotalBytes >= storageUsedBytes) {
    view.storageFreeKb = (storageTotalBytes - storageUsedBytes) / 1024;
  }
  view.freeHeap = freeHeap;
  view.uptimeSeconds = millis() / 1000UL;
  view.firmwareVersion = appcfg::FIRMWARE_VERSION;
  drawAbout(view);
}
