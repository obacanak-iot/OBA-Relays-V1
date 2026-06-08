# PlatformIO Manuel Derleme

Bu klasör artık PlatformIO projesi olarak açılabilir.

## VS Code ile Açma

1. VS Code'u açın.
2. `Dosya > Çalışma Alanını Dosyadan Aç...` seçin.
3. Şu dosyayı açın:

```text
E:\Home Assistant\GITHUB\OBA-Relays-V1-GitHub\OBA-Relays-V1.code-workspace
```

Alternatif olarak direkt klasörü açabilirsiniz:

```text
E:\Home Assistant\GITHUB\OBA-Relays-V1-GitHub
```

## PlatformIO Eklentisi

VS Code içinde PlatformIO eklentisi kurulu değilse:

1. `Uzantılar` bölümünü açın.
2. `PlatformIO IDE` arayın.
3. Kurun.
4. VS Code'u yeniden başlatın.

## Derleme

PlatformIO sol menuden:

```text
PROJECT TASKS > esp01_1m > General > Build
```

veya terminalden:

```bash
pio run -e esp01_1m
```

## Upload

ESP-01 programlama adaptörüne bağlıyken:

```text
PROJECT TASKS > esp01_1m > General > Upload
```

veya:

```bash
pio run -e esp01_1m -t upload
```

## Seri Monitör

```text
PROJECT TASKS > esp01_1m > Platform > Monitor
```

veya:

```bash
pio device monitor -b 115200
```

## İlk Açılış

Cihaz WiFi bulamazsa kurulum ağı açar:

```text
OBA-RELAYS-ESP01-V1-XXXXXXXXXXXX
```

Portalda WiFi ve MQTT bilgilerini girin. Home Assistant MQTT Discovery aktifse Role1 ve Role2 switch entity olarak görünür.

## Not

Build başlamadan önce `scripts/check_versions.py` otomatik çalışır. Şu sürümler aynı değilse derleme durur:

- `VERSION`
- `platformio.ini` içindeki `FIRMWARE_VERSION`
- `firmware/version.json`
- `oba-relays-v1/version.json`

