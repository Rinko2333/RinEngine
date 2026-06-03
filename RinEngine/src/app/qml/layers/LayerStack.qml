import QtQuick
import QtQuick.Controls

// LayerStack: 层叠架构，负责管理游戏渲染层
// 图层顺序（从下到上）:
//   0. BackgroundLayer  — 背景
//   1. CharacterLayer   — 立绘
//   2. EffectLayer      — 特效（预留）
//   3. DialogueLayer    — 对话框
//   4. MenuLayer        — 菜单层（预留）

Item {
    id: root

    default property alias content: contentArea.children

    Item {
        id: contentArea
        anchors.fill: parent
    }
}
