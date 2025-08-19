![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico-blue?logo=raspberry-pi) 
![License](https://img.shields.io/badge/license-MIT-green)  

# 📡 eHack FM Radio Module

This is the repository for the **eHack FM Radio Module**, a small device that works together with the main [eHack Project](https://github.com/kelv1n9/eHack).  
It can transmit FM signals with RDS, send battery status, and communicate with the master device over NRF24L01.  

---

## ✨ Key Features  

- **FM Transmission with RDS** – tune frequency, set station name, send RDS messages.  
- **NRF24 Communication** – receive commands from the master and send back data.  
- **Battery Monitoring** – measure and report battery voltage.  
- **LED Indicators** – show connection and signal level with simple blink patterns.  
- **Auto Power Off** – device goes to sleep if connection is lost.  

---

## 🛠️ Hardware  

- **MCU:** Raspberry Pi Pico  
- **FM Transmitter:** Si4713 (or compatible)  
- **2.4 GHz Radio:** NRF24L01+

---

## ⚙️ How It Works  

1. The module waits for connection from the master (PING/PONG).  
2. The master sends commands (e.g. set FM frequency, power off).  
3. The slave executes the command and reports back (battery, signal level).  
4. LED shows status: connected, signal strong/weak, or waiting.  

---

## 🚨 Disclaimer  

> **For Educational & Research Use Only**  
> Use only on legal frequencies and systems you are allowed to transmit on.  
> The author (**Elvin Gadirov**) takes no responsibility for misuse.  
