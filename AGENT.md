# RinEngine — AI 编程规范与项目上下文

*本文件总结了项目的架构设计、编码规范、以及历史 bug 修复中沉淀的经验教训。每次修改代码前请先阅读此文。*

---

## 项目概况

基于 **Qt 6.5.3 + QML + C++** 的跨平台视觉小说引擎。目标用户是零编程基础的内容创作者。

| 层 | 技术 |
|----|------|
| UI | Qt Quick (QML) — `RinEngine` QML module |
| 后端 | C++ 静态库 `RinEngineCore` |
| 桥接 | `qmlRegisterSingletonInstance` 注册 C++ 单例到 `RinEngine.Core` QML module |
| 构建 | CMake 3.16+, C++17, MinGW 64-bit |
AI推理过程可以是英文，但是对于用户的最终输出必须是中文！！
---

## 目录结构

```
RinEngine/
├── CMakeLists.txt              # 顶层 CMake
├── src/
│   ├── core/                   # C++ 核心库（静态库）
│   │   ├── CMakeLists.txt
│   │   ├── Logger.h/cpp        # 结构化日志（Debug/Info/Warn/Error/Fatal）
│   │   ├── SettingsManager.h/cpp     # QSettings 持久化配置
│   │   ├── ResourceManager.h/cpp     # 资源路径解析 + 缓存
│   │   ├── VariableManager.h/cpp     # 游戏变量存储 + 条件求值
│   │   ├── ScriptParser.h/cpp        # JSON 脚本解析
│   │   ├── ScriptRunner.h/cpp        # 脚本执行引擎
│   │   ├── GameStateManager.h/cpp    # 存档/读档系统
│   │   ├── AudioManager.h/cpp        # Phase 4: 三通道音频 (BGM/SE/Voice)
│   │   ├── GalleryManager.h/cpp      # Phase 5: CG/BGM 解锁追踪
│   │   └── LocalizationManager.h/cpp # Phase 6: 多语言本地化
│   └── app/                    # 可执行文件
│       ├── CMakeLists.txt
│       ├── main.cpp            # 入口，注册 C++ 单例
│       └── qml/                # QML UI
│           ├── Main.qml        # 主窗口 + StackView
│           ├── screens/        # TitleScreen
│           ├── layers/         # LayerStack (预留)
│           ├── widgets/        # BackgroundLayer, CharacterLayer, DialogueBox, ChoiceBox, VideoPlayer, etc.
│           └── effects/        # AnimationPlayer
├── assets/                     # 资源文件（bg/, char/, bgm/, se/, voice/, video/, scripts/）
├── l10n/                       # 翻译字典（zh.json, en.json）
├── build/                      # 构建输出（已 gitignore）
└── Release/                    # 分发目录
    ├── NOTICE.txt               # LGPL 声明
    ├── LICENSE.GPLv3
    ├── LICENSE.LGPLv3
    ├── Runtime/                 # Qt DLLs
    └── saves/
```

---

## 构建命令

```powershell
# 确保 Qt 6.5.3 MinGW 环境可用
cmake --build "RinEngine\build\Desktop_Qt_6_5_3_MinGW_64_bit-Debug" --target appRinEngine
```

**不要**手动 `cd` 到 build 目录运行 ninja/make，直接用 `cmake --build` 指定路径。

---

## 核心编码规范

### C++ 方面

1. **单例模式**：所有 Manager 类使用 `static T inst; return &inst;` —— 不得使用 `new` 创建。
2. **Q_PROPERTY + NOTIFY**：任何需要在 QML 中响应式使用的值必须配 NOTIFY 信号。单纯 `Q_INVOKABLE` 方法不会触发 QML 绑定更新。参见下方「历史 bug #2」。
3. **信号命名**：
   - 通用信号：`runningChanged()`, `commandChanged()`
   - 恢复专用信号：`restoreBackground()`, `restoreCharacter()`, `restoreDialogue()`, `clearDialogue()` —— 这些信号在存档加载时使用，**不应**触发脚本 `advance()` 回调。
