# PlatformIO Manuel Derleme

Bu klasor artik PlatformIO projesi olarak acilabilir.

## VS Code ile Acma

1. VS Code'u acin.
2. `File > Open Workspace from File...` secin.
3. Su dosyayi acin:

```text
E:\Home Assistant\GITHUB\OBA-Relays-V1\OBA-Relays-V1.code-workspace
```

Alternatif olarak direkt klasoru acabilirsiniz:

```text
E:\Home Assistant\GITHUB\OBA-Relays-V1
```

## PlatformIO Eklentisi

VS Code icinde PlatformIO eklentisi kurulu degilse:

1. Extensions bolumunu acin.
2. `PlatformIO IDE` arayin.
3. Kurun.
4. VS Code'u yeniden baslatin.

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

ESP-01 programlama adaptorune bagliyken:

```text
PROJECT TASKS > esp01_1m > General > Upload
```

veya:

```bash
pio run -e esp01_1m -t upload
```

## Seri Monitor

```text
PROJECT TASKS > esp01_1m > Platform > Monitor
```

veya:

```bash
pio device monitor -b 115200
```

## Ilk Acilis

Cihaz WiFi bulamazsa kurulum agi acar:

```text
OBA-RELAYS-ESP01-V1-XXXXXXXXXXXX
```

Portalda WiFi ve MQTT bilgilerini girin. Home Assistant MQTT Discovery aktifse Role1 ve Role2 switch entity olarak gorunur.

## Not

Build baslamadan once `scripts/check_versions.py` otomatik calisir. Su uc surum ayni degilse derleme durur:

- `VERSION`
- `platformio.ini` icindeki `FIRMWARE_VERSION`
- `firmware/version.json`

