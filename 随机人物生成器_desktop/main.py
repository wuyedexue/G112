# -*- coding: utf-8 -*-
"""
随机人物生成器 - Python桌面版
基于职业收集数据库，随机组合生成虚拟人物档案
使用 PyQt6 构建 UI
"""

import sys
import random
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QGridLayout, QFrame, QScrollArea, QSizePolicy
)
from PyQt6.QtCore import Qt, QPropertyAnimation, QEasingCurve
from PyQt6.QtGui import QFont, QColor, QPalette, QLinearGradient, QPainter

from data import (
    SURNAMES, MALE_NAMES, FEMALE_NAMES, OCCUPATIONS,
    PERSONALITY_BASIC, PERSONALITY_ENHANCED, PERSONALITY_INTERPERSONAL,
    PERSONALITY_DECISION, LIVING_HABITS, BAD_HABITS, FAMILY_TYPES, FAMILY_ISSUES
)


class GradientWidget(QWidget):
    """渐变背景组件"""
    def paintEvent(self, event):
        painter = QPainter(self)
        gradient = QLinearGradient(0, 0, self.width(), self.height())
        gradient.setColorAt(0, QColor(45, 27, 105))
        gradient.setColorAt(1, QColor(17, 153, 142))
        painter.fillRect(self.rect(), gradient)


class CardItem(QFrame):
    """单个人物属性卡片项"""
    def __init__(self, label_text, parent=None):
        super().__init__(parent)
        self.setStyleSheet("""
            QFrame {
                background: rgba(255, 255, 255, 0.8);
                border-radius: 8px;
                border: 1px solid #e2e8f0;
                padding: 8px;
            }
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 8, 12, 8)
        layout.setSpacing(4)

        self.label = QLabel(label_text)
        self.label.setStyleSheet("""
            QLabel {
                font-size: 11px;
                font-weight: bold;
                color: #8b5cf6;
                letter-spacing: 1px;
                background: transparent;
                border: none;
            }
        """)
        layout.addWidget(self.label)

        self.value = QLabel("-")
        self.value.setWordWrap(True)
        self.value.setStyleSheet("""
            QLabel {
                font-size: 14px;
                color: #1a1a2e;
                font-weight: 500;
                line-height: 1.6;
                background: transparent;
                border: none;
            }
        """)
        layout.addWidget(self.value)

    def set_value(self, text):
        self.value.setText(text)


class HighlightCardItem(QFrame):
    """高亮样式的卡片项（用于姓名）"""
    def __init__(self, label_text, parent=None):
        super().__init__(parent)
        self.setStyleSheet("""
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #667eea, stop:1 #764ba2);
                border-radius: 8px;
                padding: 8px;
            }
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 8, 12, 8)
        layout.setSpacing(4)

        self.label = QLabel(label_text)
        self.label.setStyleSheet("""
            QLabel {
                font-size: 11px;
                font-weight: bold;
                color: rgba(255, 255, 255, 0.8);
                letter-spacing: 1px;
                background: transparent;
                border: none;
            }
        """)
        layout.addWidget(self.label)

        self.value = QLabel("点击下方按钮生成")
        self.value.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.value.setStyleSheet("""
            QLabel {
                font-size: 20px;
                color: white;
                font-weight: bold;
                background: transparent;
                border: none;
            }
        """)
        layout.addWidget(self.value)

    def set_value(self, text):
        self.value.setText(text)


class HistoryItem(QFrame):
    """历史记录项"""
    def __init__(self, text, parent=None):
        super().__init__(parent)
        self.setStyleSheet("""
            QFrame {
                background: #f8f9ff;
                border-radius: 6px;
                border: 1px solid #e8ecf4;
                padding: 6px 10px;
            }
            QFrame:hover {
                background: #eef2ff;
                border-color: #c7d2fe;
            }
        """)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        label = QLabel(text)
        label.setStyleSheet("font-size: 12px; color: #555; background: transparent; border: none;")
        layout.addWidget(label)


