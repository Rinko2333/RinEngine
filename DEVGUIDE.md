# RinEngine 开发者操作指南

> 从零开始搭建 RinEngine 开发环境并构建项目

---

## 一、环境要求

| 组件 | 版本 | 说明 |
|------|------|------|
| **Qt** | 6.5.3 | MinGW 64-bit 套件 |
| **CMake** | 3.16+ | 随 Qt 安装提供 |
| **编译器** | MinGW 8.1.0 (x86_64) | 随 Qt 安装提供 |
| **Git** | 任意版本 | 用于克隆仓库 |
| **操作系统** | Windows 10/11 | macOS/Linux 见[第五节](#五多平台构建) |

**Qt 安装步骤：**

1. 访问 https://www.qt.io/download-qt-installer
2. 安装 Qt 6.5.3，选择以下组件：
   - Qt 6.5.3 → MinGW 64-bit
   - Qt 6.5.3 → Additional Libraries → Qt Multimedia
   - Qt 6.5.3 → Additional Libraries → Qt Concurrent (通常随 Quick 一起安装)
3. 安装到默认路径：`C:\Qt\6.5.3\mingw_64`

---

## 二、克隆仓库

```powershell
git clone git@github.com:Rinko2333/RinEngine.git
cd RinEngine
```

目录结构（根目录）：
```
RinEngine/
├── AGENT.md           # 项目编程规范 + 历史教训
├── TODO.md            # 开发路线图
├── scripts/           # 工具脚本
│   └── package_release.ps1
└── RinEngine/         # 源代码
    ├── CMakeLists.txt
    ├── src/
    │   ├── core/      # C++ 引擎核心（静态库）
    │   └── app/       # 可执行文件 + QML UI
    ├── assets/        # 游戏资源 + 脚本
    └── l10n/          # 翻译字典
```

---

## 三、构建项目

### 3.1 使用 Qt Creator（推荐新手）

1. 打开 Qt Creator
2. File → Open File or Project → 选择 `RinEngine/RinEngine/CMakeLists.txt`
3. 选择 Kit：**Desktop Qt 6.5.3 MinGW 64-bit**
4. 点击 Configure Project
5. 按 `Ctrl+B` 构建，`Ctrl+R` 运行

### 3.2 使用命令行

```powershell
# 进入构建目录（Qt Creator 首次打开项目后自动生成）
cd RinEngine\build\Desktop_Qt_6_5_3_MinGW_64_bit-Debug

# 构建
cmake --build . --target appRinEngine

# 运行
.\appRinEngine.exe

# 或从项目根目录一步构建
cmake --build "RinEngine\build\Desktop_Qt_6_5_3_MinGW_64_bit-Debug" --target appRinEngine
```

### 3.3 构建目录说明

构建输出在 `RinEngine/build/` 目录下。Qt Creator 会自动生成以 Kit 命名的构建目录。

---

## 四、项目结构速览

```
RinEngine/RinEngine/src/
├── core/                               # C++ 引擎核心（静态库）
│   ├── Logger.h/cpp                    # 结构化日志 (Debug/Info/Warn/Error/Fatal)
│   ├── SettingsManager.h/cpp           # QSettings 持久化配置（含 language 属性）
│   ├── ResourceManager.h/cpp           # 资源路径解析 + 缓存（bg:/char:/bgm: 等协议）
│   ├── VariableManager.h/cpp           # 游戏变量存储 + 条件求值 (affection>=5)
│   ├── ScriptParser.h/cpp              # JSON 脚本解析 → ScriptCommand 队列
│   ├── ScriptRunner.h/cpp              # 脚本执行引擎（say/bg/ch/select/if/jump 等）
│   ├── GameStateManager.h/cpp          # 存档/读档系统（100 槽位 + 快速存档）
│   ├── AudioManager.h/cpp              # Phase 4: BGM 双通道交叉淡入淡出 / SE / Voice
│   ├── GalleryManager.h/cpp            # Phase 5: CG/BGM 解锁追踪 (saves/gallery.json)
│   ├── LocalizationManager.h/cpp       # Phase 6: 多语言字典加载 + QML 绑定触发器
│   └── CMakeLists.txt
│
├── app/                                # 可执行文件入口
│   ├── main.cpp                        # 注册所有 C++ 单例到 QML (RinEngine.Core 模块)
│   ├── resources.qrc                   # 编译 l10n 字典进二进制
│   ├── CMakeLists.txt
│   └── qml/                            # QML UI
│       ├── Main.qml                    # 主窗口 + StackView + ESC 键盘处理
│       ├── screens/                    # TitleScreen, CGGalleryScreen, MusicRoomScreen
│       ├── layers/                     # LayerStack
│       ├── widgets/                    # BackgroundLayer, CharacterLayer, DialogueBox, ChoiceBox, etc.
│       └── effects/                    # AnimationPlayer
```

### QML 与 C++ 的桥接

所有 C++ Manager 类通过 `qmlRegisterSingletonInstance` 注册到 `RinEngine.Core` QML 模块：

| QML 名称 | C++ 类 | 功能 |
|----------|--------|------|
| `Settings` | `SettingsManager` | `bgmVolume`, `seVolume`, `language`, ... |
| `ScriptRunner` | `ScriptRunner` | `canSave`, `dialogueHistory`, `advance()` |
| `GameStateManager` | `GameStateManager` | `save()`, `load()`, `hasAnySave()` |
| `AudioManager` | `AudioManager` | `playBgm()`, `playSe()`, `playVoice()` |
| `L10n` | `LocalizationManager` | `tr(key)`, `localeVersion` |

---

## 五、多平台构建

### macOS

```bash
# 需安装 Qt 6.5.3 for macOS (Clang)
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.5.3/macos
cmake --build build --target appRinEngine
```

### Linux (Ubuntu/Debian)

```bash
# 安装依赖
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-multimedia-dev

# 构建
cmake -B build -G Ninja
cmake --build build --target appRinEngine
```

---

## 六、脚本编写教程 (script.json)

RinEngine 使用 JSON 格式的脚本文件驱动游戏流程。脚本文件存放在 `assets/scripts/` 目录下。

### 6.1 基本结构

脚本由一个 JSON 数组组成，数组中每条元素是一条指令：

```json
[
  ["指令类型", "参数1", "参数2", ...],
  ["指令类型", "参数1", {"选项名":"选项值", ...}]
]
```

每条指令的第一个元素是**指令类型**（字符串），后续元素按约定位置放置**位置参数**，最后一个元素如果是 JSON 对象 `{...}` 则为**命名选项**。

所有指令**顺序执行**，遇到 `say`、`bg`、`ch` 等需要玩家点击的指令时会暂停等待点击。

---

### 6.2 指令详解

#### `label` — 标签锚点

定义跳转目标，本身不产生任何游戏效果。

```
["label", "标签名"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 标签名（字符串），用于 `jump`/`call` 定位 |

#### `say` — 对话文本

显示角色对话。引擎等待玩家点击后继续。

```
["say", "说话者", "对话文本"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 说话者名字（显示在对话框顶部） |
| args[1] | 对话文本。以 `@` 开头表示翻译键，如 `@DLG_WELCOME` |

**多语言支持：** 文本以 `@` 开头时，引擎从 `l10n/{语言}.json` 中查找翻译；不以 `@` 开头则原样显示。

```
["say", "Alice", "@intro_001"]   ← 多语言键
["say", "Alice", "你好！"]       ← 直接文本（向后兼容）
```

#### `bg` — 切换背景

切换全屏背景图片，支持过渡动画。

```
["bg", "背景ID", {"tr":"fade", "d":1.0}]
```

| 参数 | 说明 |
|------|------|
| args[0] | 背景 ID，对应 `assets/bg/{id}.png` 文件 |
| options.tr | 过渡效果：`"fade"`（淡入淡出）、`"none"`（无动画），默认 `"fade"` |
| options.d | 过渡持续时间（秒），默认 `1.0` |

**引擎等待过渡动画完成后才继续。**

#### `ch` — 显示/更新立绘

在指定位置显示或更新角色立绘，可带入场效果。

```
["ch", "角色名", "表情", "位置", {"effect":"bounce", "d":0.5}]
```

| 参数 | 说明 |
|------|------|
| args[0] | 角色名，对应 `assets/char/{name}_{face}.png` |
| args[1] | 表情名称，对应文件名中的 `_happy`/`_sad` 等后缀，默认 `"normal"` |
| args[2] | 显示位置：`"left"`、`"center"`、`"right"`，默认 `"center"` |
| options.effect | 入场效果：`"bounce"`、`"shake"`、`""`（无效果） |
| options.d | 动画持续时间（秒），默认 `0.3` |

**同一角色在同一位置再次调用时，会替换当前立绘（可配合特效做表情切换）。**

#### `ch_hide` — 隐藏立绘

隐藏指定位置的角色。

```
["ch_hide", "位置"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 要隐藏的位置：`"left"`、`"center"`、`"right"` |

#### `ch_move` — 移动立绘

将角色从一个位置移动到另一个位置。

```
["ch_move", "源位置", "目标位置", {"d":0.5}]
```

| 参数 | 说明 |
|------|------|
| args[0] | 源位置 |
| args[1] | 目标位置 |
| options.d | 移动持续时间（秒），默认 `0.5` |

#### `select` — 选项分支

显示选项按钮，玩家选择一个后跳转到对应标签。

```
["select", [
  ["选项文本1", "跳转标签1", "显示条件"],
  ["选项文本2", "跳转标签2", "显示条件"],
  ["选项文本3", "跳转标签3", ""]
]]
```

| 字段 | 说明 |
|------|------|
| `[0]` 选项文本 | 显示在按钮上的文字 |
| `[1]` 跳转标签 | 选择后跳转到的 `label` 名称 |
| `[2]` 显示条件 | 条件表达式（见下方），`""` 为始终显示。条件不满足时选项灰显/隐藏 |

```
["label", "路口"],
["select", [
  ["去天台", "route_rooftop", "affection>=10"],
  ["去图书馆", "route_library", ""],
  ["回家", "route_home", ""]
]],
```

#### `jump` — 跳转

无条件跳转到指定标签，继续从标签处执行。

```
["jump", "标签名"]
```

#### `call` / `return` — 子程序调用

类似函数调用：`call` 跳转到标签并记住返回位置，`return` 从子程序返回。

```
["call", "子程序标签"]
...子程序指令...
["return"]
```

```
["call", "show_intro"]

["label", "show_intro"],
["say", "", "这是一个通用介绍。"]  // 空说话者名 = 旁白
["return"],
```

**注意：** 暂无 `call` 嵌套限制，但过深嵌套可能导致堆栈溢出。

#### `if` — 条件分支

根据条件表达式决定是否执行子指令块。

```
["if", "条件表达式", [
  ["say", "", "条件满足时执行的指令"],
  ["add", "affection", 5]
]]
```

**条件表达式语法：**

| 运算符 | 示例 | 说明 |
|--------|------|------|
| `>=` | `affection>=10` | 大于等于 |
| `<=` | `health<=0` | 小于等于 |
| `>` | `score>100` | 大于 |
| `<` | `timer<30` | 小于 |
| `==` | `flag==true` | 等于（数字或布尔） |
| `!=` | `name!=Alice` | 不等于 |
| 裸变量 | `flag_met` | 无需运算符 — 存在且非零/非空即为真 |

支持布尔值：`flag==true`、`flag==false`。数值比较使用 `qFuzzyCompare`。

#### `set` — 设置变量

设置游戏变量值。支持数字、布尔、字符串。

```
["set", "变量名", 值]
```

| 示例 | 效果 |
|------|------|
| `["set", "affection", 0]` | 设置 `affection` = 0（数字） |
| `["set", "flag_met", true]` | 设置 `flag_met` = true（布尔） |
| `["set", "player_name", "Alice"]` | 设置 `player_name` = "Alice"（字符串） |

#### `add` — 修改变量

对数值变量进行加法操作。

```
["add", "变量名", 增量]
```

| 示例 | 效果 |
|------|------|
| `["add", "affection", 3]` | `affection += 3` |
| `["add", "health", -10]` | `health -= 10` |

#### `bgm` — 播放背景音乐

播放背景音乐，BGM 通道支持交叉淡入淡出（500ms）。

```
["bgm", "曲目ID", {"vol":80}]
```

| 参数 | 说明 |
|------|------|
| args[0] | 曲目 ID，对应 `assets/bgm/{id}.{mp3/ogg/wav}` |
| options.vol | 音量（0-100），叠加系统音量设置，默认 `80` |

**BGM 不等待，立即继续执行下一条指令。播放过的 BGM 自动在音乐鉴赏中解锁。**

#### `se` — 播放音效

播放短音效，不等待。

```
["se", "音效ID"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 音效 ID，对应 `assets/se/{id}.{mp3/ogg/wav}` |

#### `voice` — 播放语音

播放角色语音，Voice 通道独占（同时只能一个语音）。

```
["voice", "语音ID"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 语音 ID，对应 `assets/voice/{id}.{mp3/ogg/wav}` |

#### `video` — 播放视频

全屏播放视频，玩家可点击或按 ESC 跳过。播完自动继续。

```
["video", "视频ID"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 视频 ID，对应 `assets/video/{id}.{mp4/webm}` |

**视频播放期间禁用底层鼠标区域，ESC 优先跳过视频。**

#### `unlock` — 解锁鉴赏

手动解锁 CG 或 BGM 到鉴赏模式。

```
["unlock", "cg", "CG_ID"]
["unlock", "bgm", "BGM_ID"]
```

| 参数 | 说明 |
|------|------|
| args[0] | 类别：`"cg"` 或 `"bgm"` |
| args[1] | 要解锁的资源 ID |

**注意：** BGM 也会在执行 `bgm` 指令时自动解锁。CG 需要手动用本指令解锁。重复解锁无影响。

#### `wait` — 暂停等待

暂停脚本执行，等待玩家点击后继续。相当于一条空白对话。

```
["wait"]
```

**用途：** 在场景切换后给玩家缓冲时间，或配合 `if` 之后的逻辑分离。

---

### 6.3 完整示例脚本

```json
[
  ["label", "start"],

  ["set", "affection", 0],

  ["bgm", "morning", {"vol":80}],
  ["bg", "school", {"tr":"fade", "d":1.0}],
  ["unlock", "cg", "cg_school"],

  ["ch", "alice", "happy", "center", {"effect":"bounce", "d":0.5}],
  ["say", "Alice", "你好！欢迎来到我的世界！"],
  ["add", "affection", 3],

  ["label", "choice_1"],
  ["select", [
    ["打招呼", "route_greet", ""],
    ["沉默", "route_silent", ""]
  ]],

  ["label", "route_greet"],
  ["say", "Alice", "很高兴认识你！"],
  ["add", "affection", 5],
  ["jump", "check_affection"],

  ["label", "route_silent"],
  ["say", "Alice", "……"],
  ["jump", "check_affection"],

  ["label", "check_affection"],
  ["if", "affection>=8", [
    ["say", "Alice", "我感觉我们很合得来！"],
    ["unlock", "cg", "cg_alice_smile"]
  ]],

  ["bg", "garden", {"tr":"fade", "d":0.8}],

  ["say", "Alice", "下次再见！"],
  ["say", "", "—— 完 ——"]
]
```

---

### 6.4 资源目录约定

| 指令 | 资源位置 | 文件格式 |
|------|---------|---------|
| `bg` | `assets/bg/{id}.png` | PNG |
| `ch` | `assets/char/{name}_{face}.png` | PNG |
| `bgm` | `assets/bgm/{id}.{mp3,ogg,wav}` | MP3 / OGG / WAV |
| `se` | `assets/se/{id}.{mp3,ogg,wav}` | MP3 / OGG / WAV |
| `voice` | `assets/voice/{id}.{mp3,ogg,wav}` | MP3 / OGG / WAV |
| `video` | `assets/video/{id}.{mp4,webm}` | MP4 / WebM |

**立绘命名规则：** `{角色名}_{表情}.png`，例如 `alice_happy.png`、`alice_sad.png`。`ch` 指令的 `args[1]` 对应文件名中的 `{表情}` 部分。

---

### 6.5 变量系统

变量可用于条件分支和选项显示条件。

**数据类型：** 数字（整数/浮点数）、布尔值（`true`/`false`）、字符串。

**生命周期：** 变量在脚本启动时自动清空，通过 `set`/`add` 创建或修改。存档时变量会被序列化保存。

**常用模式：**

```
// 好感度系统
["set", "affection", 0]
["add", "affection", 3]

// 标记系统（记录玩家选择）
["set", "met_alice", true]
["set", "chose_fight", false]

// 条件选项
["select", [
  ["告白", "confess", "affection>=10"],
  ["再等等", "wait", ""]
]]
```

---

### 6.6 多语言脚本

使用 `@key` 语法编写支持多语言的对话：

```json
["say", "Alice", "@intro_001"]
```

翻译写在 `l10n/zh.json`（中文）和 `l10n/en.json`（英文）中：

```json
{
  "intro_001": "你好，我是 Alice。",
  "intro_002": "今天天气真好。"
}
```

不以 `@` 开头的对话文本直接显示，适用于不需要翻译的内容（如系统提示、旁白过渡句等）。

---

## 七、添加新功能指南

### 添加新的 C++ Manager 类

1. 创建 `src/core/NewManager.h` 和 `src/core/NewManager.cpp`
2. 使用 Meyer's Singleton 模式：
   ```cpp
   class NewManager : public QObject {
       Q_OBJECT
   public:
       static NewManager* instance() { static NewManager inst; return &inst; }
       explicit NewManager(QObject *parent = nullptr);
   };
   ```
3. 在 `src/core/CMakeLists.txt` 中添加源文件
4. 在 `src/app/main.cpp` 中注册：
   ```cpp
   qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "NewManager", NewManager::instance());
   ```

### 添加新的 QML 文件

1. 创建 `.qml` 文件到 `src/app/qml/` 合适子目录
2. 在 `src/app/CMakeLists.txt` 的 `QML_FILES` 列表中添加

### 添加新的脚本指令

1. 在 `src/core/ScriptRunner.cpp` 的 `executeCommand()` 方法中添加 `else if (cmd.type == "xxx")` 分支
2. 如需玩家点击等待，使用 `setWaitState(WAIT_CLICK)` + `return`（不调用 `advanceToNext`）
3. 如无需等待，末尾调用 `advanceToNext()` 或 `executeNextChild()`

---

## 八、发布打包

```powershell
# 确保已完成构建
cmake --build "RinEngine\build\Desktop_Qt_6_5_3_MinGW_64_bit-Debug" --target appRinEngine

# 运行打包脚本
.\scripts\package_release.ps1

# 自定义 Qt 路径（如安装在其他位置）
.\scripts\package_release.ps1 -QtPath "D:\Qt\6.5.3\mingw_64"
```

脚本执行以下操作：
1. 复制 `appRinEngine.exe` 到 `Release/`
2. 运行 `windeployqt` 收集 Qt DLLs 和插件到 `Release/Runtime/`
3. 创建 `qt.conf` 重定向库路径
4. 复制 `assets/` 到 `Release/assets/`
5. 保留许可文档（NOTICE.txt, LICENSE.*）

输出结构：
```
D:\RinEngine\Release\
├── appRinEngine.exe
├── qt.conf
├── NOTICE.txt
├── LICENSE.GPLv3
├── LICENSE.LGPLv3
├── Runtime/          # Qt DLLs + 插件
├── assets/           # 游戏资源 + 脚本
└── saves/
```

---

## 九、常见问题

### Q: 构建报 "Could NOT find Qt6"

确认 CMake 能找到 Qt 安装路径。在 Qt Creator 中构建通常自动解决。命令行构建时需设置 `CMAKE_PREFIX_PATH`。

### Q: 运行时找不到 DLL

在 Qt Creator 中直接运行即可（Creator 会自动设置环境变量）。命令行运行需先执行：
```powershell
$env:PATH = "C:\Qt\6.5.3\mingw_64\bin;$env:PATH"
.\appRinEngine.exe
```

### Q: 如何切换语言

启动后进入设置界面（标题画面或 ESC 菜单 → 设置），在"语言"下拉框中选择。

### Q: 存档文件在哪

存档保存在 `saves/` 目录（相对于 exe 运行目录）。调试时可能在构建目录的 `saves/` 下。

### Q: l10n 字典文件在哪

开发时装在 `RinEngine/l10n/`。构建时通过 `.qrc` 编译进二进制（`:/l10n/`）。引擎优先从 Qt 资源加载，文件系统作为后备。

---

## 十、QML 编码注意事项

1. **绑定刷新**：`Q_INVOKABLE` 方法不会触发 QML 绑定重计算。需要响应式更新的值必须是 `Q_PROPERTY` 且配 NOTIFY 信号。
2. **本地化文本**：使用 `t("KEY")` 模式，定义在文件顶部：
   ```qml
   function t(key) { L10n.localeVersion; return L10n.tr(key); }
   ```
3. **存档界面刷新**：用 `property int _refreshCounter` 作为绑定显式依赖，而非在 onClicked 中覆盖属性。
4. **焦点管理**：覆盖层显隐后需调用 `gameScreen.forceActiveFocus()` 归还焦点。
5. **脚本结束**：用 `Qt.callLater(function() { sceneStack.pop(); })`，避免在信号链中直接操作 StackView。
