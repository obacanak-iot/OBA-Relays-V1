# OBA Relays V1

ESP8266 ESP-01 tabanli iki kanalli relay kontrol projesi.

Bu proje iki kullanim yolu sunar:

- `src/main.cpp`: PlatformIO / Arduino firmware.
- `devices/oba-relays-v1.yaml`: ESPHome firmware tanimi.

## PlatformIO Firmware

PlatformIO firmware su ozellikleri saglar:

- WiFiManager kurulum portali
- MQTT ayarlari icin portal alanlari
- Home Assistant MQTT Discovery
- Role1 ve Role2 switch entity'leri
- UART relay komutlari
- Arduino OTA
- Firmware version/update bildirimi

VS Code icinde su workspace dosyasini acabilirsiniz:

```text
OBA-Relays-V1.code-workspace
```

Derleme:

```bash
pio run -e esp01_1m
```

Upload:

```bash
pio run -e esp01_1m -t upload
```

## ESPHome Firmware

ESPHome YAML WiFi bilgilerini `devices/secrets.yaml` dosyasindan okur.

Ornek dosyadan baslayin:

```bash
cp devices/secrets.example.yaml devices/secrets.yaml
```

Sonra `devices/secrets.yaml` icindeki WiFi bilgilerini kendi aginiza gore doldurun.

## GitHub'a Gonderilmeyecek Dosyalar

Su dosyalar ve klasorler `.gitignore` icindedir:

- `devices/secrets.yaml`
- `.pio/`
- `.esphome/`
- `artifacts/`

Bu sayede yerel WiFi bilgileri, build ciktilari ve gecici firmware dosyalari GitHub'a gitmez.

## Surum

Surum su dosyalarda ayni tutulur:

- `VERSION`
- `platformio.ini` icindeki `FIRMWARE_VERSION`
- `firmware/version.json`
- `oba-relays-v1/version.json`

PlatformIO build basinda `scripts/check_versions.py` bu surumleri kontrol eder.

## GitHub Actions

Iki guvenli CI kontrolu vardir:

- `PlatformIO Build`: Arduino firmware derlenebilir mi kontrol eder.
- `ESPHome Compile Check`: ESPHome YAML derlenebilir mi kontrol eder.

Bu workflow'lar firmware yayinlamaz. Dummy WiFi ile uretilen firmware public OTA olarak paylasilmaz.