class MainWindow(QMainWindow):
    """主窗口"""
    def __init__(self):
        super().__init__()
        self.setWindowTitle("随机人物生成器")
        self.setMinimumSize(850, 700)
        self.resize(850, 800)

        self.generate_count = 0
        self.history = []

        self.setup_ui()

    def setup_ui(self):
        # 主背景
        central = GradientWidget()
        self.setCentralWidget(central)

        # 外层布局
        outer_layout = QVBoxLayout(central)
        outer_layout.setContentsMargins(30, 30, 30, 30)
        outer_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # 滚动区域
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setStyleSheet("""
            QScrollArea {
                background: transparent;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background: transparent;
            }
        """)

        # 内容容器
        container = QWidget()
        container.setStyleSheet("""
            QWidget {
                background: rgba(255, 255, 255, 0.97);
                border-radius: 20px;
            }
        """)
        container.setMaximumWidth(800)

        container_layout = QVBoxLayout(container)
        container_layout.setContentsMargins(40, 40, 40, 40)
        container_layout.setSpacing(16)

        # 标题
        title = QLabel("随机人物生成器")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title.setStyleSheet("""
            QLabel {
                font-size: 26px;
                font-weight: bold;
                color: #2d1b69;
                letter-spacing: 2px;
                background: transparent;
            }
        """)
        container_layout.addWidget(title)

        subtitle = QLabel("基于职业收集数据库，随机组合生成虚拟人物档案")
        subtitle.setAlignment(Qt.AlignmentFlag.AlignCenter)
        subtitle.setStyleSheet("""
            QLabel {
                font-size: 13px;
                color: #666;
                margin-bottom: 16px;
                background: transparent;
            }
        """)
        container_layout.addWidget(subtitle)

        # 人物卡片区
        card_frame = QFrame()
        card_frame.setStyleSheet("""
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #f8f9ff, stop:1 #f0f4ff);
                border-radius: 16px;
                border: 1px solid #e8ecf4;
                padding: 12px;
            }
        """)
        card_layout = QVBoxLayout(card_frame)
        card_layout.setSpacing(12)
        card_layout.setContentsMargins(16, 16, 16, 16)

        # 第一行：姓名 + 性别
        row1 = QHBoxLayout()
        self.name_item = HighlightCardItem("姓 名")
        self.gender_item = CardItem("性 别")
        row1.addWidget(self.name_item, 2)
        row1.addWidget(self.gender_item, 1)
        card_layout.addLayout(row1)

        # 第二行：年龄 + 职业
        row2 = QHBoxLayout()
        self.age_item = CardItem("年 龄")
        self.occupation_item = CardItem("职 业")
        row2.addWidget(self.age_item)
        row2.addWidget(self.occupation_item)
        card_layout.addLayout(row2)

        # 性格
        self.personality_item = CardItem("性 格")
        card_layout.addWidget(self.personality_item)

        # 生活习惯
        self.habits_item = CardItem("生活习惯")
        card_layout.addWidget(self.habits_item)

        # 不良习惯
        self.bad_habits_item = CardItem("不良习惯")
        card_layout.addWidget(self.bad_habits_item)

        # 家庭关系
        self.family_item = CardItem("家庭关系及问题")
        card_layout.addWidget(self.family_item)

        container_layout.addWidget(card_frame)

        # 生成按钮
        btn_layout = QHBoxLayout()
        btn_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.btn_generate = QPushButton("🎲  随机生成")
        self.btn_generate.setFixedSize(220, 50)
        self.btn_generate.setCursor(Qt.CursorShape.PointingHandCursor)
        self.btn_generate.setStyleSheet("""
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #667eea, stop:1 #764ba2);
                color: white;
                border: none;
                font-size: 17px;
                font-weight: bold;
                border-radius: 25px;
                letter-spacing: 2px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #5a6fdf, stop:1 #6a3f97);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #4e5fd4, stop:1 #5e358c);
            }
        """)
        self.btn_generate.clicked.connect(self.generate)
        btn_layout.addWidget(self.btn_generate)
        container_layout.addLayout(btn_layout)

        # 计数器
        self.counter_label = QLabel("已生成 0 个人物")
        self.counter_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.counter_label.setStyleSheet("""
            QLabel {
                font-size: 12px;
                color: #999;
                margin-top: 8px;
                background: transparent;
            }
        """)
        container_layout.addWidget(self.counter_label)

        # 历史记录区
        self.history_frame = QFrame()
        self.history_frame.setStyleSheet("""
            QFrame {
                border-top: 2px solid #e8ecf4;
                padding-top: 16px;
                background: transparent;
            }
        """)
        self.history_frame.setVisible(False)

        history_layout = QVBoxLayout(self.history_frame)
        history_layout.setContentsMargins(0, 16, 0, 0)

        history_header = QHBoxLayout()
        history_title = QLabel("历史记录")
        history_title.setStyleSheet("font-size: 14px; color: #666; background: transparent;")
        history_header.addWidget(history_title)

        btn_clear = QPushButton("清空")
        btn_clear.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_clear.setStyleSheet("""
            QPushButton {
                font-size: 11px;
                color: #999;
                padding: 3px 10px;
                border-radius: 10px;
                background: #f0f0f0;
                border: none;
            }
            QPushButton:hover {
                background: #e0e0e0;
                color: #666;
            }
        """)
        btn_clear.clicked.connect(self.clear_history)
        history_header.addStretch()
        history_header.addWidget(btn_clear)
        history_layout.addLayout(history_header)

        self.history_list_layout = QVBoxLayout()
        self.history_list_layout.setSpacing(6)
        history_layout.addLayout(self.history_list_layout)

        container_layout.addWidget(self.history_frame)

        scroll.setWidget(container)
        outer_layout.addWidget(scroll)

    def generate(self):
        """生成随机人物"""
        gender = random.choice(["男", "女"])
        name = self._generate_name(gender)
        age = self._generate_age()
        occupation = random.choice(OCCUPATIONS)
        personality = self._generate_personality()
        habits = self._generate_living_habits()
        bad_habits = self._generate_bad_habits()
        family = self._generate_family()

        # 更新UI
        self.name_item.set_value(name)
        self.gender_item.set_value(gender)
        self.age_item.set_value(f"{age} 岁")
        self.occupation_item.set_value(occupation)
        self.personality_item.set_value(personality)
        self.habits_item.set_value(habits)
        self.bad_habits_item.set_value(bad_habits)
        self.family_item.set_value(family)

        # 更新计数
        self.generate_count += 1
        self.counter_label.setText(f"已生成 {self.generate_count} 个人物")

        # 添加历史记录
        record = f"#{self.generate_count}  {name}（{gender}，{age}岁）- {occupation}"
        self.history.insert(0, record)
        if len(self.history) > 10:
            self.history.pop()
        self._update_history()

    def _generate_name(self, gender):
        """生成姓名"""
        surname = random.choice(SURNAMES)
        name_pool = MALE_NAMES if gender == "男" else FEMALE_NAMES
        if random.random() < 0.4:
            return surname + random.choice(name_pool)
        else:
            n1 = random.choice(name_pool)
            n2 = random.choice(name_pool)
            while n2 == n1:
                n2 = random.choice(name_pool)
            return surname + n1 + n2

    def _generate_age(self):
        """生成年龄（正态分布偏向25-45）"""
        base = random.random() + random.random() + random.random()
        age = round(18 + base / 3 * 47)
        return max(18, min(65, age))

    def _generate_personality(self):
        """生成性格描述"""
        basic = random.choice(PERSONALITY_BASIC)
        enhanced = random.choice(PERSONALITY_ENHANCED)
        interpersonal = random.choice(PERSONALITY_INTERPERSONAL)
        decision = random.choice(PERSONALITY_DECISION)
        return (
            f"【{basic['trait']}】{basic['desc']}；"
            f"工作中逐渐变得【{enhanced['trait']}】{enhanced['desc']}；"
            f"人际风格偏【{interpersonal['trait']}】{interpersonal['desc']}；"
            f"决策风格：{decision['trait']} - {decision['desc']}。"
        )

    def _generate_living_habits(self):
        """生成生活习惯"""
        count = random.randint(2, 3)
        selected = random.sample(LIVING_HABITS, count)
        return "；".join(f"【{h['habit']}】{h['desc']}" for h in selected) + "。"

    def _generate_bad_habits(self):
        """生成不良习惯"""
        count = random.randint(1, 2)
        selected = random.sample(BAD_HABITS, count)
        return "；".join(f"【{h['habit']}】{h['desc']}" for h in selected) + "。"

    def _generate_family(self):
        """生成家庭关系"""
        family_type = random.choice(FAMILY_TYPES)
        issue = random.choice(FAMILY_ISSUES)
        return f"家庭结构：{family_type['type']} - {family_type['desc']}。主要问题：{issue}"

    def _update_history(self):
        """更新历史记录显示"""
        # 清除旧的
        while self.history_list_layout.count():
            child = self.history_list_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()

        if not self.history:
            self.history_frame.setVisible(False)
            return

        self.history_frame.setVisible(True)
        for record in self.history:
            item = HistoryItem(record)
            self.history_list_layout.addWidget(item)

    def clear_history(self):
        """清空历史"""
        self.history.clear()
        self._update_history()


def main():
    app = QApplication(sys.argv)

    # 设置全局字体
    font = QFont("Microsoft YaHei", 10)
    app.setFont(font)

    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
