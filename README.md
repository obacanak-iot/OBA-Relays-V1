# OBA Relays V1

ESP8266 ESP-01 tabanlı iki kanallı röle kontrol projesi.

Bu proje iki kullanım yolu sunar:

- `src/main.cpp`: PlatformIO / Arduino firmware.
- `devices/oba-relays-v1.yaml`: ESPHome firmware tanımı.

## PlatformIO Firmware

PlatformIO firmware şu özellikleri sağlar:

- WiFiManager kurulum portalı
- MQTT ayarları için portal alanları
- Home Assistant MQTT Discovery
- Role1 ve Role2 switch entity'leri
- UART relay komutlari
- Arduino OTA
- Firmware version/update bildirimi

VS Code içinde şu workspace dosyasını açabilirsiniz:

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

ESPHome YAML WiFi bilgilerini `devices/secrets.yaml` dosyasından okur.

Örnek dosyadan başlayın:

```bash
cp devices/secrets.example.yaml devices/secrets.yaml
```

Sonra `devices/secrets.yaml` içindeki WiFi bilgilerini kendi ağınıza göre doldurun.

## GitHub'a Gönderilmeyecek Dosyalar

Şu dosyalar ve klasörler `.gitignore` içindedir:

- `devices/secrets.yaml`
- `.pio/`
- `.esphome/`
- `artifacts/`

Bu sayede yerel WiFi bilgileri, build çıktıları ve geçici firmware dosyaları GitHub'a gitmez.

## Sürüm

Sürüm şu dosyalarda aynı tutulur:

- `VERSION`
- `platformio.ini` içindeki `FIRMWARE_VERSION`
- `firmware/version.json`
- `oba-relays-v1/version.json`

PlatformIO build başında `scripts/check_versions.py` bu sürümleri kontrol eder.

## GitHub Actions

İki güvenli CI kontrolü vardır:

- `PlatformIO Build`: Arduino firmware derlenebilir mi kontrol eder.
- `ESPHome Compile Check`: ESPHome YAML derlenebilir mi kontrol eder.

Bu workflow'lar firmware yayınlamaz. Dummy WiFi ile üretilen firmware public OTA olarak paylaşılmaz.