4. **JSON 脚本解析**：`QJsonDocument::fromJson()` → `QJsonArray::toVariantList()`。但子命令（if 的 children）不要使用 `QVariant::toList()` 二次解析，必须递归调用 `parseCommand()`。详见「历史 bug #1」。
5. **脚本命令结构**：
   ```cpp
   struct ScriptCommand {
       QString type;           // "bg", "ch", "say", "select", "if" ...
       QStringList args;       // 位置参数
       QVariantMap options;    // 命名选项（JSON 对象）
       QVariantList choices;   // select 选项数组 [text, jump, condition]
       QVector<ScriptCommand> children;  // if 子命令（递归解析）
   };
   ```
6. **WaitState 管理**：所有 `m_waitState` 赋值必须走 `setWaitState(state)` —— 它会自动 emit `canSaveChanged()`。
7. **存档加载**：`GameStateManager::load()` 调用 `resume()` 而非 `start()`。`resume()` 不执行当前命令，只设 `WAIT_CLICK` + 恢复对话文本。

### QML 方面

1. **焦点管理**：
   - 弹出面板（SystemMenu、SaveLoadScreen、HistoryScreen、SettingsScreen）显隐后必须调用 `gameScreen.forceActiveFocus()` 归还键盘焦点。
   - 选择按钮（ChoiceBox 的 Repeater delegate）必须设 `focusPolicy: Qt.NoFocus` 防止偷焦点。
2. **信号处理器语法**（Qt 6.5）：
   - 使用 `function onXxx(params) { ... }` 或 `onXxx: (params) => { ... }`
   - **不要**使用 `onXxx: { implicit_param }` 隐式参数语法（Qt 6.5 deprecation 警告）
   - **不要**使用 `onXxx: function(param) { ... }` 旧式语法（参数注入已废弃）
3. **覆盖层显隐**：不要在 QML 实例中和组件 QML 文件中重复定义同名 `show()`/`hide()` 函数 —— 组件自带即可，实例级覆盖仅用于需要特殊行为的情况。
4. **存档界面**：`saveLoadScreen.saveMode` 是 `bool`，不是字符串 `"save"`/`"load"`。
5. **存档槽位刷新**：用 `property int _refreshCounter: 0` 作为绑定的显式依赖，而不是在 onClicked 中覆写绑定的属性。
6. **脚本结束**：用 `Qt.callLater(function() { sceneStack.pop(); })` 延迟弹出，避免在信号链中直接操作 StackView。
7. **恢复对话文本**：加载存档后显示对话历史最后一条，用 `restoreDialogue` 信号 + `dialogueBox.finishTyping()` 立即完成，无需打字机动画。

---

## 历史 Bug 教训

### Bug #1 — QVariant::canConvert / toList 在 Qt 6.5.3 中的隐式转换问题

**症状**：`if` 命令的子命令被跳过，`childArgs` 为空。

**根因**：Qt 6.5.3 中 `QVariant::canConvert<QVariantList>()` 对 `QString` 也返回 `true`，导致 `"affection>=5"` 被误判为列表类型，条件字符串被吞掉。进一步，即使正确检测到子命令列表，`QVariant::toList()` 对深层嵌套的 `QJsonArray → QVariant` 转换会丢失元素。

**解决方案**：`if` 的子命令**完全绕过 QVariant 二次解析**：
- `parseCommand()` 中对 `if` 类型递归调用自身，将每个子命令解析为完整的 `ScriptCommand` 对象存入 `cmd.children`
- `executeNextChild()` 直接使用 `childCmd.args`、`childCmd.options` 等字段，不再从 QVariantList 重新解析

**教训**：不要依赖 `QVariant::canConvert<T>()` 做类型检测，不要对深层嵌套数据使用 `toList()`。嵌套结构必须用递归解析或 `QJsonDocument` round-trip。

### Bug #2 — Q_INVOKABLE 不会触发 QML 绑定更新

**症状**：菜单中 "保存" 按钮始终灰色（disabled），即使脚本处于可保存状态。

