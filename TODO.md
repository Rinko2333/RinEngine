# RinEngine 开发路线图

## 项目概述

基于 Qt6 + QML + C++ 的跨平台视觉小说引擎，目标用户是零编程基础的内容创作者。

**技术栈：**
- Qt6 Quick (QML) — UI 层
- C++ — 后端核心逻辑
- CMake — 构建系统
- JSON/YAML — 脚本格式（初期）

---

## Phase 0：项目基础设施

> 先把工程架子搭好，确保可编译、可调试、可扩展。

- [ ] **0.1 项目目录结构设计**
  - 确定源码目录 layout（见下方项目结构）
- [ ] **0.2 CMake 分层构建**
  - 将项目拆分为 `core`（C++ 静态库）和 `app`（可执行文件）
  - Qt6 Quick + Multimedia + Concurrent 模块依赖
- [ ] **0.3 日志系统**
  - `Logger` 单例，支持 LogLevel（Debug/Info/Warn/Error/Fatal）
  - 输出到控制台 + 文件（可选）
  - `QML_LOGGER` 暴露到 QML 供前端使用
- [ ] **0.4 配置系统**
  - `SettingsManager` — 基于 QSettings 的全局配置
  - 窗口大小、语言、音量等持久化存储
- [ ] **0.5 应用基础窗口框架**
  - 主窗口 + 基本 Layer 层级结构（BG -> Character -> Effect -> Dialogue -> Menu）

### 项目目录结构

```
RinEngine/
├── CMakeLists.txt              # 顶层 CMake
├── src/
│   ├── core/                   # C++ 核心库（静态库）
│   │   ├── CMakeLists.txt
│   │   ├── Logger.h/cpp
│   │   ├── SettingsManager.h/cpp
│   │   └── ...
│   ├── app/                    # 可执行文件入口
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   └── main.qml
│   └── ui/                     # QML 界面组件
│       ├── qmldir
│       ├── screens/
│       │   ├── TitleScreen.qml
│       │   └── GameScreen.qml
│       ├── layers/
│       │   ├── LayerStack.qml
│       │   ├── BackgroundLayer.qml
│       │   ├── CharacterLayer.qml
│       │   └── DialogueLayer.qml
│       └── widgets/
│           ├── DialogueBox.qml
│           └── ChoiceBox.qml
├── assets/                     # 资源文件夹
│   ├── bg/
│   ├── char/
│   ├── bgm/
│   ├── se/
│   ├── voice/
│   └── video/
├── scripts/                    # 剧情脚本
│   └── demo/
└── build/                      # 构建目录
```

---

## Phase 1：核心渲染 — 让画面动起来

> 目标是：显示背景、立绘、对话框、文字，点击鼠标推进剧情。

### 1.1 资源管理系统

- [ ] **1.1.1 `ResourceManager`（C++）**
  - 统一管理所有游戏资源（图片、音频、视频等）
  - 提供 `load(path) -> QVariant` / `preload(paths)` / `release(path)` 接口
  - 缓存机制（`QHash<QString, QSharedPointer<...>>`）
  - **预加载支持** — 场景切换时异步预加载下一场景资源
- [ ] **1.1.2 资源路径约定**
  - 基于 `assets/` 的相对路径寻址
  - 通过 `ResourceManager::resolve("bg:school")` 解引用
- [ ] **1.1.3 QML 绑定**
  - `ResourceManager` 注册为 QML 单例
  - 提供 `getImage(path)` 返回 `QImage`/`QPixmap`

### 1.2 背景系统

- [ ] **1.2.1 `BackgroundLayer`**
  - QML 层，全屏显示当前背景图片
  - 支持 `crossfade` 淡入淡出切换（`PropertyAnimation`）
- [ ] **1.2.2 背景切换动画**
  - 预设 Transition 效果：淡入淡出、从黑场切入、滑动

### 1.3 立绘系统

- [ ] **1.3.1 `CharacterLayer`**
  - 支持多个角色同时显示（最多 4-6 个位置）
  - 每个角色有预设位置：左、中左、中、中右、右
- [ ] **1.3.2 立绘状态**
  - 每个角色支持多种表情/姿态，通过 `CharacterState` 切换
  - 切换时支持交叉淡入淡出
- [ ] **1.3.3 立绘运动**
  - 支持 `moveTo(position, duration)` 动画移动
  - 支持 `fadeIn`/`fadeOut`/`slideIn` 入场效果

### 1.4 对话框系统

- [ ] **1.4.1 `DialogueBox`**
  - 底部半透明文本框
  - 显示：说话者名字 + 对话文本
  - **逐字显示效果（Typewriter Effect）**
  - 点击跳过逐字动画，直接显示完整文本
- [ ] **1.4.2 对话历史**
  - `DialogueHistory` 存储已显示的对话
  - QML `ListView` 展示历史记录
  - 支持从历史回跳（可选）
- [ ] **1.4.3 自动播放模式**
  - 每句显示完后等待设定秒数自动推进
  - 玩家可随时点击中断

---

