# 🧠 Nemo – Lightweight Terminal Reminder

**Nemo** is a minimalist terminal tool to schedule one-time reminders that trigger after a set delay or at a specific time — without blocking your terminal. It's perfect for developers who live in the command line.

---

## ✨ Features

- ⏱ Schedule reminders by delay in **minutes**
- 🕒 Or set them for a specific **HH:MM** time (`at:`)
- 🔁 Optional repeating check-ins (`rm:X`) to show time left
- ✅ Cancel reminders early with `nemo done`
- 🧵 Runs in the background — doesn’t freeze your terminal
- 🖥️ Works on macOS and Linux

---

## 🚀 Installation

```bash
git clone https://github.com/YOUR_USERNAME/nemo.git
cd nemo
make
sudo make install
```
- To unistall 
```bash
sudo make uninstall
```
## Usage
```bash
nemo "message" <minutes>
nemo "message" at:HH:MM
nemo "message" <minutes|at:HH:MM> rm:X
nemo done
```
## Example usage
```bash
nemo "Stand up and stretch!" 30
nemo "Join Zoom meeting" at:14:15
nemo "Finish paper" 60 rm:15
nemo "Presentation in 1 hour" at:17:00 rm:10
nemo done
```
- rm: X shows : progress every X minutes
- nemo done : cancels any running reminder before it's finished