**根因**：`enabled: ScriptRunner.canSave()` 是一个 QML 属性绑定。`canSave()` 原来是 `Q_INVOKABLE` 方法，内部访问 `m_waitState` 字段。QML 绑定引擎无法追踪 C++ 内部字段变化，只在 `runningChanged` 信号触发时重求值。但 `m_waitState` 变化时不 emit 任何信号。

**解决方案**：
- 将 `canSave` 升级为 `Q_PROPERTY(bool canSave READ canSave NOTIFY canSaveChanged)`
- 所有 `m_waitState` 赋值通过 `setWaitState()` 统一管理，自动 emit `canSaveChanged`
- QML 中使用 `ScriptRunner.canSave`（属性语法），不带括号

**教训**：任何 QML 中需要响应式访问的值必须是 `Q_PROPERTY` 且配 NOTIFY。

### Bug #3 — 存档加载后 auto-advance

**症状**：加载存档后 `applyVisualSnapshot` 触发的背景/立绘恢复信号携带了 `advance()` 回调，导致脚本自动推进。

**根因**：`applyVisualSnapshot` 原本用 `showBackground`/`showCharacter` 信号恢复画面，这些信号的 QML handler 回调中调用了 `ScriptRunner.advance()`。当 transition 为 "none" 时回调立即执行。

**解决方案**：新增专用恢复信号 `restoreBackground`/`restoreCharacter`，QML handler 中以空回调调用 `show()`，不会触发 advance。

**教训**：存档恢复和正常游戏流程复用同一信号时要小心回调副作用。必要时新增专用信号。

### Bug #4 — 继续游戏时画面不加载

**症状**：标题界面点"继续游戏"，视觉状态（背景、立绘）不显示。

**根因**：`quickLoad()` 在游戏画面推入 StackView 之前执行，`applyVisualSnapshot` 发出的信号没有接收方（Connections 尚未创建）。

**解决方案**：引入 **deferred load** 机制：
- `GameStateManager::scheduleLoad(slot)` — 标记待加载槽位
- `GameStateManager::executePendingLoad()` — 在游戏画面 `Component.onCompleted` 中执行
- "继续游戏" 按钮：先 push 游戏画面 → `onCompleted` 检测 `hasPendingLoad()` → 执行加载

**教训**：涉及 QML 信号的操作必须确保接收方已就绪。跨组件生命周期的操作用 deferred 机制。

### Bug #5 — ESC 在选择界面无效

**症状**：ChoiceBox 显示时按 ESC 无反应。

**根因**：ChoiceBox 的 Repeater delegate（Button）通过焦点链抢走键盘焦点，gameScreen 的 `Keys.onEscapePressed` 接收不到事件。

**解决方案**：ChoiceBox 的 Button delegate 设置 `focusPolicy: Qt.NoFocus`。

**教训**：Qt Quick Controls 2 的 Button 默认参与焦点链。覆盖层弹出后要确保焦点不丢失。

---

## Phase 4 — 完成 ✅

Phase 4 目标：音频管理、视频播放、演出动画。**已全部实现。**

### 实现摘要

| 组件 | 文件 | 要点 |
|------|------|------|
| **AudioManager** | `src/core/AudioManager.h/.cpp` | BGM 双通道交叉淡入淡出 (QTimer 50ms/step)、SE 单通道、Voice 单通道 |
| **VideoPlayer** | `src/app/qml/widgets/VideoPlayer.qml` | VideoOutput + MediaPlayer，支持点击/ESC 跳过，播完自动 advance |
| **AnimationPlayer 扩展** | `src/app/qml/effects/AnimationPlayer.qml` | 新增 `screenShake` (10步 XY 抖动) 和 `screenFlash` (白色闪屏) |
| **ResourceManager 扩展** | `src/core/ResourceManager.h/.cpp` | 新增 `getAudio()`/`getVideo()` + 音频/视频扩展名搜索 |
| **CMake** | 三层 CMakeLists.txt | `find_package` 添加 `Multimedia` + `MultimediaWidgets` |

### Phase 4 编码模式

