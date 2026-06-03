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

## 六、添加新功能指南

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

1. 在 `src/core/ScriptRunner.cpp` 的 `executeCurrent()` 方法中添加 `else if (cmd.type == "xxx")` 分支
2. 如需 Wait 机制，使用 `setWaitState(WAIT_*)` + `return`（不调用 `advanceToNext`）
3. 如不需要等待，末尾调用 `advanceToNext()` 或 `executeNextChild()`

---

## 七、发布打包

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

## 八、常见问题

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

## 九、QML 编码注意事项

1. **绑定刷新**：`Q_INVOKABLE` 方法不会触发 QML 绑定重计算。需要响应式更新的值必须是 `Q_PROPERTY` 且配 NOTIFY 信号。
2. **本地化文本**：使用 `t("KEY")` 模式，定义在文件顶部：
   ```qml
   function t(key) { L10n.localeVersion; return L10n.tr(key); }
   ```
3. **存档界面刷新**：用 `property int _refreshCounter` 作为绑定显式依赖，而非在 onClicked 中覆盖属性。
4. **焦点管理**：覆盖层显隐后需调用 `gameScreen.forceActiveFocus()` 归还焦点。
5. **脚本结束**：用 `Qt.callLater(function() { sceneStack.pop(); })`，避免在信号链中直接操作 StackView。
