# OBA Relays V1 PlatformIO Firmware

Bu klasör OBA Relays V1 için alternatif PlatformIO / Arduino firmware yolunu ekler.

ESPHome YAML korunur. PlatformIO firmware şu özellikleri sağlar:

- ESP8266 ESP-01 / `esp01_1m`
- WiFiManager kurulum portalı
- MQTT ayarları için portal alanları
- Home Assistant MQTT Discovery
- Role1 ve Role2 relay switch entity'leri
- UART relay komutlari
- Arduino OTA
- Firmware version ve update entity bildirimi

## PlatformIO Komutları

Build:

```bash
pio run -e esp01_1m
```

Upload:

```bash
pio run -e esp01_1m -t upload
```

Seri monitör:

```bash
pio device monitor -b 115200
```

## İlk Kurulum

Cihaz ilk açılışta WiFi bulamazsa `OBA-RELAYS-ESP01-V1-XXXXXXXXXXXX` formatında kurulum ağı açar.

Portal içinden:

- WiFi bilgisi
- MQTT sunucu
- MQTT port
- MQTT kullanıcı
- MQTT şifre
- Opsiyonel cihaz adı

girilir.

## Version Güncelleme

Sürüm güncellerken şu alanlar aynı tutulmalıdır:

- `VERSION`
- `platformio.ini` içindeki `FIRMWARE_VERSION`
- `firmware/version.json` içindeki `version`
- `oba-relays-v1/version.json` içindeki `version`

PlatformIO build başlarken `scripts/check_versions.py` bu alanların aynı olduğunu kontrol eder.

## ESPHome Secrets

ESPHome YAML WiFi bilgisini `devices/secrets.yaml` dosyasından okur. Bu dosya GitHub'a gönderilmez.

Örnek dosya:

```bash
cp devices/secrets.example.yaml devices/secrets.yaml
```

Sonra `devices/secrets.yaml` içindeki WiFi bilgilerini yerel ağ bilgilerine göre doldurun.
