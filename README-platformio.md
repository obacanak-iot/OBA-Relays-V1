# OBA Relays V1 PlatformIO Firmware

Bu klasor OBA Relays V1 icin alternatif PlatformIO / Arduino firmware yolunu ekler.

ESPHome YAML korunur. PlatformIO firmware su ozellikleri saglar:

- ESP8266 ESP-01 / `esp01_1m`
- WiFiManager kurulum portali
- MQTT ayarlari icin portal alanlari
- Home Assistant MQTT Discovery
- Role1 ve Role2 relay switch entity'leri
- UART relay komutlari
- Arduino OTA
- Firmware version ve update entity bildirimi

## PlatformIO Komutlari

Build:

```bash
pio run -e esp01_1m
```

Upload:

```bash
pio run -e esp01_1m -t upload
```

Seri monitor:

```bash
pio device monitor -b 115200
```

## Ilk Kurulum

Cihaz ilk acilista WiFi bulamazsa `OBA-RELAYS-ESP01-V1-XXXXXXXXXXXX` formatinda kurulum agi acar.

Portal icinden:

- WiFi bilgisi
- MQTT sunucu
- MQTT port
- MQTT kullanici
- MQTT sifre
- Opsiyonel cihaz adi

girilir.

## Version Guncelleme

Surum guncellerken su uc yer ayni tutulmalidir:

- `VERSION`
- `platformio.ini` icindeki `FIRMWARE_VERSION`
- `firmware/version.json` icindeki `version`

PlatformIO build baslarken `scripts/check_versions.py` bu uc alanin ayni oldugunu kontrol eder.

## ESPHome Secrets

ESPHome YAML WiFi bilgisini `devices/secrets.yaml` dosyasindan okur. Bu dosya GitHub'a gonderilmez.

Ornek dosya:

```bash
cp devices/secrets.example.yaml devices/secrets.yaml
```

Sonra `devices/secrets.yaml` icindeki WiFi bilgilerini yerel ag bilgilerine gore doldurun.
