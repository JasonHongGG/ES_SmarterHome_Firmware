# ESP32 + STM32 + Arduino IoT Cloud + PC Server Bridge

此韌體將 STM32 透過 UART2 傳來的訊息轉送到遠端 PC Server，同時與 Arduino IoT Cloud 連線，可接收雲端指令並轉發給 STM32。

## 硬體需求
- ESP32 開發板
- STM32（UART 連接 ESP32）
- Wi-Fi AP
- PC Server（可接受 TCP 連線）
- 連線腳位：ESP32 UART2  
  - RX: GPIO16  
  - TX: GPIO17  
  - 波特率：115200

## 韌體主要流程
- `connectWifi()`：連上 Wi-Fi。
- `ArduinoCloud.begin(...)`：連上 Arduino IoT Cloud，執行 `ArduinoCloud.update()` 維持連線。
- `connectPCServer()`：以 `WiFiClient` 連到 PC Server (`HOST`, `PORT`)。
- `SendMsgFromSTM32ToPC()`：STM32 → ESP32 → PC Server。
- `SenderMsgFromPCToSTM32()`：PC Server → ESP32 → STM32。
- IoT Cloud 回呼：
  - `onMsgFromPhoneChange()`：控制內建 LED 或轉發訊息給 STM32。
  - `onRGBChange()`：將 HSV 轉 RGB 後送給 STM32（指令格式 `led {...}`）。
  - `onLightSwitchChange()`：控制繼電器（指令格式 `relay 1/0`）。

## 先備設定
1. 在 `thingProperties.h`（由 Arduino IoT Cloud 產生）填好：
   - Wi-Fi：`SSID`, `PASS`
   - Thing ID、Device Key
2. 在 `IoT.ino` 設定 PC Server：
   - `HOST`（IP 或網域）
   - `PORT`（TCP 連接埠）
3. 確認 UART 腳位與波特率（預設 RX=16, TX=17, 115200）。

## 編譯與燒錄
- Arduino IDE / VS Code + Arduino CLI / PlatformIO 皆可。
- 板子選擇：`ESP32 Dev Module`（或對應型號）。
- 燒錄後以 Serial Monitor 115200 檢視輸出（若需靜音可關閉 `Serial.println`）。

## 執行與測試
1. 上電後，等待 Wi-Fi 連線成功。
2. 確認 Arduino IoT Cloud Dashboard 可連線。
3. 確認 PC Server 已開啟並可接受 TCP 連線（`HOST:PORT`）。
4. 從 STM32 發送一行結尾為 `\n` 的訊息，可在 PC Server 端收到。
5. 從 PC Server 發送一行 `\n` 結尾的訊息，可透過 UART 送達 STM32。
6. 在 IoT Cloud Dashboard 切換：
   - `msgFromPhone`：`on`/`off` 控制板上 LED；其他字串會轉發給 STM32。
   - `rGB`：改變 HSV 以 `led {"r":...,"g":...,"b":...}` 格式送給 STM32。
   - `lightSwitch`：以 `relay 1/0` 指令送給 STM32。

## 常見問題
- 一直看到 `rst:0xc ...`：這是 ESP32 重啟訊息，若出現 assert/backtrace，通常是尚未連上 Wi-Fi 就嘗試 TCP 連線，請確保 `connectWifi()` 成功後再 `connectPCServer()`。
- 無法連到 PC Server：確認 `HOST/PORT`、Server 有監聽、防火牆已放行。
- 雲端指令無效：檢查 Arduino IoT Cloud 的 Property 名稱、型別是否與 `thingProperties.h` 一致，並確保 `ArduinoCloud.update()` 在 `loop()` 中持續執行。