1. **AudioManager 音量链路**：Script 指令音量(0-100) × Settings 全局音量(0-100) → `QAudioOutput::setVolume(0.0-1.0)`。`SettingsManager` 的 NOTIFY 信号直连 AudioManager 内部 lambda，实时生效。
2. **BGM 交叉淡入淡出**：两个 `QMediaPlayer` + `QAudioOutput` 对 (`m_bgmA`/`m_bgmB`)，轮流用作 active/fading。`m_fadeTimer` (50ms × 10步 = 500ms) 逐步调整两者音量。
3. **媒体文件容错**：音频/视频文件不存在时只 Logger::warn，不崩溃不阻塞。ScriptRunner 中 `bgm`/`se`/`voice` 指令不等待 (`advanceToNext()`)，`video` 指令等待点击。
4. **VideoPlayer 集成**：作为游戏画面内的覆盖层 (z: 70)，显示时禁用底层 MouseArea。ESC 链中插入 `videoPlayer.skip()` 优先处理。
5. **AnimationPlayer API**：通用入口 `play(target, preset, duration, callback)` + 专用方法 `screenShake(target, intensity, callback)` 和 `screenFlash(callback)`。

### Phase 4 注意事项

- 音频文件暂缺 (`assets/bgm/`, `assets/se/`, `assets/voice/` 目录为空)，引擎会静默跳过
- `Voice` 通道单个 `QMediaPlayer`，不叠加；`SE` 同理
- `QAudioOutput` 构造时需传入 parent (this)，否则 Qt 6.5 可能内存泄漏
- `VisualSnapshot` 暂不含音频状态，存档加载后不会恢复 BGM（Phase 5 可补）

---

## Phase 5 — 完成 ✅

Phase 5 目标：标题界面增强、CG 鉴赏、音乐鉴赏。**已全部实现。**

### 实现摘要

| 组件 | 文件 | 要点 |
|------|------|------|
| **GalleryManager** | `src/core/GalleryManager.h/.cpp` | CG/BGM 解锁追踪，持久化到 `saves/gallery.json` |
| **CGGalleryScreen** | `src/app/qml/screens/CGGalleryScreen.qml` | 8 个 demo CG 网格布局，🔒/🖼 状态，点击放大查看 |
| **MusicRoomScreen** | `src/app/qml/screens/MusicRoomScreen.qml` | 8 个 demo BGM 列表，▶/⏸ 播放控制，未解锁显示 🔒 |
| **TitleScreen 重写** | `src/app/qml/screens/TitleScreen.qml` | 新增 CG鉴赏/音乐鉴赏按钮、分隔线、`hasAnySave()` 绑定 |
| **unlock 指令** | `src/core/ScriptRunner.cpp` | 脚本 `["unlock", "cg", "id"]`/`["unlock", "bgm", "id"]` |
| **GameStateManager** | `src/core/GameStateManager.h/.cpp` | 新增 `hasAnySave()` 方法 |

### Phase 5 编码模式

1. **GalleryManager 持久化**：`saves/gallery.json` 存储 `{"cgs":[...], "bgms":[...]}`。`load()` 在构造函数调用，`save()` 在每次 `unlock` 后立即写入。文件路径按 executable dir / current dir / saves/ 优先级搜索。
2. **解锁机制**：BGM 自动解锁（ScriptRunner 中 `bgm` 指令执行后自动调 `GalleryManager::unlockBgm`），CG 手动解锁（脚本显式 `["unlock", "cg", "id"]`）。防止重复解锁：`QSet::contains` 检查。
3. **鉴赏 UI 数据驱动**：`cgCatalog`/`trackCatalog` 在 QML 中定义为 `property var` 数组，`GridView`/`ListView` 绑定 delegate。解锁状态通过 `GalleryManager.isCgUnlocked(id)` 实时查询。
4. **StackView 导航**：鉴赏界面作为独立 Component 通过 `sceneStack.push()` 进入，`back` 信号触发 `sceneStack.pop()` 返回。音乐鉴赏返回时自动 `AudioManager.stopBgm(200)` 停止预览。
5. **继续游戏绑定**：`enabled: GameStateManager.hasAnySave()` 替代之前的 `slotExists(98) || listSaves().length > 0`。

### Phase 5 注意事项

