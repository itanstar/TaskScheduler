# 컴파일 옵션
## 🎨 UI
```bash
gcc -Wall -o UI UI.c -std=c99 -D_XOPEN_SOURCE=700 -lncurses
```
## 💻 System
### 📖라이브러리 설치
```bash
sudo apt update
sudo apt install libnotify-dev
```

### system alarm 관련
```bash
gcc -o example example.c `pkg-config --cflags --libs libnotify
```
### SystemEventListener
```bash
gcc -o SystemEventListener SystemEventListener.c
```

## ⏰ timer 관련

## 🏃 process 관련

## 📁 file 관련
