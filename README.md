# Schneggr 0.1 – Windows-Desktop-Pet (Prototyp)

Ein kleiner, nativer Qt-6-Prototyp für Windows 11 mit weicher, vektorgezeichneter Cartoon-Schnecke. Er kann kriechen, fressen, schlafen, gestreichelt werden und mit einem Ball spielen. Rechtsklick öffnet das Menü. Das Tray-Symbol erlaubt Einblenden und Beenden.

## Voraussetzungen
- Windows 11
- Qt 6 (Widgets und Svg) und CMake 3.21+
- Visual Studio 2022 mit C++ Desktopentwicklung / MSVC

## Build (Qt-6-kompatible Developer PowerShell)
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64"
cmake --build build --config Release
```
Qt-Pfad an die installierte Version anpassen. Für einen anderen Rechner Qt-DLLs mit `windeployqt` neben die EXE kopieren.

## Aktueller Stand / bewusst noch offen
- Fensterbewegungen und Fensterschließen werden **noch nicht** überwacht. Dafür ist später ein eigenes Windows-Modul mit `SetWinEventHook` vorgesehen.
- Die Bewegung am unteren Bildschirmrand folgt dem nutzbaren Bildschirmbereich, nicht pixelgenau der Taskleistenoberkante.
- Ausflüge führen derzeit nach oben und zurück, nicht frei in zwei Dimensionen.
- Kein Click-through außerhalb der gezeichneten Schnecke: Das transparente rechteckige Fenster kann Klicks abfangen. Für produktiven Einsatz ist eine Eingabemaske bzw. native Hit-Test-Logik nötig.
- Die Cartoon-Grafik ist vorläufig und nicht die originale Schneggr-Illustration.
- Keine Messung des RAM-/CPU-Verbrauchs durchgeführt.