## Phase 2：脚本引擎 — 让剧情活起来

> 目标是：解析脚本文件，驱动整个游戏流程。

### 2.1 脚本解析器

- [ ] **2.1.1 脚本格式设计**
  - 第一版使用 JSON 格式（易于调试）
  - 每条指令是一个结构化的命令对象
  - 示例：
    ```json
    [
      { "cmd": "bg", "value": "school", "transition": "fade", "duration": 1.0 },
      { "cmd": "char", "name": "alice", "face": "happy", "position": "center" },
      { "cmd": "say", "speaker": "alice", "text": "早上好！" },
      { "cmd": "bgm", "value": "morning", "volume": 80 },
      { "cmd": "choice", "options": [
          { "text": "去天台", "jump": "route_a", "condition": "affection > 20" },
          { "text": "回家", "jump": "route_b" }
      ]}
    ]
    ```
- [ ] **2.1.2 `ScriptParser`（C++）**
  - 读取 JSON 脚本文件 → 解析为 `QVector<ScriptCommand>`
  - 指令验证（参数完整性检查）
  - 错误报告（行号 + 错误信息）

### 2.2 脚本执行器

- [ ] **2.2.1 `ScriptRunner`（C++）**
  - 指令队列：顺序执行 `ScriptCommand`
  - **Wait 机制**：播放动画/语音时等待完成再执行下一条
  - 支持 `jump` 跳转到指定标签
  - 支持 `call`/`return` 子程序调用
- [ ] **2.2.2 标签系统**
  - 脚本中的锚点标签：`# scene_01`
  - `jump` 和 `call` 通过标签定位

### 2.3 剧情变量系统

- [ ] **2.3.1 `VariableManager`（C++）**
  - 持久化变量存储（整数、浮点数、字符串、布尔值、字符串数组）
  - 变量的读取/写入/运算
- [ ] **2.3.2 条件分支**
  - 脚本中支持 `if` 条件判断
  - 支持比较运算：`==`, `!=`, `>`, `<`, `>=`, `<=`
  - 支持逻辑运算：`&&`, `||`, `!`
- [ ] **2.3.3 修改变量指令**
  - `set var = value`
  - `add var += n`
  - `mul var *= n`

### 2.4 选择系统

- [ ] **2.4.1 `ChoiceBox`（QML）**
  - 选项按钮组
  - 支持最多 6 个选项
  - 选中高亮 + hover 效果
- [ ] **2.4.2 条件选项**
  - 选项的 `condition` 不满足时灰显/隐藏
- [ ] **2.4.3 限时选项**
  - 倒计时显示，超时自动选择默认项/进入默认路线
- [ ] **2.4.4 隐藏选项**
  - 显示条件满足后才会出现的选项

---

## Phase 3：游戏系统 — 完整的 VN 体验

> 目标是：存档、读档、设置、鉴赏等完整游戏功能。

### 3.1 存档系统

- [ ] **3.1.1 `SaveManager`（C++）**
  - 序列化游戏状态（当前脚本、变量、已显示的对话等）
  - 多存档位支持（至少 100 个）
  - 快速存档/快速读档（2-3 个专用槽位）
  - 自动存档（场景切换时自动保存）
- [ ] **3.1.2 存档 UI**
  - 存档页：显示缩略图、时间戳、场景名
  - 读档页：同上，点击读档
  - 存档/读档预览图
- [ ] **3.1.3 序列化格式**
  - 使用 JSON 或 Qt 的 `QDataStream` 二进制格式
  - 包含：当前脚本文件 + 指令索引 + 所有变量 + UI 状态

### 3.2 回滚系统（Rollback）

- [ ] **3.2.1 状态快照**
  - 每执行完一条"对话指令"后保存快照
  - 快照内容：变量 + 脚本位置 + 画面状态
- [ ] **3.2.2 Rollback 操作**
  - 鼠标滚轮 / ↑ 键回退到上一句
  - 回退后恢复完整的画面状态
  - Rollback 期间禁用存档

### 3.3 快进系统

- [ ] **3.3.1 快进模式**
  - 跳过已读文本（基于对话历史判断）
  - 快进速度：5x / 10x / 跳过
  - 遇到选项时自动暂停
- [ ] **3.3.2 未读/已读追踪**
  - 记录每条对话是否已被玩家阅读过
  - 存储到 `SaveManager`

### 3.4 系统菜单

- [ ] **3.4.1 游戏内菜单**
  - 保存 / 读取 / 设置 / 标题画面 / 退出
  - 覆盖层显示，不中断游戏状态
- [ ] **3.4.2 设置菜单**
  - 文字速度（滑块）
  - 自动播放速度（滑块）
  - BGM 音量、SE 音量、语音音量
  - 全屏开关
  - 语言选择（预留）
- [ ] **3.4.3 右键菜单**
  - 快速操作：保存、读档、设置、隐藏文字

---

## Phase 4：多媒体与演出

> 目标是：音频管理、视频播放、演出动画。

### 4.1 音频系统

