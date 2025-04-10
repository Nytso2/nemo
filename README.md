# 🧠 Nemo – Lightweight Terminal Reminder

**Nemo** is a minimalist terminal tool to schedule one-time reminders that trigger after a set delay or at a specific time — without blocking your terminal. It's perfect for developers who live in the command line.

---

## ✨ Features

- ⏱ Schedule reminders by delay in **minutes**
- 🕒 Or set them for a specific **HH:MM** time
- 🧵 Forks into the background — doesn’t freeze your terminal
- ✅ Message shows directly in your terminal
- 🖥️ Works on macOS and Linux

---

## 🚀 Installation

```bash
git clone https://github.com/YOUR_USERNAME/nemo.git
cd nemo
make
sudo make install
```
 - to Uninstall
```bash
sudo make uninstall
```
## Usage
```bash
nemo "message" <minutes>
nemo "message" at:HH:MM
```
## Example Usage
```bash
nemo "Stand up and stretch!" 30
nemo "Join Zoom meeting" at:14:15
```