- CG/BGM 目录数据硬编码在 QML 的 `cgCatalog`/`trackCatalog` 中，未从外部文件加载（后续可迁移到 JSON 配置）
- 音乐鉴赏直接调用 `AudioManager.playBgm()`，返回标题后引擎不会自动恢复 BGM（开始新游戏/读档时会重新设置）
- `assets/cg/` 目录已创建但无实际图片，CG 缩略图显示占位符
- 解锁文件 `saves/gallery.json` 独立于存档槽位，跨所有存档共享

---

## Phase 6 — 完成 ✅

Phase 6 目标：多语言支持、资源打包、跨平台发布。

### 实现摘要

| 组件 | 文件 | 要点 |
|------|------|------|
| **LocalizationManager** | `src/core/LocalizationManager.h/.cpp` | 字典加载、`localeVersion` Q_PROPERTY 触发 QML 绑定刷新、`tr(key)` 查询 |
| **翻译字典** | `l10n/zh.json`, `l10n/en.json` | ~55 个 UI 键 + 21 个对话键 |
| **语言切换** | `SettingsScreen.qml` | ComboBox 绑定 `Settings.language`，`currentIndex` 实时反映 |
| **脚本 @key** | `ScriptRunner.cpp` | `say` 指令文本以 `@` 开头时通过 `LocalizationManager.tr()` 解析 |
| **资源打包** | `src/app/resources.qrc` | l10n 字典编译进二进制 (`:/l10n/`) |
| **scripts 迁移** | `scripts/` → `assets/scripts/` | 脚本纳入资源目录统一管理 |
| **许可合规** | `Release/NOTICE.txt` 等 | LGPLv3 动态链接合规文档 |

### Phase 6 编码模式

1. **LocalizationManager 架构**（Meyer's singleton + QObject）：
   - `Q_PROPERTY(int localeVersion READ localeVersion NOTIFY languageChanged)` —— "挂载属性"，用于触发 QML 绑定重求值（Bug #2 教训）
   - 构造时从 `Settings.language` 读取当前语言，连接 `SettingsManager::languageChanged` 信号自动重载字典
   - 字典加载优先级：Qt 资源 `:/l10n/{lang}.json` → 文件系统 `l10n/{lang}.json` → 当前目录
   - 在 main.cpp 注册为 `QML` 单例 `"L10n"`，QML 中通过 `import RinEngine.Core 1.0` 访问

2. **QML 本地化绑定模式**：
   ```qml
   function t(key) { L10n.localeVersion; return L10n.tr(key); }
   Text { text: t("TITLE_START") }
   ```
   `L10n.localeVersion` 访问建立绑定依赖，语言切换时 `languageChanged` 信号触发绑定链重计算。

3. **带参数字符串**：使用 JS `.replace("%1", arg)` 处理，如 `t("SAVE_EMPTY").replace("%1", index + 1)`。

4. **脚本对话 @key 约定**：`say` 指令 `args[1]` 以 `@` 开头时（如 `@DLG_WELCOME`），`ScriptRunner` 调用 `LocalizationManager::tr(key)` 解析。不以 `@` 开头则原样输出（向后兼容）。

5. **语言切换**：设置界面的 ComboBox `model: ["中文", "English"]`，`currentIndex` 绑定 `Settings.language`，`onCurrentIndexChanged` 写入语言代码 `"zh"`/`"en"`。语言名称（ComboBox 选项）不本地化——始终显示本语言名称。

### Phase 6 注意事项

- l10n 文件通过 `.qrc` 编译进二进制（`:l10n/`），发布时不需额外携带字典文件
- CG/BGM 目录数据（`cgCatalog`/`trackCatalog` 中的 `name`/`desc`）不本地化——这是内容，不是引擎 UI
- `assets.rsc` 资源打包系统留待 Phase 6.1/7 实现
- 分发目录 `D:\RinEngine\Release\` 包含 NOTICE.txt + LGPLv3 + GPLv3 许可文档，满足 Qt LGPL 动态链接合规要求
- Qt 6 模块使用 LGPLv3（非 v2.1），动态链接（DLL 在 Runtime/ 目录）满足 §4d1 合规路径
