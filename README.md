# ELF file generator

一個用純 C 語言手動構建 ELF 64 位可執行文件。展示了如何從零開始創建符合 ELF 格式規範的可執行文件，無需使用編譯器或連結器。

##  簡介

這個專案實現了一個簡單的 ELF 文件生成器，能夠創建兩個範例可執行文件：
- `exit.elf`: 執行系統呼叫 `exit(42)` 並返回退出碼 42
- `hello.elf`: 輸出 "Hello, ELF World!" 訊息後正常退出


##  ELF 結構

實現了以下 ELF 結構：

1. **ELF Header (Elf64_Ehdr)**
   - 魔數識別（0x7F 'E' 'L' 'F'）
   - 64 位小端序格式
   - 可執行文件類型（ET_EXEC）
   - x86-64 架構（EM_X86_64）

2. **Program Header (Elf64_Phdr)**
   - PT_LOAD 類型段
   - 可讀可執行權限（PF_R | PF_X）
   - 頁面對齊（0x1000 字節）

##  編譯與使用

### 編譯

```bash
gcc -o main main.c
```

或使用 Makefile：

```bash
make
```

### 執行

```bash
./main
```

生成兩個 ELF 文件：
- `exit.elf`
- `hello.elf`

### 測試生成的 ELF 文件

```bash
# 執行 exit.elf（會返回退出碼 42）
./exit.elf
echo $?  # 應該輸出 42

# 執行 hello.elf（會輸出訊息）
./hello.elf
# 輸出: Hello, ELF World!
```

### 檢查 ELF 文件結構

```bash
# 查看 ELF Header
readelf -h exit.elf

# 查看 Program Headers
readelf -l exit.elf

# 查看文件類型
file hello.elf
```


## 程式結構說明

### 主要函數

- `create_elf_header()`: 構建 ELF 文件頭
- `create_program_header()`: 構建程式段頭
- `generate_elf_file()`: 生成完整的 ELF 可執行文件

### 關鍵常數

- `base_vaddr = 0x400000`: 基礎虛擬地址（頁面對齊）
- `entry_point = base_vaddr + 0x78`: 程式進入點
- `p_align = 0x1000`: 頁面對齊（4096 字節）



##  注意事項

- 此專案僅用於教學目的
- 生成的 ELF 文件沒有 Section Header Table（因此 `objdump` 可能無法顯示反彙編）
- 僅支援 x86-64 Linux 系統
- 程式碼使用硬編碼的地址，不適用於所有系統配置

##  已知限制

- 沒有 Section Header Table，某些工具（如 `objdump -D`）可能無法正確解析
- 地址計算是硬編碼的，不支援動態連結
- 僅包含基本的程式段，沒有動態連結器資訊

## 參考
- Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification
- https://zh.wikipedia.org/zh-tw/%E5%8F%AF%E5%9F%B7%E8%A1%8C%E8%88%87%E5%8F%AF%E9%8F%88%E6%8E%A5%E6%A0%BC%E5%BC%8F
- <img width="680" height="1135" alt="image" src="https://github.com/user-attachments/assets/e283bb98-8f7d-49db-b885-4059e2e17c9c" />