- [x] **4.1.1 `AudioManager`（C++）**
  - 基于 Qt Multimedia
  - 三个独立通道：BGM、SE、Voice
  - 交叉淡入淡出切换 BGM
  - 音量控制（每个通道独立）
- [x] **4.1.2 音频指令**
  - `play_bgm`, `stop_bgm`, `fadeout_bgm`
  - `play_se`（音效，可叠加）
  - `play_voice`（语音，独占 Voice 通道）
- [x] **4.1.3 QML 绑定**
  - 设置界面音量滑块直连 `AudioManager`

### 4.2 视频播放

- [x] **4.2.1 视频播放器**
  - 基于 Qt Multimedia `VideoOutput`
  - 全屏播放 OP/ED/过场动画
  - 支持跳过（点击/按 ESC）
  - 播放结束后返回游戏

### 4.3 演出动画系统

- [x] **4.3.1 补间动画引擎**
  - 基于 QML `Behavior` + `Animation`
  - 支持的动画类型：`move`, `fade`, `scale`, `rotate`, `shake`, `color`
  - 缓动函数：`linear`, `easeIn`, `easeOut`, `easeInOut`, `bounce`
- [x] **4.3.2 屏幕特效**
  - 全屏闪白（`screenflash`）
  - 画面震动（`screenShake`）
  - 画面色调变化（通过 `ColorOverlay`）

---

## Phase 5：标题与鉴赏

> 目标是：完整的标题界面、CG 和音乐鉴赏。

### 5.1 标题界面

- [x] **5.1.1 `TitleScreen`（QML）**
  - 背景 + 标题 logo + 主菜单按钮
  - 开始游戏、继续游戏、CG鉴赏、音乐鉴赏、设置、退出
  - 继续游戏按钮仅在存在存档时可用

### 5.2 CG 鉴赏

- [x] **5.2.1 `CGGallery`**
  - 解锁条件：在游戏中触发过对应场景
  - 网格展示缩略图（已解锁/未解锁状态）
  - 点击查看大图 + 场景名
- [x] **5.2.2 解锁持久化**
  - 解锁状态存入 `GalleryManager`（`saves/gallery.json`）

### 5.3 音乐鉴赏

- [x] **5.3.1 `MusicRoom`**
  - 解锁条件：在游戏中播放过对应曲目
  - 播放列表界面 + 播放/暂停控制
  - 显示曲名、时长

---

## Phase 6：国际化与发布

> 目标是：多语言支持、资源打包、跨平台发布。

### 6.1 多语言系统

- [ ] **6.1.1 `LocalizationManager`（C++）**
  - 文本 ID -> 多语言映射（JSON 格式字典）
  - 运行时切换语言
  - Qt `qsTr()` 集成
- [ ] **6.1.2 `l10n/` 目录结构**
  ```
  l10n/
  ├── zh.json    # { "SYS_001": "开始游戏", ... }
  ├── en.json    # { "SYS_001": "Start Game", ... }
  └── ja.json    # { "SYS_001": "ゲーム開始", ... }
  ```
- [ ] **6.1.3 脚本多语言方案**
  - 脚本中的文本使用 ID 引用
  - 运行时通过 `LocalizationManager` 获取当前语言文本

### 6.2 资源打包

- [ ] **6.2.1 Qt Resources 集成**
  - 使用 `.qrc` 管理内置资源
  - 支持外部资源包加载（导出为 ZIP）
- [ ] **6.2.2 `arc` 打包工具**
  - 命令行工具，将 `assets/` 打包为二进制资源包
  - 支持加密（可选的）

### 6.3 跨平台发布

- [ ] **6.3.1 平台适配**
  - Windows（MSVC/MinGW）
  - macOS（通过 Qt for macOS）
  - Linux（AppImage / Flatpak）
  - Android / iOS（长远目标）
- [ ] **6.3.2 一键导出工具**
  - 将脚本 + 资源 + 运行时打包为可分发的游戏包

---

## Phase 7：创作者工具（长远）

> 目标是：让零编程基础的用户也能创作游戏。

- [ ] **7.1 可视化场景编辑器**
  - 拖拽式场景编辑：背景、角色、对话
  - 实时预览
- [ ] **7.2 脚本编辑器**
  - 语法高亮
  - 场景树 / 导航
  - 错误检查 + 预览
- [ ] **7.3 资源管理工具**
  - 项目创建向导
  - 资源导入 / 裁剪 / 管理
- [ ] **7.4 发布工具**
  - 一键打包发布
  - Steam / Itch.io 集成

---

## 并发路径

Phase 1-3 是**核心引擎**的核心功能链，建议串行完成。

Phase 4-5 可以视需要进行。

Phase 6-7 属于增强功能，在核心引擎稳定后开发。

---

## 当前状态

- [x] Phase 0-3: 核心引擎（渲染、脚本、存档） ✓
- [x] Phase 4: 多媒体与演出（音频、视频、动画） ✓
- [x] Phase 5: 标题与鉴赏（TitleScreen、CG、音乐） ✓
- [ ] Phase 6: 国际化与发布 — 待开发
