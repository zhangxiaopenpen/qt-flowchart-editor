#include "mainwindow.h"
#include <QMessageBox>
#include <QMenu>
#include <QTimer>
#include <QScrollBar>
#include <QFileDialog>
#include <QPageSize>
#include <QComboBox>

// No UI header needed

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Set window size to 2000x1600
    this->resize(2000, 1600);
    this->setMinimumSize(2000, 1600);

    // 初始化多选相关变量
    isMultiSelecting = false;

    // 初始化框选相关变量
    isRectSelecting = false;
    selectionRect = QRect();
    selectStartPoint = QPoint(0, 0);

    // 设置窗口整体样式
    this->setStyleSheet(
        "QMainWindow {"
        "   background-color: #f5f5f5;"  // 莫兰迪米白色背景
        "}"
    );

    // Initialize default settings
    backgroundColor = Qt::white;
    pageWidth = 1200;
    pageHeight = 1000;
    showGrid = false;
    currentGridSize = None;
    currentTool = "Select";
    alignmentOption = "Left";
    canvasPosition = QPoint(400, 300);
    isDragging = false;
    isMoving = false;
    moveStartPoint = QPoint(0, 0);
    moveShapeIndex = -1;
    lastMousePos = QPoint(0, 0);

    // 初始化绘图相关变量
    currentShape = nullptr;
    isDrawing = false;
    startPoint = QPoint(0, 0);
    endPoint = QPoint(0, 0);
    selectedShapeIndex = -1;

    // 初始化撤销和重做栈
    undoStack.clear();
    redoStack.clear();
    saveState();

    // 创建主布局容器
    QWidget* mainContainer = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建顶部菜单栏
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setAllowedAreas(Qt::TopToolBarArea);
    toolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #2c3e50;"
        "   border-radius: 0px;"
        "   margin: 0px;"
        "   padding: 5px;"
        "}"
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   color: #ecf0f1;"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #34495e;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:checked {"
        "   background-color: #3498db;"  // 选中状态使用亮蓝色
        "   color: #ffffff;"  // 选中状态文字颜色更亮
        "   border-bottom: 3px solid #2980b9;"  // 添加底部边框
        "}"
    );
    this->addToolBar(toolbar);

    // 创建菜单按钮容器
    QWidget* menuWidget = new QWidget(toolbar);
    QHBoxLayout* menuLayout = new QHBoxLayout(menuWidget);
    menuLayout->setContentsMargins(0, 0, 0, 0);

    // 添加菜单按钮
    startBtn = new QPushButton("开始", this);
    arrangeBtn = new QPushButton("排列", this);
    pageBtn = new QPushButton("页面", this);
    exportBtn = new QPushButton("导出", this);

    // 设置按钮可选中
    startBtn->setCheckable(true);
    arrangeBtn->setCheckable(true);
    pageBtn->setCheckable(true);
    exportBtn->setCheckable(true);

    startBtn->setMinimumSize(120, 40);
    arrangeBtn->setMinimumSize(120, 40);
    pageBtn->setMinimumSize(120, 40);
    exportBtn->setMinimumSize(120, 40);

    menuLayout->addStretch();
    menuLayout->addWidget(startBtn);
    menuLayout->addWidget(arrangeBtn);
    menuLayout->addWidget(pageBtn);
    menuLayout->addWidget(exportBtn);
    menuLayout->addStretch();

    toolbar->addWidget(menuWidget);

    // 创建功能工具栏容器
    secondaryToolbarWidget = new QWidget(this);
    secondaryToolbarWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #d8e1e9;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
    );
    secondaryToolbarLayout = new QVBoxLayout(secondaryToolbarWidget);
    secondaryToolbarLayout->setContentsMargins(5, 5, 5, 5);
    secondaryToolbarLayout->setSpacing(5);

    // 创建功能工具栏
    createStartToolbar();
    createArrangeToolbar();
    createPageToolbar();
    createExportToolbar();  // 添加创建Export工具栏
    secondaryToolbarLayout->addWidget(startToolbar);
    secondaryToolbarLayout->addWidget(arrangeToolbar);
    secondaryToolbarLayout->addWidget(pageToolbar);
    secondaryToolbarLayout->addWidget(exportToolbar);  // 添加Export工具栏到布局
 
    // 将功能工具栏添加到主布局
    mainLayout->addWidget(secondaryToolbarWidget);

    // 创建中间内容区域容器
    QWidget* contentContainer = new QWidget(this);
    QHBoxLayout* contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建左侧工具栏
    createLeftToolbar();
    contentLayout->addWidget(leftToolbar);

    // 创建画布区域
    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: #e6e6e6;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #f0f0f0;"
        "   width: 10px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #c0c0c0;"
        "   border-radius: 5px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );

    canvasWidget = new QWidget(scrollArea);
    canvasWidget->setMinimumSize(3000, 2000);
    canvasWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "   border-radius: 10px;"
        "}"
    );
    scrollArea->setWidget(canvasWidget);

    // 计算并设置画布的初始位置，使其居中显示
    QTimer::singleShot(0, this, [this]() {
        QSize scrollAreaSize = scrollArea->viewport()->size();
        int x = (scrollAreaSize.width() - pageWidth * scaleFactor) / 2;
        int y = (scrollAreaSize.height() - pageHeight * scaleFactor) / 2;
        canvasPosition = QPoint(x, y);

        // 设置滚动条位置
        QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
        if (vScrollBar) {
            int maxScroll = vScrollBar->maximum();
            vScrollBar->setValue(maxScroll / 3);  // 设置滚动条到中间位置
        }

        canvasWidget->update();
        });

    // 创建属性面板
    createPropertyPanel();

    // 创建分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);  // 防止完全折叠
    splitter->setHandleWidth(5);  // 设置分割线宽度
    splitter->setStyleSheet(
        "QSplitter::handle {"
        "   background-color: #d8e1e9;"
        "}"
        "QSplitter::handle:hover {"
        "   background-color: #a8b8c8;"
        "}"
    );

    // 将画布和属性面板添加到分割器
    splitter->addWidget(scrollArea);
    splitter->addWidget(propertyPanel);

    // 设置分割器的初始大小
    splitter->setSizes(QList<int>() << 1500 << 300);

    contentLayout->addWidget(splitter);

    // 将内容区域添加到主布局
    mainLayout->addWidget(contentContainer);

    // 设置主窗口的中心部件
    setCentralWidget(mainContainer);

    // 初始显示Start工具栏
    hideAllSecondaryToolbars();
    startToolbar->show();

    setWindowTitle("Drawing Application");

    // Install event filter for mouse events
    canvasWidget->installEventFilter(this);
    canvasWidget->setMouseTracking(true);

    // 初始化缩放因子
    scaleFactor = 1.0;

    // 在 MainWindow 类的私有成员中添加复制缓冲区
    clipboardShape = nullptr;


    // 初始化拖拽相关变量
    isDraggingShape = false;
    draggedShapeType = "";
    dragStartPoint = QPoint(0, 0);
    dragCurrentPoint = QPoint(0, 0);
    draggedButton = nullptr;
    draggedPixmap = QPixmap();
    dragOffset = QPoint(0, 0);


    // 连接按钮信号
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(arrangeBtn, &QPushButton::clicked, this, &MainWindow::onArrangeClicked);
    connect(pageBtn, &QPushButton::clicked, this, &MainWindow::onPageClicked);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportClicked);

    // 创建File按钮
    fileBtn = new QPushButton(this);
    fileBtn->setIcon(QIcon(":/image/File.png"));
    fileBtn->setIconSize(QSize(24, 24));
    fileBtn->setFixedSize(120, 40);  // 增加按钮宽度以容纳文字
    fileBtn->setToolTip("文件");

    // 设置按钮样式，添加文字
    fileBtn->setStyleSheet(
        "QPushButton {"
        "    text-align: left;"
        "    padding-left: 8px;"
        "    font-size: 24px;"
        "    color: white;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.1);"
        "}"
    );
    fileBtn->setText("文件");

    connect(fileBtn, &QPushButton::clicked, this, &MainWindow::onFileClicked);

    // 将File按钮添加到工具栏最左侧
    toolbar->insertWidget(0, fileBtn);

    // 为画布添加事件过滤器
    canvasWidget->installEventFilter(this);

    // 初始化内嵌文本编辑相关变量
    inlineTextEdit = new QTextEdit(canvasWidget);
    inlineTextEdit->hide();
    inlineTextEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: white;"
        "   border: 2px solid #0078D7;"
        "   border-radius: 4px;"
        "   padding: 2px;"
        "}"
    );
    editingShapeIndex = -1;
    isEditingText = false;

    // 连接文本编辑器的信号
    connect(inlineTextEdit, &QTextEdit::textChanged, [this]() {
        // 当文本改变时，调整编辑器大小以适应内容
        QTextDocument* doc = inlineTextEdit->document();
        QSizeF size = doc->size();
        QRect geometry = inlineTextEdit->geometry();
        geometry.setHeight(qMin(size.height() + 4, 200.0)); // 限制最大高度
        inlineTextEdit->setGeometry(geometry);
        });

    // 添加事件过滤器来处理ESC键
    inlineTextEdit->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    if (clipboardShape) {
        delete clipboardShape;
    }
}

// 实现检查点是否在形状内的方法
bool MainWindow::shapeContainsPoint(const Shape& shape, const QPoint& point)
{
    if (shape.type == "Rectangle" || shape.type == "Circle") {
        return shape.rect.contains(point);
    }
    else if (shape.type == "Triangle") {
        QPolygon triangle;
        triangle << QPoint(shape.rect.left() + shape.rect.width() / 2, shape.rect.top())
            << QPoint(shape.rect.right(), shape.rect.bottom())
            << QPoint(shape.rect.left(), shape.rect.bottom());
        return triangle.containsPoint(point, Qt::OddEvenFill);
    }
    else if (shape.type == "Arrow") {
        // 对于箭头，我们将其视为线段，而不是矩形
        // 计算点到线段的距离，如果小于阈值，则认为点在箭头上
        QLineF line(shape.rect.topLeft(), shape.rect.bottomRight());

        // 计算点到线段的距离
        QPointF v = line.p2() - line.p1();
        QPointF w = point - line.p1();

        double c1 = QPointF::dotProduct(w, v);
        if (c1 <= 0) {
            return QLineF(point, line.p1()).length() < 10; // 距离起点10像素以内
        }

        double c2 = QPointF::dotProduct(v, v);
        if (c2 <= c1) {
            return QLineF(point, line.p2()).length() < 10; // 距离终点10像素以内
        }

        double b = c1 / c2;
        QPointF pb = line.p1() + b * v;
        return QLineF(point, pb).length() < 10; // 距离线段10像素以内
    }
    else if (shape.type == "StartEnd") {
        // 对于开始/结束框，使用圆角矩形的检测
        QPainterPath path;
        int radius = qMin(shape.rect.width(), shape.rect.height()) / 2;
        path.addRoundedRect(shape.rect, radius, radius);
        return path.contains(point);
    }
    else if (shape.type == "Diamond") {
        // 对于菱形，创建多边形并检测点是否在其中
        QPolygon diamond;
        int centerX = shape.rect.center().x();
        int centerY = shape.rect.center().y();
        int width = shape.rect.width() / 2;
        int height = shape.rect.height() / 2;

        diamond << QPoint(centerX, centerY - height)  // 上
            << QPoint(centerX + width, centerY)   // 右
            << QPoint(centerX, centerY + height)  // 下
            << QPoint(centerX - width, centerY);  // 左

        return diamond.containsPoint(point, Qt::OddEvenFill);
    }

    return false;
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制所有图形
    for (const Shape& shape : shapes) {
        // 设置画笔
        QColor borderColor = shape.borderColor;
        borderColor.setAlpha(shape.opacity * 255 / 100);

        QPen pen;
        if (shape.selected) {
            // 选中状态使用高亮蓝色
            QColor highlightBlue(0, 120, 255);
            highlightBlue.setAlpha(shape.opacity * 255 / 100);
            pen = QPen(highlightBlue, shape.lineWidth + 2, shape.penStyle);
        }
        else {
            pen = QPen(borderColor, shape.lineWidth, shape.penStyle);
        }
        painter.setPen(pen);

        QColor fillColor = shape.color;
        fillColor.setAlpha(shape.opacity * 255 / 100);
        painter.setBrush(QBrush(fillColor));

        // 绘制形状
        if (shape.type == "Rectangle") {
            painter.drawRect(shape.rect);
        }
        else if (shape.type == "Circle") {
            painter.drawEllipse(shape.rect);
        }
        else if (shape.type == "Triangle") {
            QPolygon triangle;
            triangle << QPoint(shape.rect.left() + shape.rect.width() / 2, shape.rect.top())
                << QPoint(shape.rect.right(), shape.rect.bottom())
                << QPoint(shape.rect.left(), shape.rect.bottom());
            painter.drawPolygon(triangle);
        }
        else if (shape.type == "Arrow") {
            // 获取起点和终点
            QPoint start = shape.rect.topLeft();
            QPoint end = shape.rect.bottomRight();

            // 计算箭头方向
            QLineF line(start, end);
            double angle = line.angle();  // Qt的angle()返回的是从X轴正方向逆时针的角度

            // 绘制箭头线
            painter.drawLine(start, end);

            // 箭头头部大小
            double arrowSize = 20.0;

            // 计算箭头头部的两个点
            double arrowAngle = 30.0;  // 箭头角度
            QPointF arrowP1 = end - QPointF(
                arrowSize * cos((angle + arrowAngle) * M_PI / 180.0),
                -arrowSize * sin((angle + arrowAngle) * M_PI / 180.0)  // 注意Y轴方向
            );
            QPointF arrowP2 = end - QPointF(
                arrowSize * cos((angle - arrowAngle) * M_PI / 180.0),
                -arrowSize * sin((angle - arrowAngle) * M_PI / 180.0)  // 注意Y轴方向
            );

            // 根据箭头样式绘制箭头
            switch (shape.arrowStyle) {
            case 0: // 无箭头
                break;
            case 1: // 单箭头
                painter.drawLine(end, arrowP1);
                painter.drawLine(end, arrowP2);
                break;
            case 2: // 双箭头
                // 绘制终点箭头
                painter.drawLine(end, arrowP1);
                painter.drawLine(end, arrowP2);
                // 绘制起点箭头
                QPointF startArrowP1 = start + QPointF(
                    arrowSize * cos((angle + arrowAngle) * M_PI / 180.0),
                    -arrowSize * sin((angle + arrowAngle) * M_PI / 180.0)
                );
                QPointF startArrowP2 = start + QPointF(
                    arrowSize * cos((angle - arrowAngle) * M_PI / 180.0),
                    -arrowSize * sin((angle - arrowAngle) * M_PI / 180.0)
                );
                painter.drawLine(start, startArrowP1);
                painter.drawLine(start, startArrowP2);
                break;
            }
        }
        else if (shape.type == "StartEnd") {
            // 使用更大的圆角半径，使图形更加圆润
            int radius = qMin(shape.rect.width(), shape.rect.height()) / 2;
            painter.drawRoundedRect(shape.rect, radius, radius);
        }
        else if (shape.type == "Diamond") {
            QPolygon diamond;
            int centerX = shape.rect.center().x();
            int centerY = shape.rect.center().y();
            int width = shape.rect.width() / 2;
            int height = shape.rect.height() / 2;

            diamond << QPoint(centerX, centerY - height)
                << QPoint(centerX + width, centerY)
                << QPoint(centerX, centerY + height)
                << QPoint(centerX - width, centerY);

            painter.drawPolygon(diamond);
        }

        // 如果形状被选中，绘制控制点
        if (shape.selected) {
            QColor highlightBlue(0, 120, 255);
            painter.setPen(QPen(highlightBlue, 2));
            painter.setBrush(QBrush(Qt::white));

            if (shape.type == "Arrow") {
                // 对于箭头，只绘制起点和终点两个控制点
                QRect rect = shape.rect;
                // 起点控制点
                painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 终点控制点
                painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
            }
            else {
                // 其他形状保持原有的8个控制点
                QRect rect = shape.rect;
                QPoint center = rect.center();

                // 左上
                painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 上中
                painter.drawRect(center.x() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 右上
                painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 右中
                painter.drawRect(rect.right() - HANDLE_SIZE / 2, center.y() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 右下
                painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 下中
                painter.drawRect(center.x() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 左下
                painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
                // 左中
                painter.drawRect(rect.left() - HANDLE_SIZE / 2, center.y() - HANDLE_SIZE / 2,
                    HANDLE_SIZE, HANDLE_SIZE);
            }
        }

        // Draw text if exists and not being edited
        if (!shape.text.isEmpty() && !(isEditingText && shapes.indexOf(shape) == editingShapeIndex)) {
            painter.setPen(shape.textColor);
            painter.setFont(shape.textFont);

            // 计算文本绘制区域
            QRect textRect = shape.rect;
            int margin = 10;  // 文本边距
            textRect.adjust(margin, margin, -margin, -margin);

            // 根据图形类型调整文本区域
            if (shape.type == "Circle") {
                // 圆形使用较小的文本区域
                int size = qMin(textRect.width(), textRect.height()) * 0.9;
                textRect.setWidth(size);
                textRect.setHeight(size);
                textRect.moveCenter(shape.rect.center());
            }
            else if (shape.type == "Triangle") {
                // 三角形使用较小的文本区域
                textRect.setWidth(textRect.width() * 0.8);
                textRect.setHeight(textRect.height() * 0.3);
                textRect.moveCenter(shape.rect.center());
            }

            // 创建文本布局
            QTextLayout textLayout(shape.text, shape.textFont);
            textLayout.setCacheEnabled(true);

            // 设置文本布局的宽度和换行模式
            textLayout.beginLayout();
            QTextOption textOption;
            textOption.setAlignment(shape.textAlignment);  // 使用形状中保存的对齐方式
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            textLayout.setTextOption(textOption);

            // 创建文本行
            QTextLine line = textLayout.createLine();
            while (line.isValid()) {
                line.setLineWidth(textRect.width());
                line = textLayout.createLine();
            }
            textLayout.endLayout();

            // 计算文本的总高度
            qreal totalHeight = 0;
            for (int i = 0; i < textLayout.lineCount(); ++i) {
                totalHeight += textLayout.lineAt(i).height();
            }

            // 计算起始Y坐标，使文本垂直居中
            qreal y = textRect.y() + (textRect.height() - totalHeight) / 2;

            // 绘制每一行文本
            for (int i = 0; i < textLayout.lineCount(); ++i) {
                QTextLine line = textLayout.lineAt(i);
                qreal x;

                // 根据对齐方式计算x坐标
                switch (shape.textAlignment) {
                case Qt::AlignLeft:
                    x = textRect.x();
                    break;
                case Qt::AlignRight:
                    x = textRect.x() + (textRect.width() - line.naturalTextWidth()) / 2;
                    break;
                case Qt::AlignCenter:
                default:
                    x = textRect.x() + (textRect.width() - line.naturalTextWidth()) / 2;
                    break;
                }

                line.draw(&painter, QPointF(x, y));
                y += line.height();
            }
        }

        // 如果鼠标悬停在未选中的图形上，绘制特殊控制点
        if ((shape.type == "Rectangle" || shape.type == "Circle" ||
            shape.type == "Diamond" || shape.type == "StartEnd" ||
            shape.type == "Triangle") &&
            !shape.selected && isHoveringOverShape &&
            hoveredShapeIndex == shapes.indexOf(shape)) {
            drawSpecialHandles(painter, shape);
        }
    }

    // 如果正在绘制形状，绘制当前形状
    if (isDrawing && currentShape) {
        drawShape(painter, *currentShape);
    }
}

int MainWindow::getHandleAtPosition(const QPoint& pos, const Shape& shape)
{
    QRect rect = shape.rect;

    if (shape.type == "Arrow") {
        // 对于箭头，只检查起点和终点
        if (QLineF(pos, rect.topLeft()).length() < HANDLE_SIZE) {
            return 0;  // 起点
        }
        else if (QLineF(pos, rect.bottomRight()).length() < HANDLE_SIZE) {
            return 1;  // 终点
        }
    }
    else {
        // 其他形状检查8个控制点
        QPoint center = rect.center();
        QPoint handles[8] = {
            rect.topLeft(),                    // 0: 左上
            QPoint(center.x(), rect.top()),    // 1: 上中
            rect.topRight(),                   // 2: 右上
            QPoint(rect.right(), center.y()),  // 3: 右中
            rect.bottomRight(),                // 4: 右下
            QPoint(center.x(), rect.bottom()), // 5: 下中
            rect.bottomLeft(),                 // 6: 左下
            QPoint(rect.left(), center.y())    // 7: 左中
        };

        for (int i = 0; i < 8; i++) {
            if (QLineF(pos, handles[i]).length() < HANDLE_SIZE) {
                return i;
            }
        }
    }

    // 如果不在控制点上，检查是否在图形内部
    if (shapeContainsPoint(shape, pos)) {
        return -2;  // 使用-2表示在图形内部
    }

    return -1;  // 不在图形上
}

void MainWindow::updateCursorForHandle(int handleIndex)
{
    if (handleIndex == -2) {
        // 在图形内部，显示移动光标
        canvasWidget->setCursor(Qt::SizeAllCursor);
        return;
    }

    if (handleIndex == -1) {
        canvasWidget->setCursor(Qt::ArrowCursor);
        return;
    }

    if (selectedShapeIndex >= 0 && shapes[selectedShapeIndex].type == "Arrow") {
        // 箭头的控制点使用十字光标
        canvasWidget->setCursor(Qt::CrossCursor);
    }
    else {
        // 其他形状的控制点使用对应的调整大小光标
        switch (handleIndex) {
        case 0: // 左上
        case 4: // 右下
            canvasWidget->setCursor(Qt::SizeFDiagCursor);
            break;
        case 2: // 右上
        case 6: // 左下
            canvasWidget->setCursor(Qt::SizeBDiagCursor);
            break;
        case 1: // 上中
        case 5: // 下中
            canvasWidget->setCursor(Qt::SizeVerCursor);
            break;
        case 3: // 右中
        case 7: // 左中
            canvasWidget->setCursor(Qt::SizeHorCursor);
            break;
        default:
            canvasWidget->setCursor(Qt::ArrowCursor);
        }
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // 处理内嵌文本编辑器的ESC键和回车键
    if (watched == inlineTextEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            cancelInlineTextEdit();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                finishInlineTextEdit();
                return true;
            }
        }
    }

    // 处理工具按钮的拖拽事件
    if (watched == rectangleTool || watched == circleTool ||
        watched == triangleTool || watched == arrowTool ||
        watched == startEndTool || watched == diamondTool) {

        QToolButton* button = qobject_cast<QToolButton*>(watched);
        if (button) {
            if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    QString shapeType;
                    if (button == rectangleTool) shapeType = "Rectangle";
                    else if (button == circleTool) shapeType = "Circle";
                    else if (button == triangleTool) shapeType = "Triangle";
                    else if (button == arrowTool) shapeType = "Arrow";
                    else if (button == startEndTool) shapeType = "StartEnd";
                    else if (button == diamondTool) shapeType = "Diamond";

                    // 开始拖拽
                    draggedButton = button;
                    draggedPixmap = button->icon().pixmap(button->iconSize());
                    dragStartPoint = mouseEvent->pos();
                    dragOffset = QPoint(250, 250);
                    isDraggingShape = true;

                    // 设置按钮为按下状态
                    button->setDown(true);

                    startDraggingShape(shapeType, mouseEvent->globalPos());
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseMove && isDraggingShape) {
                if (draggedButton == button)
                {
                    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                    dragCurrentPoint = mouseEvent->pos();

                    // 更新画布
                    canvasWidget->update();
                    updateDraggingShape(mouseEvent->globalPos());
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseButtonRelease && isDraggingShape) {
                if (draggedButton == button)
                {
                    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                    if (mouseEvent->button() == Qt::LeftButton) {
                        finishDraggingShape(mouseEvent->globalPos());
                        button->setDown(false);
                        return true;
                    }
                }
            }
        }
    }

    // 处理画布的事件
    if (watched == canvasWidget) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(canvasWidget);

            // 设置浅灰色背景
            painter.fillRect(canvasWidget->rect(), QColor(240, 240, 240));  // 更浅的灰色背景

            // 应用缩放
            painter.translate(canvasPosition);
            painter.scale(scaleFactor, scaleFactor);

            // Draw canvas background
            QRect canvasRect(0, 0, pageWidth, pageHeight);
            painter.fillRect(canvasRect, backgroundColor);

            // Draw grid if enabled
            if (showGrid) {
                painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
                int gridSpacing;
                switch (currentGridSize) {
                case Small:
                    gridSpacing = 10;
                    break;
                case Medium:
                    gridSpacing = 20;
                    break;
                case Large:
                    gridSpacing = 50;
                    break;
                default:
                    gridSpacing = 20;
                }
                for (int y = 0; y < pageHeight; y += gridSpacing) {
                    painter.drawLine(0, y, pageWidth, y);
                }
                for (int x = 0; x < pageWidth; x += gridSpacing) {
                    painter.drawLine(x, 0, x, pageHeight);
                }
            }

            if (isDraggingShape && draggedButton)
            {
                QPoint globalPos = QCursor::pos();
                QPoint localPos = canvasWidget->mapFromGlobal(globalPos);
                painter.drawPixmap(localPos - dragOffset, draggedPixmap);
            }

            // Draw all shapes
            for (const Shape& shape : shapes) {
                drawShape(painter, shape);
            }

            // Draw current shape if drawing
            if (isDrawing && currentShape) {
                drawShape(painter, *currentShape);
            }

            // 如果正在框选，绘制选择框
            if (isRectSelecting) {
                // 设置选择框样式，只使用边框，不使用填充
                painter.setPen(QPen(QColor(0, 120, 255), 1, Qt::DashLine));
                painter.setBrush(Qt::NoBrush); // 移除填充色

                // 绘制选择框
                painter.drawRect(selectionRect);
            }

            // Draw canvas border
            painter.setPen(QPen(QColor(210, 210, 210), 1));
            painter.drawRect(canvasRect);
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QPoint pos = (mouseEvent->pos() - canvasPosition) / scaleFactor;

                if (pos.x() >= 0 && pos.x() <= pageWidth &&
                    pos.y() >= 0 && pos.y() <= pageHeight) {

                    // 检查是否按住Ctrl键进行多选
                    isMultiSelecting = mouseEvent->modifiers() & Qt::ControlModifier;

                    if (currentTool == "Select") {
                        // 首先检查是否点击了特殊控制点
                        bool clickedSpecialHandle = false;
                        for (int i = shapes.size() - 1; i >= 0; --i) {
                            if (!shapes[i].selected && isPointNearSpecialHandle(pos, shapes[i])) {
                                startDrawingFromHandle(pos, shapes[i]);
                                clickedSpecialHandle = true;
                                break;
                            }
                        }

                        if (!clickedSpecialHandle) {
                            // 如果没有点击特殊控制点，则检查是否点击了控制点
                            if (selectedShapeIndex >= 0) {
                                QRect rect = shapes[selectedShapeIndex].rect;

                                if (shapes[selectedShapeIndex].type == "Arrow") {
                                    // 对于箭头，只检查起点和终点
                                    if (QLineF(pos, rect.topLeft()).length() < HANDLE_SIZE) {
                                        isResizing = true;
                                        resizeStartPoint = pos;
                                        resizeHandle = 0;  // 起点
                                        return true;
                                    }
                                    else if (QLineF(pos, rect.bottomRight()).length() < HANDLE_SIZE) {
                                        isResizing = true;
                                        resizeStartPoint = pos;
                                        resizeHandle = 1;  // 终点
                                        return true;
                                    }
                                }
                                else {
                                    // 对于其他形状，检查8个控制点
                                    int handleIndex = getHandleAtPosition(pos, shapes[selectedShapeIndex]);
                                    if (handleIndex >= 0) {
                                        isResizing = true;
                                        resizeStartPoint = pos;
                                        resizeHandle = handleIndex;
                                        return true;
                                    }
                                }
                            }

                            // 然后检查是否点击了已有图形
                            bool clickedShape = false;
                            for (int i = shapes.size() - 1; i >= 0; --i) {
                                if (shapeContainsPoint(shapes[i], pos)) {
                                    // 如果是多选模式且已经被选中，则移除选择
                                    if (isMultiSelecting && shapes[i].selected) {
                                        removeFromSelection(i);
                                    }
                                    // 如果是多选模式且未被选中，则添加到选择中
                                    else if (isMultiSelecting && !shapes[i].selected) {
                                        addToSelection(i);
                                    }
                                    // 如果不是多选模式且当前图形未被选中，则清除其他选择并选中当前图形
                                    else if (!isMultiSelecting && !shapes[i].selected) {
                                        clearSelection();
                                        addToSelection(i);
                                    }
                                    // 如果不是多选模式且点击的是已选中的图形，保持其选中状态
                                    // 这样可以支持移动一个或多个已选中的图形

                                    isMoving = true;
                                    moveStartPoint = pos;
                                    moveShapeIndex = i;  // 记录当前点击的图形索引，但移动时会移动所有选中的图形
                                    clickedShape = true;
                                    break;
                                }
                            }

                            // 如果点击了空白区域且不是多选模式，清除选择并开始框选
                            if (!clickedShape) {
                                if (!isMultiSelecting) {
                                    clearSelection();
                                }

                                // 开始框选
                                isRectSelecting = true;
                                selectStartPoint = pos;
                                selectionRect = QRect(selectStartPoint, QSize(0, 0));
                            }
                        }
                        canvasWidget->update();
                    }
                    else {
                        clearSelection();
                        startDrawing(pos);
                    }
                }
            }
            else if (mouseEvent->button() == Qt::MiddleButton) {
                isDragging = true;
                lastMousePos = mouseEvent->pos();
                canvasWidget->setCursor(Qt::ClosedHandCursor);
            }
            else if (mouseEvent->button() == Qt::RightButton) {
                // 显示右键菜单
                QMenu contextMenu(this);

                // 添加缩放选项
                QMenu* zoomMenu = contextMenu.addMenu("缩放");
                QAction* zoomInAction = zoomMenu->addAction("放大");
                QAction* zoomOutAction = zoomMenu->addAction("缩小");
                QAction* resetZoomAction = zoomMenu->addAction("重置大小");

                // 添加分隔线
                contextMenu.addSeparator();

                // 添加复制选项
                QAction* copyAction = contextMenu.addAction("复制");
                copyAction->setShortcut(QKeySequence::Copy);
                copyAction->setEnabled(selectedShapeIndex >= 0 || !selectedShapeIndices.isEmpty());

                // 添加剪切选项
                QAction* cutAction = contextMenu.addAction("剪切");
                cutAction->setShortcut(QKeySequence::Cut);
                cutAction->setEnabled(selectedShapeIndex >= 0 || !selectedShapeIndices.isEmpty());

                // 添加粘贴选项
                QAction* pasteAction = contextMenu.addAction("粘贴");
                pasteAction->setShortcut(QKeySequence::Paste);
                pasteAction->setEnabled(clipboardShape != nullptr || !clipboardShapes.isEmpty());

                // 添加删除选项
                QAction* deleteAction = contextMenu.addAction("删除");
                deleteAction->setShortcut(QKeySequence::Delete);
                deleteAction->setEnabled(selectedShapeIndex >= 0 || !selectedShapeIndices.isEmpty());

                // 添加分隔线
                contextMenu.addSeparator();

                // 添加层次排列选项
                QAction* bringToFrontAction = contextMenu.addAction("置于顶层");
                bringToFrontAction->setEnabled(selectedShapeIndex >= 0);

                QAction* sendToBackAction = contextMenu.addAction("置于底层");
                sendToBackAction->setEnabled(selectedShapeIndex >= 0);

                QAction* moveUpAction = contextMenu.addAction("上移一层");
                moveUpAction->setEnabled(selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() - 1);

                QAction* moveDownAction = contextMenu.addAction("下移一层");
                moveDownAction->setEnabled(selectedShapeIndex > 0);

                // 添加分隔线
                contextMenu.addSeparator();

                // 添加导出选项
                QAction* exportAction = contextMenu.addAction("导出为PNG文件");

                // 连接导出菜单的信号槽
                connect(exportAction, &QAction::triggered, [this]() {
                    QString fileName = QFileDialog::getSaveFileName(this,
                        "Export as PNG", "", "PNG Files (*.png)");
                    if (!fileName.isEmpty()) {
                        QPixmap pixmap(canvasWidget->size());
                        canvasWidget->render(&pixmap);
                        pixmap.save(fileName, "PNG");
                    }
                    });

                // 添加分隔线
                contextMenu.addSeparator();

                // 连接缩放菜单的信号槽
                connect(zoomInAction, &QAction::triggered, [this]() {
                    scaleFactor *= 1.2;  // 放大20%
                    canvasWidget->update();
                    centerCanvas();
                    emit scaleFactorChanged(scaleFactor);

                    // 更新Page菜单栏中的宽度和高度值
                    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
                    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

                    if (widthSpin) {
                        widthSpin->blockSignals(true);
                        widthSpin->setValue(pageWidth * scaleFactor);
                        widthSpin->blockSignals(false);
                    }

                    if (heightSpin) {
                        heightSpin->blockSignals(true);
                        heightSpin->setValue(pageHeight * scaleFactor);
                        heightSpin->blockSignals(false);
                    }
                    });

                connect(zoomOutAction, &QAction::triggered, [this]() {
                    scaleFactor /= 1.2;  // 缩小20%
                    canvasWidget->update();
                    centerCanvas();
                    emit scaleFactorChanged(scaleFactor);

                    // 更新Page菜单栏中的宽度和高度值
                    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
                    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

                    if (widthSpin) {
                        widthSpin->blockSignals(true);
                        widthSpin->setValue(pageWidth * scaleFactor);
                        widthSpin->blockSignals(false);
                    }

                    if (heightSpin) {
                        heightSpin->blockSignals(true);
                        heightSpin->setValue(pageHeight * scaleFactor);
                        heightSpin->blockSignals(false);
                    }
                    });

                connect(resetZoomAction, &QAction::triggered, [this]() {
                    scaleFactor = 1.0;  // 重置缩放
                    canvasWidget->update();
                    centerCanvas();
                    emit scaleFactorChanged(scaleFactor);

                    // 更新Page菜单栏中的宽度和高度值
                    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
                    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

                    if (widthSpin) {
                        widthSpin->blockSignals(true);
                        widthSpin->setValue(pageWidth);
                        widthSpin->blockSignals(false);
                    }

                    if (heightSpin) {
                        heightSpin->blockSignals(true);
                        heightSpin->setValue(pageHeight);
                        heightSpin->blockSignals(false);
                    }
                    });

                // 连接其他菜单项的信号槽
                connect(copyAction, &QAction::triggered, [this]() {
                    copySelectedShapes();
                    });

                connect(cutAction, &QAction::triggered, [this]() {
                    cutSelectedShapes();
                    });

                connect(pasteAction, &QAction::triggered, [this]() {
                    pasteShapes();
                    });

                connect(deleteAction, &QAction::triggered, [this]() {
                    if (!selectedShapeIndices.isEmpty()) {
                        // 保存状态用于撤销
                        saveState();

                        // 从后向前删除选中的图形，避免索引变化问题
                        QList<int> indicesToDelete = selectedShapeIndices;
                        std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<int>());

                        for (int index : indicesToDelete) {
                            if (index >= 0 && index < shapes.size()) {
                                shapes.removeAt(index);
                            }
                        }

                        // 清空选择
                        selectedShapeIndex = -1;
                        selectedShapeIndices.clear();

                        canvasWidget->update();
                    }
                    else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                        saveState();
                        shapes.removeAt(selectedShapeIndex);
                        selectedShapeIndex = -1;
                        canvasWidget->update();
                    }
                    });

                // 连接层次排列的信号槽
                connect(bringToFrontAction, &QAction::triggered, [this]() {
                    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                        saveState();
                        // 将选中的形状移到最上层
                        Shape selectedShape = shapes[selectedShapeIndex];
                        shapes.removeAt(selectedShapeIndex);
                        shapes.append(selectedShape);
                        selectedShapeIndex = shapes.size() - 1;
                        canvasWidget->update();
                    }
                    });

                connect(sendToBackAction, &QAction::triggered, [this]() {
                    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                        saveState();
                        // 将选中的形状移到最下层
                        Shape selectedShape = shapes[selectedShapeIndex];
                        shapes.removeAt(selectedShapeIndex);
                        shapes.prepend(selectedShape);
                        selectedShapeIndex = 0;
                        canvasWidget->update();
                    }
                    });

                connect(moveUpAction, &QAction::triggered, [this]() {
                    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() - 1) {
                        saveState();
                        // 将选中的形状上移一层
                        shapes.swapItemsAt(selectedShapeIndex, selectedShapeIndex + 1);
                        selectedShapeIndex++;
                        canvasWidget->update();
                    }
                    });

                connect(moveDownAction, &QAction::triggered, [this]() {
                    if (selectedShapeIndex > 0) {
                        saveState();
                        // 将选中的形状下移一层
                        shapes.swapItemsAt(selectedShapeIndex, selectedShapeIndex - 1);
                        selectedShapeIndex--;
                        canvasWidget->update();
                    }
                    });

                // 显示菜单
                contextMenu.exec(mouseEvent->globalPos());
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (isDrawing) {
                    finishDrawing();
                }
                // 完成框选
                else if (isRectSelecting) {
                    if (selectionRect.width() > 5 && selectionRect.height() > 5) {
                        // 执行框选操作
                        selectShapesInRect(selectionRect);
                    }
                    isRectSelecting = false;
                    canvasWidget->update();
                }
                // 结束移动
                if (isMoving) {
                    isMoving = false;
                    moveShapeIndex = -1;
                }
                if (isResizing) {
                    isResizing = false;
                    resizeHandle = -1;
                }
            }
            else if (mouseEvent->button() == Qt::MiddleButton) {
                isDragging = false;
                canvasWidget->setCursor(Qt::ArrowCursor);
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint pos = (mouseEvent->pos() - canvasPosition) / scaleFactor;

            if (isResizing && selectedShapeIndex >= 0) {
                QRect& rect = shapes[selectedShapeIndex].rect;
                QPoint delta = pos - resizeStartPoint;

                if (shapes[selectedShapeIndex].type == "Arrow") {
                    // 对于箭头，只调整起点或终点
                    if (resizeHandle == 0) {  // 起点
                        rect.setTopLeft(rect.topLeft() + delta);
                    }
                    else {  // 终点
                        rect.setBottomRight(rect.bottomRight() + delta);
                    }
                }
                else {
                    // 其他形状保持原有的调整方式
                    QRect originalRect = rect;
                    switch (resizeHandle) {
                    case 0: rect.setTopLeft(rect.topLeft() + delta); break;
                    case 1: rect.setTop(rect.top() + delta.y()); break;
                    case 2: rect.setTopRight(rect.topRight() + delta); break;
                    case 3: rect.setRight(rect.right() + delta.x()); break;
                    case 4: rect.setBottomRight(rect.bottomRight() + delta); break;
                    case 5: rect.setBottom(rect.bottom() + delta.y()); break;
                    case 6: rect.setBottomLeft(rect.bottomLeft() + delta); break;
                    case 7: rect.setLeft(rect.left() + delta.x()); break;
                    }

                    // 确保矩形不会太小
                    if (rect.width() < 20 || rect.height() < 20) {
                        rect = originalRect;
                    }
                }

                resizeStartPoint = pos;
                canvasWidget->update();
            }
            else if (isMoving) {
                QPoint delta = pos - moveStartPoint;

                // 如果有图形被选中，移动所有选中的图形
                if (!selectedShapeIndices.isEmpty()) {
                    for (int index : selectedShapeIndices) {
                        if (index >= 0 && index < shapes.size()) {
                            QRect newRect = shapes[index].rect;
                            newRect.translate(delta);

                            // 如果网格显示，进行网格对齐
                            if (showGrid) {
                                int gridSpacing;
                                switch (currentGridSize) {
                                case Small:
                                    gridSpacing = 10;
                                    break;
                                case Medium:
                                    gridSpacing = 20;
                                    break;
                                case Large:
                                    gridSpacing = 50;
                                    break;
                                default:
                                    gridSpacing = 20;
                                }
                                
                                // 计算吸附阈值（网格间距的1/4）
                                int snapThreshold = gridSpacing / 4;
                                
                                // 计算最近的网格点
                                int newX = round(newRect.x() / (double)gridSpacing) * gridSpacing;
                                int newY = round(newRect.y() / (double)gridSpacing) * gridSpacing;
                                
                                // 只有当距离网格点足够近时才进行对齐
                                if (abs(newRect.x() - newX) <= snapThreshold) {
                                    newRect.moveLeft(newX);
                                }
                                if (abs(newRect.y() - newY) <= snapThreshold) {
                                    newRect.moveTop(newY);
                                }
                            }

                            shapes[index].rect = newRect;

                            // 更新所有从该图形特殊控制点出发的箭头
                            updateConnectedArrows(index);
                        }
                    }
                }
                // 如果没有多选，但有单个图形被选中且正在移动
                else if (moveShapeIndex >= 0 && moveShapeIndex < shapes.size()) {
                    QRect newRect = shapes[moveShapeIndex].rect;
                    newRect.translate(delta);

                    // 如果网格显示，进行网格对齐
                    if (showGrid) {
                        int gridSpacing;
                        switch (currentGridSize) {
                        case Small:
                            gridSpacing = 10;
                            break;
                        case Medium:
                            gridSpacing = 20;
                            break;
                        case Large:
                            gridSpacing = 50;
                            break;
                        default:
                            gridSpacing = 20;
                        }
                        
                        // 计算吸附阈值（网格间距的1/4）
                        int snapThreshold = gridSpacing / 4;
                        
                        // 计算最近的网格点
                        int newX = round(newRect.x() / (double)gridSpacing) * gridSpacing;
                        int newY = round(newRect.y() / (double)gridSpacing) * gridSpacing;
                        
                        // 只有当距离网格点足够近时才进行对齐
                        if (abs(newRect.x() - newX) <= snapThreshold) {
                            newRect.moveLeft(newX);
                        }
                        if (abs(newRect.y() - newY) <= snapThreshold) {
                            newRect.moveTop(newY);
                        }
                    }

                    shapes[moveShapeIndex].rect = newRect;

                    // 更新所有从该图形特殊控制点出发的箭头
                    updateConnectedArrows(moveShapeIndex);
                }

                moveStartPoint = pos;
                canvasWidget->update();
            }
            else if (isDrawing) {
                updateDrawing(pos);
            }
            else if (isRectSelecting) {
                // 更新框选矩形
                selectionRect = QRect(selectStartPoint, pos).normalized();
                canvasWidget->update();
            }
            else {
                // 检查是否在画布范围内
                if (pos.x() >= 0 && pos.x() <= pageWidth &&
                    pos.y() >= 0 && pos.y() <= pageHeight) {

                    // 首先检查是否在选中形状的控制点上
                    if (selectedShapeIndex >= 0) {
                        int handleIndex = getHandleAtPosition(pos, shapes[selectedShapeIndex]);
                        if (handleIndex >= 0) {  // 在控制点上
                            updateCursorForHandle(handleIndex);
                            isHoveringOverShape = false;
                            canvasWidget->update();
                            return true;
                        }
                    }

                    // 然后检查是否在任何图形内部
                    bool foundHover = false;
                    for (int i = shapes.size() - 1; i >= 0; --i) {
                        if (shapeContainsPoint(shapes[i], pos)) {
                            // 检查是否在特殊控制点上
                            if (isPointNearSpecialHandle(pos, shapes[i])) {
                                canvasWidget->setCursor(Qt::CrossCursor);
                                isHoveringOverShape = true;
                                hoveredShapeIndex = i;
                                foundHover = true;
                                canvasWidget->update();
                                return true;
                            }

                            isHoveringOverShape = true;
                            hoveredShapeIndex = i;
                            foundHover = true;
                            canvasWidget->setCursor(Qt::SizeAllCursor);
                            canvasWidget->update();
                            return true;
                        }
                    }

                    if (!foundHover) {
                        isHoveringOverShape = false;
                        canvasWidget->setCursor(Qt::ArrowCursor);
                        canvasWidget->update();
                    }
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseDoubleClickEvent(mouseEvent);
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            isHoveringOverShape = false;
            canvasWidget->setCursor(Qt::ArrowCursor);
            canvasWidget->update();
        }
        else if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

            // 处理复制、粘贴和剪切快捷键
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                switch (keyEvent->key()) {
                case Qt::Key_C:  // 复制 Ctrl+C
                    copySelectedShapes();
                    return true;
                case Qt::Key_V:  // 粘贴 Ctrl+V
                    pasteShapes();
                    return true;
                case Qt::Key_X:  // 剪切 Ctrl+X
                    cutSelectedShapes();
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}


// 清除所有选择
void MainWindow::clearSelection()
{
    for (int i = 0; i < shapes.size(); ++i) {
        shapes[i].selected = false;
    }
    selectedShapeIndex = -1;
    selectedShapeIndices.clear();
    emit selectionChanged();  // 发出选择状态改变信号
}

// 添加图形到选择中
void MainWindow::addToSelection(int shapeIndex)
{
    if (shapeIndex >= 0 && shapeIndex < shapes.size()) {
        shapes[shapeIndex].selected = true;

        // 如果不在已选列表中，则添加
        if (!selectedShapeIndices.contains(shapeIndex)) {
            selectedShapeIndices.append(shapeIndex);
        }

        // 更新当前选中的图形索引（最后选中的那个）
        selectedShapeIndex = shapeIndex;

        emit selectionChanged();
    }
}

// 从选择中移除图形
void MainWindow::removeFromSelection(int shapeIndex)
{
    if (shapeIndex >= 0 && shapeIndex < shapes.size()) {
        shapes[shapeIndex].selected = false;

        // 从已选列表中移除
        selectedShapeIndices.removeAll(shapeIndex);

        // 更新当前选中的图形索引
        if (selectedShapeIndices.isEmpty()) {
            selectedShapeIndex = -1;
        }
        else {
            selectedShapeIndex = selectedShapeIndices.last();
        }

        emit selectionChanged();
    }
}

// 切换图形的选择状态
void MainWindow::toggleSelection(int shapeIndex)
{
    if (shapeIndex >= 0 && shapeIndex < shapes.size()) {
        if (shapes[shapeIndex].selected) {
            removeFromSelection(shapeIndex);
        }
        else {
            addToSelection(shapeIndex);
        }
    }
}

// 选择矩形区域内的图形
void MainWindow::selectShapesInRect(const QRect& rect)
{
    // 如果不是多选模式，需要先清除已有选择
    if (!isMultiSelecting) {
        clearSelection();
    }

    // 遍历所有图形，检查是否与选择框相交
    for (int i = 0; i < shapes.size(); ++i) {
        // 获取图形的矩形区域
        QRect shapeRect = shapes[i].rect;

        // 如果图形与选择框相交，则添加到选择中
        if (rect.intersects(shapeRect)) {
            addToSelection(i);
        }
    }
}

void MainWindow::drawShape(QPainter& painter, const Shape& shape)
{
    // 保存当前画笔状态
    painter.save();

    //设置形状基本属性
    QColor borderColor = shape.borderColor;
    borderColor.setAlpha(shape.opacity * 255 / 100);

    QPen pen;
    if (shape.selected) {
        // 选中状态使用高亮蓝色
        QColor highlightBlue(0, 120, 255);
        highlightBlue.setAlpha(shape.opacity * 255 / 100);
        pen = QPen(highlightBlue, shape.lineWidth + 2, shape.penStyle);
    }
    else {
        pen = QPen(borderColor, shape.lineWidth, shape.penStyle);
    }
    painter.setPen(pen);

    QColor fillColor = shape.color;
    fillColor.setAlpha(shape.opacity * 255 / 100);
    painter.setBrush(QBrush(fillColor));

    // 根据形状类型绘制
    if (shape.type == "Rectangle") {
        painter.drawRect(shape.rect);
    }
    else if (shape.type == "Circle") {
        painter.drawEllipse(shape.rect);
    }
    else if (shape.type == "Diamond") {
        QPolygon diamond;
        QPoint center = shape.rect.center();
        int width = shape.rect.width() / 2;
        int height = shape.rect.height() / 2;
        diamond << QPoint(center.x(), center.y() - height)
            << QPoint(center.x() + width, center.y())
            << QPoint(center.x(), center.y() + height)
            << QPoint(center.x() - width, center.y());
        painter.drawPolygon(diamond);
    }
    else if (shape.type == "StartEnd") {
        // 使用更大的圆角半径，使图形更加圆润
        int radius = qMin(shape.rect.width(), shape.rect.height()) / 2;
        painter.drawRoundedRect(shape.rect, radius, radius);
    }
    else if (shape.type == "Triangle") {
        QPolygon triangle;
        triangle << QPoint(shape.rect.left() + shape.rect.width() / 2, shape.rect.top())
            << QPoint(shape.rect.right(), shape.rect.bottom())
            << QPoint(shape.rect.left(), shape.rect.bottom());
        painter.drawPolygon(triangle);
    }
    else if (shape.type == "Arrow") {
        // 获取起点和终点
        QPoint start = shape.rect.topLeft();
        QPoint end = shape.rect.bottomRight();

        // 计算箭头方向
        QLineF line(start, end);
        double angle = line.angle();  // Qt的angle()返回的是从X轴正方向逆时针的角度

        // 绘制箭头线
        painter.drawLine(start, end);

        // 箭头头部大小
        double arrowSize = 20.0;

        // 计算箭头头部的两个点
        double arrowAngle = 30.0;  // 箭头角度
        QPointF arrowP1 = end - QPointF(
            arrowSize * cos((angle + arrowAngle) * M_PI / 180.0),
            -arrowSize * sin((angle + arrowAngle) * M_PI / 180.0)  // 注意Y轴方向
        );
        QPointF arrowP2 = end - QPointF(
            arrowSize * cos((angle - arrowAngle) * M_PI / 180.0),
            -arrowSize * sin((angle - arrowAngle) * M_PI / 180.0)  // 注意Y轴方向
        );

        // 根据箭头样式绘制箭头
        switch (shape.arrowStyle) {
        case 0: // 无箭头
            break;
        case 1: // 单箭头
            painter.drawLine(end, arrowP1);
            painter.drawLine(end, arrowP2);
            break;
        case 2: // 双箭头
            // 绘制终点箭头
            painter.drawLine(end, arrowP1);
            painter.drawLine(end, arrowP2);
            // 绘制起点箭头
            QPointF startArrowP1 = start + QPointF(
                arrowSize * cos((angle + arrowAngle) * M_PI / 180.0),
                -arrowSize * sin((angle + arrowAngle) * M_PI / 180.0)
            );
            QPointF startArrowP2 = start + QPointF(
                arrowSize * cos((angle - arrowAngle) * M_PI / 180.0),
                -arrowSize * sin((angle - arrowAngle) * M_PI / 180.0)
            );
            painter.drawLine(start, startArrowP1);
            painter.drawLine(start, startArrowP2);
            break;
        }
    }

    // 如果形状被选中，绘制控制点
    if (shape.selected) {
        QColor highlightBlue(0, 120, 255);
        painter.setPen(QPen(highlightBlue, 2));
        painter.setBrush(QBrush(Qt::white));

        if (shape.type == "Arrow") {
            // 对于箭头，只绘制起点和终点两个控制点
            QRect rect = shape.rect;
            // 起点控制点
            painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 终点控制点
            painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
        }
        else {
            // 其他形状保持原有的8个控制点
            QRect rect = shape.rect;
            QPoint center = rect.center();

            // 左上
            painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 上中
            painter.drawRect(center.x() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 右上
            painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.top() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 右中
            painter.drawRect(rect.right() - HANDLE_SIZE / 2, center.y() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 右下
            painter.drawRect(rect.right() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 下中
            painter.drawRect(center.x() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 左下
            painter.drawRect(rect.left() - HANDLE_SIZE / 2, rect.bottom() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
            // 左中
            painter.drawRect(rect.left() - HANDLE_SIZE / 2, center.y() - HANDLE_SIZE / 2,
                HANDLE_SIZE, HANDLE_SIZE);
        }
    }

    // Draw text if exists
    if (!shape.text.isEmpty()) {
        painter.setPen(shape.textColor);
        painter.setFont(shape.textFont);

        // 计算文本绘制区域
        QRect textRect = shape.rect;
        int margin = 10;  // 文本边距
        textRect.adjust(margin, margin, -margin, -margin);

        // 根据图形类型调整文本区域
        if (shape.type == "Circle") {
            // 圆形使用较小的文本区域
            int size = qMin(textRect.width(), textRect.height()) * 0.9;
            textRect.setWidth(size);
            textRect.setHeight(size);
            textRect.moveCenter(shape.rect.center());
        }
        else if (shape.type == "Triangle") {
            // 三角形使用较小的文本区域
            textRect.setWidth(textRect.width() * 0.8);
            textRect.setHeight(textRect.height() * 0.3);
            textRect.moveCenter(shape.rect.center());
        }

        // 创建文本布局
        QTextLayout textLayout(shape.text, shape.textFont);
        textLayout.setCacheEnabled(true);

        // 设置文本布局的宽度和换行模式
        textLayout.beginLayout();
        QTextOption textOption;
        textOption.setAlignment(shape.textAlignment);  // 使用形状中保存的对齐方式
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textLayout.setTextOption(textOption);

        // 创建文本行
        QTextLine line = textLayout.createLine();
        while (line.isValid()) {
            line.setLineWidth(textRect.width());
            line = textLayout.createLine();
        }
        textLayout.endLayout();

        // 计算文本的总高度
        qreal totalHeight = 0;
        for (int i = 0; i < textLayout.lineCount(); ++i) {
            totalHeight += textLayout.lineAt(i).height();
        }

        // 计算起始Y坐标，使文本垂直居中
        qreal y = textRect.y() + (textRect.height() - totalHeight) / 2;

        // 绘制每一行文本
        for (int i = 0; i < textLayout.lineCount(); ++i) {
            QTextLine line = textLayout.lineAt(i);
            qreal x;

            // 根据对齐方式计算x坐标
            switch (shape.textAlignment) {
            case Qt::AlignLeft:
                x = textRect.x();
                break;
            case Qt::AlignRight:
                x = textRect.x() + (textRect.width() - line.naturalTextWidth()) / 2;
                break;
            case Qt::AlignCenter:
            default:
                x = textRect.x() + (textRect.width() - line.naturalTextWidth()) / 2;
                break;
            }

            line.draw(&painter, QPointF(x, y));
            y += line.height();
        }
    }

    // 如果鼠标悬停在未选中的图形上，绘制特殊控制点
    if ((shape.type == "Rectangle" || shape.type == "Circle" ||
        shape.type == "Diamond" || shape.type == "StartEnd" ||
        shape.type == "Triangle") &&
        !shape.selected && isHoveringOverShape &&
        hoveredShapeIndex == shapes.indexOf(shape)) {
        drawSpecialHandles(painter, shape);
    }

    // 恢复画笔状态
    painter.restore();

    // 如果是正在绘制箭头，且鼠标悬停在某个图形上，绘制该图形的特殊控制点
    if (isDrawing && currentShape && currentShape->type == "Arrow") {
        for (int i = 0; i < shapes.size(); ++i) {
            if (shapeContainsPoint(shapes[i], endPoint) && !shapes[i].selected) {
                drawSpecialHandles(painter, shapes[i]);
                break;
            }
        }
    }
}

void MainWindow::drawSpecialHandles(QPainter& painter, const Shape& shape)
{
    // 检查是否是支持特殊控制点的图形类型
    if (!(shape.type == "Rectangle" || shape.type == "Circle" ||
        shape.type == "Diamond" || shape.type == "StartEnd" ||
        shape.type == "Triangle") ||
        shape.selected) return;

    QRect rect = shape.rect;
    QPoint center = rect.center();

    // 保存当前画笔状态
    painter.save();

    // 设置特殊控制点的样式
    painter.setPen(QPen(specialHandleColor, 2));  // 红色，2像素宽
    painter.setBrush(Qt::NoBrush);  // 不填充

    if (shape.type == "Triangle") {
        // 计算三角形的四个特殊控制点位置
        QPoint top = QPoint(rect.left() + rect.width() / 2, rect.top());  // 顶点
        QPoint bottomLeft = QPoint(rect.left(), rect.bottom());  // 左下角
        QPoint bottomRight = QPoint(rect.right(), rect.bottom());  // 右下角
        QPoint bottomCenter = QPoint(rect.left() + rect.width() / 2, rect.bottom());  // 底边中点

        // 计算两个腰的中点
        QPoint leftMid = QPoint((top.x() + bottomLeft.x()) / 2, (top.y() + bottomLeft.y()) / 2);
        QPoint rightMid = QPoint((top.x() + bottomRight.x()) / 2, (top.y() + bottomRight.y()) / 2);

        // 绘制四个特殊控制点
        painter.drawRect(top.x() - SPECIAL_HANDLE_RADIUS,
            top.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        painter.drawRect(leftMid.x() - SPECIAL_HANDLE_RADIUS,
            leftMid.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        painter.drawRect(rightMid.x() - SPECIAL_HANDLE_RADIUS,
            rightMid.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        painter.drawRect(bottomCenter.x() - SPECIAL_HANDLE_RADIUS,
            bottomCenter.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);
    }
    else {
        // 其他图形的特殊控制点绘制保持不变
        // 上中
        painter.drawRect(center.x() - SPECIAL_HANDLE_RADIUS,
            rect.top() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        // 右中
        painter.drawRect(rect.right() - SPECIAL_HANDLE_RADIUS,
            center.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        // 下中
        painter.drawRect(center.x() - SPECIAL_HANDLE_RADIUS,
            rect.bottom() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);

        // 左中
        painter.drawRect(rect.left() - SPECIAL_HANDLE_RADIUS,
            center.y() - SPECIAL_HANDLE_RADIUS,
            SPECIAL_HANDLE_SIZE, SPECIAL_HANDLE_SIZE);
    }

    // 恢复画笔状态
    painter.restore();
}

bool MainWindow::isPointNearSpecialHandle(const QPoint& point, const Shape& shape)
{
    // 检查是否是支持特殊控制点的图形类型
    if (!(shape.type == "Rectangle" || shape.type == "Circle" ||
        shape.type == "Diamond" || shape.type == "StartEnd" ||
        shape.type == "Triangle") ||
        shape.selected) return false;

    QRect rect = shape.rect;

    if (shape.type == "Triangle") {
        // 计算三角形的四个特殊控制点位置
        QPoint top = QPoint(rect.left() + rect.width() / 2, rect.top());  // 顶点
        QPoint bottomLeft = QPoint(rect.left(), rect.bottom());  // 左下角
        QPoint bottomRight = QPoint(rect.right(), rect.bottom());  // 右下角

        // 计算两个腰的中点
        QPoint leftMid = QPoint((top.x() + bottomLeft.x()) / 2, (top.y() + bottomLeft.y()) / 2);
        QPoint rightMid = QPoint((top.x() + bottomRight.x()) / 2, (top.y() + bottomRight.y()) / 2);
        QPoint bottomCenter = QPoint(rect.left() + rect.width() / 2, rect.bottom());  // 底边中点

        // 检查四个特殊控制点
        QPoint handles[4] = {
            top,           // 顶点
            leftMid,       // 左腰中点
            rightMid,      // 右腰中点
            bottomCenter   // 底边中点
        };

        for (const QPoint& handle : handles) {
            if (QLineF(point, handle).length() < SPECIAL_HANDLE_SIZE) {
                return true;
            }
        }
    }
    else {
        // 其他图形的特殊控制点检测保持不变
        QPoint center = rect.center();
        QPoint handles[4] = {
            QPoint(center.x(), rect.top()),     // 上中
            QPoint(rect.right(), center.y()),   // 右中
            QPoint(center.x(), rect.bottom()),  // 下中
            QPoint(rect.left(), center.y())     // 左中
        };

        for (const QPoint& handle : handles) {
            if (QLineF(point, handle).length() < SPECIAL_HANDLE_SIZE) {
                return true;
            }
        }
    }

    return false;
}

void MainWindow::startDrawing(const QPoint& pos)
{
    if (currentTool == "Select") return;

    isDrawing = true;
    startPoint = pos;
    endPoint = pos;

    currentShape = new Shape;
    currentShape->type = currentTool;
    currentShape->rect = QRect(startPoint, endPoint);
    currentShape->selected = false;  // 新形状初始不选中

    // 设置所有形状的初始样式
    currentShape->color = Qt::white;  // 白色填充
    currentShape->borderColor = Qt::black;  // 黑色边框
    currentShape->penStyle = Qt::SolidLine;  // 实线
    currentShape->opacity = 100;  // 不透明度100%
    currentShape->lineWidth = 3;
    currentShape->text = "";  // Initialize empty text
    currentShape->textFont = QFont("Arial", 12);  // Default font
    currentShape->textColor = Qt::black;  // Default text color
    currentShape->textAlignment = Qt::AlignCenter;  // 默认居中对齐
    
    // 如果是箭头，设置默认样式为单箭头
    if (currentShape->type == "Arrow") {
        currentShape->arrowStyle = 1;  // 1表示单箭头
    }

    canvasWidget->update();
}

void MainWindow::updateDrawing(const QPoint& pos)
{
    if (!isDrawing || !currentShape) return;

    endPoint = pos;
    // 对于箭头，我们保持起点不变，只更新终点
    if (currentShape->type == "Arrow") {
        // 检查是否在某个图形的特殊控制点附近
        bool foundSnapPoint = false;
        for (int i = 0; i < shapes.size(); ++i) {
            if (isPointNearSpecialHandle(pos, shapes[i])) {
                // 获取特殊控制点的位置
                int handleIndex = getSpecialHandleIndex(pos, shapes[i].rect, shapes[i].type);
                endPoint = getSpecialHandlePosition(shapes[i].rect, handleIndex, shapes[i].type);
                foundSnapPoint = true;
                break;
            }
        }
        currentShape->rect = QRect(startPoint, endPoint);
    }
    else {
        currentShape->rect = QRect(startPoint, endPoint).normalized();
    }
    canvasWidget->update();
}

void MainWindow::finishDrawing()
{
    if (!isDrawing || !currentShape) return;

    // 在完成绘制前保存当前状态
    saveState();

    // 对于箭头，检查终点是否在某个图形的特殊控制点上
    if (currentShape->type == "Arrow") {
        for (int i = 0; i < shapes.size(); ++i) {
            if (isPointNearSpecialHandle(endPoint, shapes[i])) {
                // 记录箭头的终点连接到哪个图形的哪个特殊控制点
                currentShape->targetShapeIndex = i;
                int handleIndex = getSpecialHandleIndex(endPoint, shapes[i].rect, shapes[i].type);
                // 确保handleIndex在有效范围内（0-3）
                if (handleIndex >= 0 && handleIndex <= 3) {
                    currentShape->targetHandleIndex = handleIndex;
                }
                break;
            }
        }
    }

    // 将当前形状添加到形状列表中
    shapes.append(*currentShape);

    // 设置新添加的形状为选中状态
    selectedShapeIndex = shapes.size() - 1;
    shapes[selectedShapeIndex].selected = true;

    // 发出选择状态改变信号，触发属性面板更新
    emit selectionChanged();

    delete currentShape;
    currentShape = nullptr;
    isDrawing = false;

    // 自动切换到选择模式
    currentTool = "Select";
    // 更新工具按钮状态
    rectangleTool->setChecked(false);
    circleTool->setChecked(false);
    triangleTool->setChecked(false);
    arrowTool->setChecked(false);
    startEndTool->setChecked(false);
    diamondTool->setChecked(false);
    canvasWidget->setCursor(Qt::ArrowCursor);

    // 显示属性面板
    updatePropertyPanel();

    canvasWidget->update();
}

void MainWindow::onCanvasMousePress(QMouseEvent* event)
{
    QPoint pos = event->pos();
    if (pos.x() >= canvasPosition.x() && pos.x() <= canvasPosition.x() + pageWidth &&
        pos.y() >= canvasPosition.y() && pos.y() <= canvasPosition.y() + pageHeight) {
        startDrawing(pos);
    }
}

void MainWindow::onCanvasMouseMove(QMouseEvent* event)
{
    if (isDrawing) {
        updateDrawing(event->pos());
    }
}

void MainWindow::onCanvasMouseRelease(QMouseEvent* event)
{
    if (isDrawing) {
        finishDrawing();
    }
}

void MainWindow::createStartToolbar()
{
    startToolbar = new QToolBar("Start Tools", this);
    startToolbar->setMovable(false);
    startToolbar->setFloatable(false);
    startToolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   min-height: 50px;"  // 设置最小高度
        "}"
        "QToolButton {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "   min-height: 40px;"  // 设置按钮最小高度
        "}"
        "QToolButton:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QToolButton:checked {"
        "   background-color: #c0c0c0;"  // 选中时的背景色
        "   border: 2px solid #5c4b51;"  // 选中时的边框
        "}"
    );

    // 添加选择工具按钮
    QToolButton* selectButton = new QToolButton(this);
    selectButton->setIcon(QIcon(":/image/select.png"));
    selectButton->setText("选择");
    selectButton->setToolTip("选择工具");
    selectButton->setMinimumSize(40, 40);
    selectButton->setCheckable(true);
    startToolbar->addWidget(selectButton);
    connect(selectButton, &QToolButton::clicked, [=]() {
        currentTool = "Select";
        // 更新工具按钮状态
        rectangleTool->setChecked(false);
        circleTool->setChecked(false);
        triangleTool->setChecked(false);
        arrowTool->setChecked(false);
        startEndTool->setChecked(false);
        diamondTool->setChecked(false);
        canvasWidget->setCursor(Qt::ArrowCursor);
        });

    // 添加分隔符
    startToolbar->addSeparator();

    // 添加删除按钮
    QToolButton* deleteButton = new QToolButton(this);
    deleteButton->setIcon(QIcon(":/image/delete.png"));
    deleteButton->setToolTip("删除");
    deleteButton->setMinimumSize(40, 40);
    deleteButton->setIconSize(QSize(24, 24));
    deleteButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(deleteButton);
    connect(deleteButton, &QToolButton::clicked, this, &MainWindow::onDeleteButtonClicked);

    // 添加清空按钮
    QToolButton* clearButton = new QToolButton(this);
    clearButton->setIcon(QIcon(":/image/empty.png"));
    clearButton->setToolTip("清空画布");
    clearButton->setMinimumSize(40, 40);
    clearButton->setIconSize(QSize(24, 24));
    clearButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(clearButton);
    connect(clearButton, &QToolButton::clicked, this, &MainWindow::onClearCanvasClicked);

    // 添加撤销按钮
    QToolButton* undoButton = new QToolButton(this);
    undoButton->setIcon(QIcon(":/image/undo.png"));
    undoButton->setToolTip("撤销");
    undoButton->setMinimumSize(40, 40);
    undoButton->setIconSize(QSize(24, 24));
    undoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(undoButton);
    connect(undoButton, &QToolButton::clicked, this, &MainWindow::onUndoClicked);

    // 添加重做按钮
    QToolButton* redoButton = new QToolButton(this);
    redoButton->setIcon(QIcon(":/image/redo.png"));
    redoButton->setToolTip("恢复");
    redoButton->setMinimumSize(40, 40);
    redoButton->setIconSize(QSize(24, 24));
    redoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(redoButton);
    connect(redoButton, &QToolButton::clicked, this, &MainWindow::onRedoClicked);


    // 添加分隔符
    startToolbar->addSeparator();

    // 添加文本大小设置
    QComboBox* fontSizeCombo = new QComboBox(this);
    fontSizeCombo->setMinimumSize(80, 55);
    fontSizeCombo->setStyleSheet(
        "QComboBox {"
        "   background-color: #f0f0f0;"
        "   border: 1px solid #c0c0c0;"
        "   border-radius: 8px;"
        "   padding: 5px;"
        "   color: #5c4b51;"
        "   font-size: 24px;"
        "}"
        "QComboBox:hover {"
        "   background-color: #e0e0e0;"
        "   border: 2px solid #5c4b51;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 20px;"
        "}"
    );

    // 添加常用字体大小选项
    fontSizeCombo->addItems({ "8", "9", "10", "11", "12", "14", "16", "18", "20", "22", "24", "26", "28", "36", "48", "72" });
    startToolbar->addWidget(fontSizeCombo);
    connect(fontSizeCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
        [=](const QString& size) {
            if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                saveState();
                QFont font = shapes[selectedShapeIndex].textFont;
                font.setPointSize(size.toInt());
                shapes[selectedShapeIndex].textFont = font;
                canvasWidget->update();
            }
        });

    // 添加分隔符
    startToolbar->addSeparator();

    // 添加字体选择按钮
    QToolButton* fontFamilyButton = new QToolButton(this);
    fontFamilyButton->setIcon(QIcon(":/image/text_style.png"));
    fontFamilyButton->setToolTip("字体");
    fontFamilyButton->setMinimumSize(40, 40);
    fontFamilyButton->setIconSize(QSize(24, 24));
    fontFamilyButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    fontFamilyButton->setPopupMode(QToolButton::InstantPopup);
    startToolbar->addWidget(fontFamilyButton);

    // 创建字体选择菜单
    QMenu* fontFamilyMenu = new QMenu(this);
    fontFamilyButton->setMenu(fontFamilyMenu);

    // 添加常用中文字体选项
    QAction* microsoftYaHei = new QAction("微软雅黑", this);
    microsoftYaHei->setCheckable(true);
    microsoftYaHei->setChecked(true);
    fontFamilyMenu->addAction(microsoftYaHei);

    QAction* simSun = new QAction("宋体", this);
    simSun->setCheckable(true);
    fontFamilyMenu->addAction(simSun);

    QAction* kaiTi = new QAction("楷体", this);
    kaiTi->setCheckable(true);
    fontFamilyMenu->addAction(kaiTi);

    QAction* fangSong = new QAction("仿宋", this);
    fangSong->setCheckable(true);
    fontFamilyMenu->addAction(fangSong);

    QAction* heiTi = new QAction("黑体", this);
    heiTi->setCheckable(true);
    fontFamilyMenu->addAction(heiTi);

    QAction* liShu = new QAction("隶书", this);
    liShu->setCheckable(true);
    fontFamilyMenu->addAction(liShu);

    QAction* youYuan = new QAction("幼圆", this);
    youYuan->setCheckable(true);
    fontFamilyMenu->addAction(youYuan);

    // 创建按钮组
    QActionGroup* fontFamilyGroup = new QActionGroup(this);
    fontFamilyGroup->addAction(microsoftYaHei);
    fontFamilyGroup->addAction(simSun);
    fontFamilyGroup->addAction(kaiTi);
    fontFamilyGroup->addAction(fangSong);
    fontFamilyGroup->addAction(heiTi);
    fontFamilyGroup->addAction(liShu);
    fontFamilyGroup->addAction(youYuan);
    fontFamilyGroup->setExclusive(true);  // 确保只能选中一个

    // 连接信号
    connect(microsoftYaHei, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("Microsoft YaHei");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(simSun, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("SimSun");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(kaiTi, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("KaiTi");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(fangSong, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("FangSong");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(heiTi, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("SimHei");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(liShu, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("LiSu");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });

    connect(youYuan, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            QFont font = shapes[selectedShapeIndex].textFont;
            font.setFamily("YouYuan");
            shapes[selectedShapeIndex].textFont = font;
            canvasWidget->update();
        }
        });


    // 添加分隔符
    startToolbar->addSeparator();

    // 添加文本格式化按钮组
    QToolButton* boldButton = new QToolButton(this);
    boldButton->setIcon(QIcon(":/image/B.png"));
    boldButton->setToolTip("加粗");
    boldButton->setMinimumSize(40, 40);
    boldButton->setIconSize(QSize(24, 24));
    boldButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(boldButton);
    connect(boldButton, &QToolButton::clicked, this, &MainWindow::onBoldButtonClicked);

    QToolButton* italicButton = new QToolButton(this);
    italicButton->setIcon(QIcon(":/image/I.png"));
    italicButton->setToolTip("斜体");
    italicButton->setMinimumSize(40, 40);
    italicButton->setIconSize(QSize(24, 24));
    italicButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(italicButton);
    connect(italicButton, &QToolButton::clicked, this, &MainWindow::onItalicButtonClicked);

    QToolButton* underlineButton = new QToolButton(this);
    underlineButton->setIcon(QIcon(":/image/U.png"));
    underlineButton->setToolTip("下划线");
    underlineButton->setMinimumSize(40, 40);
    underlineButton->setIconSize(QSize(24, 24));
    underlineButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(underlineButton);
    connect(underlineButton, &QToolButton::clicked, this, &MainWindow::onUnderlineButtonClicked);

    // 添加分隔符
    startToolbar->addSeparator();

    // 添加文本颜色按钮
    QToolButton* textColorButton = new QToolButton(this);
    textColorButton->setIcon(QIcon(":/image/text_color.png"));
    textColorButton->setToolTip("文本颜色");
    textColorButton->setMinimumSize(40, 40);
    textColorButton->setIconSize(QSize(24, 24));
    textColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    startToolbar->addWidget(textColorButton);
    connect(textColorButton, &QToolButton::clicked, this, &MainWindow::onTextColorButtonClicked);


    // 添加分隔符
    startToolbar->addSeparator();

    // 添加文本对齐方式按钮组
    QToolButton* alignButton = new QToolButton(this);
    alignButton->setIcon(QIcon(":/image/alignment.png"));
    alignButton->setToolTip("文本对齐");
    alignButton->setMinimumSize(40, 40);
    alignButton->setIconSize(QSize(24, 24));
    alignButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    alignButton->setPopupMode(QToolButton::InstantPopup);
    startToolbar->addWidget(alignButton);

    // 创建对齐方式菜单
    QMenu* alignMenu = new QMenu(this);
    alignButton->setMenu(alignMenu);

    // 添加左对齐按钮
    QAction* leftAlignAction = new QAction(QIcon(":/image/align_left.png"), "左对齐", this);
    leftAlignAction->setCheckable(true);
    alignMenu->addAction(leftAlignAction);

    // 添加居中对齐按钮
    QAction* centerAlignAction = new QAction(QIcon(":/image/align_center.png"), "居中对齐", this);
    centerAlignAction->setCheckable(true);
    centerAlignAction->setChecked(true);  // 默认选中居中对齐
    alignMenu->addAction(centerAlignAction);

    // 添加右对齐按钮
    QAction* rightAlignAction = new QAction(QIcon(":/image/align_right.png"), "右对齐", this);
    rightAlignAction->setCheckable(true);
    alignMenu->addAction(rightAlignAction);

    // 创建按钮组
    QActionGroup* alignGroup = new QActionGroup(this);
    alignGroup->addAction(leftAlignAction);
    alignGroup->addAction(centerAlignAction);
    alignGroup->addAction(rightAlignAction);
    alignGroup->setExclusive(true);  // 确保只能选中一个

    // 连接信号
    connect(leftAlignAction, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            shapes[selectedShapeIndex].textAlignment = Qt::AlignLeft;
            canvasWidget->update();
        }
        });

    connect(centerAlignAction, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            shapes[selectedShapeIndex].textAlignment = Qt::AlignCenter;
            canvasWidget->update();
        }
        });

    connect(rightAlignAction, &QAction::triggered, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            shapes[selectedShapeIndex].textAlignment = Qt::AlignCenter;
            canvasWidget->update();
        }
        });

    // 添加分隔符
    startToolbar->addSeparator();

    // 添加边框颜色选择按钮
    QToolButton* borderColorButton = new QToolButton(this);
    borderColorButton->setIcon(QIcon(":/image/border_color.png"));
    borderColorButton->setText("线条颜色");
    borderColorButton->setToolTip("修改线条颜色");
    borderColorButton->setMinimumSize(100, 40);
    borderColorButton->setIconSize(QSize(24, 24));
    borderColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // 图标在文字旁边
    startToolbar->addWidget(borderColorButton);
    connect(borderColorButton, &QToolButton::clicked, this, &MainWindow::onBorderColorButtonClicked);

    // 添加线条粗细按钮
    QToolButton* lineWidthButton = new QToolButton(this);
    lineWidthButton->setIcon(QIcon(":/image/line_width.png"));
    lineWidthButton->setText("线条粗细");
    lineWidthButton->setToolTip("修改线条粗细");
    lineWidthButton->setMinimumSize(100, 40);
    lineWidthButton->setIconSize(QSize(24, 24));
    lineWidthButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // 图标在文字旁边
    startToolbar->addWidget(lineWidthButton);
    connect(lineWidthButton, &QToolButton::clicked, this, &MainWindow::showLineWidthMenu);

    // 添加填充颜色选择按钮
    QToolButton* fillColorButton = new QToolButton(this);
    fillColorButton->setIcon(QIcon(":/image/fill_color.png"));
    fillColorButton->setText("填充颜色");
    fillColorButton->setToolTip("修改填充颜色");
    fillColorButton->setMinimumSize(100, 40);
    fillColorButton->setIconSize(QSize(24, 24));
    fillColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // 图标在文字旁边
    startToolbar->addWidget(fillColorButton);
    connect(fillColorButton, &QToolButton::clicked, this, &MainWindow::onFillColorButtonClicked);

    // 添加线条样式按钮
    QToolButton* lineStyleButton = new QToolButton(this);
    lineStyleButton->setIcon(QIcon(":/image/line_style.png"));
    lineStyleButton->setText("线条样式");
    lineStyleButton->setToolTip("修改线条样式");
    lineStyleButton->setMinimumSize(100, 40);
    lineStyleButton->setIconSize(QSize(24, 24));
    lineStyleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    startToolbar->addWidget(lineStyleButton);
    connect(lineStyleButton, &QToolButton::clicked, this, &MainWindow::showLineStyleMenu);

    // 添加分隔符
    startToolbar->addSeparator();

}

void MainWindow::createArrangeToolbar()
{
    arrangeToolbar = new QToolBar("Arrange Tools", this);
    arrangeToolbar->setMovable(false);
    arrangeToolbar->setFloatable(false);
    arrangeToolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   min-height: 50px;"  // 设置最小高度
        "}"
        "QToolButton {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "   min-height: 40px;"  // 设置按钮最小高度
        "}"
        "QToolButton:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QToolButton:disabled {"
        "   background-color: #f8f8f8;"  // 禁用时的背景色
        "   border: 1px solid #e0e0e0;"  // 禁用时的边框
        "}"
        "QLabel {"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 18px;"  // 字体大小
        "   font-weight: bold;"  // 加粗字体
        "   padding: 5px;"
        "}"
        "QSpinBox {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 控件间距
        "   padding: 5px;"  // 内边距
        "   color: #5c4b51;"  // 文字颜色
        "   font-size: 18px;"  // 字体大小
        "   min-width: 80px;"  // 设置最小宽度
        "   min-height: 40px;"  // 设置最小高度
        "}"
        "QSpinBox:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
    );

    // 添加层次排列标签
    QLabel* layerLabel = new QLabel("Layer:", this);
    layerLabel->setStyleSheet(
        "QLabel {"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 20px;"  // 增大字体大小
        "   font-weight: bold;"  // 加粗字体
        "   padding: 5px;"
        "}"
    );
    arrangeToolbar->addWidget(layerLabel);

    // 添加置顶按钮
    QToolButton* bringToFrontBtn = new QToolButton(this);
    bringToFrontBtn->setIcon(QIcon(":/image/bring_to_front.png"));
    bringToFrontBtn->setToolTip("置顶");
    bringToFrontBtn->setMinimumSize(40, 40);
    bringToFrontBtn->setIconSize(QSize(24, 24));
    bringToFrontBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    arrangeToolbar->addWidget(bringToFrontBtn);
    connect(bringToFrontBtn, &QToolButton::clicked, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            Shape selectedShape = shapes[selectedShapeIndex];
            shapes.removeAt(selectedShapeIndex);
            shapes.append(selectedShape);
            selectedShapeIndex = shapes.size() - 1;
            canvasWidget->update();
        }
        });

    // 添加置底按钮
    QToolButton* sendToBackBtn = new QToolButton(this);
    sendToBackBtn->setIcon(QIcon(":/image/send_to_back.png"));
    sendToBackBtn->setToolTip("置底");
    sendToBackBtn->setMinimumSize(40, 40);
    sendToBackBtn->setIconSize(QSize(24, 24));
    sendToBackBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    arrangeToolbar->addWidget(sendToBackBtn);
    connect(sendToBackBtn, &QToolButton::clicked, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            Shape selectedShape = shapes[selectedShapeIndex];
            shapes.removeAt(selectedShapeIndex);
            shapes.prepend(selectedShape);
            selectedShapeIndex = 0;
            canvasWidget->update();
        }
        });

    // 添加上移按钮
    QToolButton* moveUpBtn = new QToolButton(this);
    moveUpBtn->setIcon(QIcon(":/image/move_up.png"));
    moveUpBtn->setToolTip("上移一层");
    moveUpBtn->setMinimumSize(40, 40);
    moveUpBtn->setIconSize(QSize(24, 24));
    moveUpBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    arrangeToolbar->addWidget(moveUpBtn);
    connect(moveUpBtn, &QToolButton::clicked, [this]() {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() - 1) {
            saveState();
            shapes.swapItemsAt(selectedShapeIndex, selectedShapeIndex + 1);
            selectedShapeIndex++;
            canvasWidget->update();
        }
        });

    // 添加下移按钮
    QToolButton* moveDownBtn = new QToolButton(this);
    moveDownBtn->setIcon(QIcon(":/image/move_down.png"));
    moveDownBtn->setToolTip("下移一层");
    moveDownBtn->setMinimumSize(40, 40);
    moveDownBtn->setIconSize(QSize(24, 24));
    moveDownBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    arrangeToolbar->addWidget(moveDownBtn);
    connect(moveDownBtn, &QToolButton::clicked, [this]() {
        if (selectedShapeIndex > 0) {
            saveState();
            shapes.swapItemsAt(selectedShapeIndex, selectedShapeIndex - 1);
            selectedShapeIndex--;
            canvasWidget->update();
        }
        });

    // 添加分隔符
    QWidget* separator = new QWidget(this);
    separator->setFixedWidth(20);
    arrangeToolbar->addWidget(separator);

    // 添加XYWH控件
    // X坐标控件
    QLabel* xLabel = new QLabel("X:", this);
    arrangeToolbar->addWidget(xLabel);

    QSpinBox* arrangeXSpinBox = new QSpinBox(this);
    arrangeXSpinBox->setRange(0, 10000);
    arrangeXSpinBox->setMinimumWidth(80);
    arrangeToolbar->addWidget(arrangeXSpinBox);

    // Y坐标控件
    QLabel* yLabel = new QLabel("Y:", this);
    arrangeToolbar->addWidget(yLabel);

    QSpinBox* arrangeYSpinBox = new QSpinBox(this);
    arrangeYSpinBox->setRange(0, 10000);
    arrangeYSpinBox->setMinimumWidth(80);
    arrangeToolbar->addWidget(arrangeYSpinBox);

    // 宽度控件
    QLabel* wLabel = new QLabel("W:", this);
    arrangeToolbar->addWidget(wLabel);

    QSpinBox* arrangeWidthSpinBox = new QSpinBox(this);
    arrangeWidthSpinBox->setRange(0, 10000);
    arrangeWidthSpinBox->setMinimumWidth(80);
    arrangeToolbar->addWidget(arrangeWidthSpinBox);

    // 高度控件
    QLabel* hLabel = new QLabel("H:", this);
    arrangeToolbar->addWidget(hLabel);

    QSpinBox* arrangeHeightSpinBox = new QSpinBox(this);
    arrangeHeightSpinBox->setRange(0, 10000);
    arrangeHeightSpinBox->setMinimumWidth(80);
    arrangeToolbar->addWidget(arrangeHeightSpinBox);

    // 保存SpinBox为类成员变量
    this->arrangeXSpinBox = arrangeXSpinBox;
    this->arrangeYSpinBox = arrangeYSpinBox;
    this->arrangeWidthSpinBox = arrangeWidthSpinBox;
    this->arrangeHeightSpinBox = arrangeHeightSpinBox;

    // 连接信号槽，使工具栏中的SpinBox与属性面板中的SpinBox同步
    connect(arrangeXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算偏移量
            int delta = value - shapes[selectedShapeIndex].rect.x();

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        QRect rect = shapes[index].rect;
                        rect.moveLeft(rect.x() + delta);
                        shapes[index].rect = rect;

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.moveLeft(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            // 更新属性面板中的值（阻止信号以避免循环）
            if (xSpinBox) {
                xSpinBox->blockSignals(true);
                xSpinBox->setValue(value);
                xSpinBox->blockSignals(false);
            }

            canvasWidget->update();
        }
        });

    connect(arrangeYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算偏移量
            int delta = value - shapes[selectedShapeIndex].rect.y();

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        QRect rect = shapes[index].rect;
                        rect.moveTop(rect.y() + delta);
                        shapes[index].rect = rect;

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.moveTop(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            // 更新属性面板中的值（阻止信号以避免循环）
            if (ySpinBox) {
                ySpinBox->blockSignals(true);
                ySpinBox->setValue(value);
                ySpinBox->blockSignals(false);
            }

            canvasWidget->update();
        }
        });

    connect(arrangeWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算缩放比例
            double ratio = 1.0;
            if (shapes[selectedShapeIndex].rect.width() > 0) {
                ratio = (double)value / shapes[selectedShapeIndex].rect.width();
            }

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        if (index == selectedShapeIndex) {
                            // 对当前选中的图形直接设置值
                            QRect rect = shapes[index].rect;
                            rect.setWidth(value);
                            shapes[index].rect = rect;
                        }
                        else {
                            // 对其他选中的图形按比例缩放
                            QRect rect = shapes[index].rect;
                            rect.setWidth(qRound(rect.width() * ratio));
                            shapes[index].rect = rect;
                        }

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.setWidth(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            // 更新属性面板中的值（阻止信号以避免循环）
            if (widthSpinBox) {
                widthSpinBox->blockSignals(true);
                widthSpinBox->setValue(value);
                widthSpinBox->blockSignals(false);
            }

            canvasWidget->update();
        }
        });

    connect(arrangeHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算缩放比例
            double ratio = 1.0;
            if (shapes[selectedShapeIndex].rect.height() > 0) {
                ratio = (double)value / shapes[selectedShapeIndex].rect.height();
            }

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        if (index == selectedShapeIndex) {
                            // 对当前选中的图形直接设置值
                            QRect rect = shapes[index].rect;
                            rect.setHeight(value);
                            shapes[index].rect = rect;
                        }
                        else {
                            // 对其他选中的图形按比例缩放
                            QRect rect = shapes[index].rect;
                            rect.setHeight(qRound(rect.height() * ratio));
                            shapes[index].rect = rect;
                        }

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.setHeight(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            // 更新属性面板中的值（阻止信号以避免循环）
            if (heightSpinBox) {
                heightSpinBox->blockSignals(true);
                heightSpinBox->setValue(value);
                heightSpinBox->blockSignals(false);
            }

            canvasWidget->update();
        }
        });

    // 监听selectionChanged信号，更新工具栏中的XYWH值
    connect(this, &MainWindow::selectionChanged, [=]() {
        bool hasSelection = selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size();

        arrangeXSpinBox->setEnabled(hasSelection);
        arrangeYSpinBox->setEnabled(hasSelection);
        arrangeWidthSpinBox->setEnabled(hasSelection);
        arrangeHeightSpinBox->setEnabled(hasSelection);

        if (hasSelection) {
            const Shape& shape = shapes[selectedShapeIndex];
            // 阻断信号防止循环更新
            arrangeXSpinBox->blockSignals(true);
            arrangeYSpinBox->blockSignals(true);
            arrangeWidthSpinBox->blockSignals(true);
            arrangeHeightSpinBox->blockSignals(true);

            arrangeXSpinBox->setValue(shape.rect.x());
            arrangeYSpinBox->setValue(shape.rect.y());
            arrangeWidthSpinBox->setValue(shape.rect.width());
            arrangeHeightSpinBox->setValue(shape.rect.height());

            arrangeXSpinBox->blockSignals(false);
            arrangeYSpinBox->blockSignals(false);
            arrangeWidthSpinBox->blockSignals(false);
            arrangeHeightSpinBox->blockSignals(false);
        }
        });

    // 更新按钮状态
    connect(this, &MainWindow::selectionChanged, [=]() {
        bool hasSelection = selectedShapeIndex >= 0;
        bool canMoveUp = hasSelection && selectedShapeIndex < shapes.size() - 1;
        bool canMoveDown = selectedShapeIndex > 0;

        bringToFrontBtn->setEnabled(hasSelection);
        sendToBackBtn->setEnabled(hasSelection);
        moveUpBtn->setEnabled(canMoveUp);
        moveDownBtn->setEnabled(canMoveDown);
        });
}

void MainWindow::createPageToolbar()
{
    pageToolbar = new QToolBar("Page Settings", this);
    pageToolbar->setMovable(false);
    pageToolbar->setFloatable(false);
    pageToolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   min-height: 50px;"  // 设置最小高度
        "}"
        "QLabel {"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 20px;"  // 增大字体大小
        "   font-weight: bold;"  // 加粗字体
        "   padding: 5px;"
        "}"
        "QPushButton {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px 15px;"  // 内边距
        "   color: #5c4b51;"  // 文字颜色
        "   font-size: 20px;"  // 增大字体大小
        "   min-height: 40px;"  // 设置按钮最小高度
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QSpinBox {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "   color: #5c4b51;"  // 文字颜色
        "   font-size: 20px;"  // 增大字体大小
        "   min-width: 100px;"  // 设置最小宽度
        "   min-height: 40px;"  // 设置最小高度
        "}"
        "QSpinBox:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QComboBox {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px 15px;"  // 内边距
        "   color: #5c4b51;"  // 文字颜色
        "   font-size: 20px;"  // 增大字体大小
        "   min-width: 120px;"  // 设置最小宽度
        "   min-height: 40px;"  // 设置最小高度
        "}"
        "QComboBox:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QComboBox::drop-down {"
        "   border: none;"  // 移除下拉按钮的边框
        "   width: 30px;"  // 设置下拉按钮的宽度
        "}"
        "QComboBox::down-arrow {"
        "   image: url(:/image/arrow_down.png);"  // 使用自定义的下拉箭头图标
        "   width: 20px;"  // 设置箭头图标的宽度
        "   height: 20px;"  // 设置箭头图标的高度
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #f0f0f0;"  // 下拉列表的背景色
        "   border: 1px solid #c0c0c0;"  // 下拉列表的边框
        "   border-radius: 8px;"  // 下拉列表的圆角
        "   selection-background-color: #d8e1e9;"  // 选中项的背景色
        "   selection-color: #5c4b51;"  // 选中项的文字颜色
        "   padding: 5px;"  // 内边距
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 30px;"  // 设置每个选项的最小高度
        "   padding: 5px 15px;"  // 设置每个选项的内边距
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "}"
        "QCheckBox::indicator {"
        "   width: 20px;"  // 增大复选框大小
        "   height: 20px;"  // 增大复选框大小
        "   border: 1px solid #c0c0c0;"
        "   border-radius: 4px;"
        "   background-color: #f0f0f0;"
        "}"
        "QCheckBox::indicator:hover {"
        "   border: 2px solid #5c4b51;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #5c4b51;"
        "}"
    );

    // 1. Background Color Settings
    QLabel* bgColorLabel = new QLabel("背景颜色:", this);
    pageToolbar->addWidget(bgColorLabel);

    QPushButton* bgColorBtn = new QPushButton("Select Color", this);
    pageToolbar->addWidget(bgColorBtn);
    connect(bgColorBtn, &QPushButton::clicked, this, &MainWindow::onBackgroundColorClicked);

    pageToolbar->addSeparator();

    // 2. Page Size Settings
    QLabel* widthLabel = new QLabel("W:", this);
    pageToolbar->addWidget(widthLabel);

    QSpinBox* widthSpin = new QSpinBox(this);
    widthSpin->setRange(100, 5000);
    widthSpin->setValue(pageWidth);
    widthSpin->setSingleStep(100);
    widthSpin->setSuffix(" px");
    pageToolbar->addWidget(widthSpin);
    connect(widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
        [=](int value) { this->onPageSizeChanged(value, pageHeight); });

    QLabel* heightLabel = new QLabel("H:", this);
    pageToolbar->addWidget(heightLabel);

    QSpinBox* heightSpin = new QSpinBox(this);
    heightSpin->setRange(100, 5000);
    heightSpin->setValue(pageHeight);
    heightSpin->setSingleStep(100);
    heightSpin->setSuffix(" px");
    pageToolbar->addWidget(heightSpin);
    connect(heightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
        [=](int value) { this->onPageSizeChanged(pageWidth, value); });

    pageToolbar->addSeparator();

    // 3. Grid Display Settings
    QLabel* gridLabel = new QLabel("网格:", this);
    pageToolbar->addWidget(gridLabel);

    QComboBox* gridSizeCombo = new QComboBox(this);
    gridSizeCombo->addItem("无", None);
    gridSizeCombo->addItem("小", Small);
    gridSizeCombo->addItem("中", Medium);
    gridSizeCombo->addItem("大", Large);
    gridSizeCombo->setCurrentIndex(currentGridSize);
    pageToolbar->addWidget(gridSizeCombo);

    connect(gridSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        currentGridSize = static_cast<GridSize>(gridSizeCombo->itemData(index).toInt());
        showGrid = (currentGridSize != None);
        canvasWidget->update();
        });

    // 添加分隔线
    pageToolbar->addSeparator();

    // 4. 添加缩放控制
    QLabel* zoomLabel = new QLabel("缩放:", this);
    pageToolbar->addWidget(zoomLabel);

    // 添加缩小按钮
    QToolButton* zoomOutBtn = new QToolButton(this);
    zoomOutBtn->setIcon(QIcon(":/image/zoom_out.png"));
    zoomOutBtn->setToolTip("放大");
    zoomOutBtn->setMinimumSize(40, 40);
    zoomOutBtn->setIconSize(QSize(24, 24));
    zoomOutBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pageToolbar->addWidget(zoomOutBtn);
    connect(zoomOutBtn, &QToolButton::clicked, this, &MainWindow::zoomOut);

    // 添加放大按钮
    QToolButton* zoomInBtn = new QToolButton(this);
    zoomInBtn->setIcon(QIcon(":/image/zoom_in.png"));
    zoomInBtn->setToolTip("缩小");
    zoomInBtn->setMinimumSize(40, 40);
    zoomInBtn->setIconSize(QSize(24, 24));
    zoomInBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pageToolbar->addWidget(zoomInBtn);
    connect(zoomInBtn, &QToolButton::clicked, this, &MainWindow::zoomIn);

    // 添加缩放比例显示标签
    QLabel* zoomRatioLabel = new QLabel(QString("%1%").arg(int(scaleFactor * 100)), this);
    zoomRatioLabel->setStyleSheet(
        "QLabel {"
        "   color: #5c4b51;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   min-width: 60px;"
        "}"
    );
    pageToolbar->addWidget(zoomRatioLabel);

    // 更新缩放比例显示
    connect(this, &MainWindow::scaleFactorChanged, [=](qreal factor) {
        zoomRatioLabel->setText(QString("%1%").arg(int(factor * 100)));
        });
}

void MainWindow::createExportToolbar()
{
    exportToolbar = new QToolBar("Export Tools", this);
    exportToolbar->setMovable(false);
    exportToolbar->setFloatable(false);
    exportToolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   min-height: 50px;"  // 设置最小高度
        "}"
        "QToolButton {"
        "   background-color: #f0f0f0;"  // 浅灰色背景
        "   border: 1px solid #c0c0c0;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "   min-height: 40px;"  // 设置按钮最小高度
        "   min-width: 40px;"   // 设置按钮最小宽度
        "}"
        "QToolButton:hover {"
        "   background-color: #e0e0e0;"  // 悬停时的背景色
        "   border: 2px solid #5c4b51;"  // 悬停时的边框
        "}"
        "QToolButton:checked {"
        "   background-color: #c0c0c0;"  // 选中时的背景色
        "   border: 2px solid #5c4b51;"  // 选中时的边框
        "}"
    );

    // 添加导出PNG按钮
    QToolButton* exportPngBtn = new QToolButton(this);
    exportPngBtn->setIcon(QIcon(":/image/PNG.png"));
    exportPngBtn->setToolTip("导出为PNG格式");
    exportPngBtn->setMinimumSize(40, 40);
    exportPngBtn->setIconSize(QSize(24, 24));
    exportPngBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);  // 只显示图标
    exportToolbar->addWidget(exportPngBtn);

    // 添加导出SVG按钮
    QToolButton* exportSvgBtn = new QToolButton(this);
    exportSvgBtn->setIcon(QIcon(":/image/SVG.png"));
    exportSvgBtn->setToolTip("导出为SVG格式");
    exportSvgBtn->setMinimumSize(40, 40);
    exportSvgBtn->setIconSize(QSize(24, 24));
    exportSvgBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);  // 只显示图标
    exportToolbar->addWidget(exportSvgBtn);

    // 连接导出PNG按钮的信号槽
    connect(exportPngBtn, &QToolButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Export as PNG", "", "PNG Files (*.png)");
        if (!fileName.isEmpty()) {
            QPixmap pixmap(canvasWidget->size());
            canvasWidget->render(&pixmap);
            pixmap.save(fileName, "PNG");
        }
        });

    // 连接导出SVG按钮的信号槽
    connect(exportSvgBtn, &QToolButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Export as SVG", "", "SVG Files (*.svg)");
        if (!fileName.isEmpty()) {
            // 创建SVG文件
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);

                // 写入SVG头部
                out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
                out << "<svg width=\"" << pageWidth << "\" height=\"" << pageHeight << "\" "
                    << "xmlns=\"http://www.w3.org/2000/svg\">\n";

                // 设置背景
                out << "<rect width=\"100%\" height=\"100%\" fill=\""
                    << backgroundColor.name() << "\"/>\n";

                // 绘制所有图形
                for (const Shape& shape : shapes) {
                    if (shape.type == "Rectangle") {
                        out << "<rect x=\"" << shape.rect.x() << "\" y=\"" << shape.rect.y() << "\" "
                            << "width=\"" << shape.rect.width() << "\" height=\"" << shape.rect.height() << "\" "
                            << "fill=\"" << shape.color.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";
                    }
                    else if (shape.type == "Circle") {
                        out << "<circle cx=\"" << shape.rect.center().x() << "\" "
                            << "cy=\"" << shape.rect.center().y() << "\" "
                            << "r=\"" << shape.rect.width() / 2 << "\" "
                            << "fill=\"" << shape.color.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";
                    }
                    else if (shape.type == "Triangle") {
                        QPoint top(shape.rect.left() + shape.rect.width() / 2, shape.rect.top());
                        QPoint bottomRight(shape.rect.right(), shape.rect.bottom());
                        QPoint bottomLeft(shape.rect.left(), shape.rect.bottom());

                        out << "<polygon points=\""
                            << top.x() << "," << top.y() << " "
                            << bottomRight.x() << "," << bottomRight.y() << " "
                            << bottomLeft.x() << "," << bottomLeft.y() << "\" "
                            << "fill=\"" << shape.color.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";
                    }
                    else if (shape.type == "Arrow") {
                        QPoint start = shape.rect.topLeft();
                        QPoint end = shape.rect.bottomRight();
                        QLineF line(start, end);
                        double angle = line.angle();
                        double arrowSize = 20.0;
                        double arrowAngle = 30.0;

                        // 计算箭头头部的两个点
                        QPointF arrowP1 = end - QPointF(
                            arrowSize * cos((angle + arrowAngle) * M_PI / 180.0),
                            -arrowSize * sin((angle + arrowAngle) * M_PI / 180.0)
                        );
                        QPointF arrowP2 = end - QPointF(
                            arrowSize * cos((angle - arrowAngle) * M_PI / 180.0),
                            -arrowSize * sin((angle - arrowAngle) * M_PI / 180.0)
                        );

                        // 绘制箭头主体
                        out << "<line x1=\"" << start.x() << "\" y1=\"" << start.y() << "\" "
                            << "x2=\"" << end.x() << "\" y2=\"" << end.y() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";

                        // 绘制箭头头部
                        out << "<polygon points=\""
                            << end.x() << "," << end.y() << " "
                            << arrowP1.x() << "," << arrowP1.y() << " "
                            << arrowP2.x() << "," << arrowP2.y() << "\" "
                            << "fill=\"" << shape.borderColor.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\"/>\n";
                    }
                    else if (shape.type == "Diamond") {
                        QPoint center = shape.rect.center();
                        int width = shape.rect.width() / 2;
                        int height = shape.rect.height() / 2;

                        out << "<polygon points=\""
                            << center.x() << "," << (center.y() - height) << " "
                            << (center.x() + width) << "," << center.y() << " "
                            << center.x() << "," << (center.y() + height) << " "
                            << (center.x() - width) << "," << center.y() << "\" "
                            << "fill=\"" << shape.color.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";
                    }
                    else if (shape.type == "StartEnd") {
                        int radius = qMin(shape.rect.width(), shape.rect.height()) / 2;
                        out << "<rect x=\"" << shape.rect.x() << "\" y=\"" << shape.rect.y() << "\" "
                            << "width=\"" << shape.rect.width() << "\" height=\"" << shape.rect.height() << "\" "
                            << "rx=\"" << radius << "\" ry=\"" << radius << "\" "
                            << "fill=\"" << shape.color.name() << "\" "
                            << "stroke=\"" << shape.borderColor.name() << "\" "
                            << "stroke-width=\"" << shape.lineWidth << "\" "
                            << "stroke-dasharray=\"" << getDashArray(shape.penStyle) << "\"/>\n";
                    }

                    // 如果有文本，添加文本元素
                    if (!shape.text.isEmpty()) {
                        out << "<text x=\"" << shape.rect.center().x() << "\" "
                            << "y=\"" << shape.rect.center().y() << "\" "
                            << "text-anchor=\"middle\" dominant-baseline=\"middle\" "
                            << "fill=\"" << shape.textColor.name() << "\" "
                            << "font-family=\"" << shape.textFont.family() << "\" "
                            << "font-size=\"" << shape.textFont.pointSize() << "\">"
                            << shape.text << "</text>\n";
                    }
                }

                // 写入SVG尾部
                out << "</svg>";
                file.close();
            }
        }
        });
}

// 辅助函数：将Qt的PenStyle转换为SVG的stroke-dasharray
QString MainWindow::getDashArray(Qt::PenStyle style) {
    switch (style) {
    case Qt::SolidLine:
        return "none";
    case Qt::DashLine:
        return "5,5";
    case Qt::DotLine:
        return "1,3";
    case Qt::DashDotLine:
        return "5,3,1,3";
    case Qt::DashDotDotLine:
        return "5,3,1,3,1,3";
    default:
        return "none";
    }
}

void MainWindow::hideAllSecondaryToolbars()
{
    startToolbar->hide();
    arrangeToolbar->hide();
    pageToolbar->hide();
    exportToolbar->hide();  // 添加隐藏Export工具栏
}

// Slot implementations
void MainWindow::onStartClicked()
{
    hideAllSecondaryToolbars();
    startToolbar->show();
    // 更新按钮状态
    startBtn->setChecked(true);
    arrangeBtn->setChecked(false);
    pageBtn->setChecked(false);
    exportBtn->setChecked(false);
    update();
}

void MainWindow::onArrangeClicked()
{
    hideAllSecondaryToolbars();
    arrangeToolbar->show();
    // 更新按钮状态
    startBtn->setChecked(false);
    arrangeBtn->setChecked(true);
    pageBtn->setChecked(false);
    exportBtn->setChecked(false);
    update();
}

void MainWindow::onPageClicked()
{
    hideAllSecondaryToolbars();
    pageToolbar->show();
    // 更新按钮状态
    startBtn->setChecked(false);
    arrangeBtn->setChecked(false);
    pageBtn->setChecked(true);
    exportBtn->setChecked(false);
    update();
}

void MainWindow::onBackgroundColorClicked()
{
    QColor color = QColorDialog::getColor(backgroundColor, this, "Select Background Color");
    if (color.isValid()) {
        backgroundColor = color;
        canvasWidget->update();
    }
}

void MainWindow::onPageSizeChanged(int width, int height)
{
    // 计算新的缩放因子
    qreal newScaleFactor = qMin(
        static_cast<qreal>(width) / pageWidth,
        static_cast<qreal>(height) / pageHeight
    );

    // 更新缩放因子
    scaleFactor = newScaleFactor;

    // 更新页面尺寸
    pageWidth = width;
    pageHeight = height;

    // 更新Page菜单栏中的宽度和高度值
    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

    if (widthSpin) {
        widthSpin->blockSignals(true);  // 阻止信号循环
        widthSpin->setValue(width);
        widthSpin->blockSignals(false);
    }

    if (heightSpin) {
        heightSpin->blockSignals(true);  // 阻止信号循环
        heightSpin->setValue(height);
        heightSpin->blockSignals(false);
    }

    // 发送缩放因子改变信号，更新百分比显示
    emit scaleFactorChanged(scaleFactor);

    canvasWidget->update();
}

void MainWindow::onGridToggled(bool checked)
{
    showGrid = checked;
    canvasWidget->update();
}

void MainWindow::onToolSelected(const QString& tool)
{
    currentTool = tool;
    update();
}

void MainWindow::onAlignmentChanged(const QString& alignment)
{
    alignmentOption = alignment;
    update();
}

void MainWindow::onUndoClicked()
{
    undo();
}

void MainWindow::onRedoClicked()
{
    if (!redoStack.isEmpty()) {
        // 保存当前状态到撤销栈
        undoStack.append(shapes);

        // 获取重做状态
        shapes = redoStack.last();
        redoStack.removeLast();

        canvasWidget->update();
    }
}

void MainWindow::onBorderColorButtonClicked()
{
    // 检查是否有选中的形状
    if (!selectedShapeIndices.isEmpty() || (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())) {
        // 获取当前选中形状的边框颜色作为初始颜色
        QColor currentColor;
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            currentColor = shapes[selectedShapeIndex].borderColor.isValid() ?
                shapes[selectedShapeIndex].borderColor :
                shapes[selectedShapeIndex].color.darker(150);
        }
        else if (!selectedShapeIndices.isEmpty() && selectedShapeIndices.first() >= 0 && selectedShapeIndices.first() < shapes.size()) {
            // 如果没有当前选中形状，但有多选形状，使用第一个选中形状的颜色
            currentColor = shapes[selectedShapeIndices.first()].borderColor.isValid() ?
                shapes[selectedShapeIndices.first()].borderColor :
                shapes[selectedShapeIndices.first()].color.darker(150);
        }

        // 设置对话框标题
        QString dialogTitle = "Select Border Color";

        // 获取用户选择的颜色
        QColor color = QColorDialog::getColor(currentColor, this, dialogTitle);

        if (color.isValid()) {
            // 保存状态用于撤销
            saveState();

            // 检查是否有多个选中的形状
            if (!selectedShapeIndices.isEmpty()) {
                // 批量更新所有选中形状的边框颜色
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        shapes[index].borderColor = color;
                    }
                }
            }
            // 如果只有一个形状被选中
            else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                shapes[selectedShapeIndex].borderColor = color;
            }

            // 更新边框颜色预览
            QLabel* borderColorPreview = propertyPanel->findChild<QLabel*>("borderColorPreview");
            if (borderColorPreview && selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                QString colorStyle = QString("QLabel { "
                    "border: 1px solid #d8e1e9; "
                    "border-radius: 4px; "
                    "background-color: %1; "
                    "min-height: 20px; "
                    "}")
                    .arg(color.name());
                borderColorPreview->setStyleSheet(colorStyle);
            }

            canvasWidget->update();
        }
    }
    else if (currentShape) {
        // 如果正在绘制新形状，可以设置其边框颜色
        QColor currentColor = currentShape->borderColor.isValid() ?
            currentShape->borderColor :
            currentShape->color.darker(150);

        // 根据形状类型设置不同的对话框标题
        QString dialogTitle;
        if (currentShape->type == "StartEnd") {
            dialogTitle = "Select Start/End Border Color";
        }
        else if (currentShape->type == "Diamond") {
            dialogTitle = "Select Diamond Border Color";
        }
        else {
            dialogTitle = "Select Border Color";
        }

        QColor color = QColorDialog::getColor(currentColor, this, dialogTitle);

        if (color.isValid()) {
            currentShape->borderColor = color;

            // 更新边框颜色预览
            QLabel* borderColorPreview = propertyPanel->findChild<QLabel*>("borderColorPreview");
            if (borderColorPreview) {
                QString colorStyle = QString("QLabel { "
                    "border: 1px solid #d8e1e9; "
                    "border-radius: 4px; "
                    "background-color: %1; "
                    "min-height: 20px; "
                    "}")
                    .arg(color.name());
                borderColorPreview->setStyleSheet(colorStyle);
            }

            canvasWidget->update();
        }
    }
    else {
        // 提示用户需要先选择一个形状
        QMessageBox::information(this, "Select Shape", "请先选择一个形状。");
    }
}

void MainWindow::onFillColorButtonClicked()
{
    // 检查是否有选中的形状
    if (!selectedShapeIndices.isEmpty() || (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())) {
        // 获取当前选中形状的填充颜色作为初始颜色
        QColor currentColor;
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            currentColor = shapes[selectedShapeIndex].color;
        }
        else if (!selectedShapeIndices.isEmpty() && selectedShapeIndices.first() >= 0 && selectedShapeIndices.first() < shapes.size()) {
            // 如果没有当前选中形状，但有多选形状，使用第一个选中形状的颜色
            currentColor = shapes[selectedShapeIndices.first()].color;
        }

        // 获取用户选择的颜色
        QColor color = QColorDialog::getColor(currentColor, this, "Select Fill Color");

        if (color.isValid()) {
            // 保存状态用于撤销
            saveState();

            // 检查是否有多个选中的形状
            if (!selectedShapeIndices.isEmpty()) {
                // 批量更新所有选中形状的填充颜色
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        shapes[index].color = color;
                    }
                }
            }
            // 如果只有一个形状被选中
            else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                shapes[selectedShapeIndex].color = color;
            }

            // 更新填充颜色预览
            QLabel* fillColorPreview = propertyPanel->findChild<QLabel*>("fillColorPreview");
            if (fillColorPreview && selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                QString colorStyle = QString("QLabel { border: 1px solid #d8e1e9; border-radius: 4px; background-color: %1; }")
                    .arg(color.name());
                fillColorPreview->setStyleSheet(colorStyle);
            }

            canvasWidget->update();
        }
    }
    else if (currentShape) {
        // 如果正在绘制新形状，可以设置其填充颜色
        QColor color = QColorDialog::getColor(currentShape->color, this, "Select Fill Color");

        if (color.isValid()) {
            currentShape->color = color;

            // 更新填充颜色预览
            QLabel* fillColorPreview = propertyPanel->findChild<QLabel*>("fillColorPreview");
            if (fillColorPreview) {
                QString colorStyle = QString("QLabel { border: 1px solid #d8e1e9; border-radius: 4px; background-color: %1; }")
                    .arg(color.name());
                fillColorPreview->setStyleSheet(colorStyle);
            }

            canvasWidget->update();
        }
    }
    else {
        // 提示用户需要先选择一个形状
        QMessageBox::information(this, "Select Shape", "请先选择一个形状。");
    }
}

void MainWindow::saveState()
{
    // 保存当前状态到撤销栈
    QList<Shape> currentState;
    for (const Shape& shape : shapes) {
        currentState.append(shape);
    }
    undoStack.append(currentState);

    // 清空重做栈
    redoStack.clear();
}

void MainWindow::undo()
{
    if (!undoStack.isEmpty()) {
        // 保存当前状态到重做栈
        QList<Shape> currentState;
        for (const Shape& shape : shapes) {
            currentState.append(shape);
        }
        redoStack.append(currentState);

        // 恢复上一个状态
        shapes = undoStack.takeLast();

        // 清除选择
        selectedShapeIndex = -1;
        emit selectionChanged();

        canvasWidget->update();
    }
}

void MainWindow::redo()
{
    if (!redoStack.isEmpty()) {
        // 保存当前状态到撤销栈
        QList<Shape> currentState;
        for (const Shape& shape : shapes) {
            currentState.append(shape);
        }
        undoStack.append(currentState);

        // 恢复下一个状态
        shapes = redoStack.takeLast();

        // 清除选择
        selectedShapeIndex = -1;
        emit selectionChanged();

        canvasWidget->update();
    }
}

void MainWindow::createLeftToolbar()
{
    // 创建左侧工具栏
    leftToolbar = new QToolBar("Drawing Tools", this);
    leftToolbar->setOrientation(Qt::Vertical);
    leftToolbar->setMovable(false);
    leftToolbar->setFloatable(false);
    leftToolbar->setMinimumWidth(300);

    // 设置工具栏样式
    leftToolbar->setStyleSheet(
        "QToolBar {"
        "   background-color: #e6e8f0;"  // 莫兰迪淡蓝色背景
        "   border-radius: 10px;"
        "   padding: 10px;"
        "}"
        "QToolButton {"
        "   background-color: #f02f8;"  // 更浅的淡蓝色背景
        "   border: 1px solid #d8dce8;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "}"
        "QToolButton:hover {"
        "   background-color: #e6e8f0;"  // 悬停时的背景色
        "   border: 2px solid #b8c0d8;"  // 悬停时的边框
        "}"
        "QToolButton:checked {"
        "   background-color: #d8dce8;"  // 选中时的背景色
        "   border: 2px solid #b8c0d8;"  // 选中时的边框
        "}"
        "QPushButton {"
        "   background-color: #f0f2f8;"  // 更浅的淡蓝色背景
        "   border: 1px solid #d8dce8;"  // 边框颜色
        "   border-radius: 8px;"  // 圆角
        "   margin: 3px;"  // 按钮间距
        "   padding: 5px;"  // 内边距
        "   text-align: left;"  // 文字左对齐
        "   font-size: 20px;"  // 字体大小
        "   font-weight: bold;"  // 加粗字体
        "   color: #5c4b51;"  // 文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: #e6e8f0;"  // 悬停时的背景色
        "   background-color: #e6d5c3;"  // 悬停时的背景色
        "   border: 2px solid #8b7d6b;"  // 悬停时的边框
        "}"
    );

    // 创建工具栏内容容器
    QWidget* toolbarContent = new QWidget(leftToolbar);
    QVBoxLayout* vLayout = new QVBoxLayout(toolbarContent);
    vLayout->setContentsMargins(5, 5, 5, 5);
    vLayout->setSpacing(5);

    // 创建基础工具组
    basicToolsGroup = new QWidget(this);
    QVBoxLayout* basicLayout = new QVBoxLayout(basicToolsGroup);
    basicLayout->setContentsMargins(0, 0, 0, 0);
    basicLayout->setSpacing(5);

    // 创建基础工具标题按钮
    basicTitleBtn = new QPushButton("Basic Tools", this);
    basicTitleBtn->setCheckable(true);
    basicTitleBtn->setChecked(true);  // 默认展开
    basicToolsExpanded = true;
    basicTitleBtn->setMinimumHeight(40);
    basicTitleBtn->setIcon(QIcon(":/image/arrow_down.png"));  // 默认显示下三角
    basicTitleBtn->setIconSize(QSize(18, 18));
    vLayout->addWidget(basicTitleBtn);

    // 创建基础工具按钮容器
    QWidget* basicButtonsContainer = new QWidget(this);
    QGridLayout* basicGrid = new QGridLayout(basicButtonsContainer);
    basicGrid->setContentsMargins(0, 0, 0, 0);
    basicGrid->setSpacing(5);

    // 创建基础工具按钮
    rectangleTool = new QToolButton(this);
    rectangleTool->setIcon(QIcon(":/image/rectangle.png"));
    rectangleTool->setToolTip("矩形");
    rectangleTool->setCheckable(true);
    rectangleTool->setMinimumSize(80, 80);
    rectangleTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    rectangleTool->installEventFilter(this);  // 安装事件过滤器
    basicGrid->addWidget(rectangleTool, 0, 0);

    circleTool = new QToolButton(this);
    circleTool->setIcon(QIcon(":/image/circle.png"));
    circleTool->setToolTip("圆形");
    circleTool->setCheckable(true);
    circleTool->setMinimumSize(80, 80);
    circleTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    circleTool->installEventFilter(this);  // 安装事件过滤器
    basicGrid->addWidget(circleTool, 0, 1);

    triangleTool = new QToolButton(this);
    triangleTool->setIcon(QIcon(":/image/triangle.png"));
    triangleTool->setToolTip("三角形");
    triangleTool->setCheckable(true);
    triangleTool->setMinimumSize(80, 80);
    triangleTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    triangleTool->installEventFilter(this);  // 安装事件过滤器
    basicGrid->addWidget(triangleTool, 0, 2);

    arrowTool = new QToolButton(this);
    arrowTool->setIcon(QIcon(":/image/arrow.png"));
    arrowTool->setToolTip("箭头");
    arrowTool->setCheckable(true);
    arrowTool->setMinimumSize(80, 80);
    arrowTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    arrowTool->installEventFilter(this);  // 安装事件过滤器
    basicGrid->addWidget(arrowTool, 1, 0);

    basicLayout->addWidget(basicButtonsContainer);
    vLayout->addWidget(basicToolsGroup);

    // 创建流程图工具组
    flowToolsGroup = new QWidget(this);
    QVBoxLayout* flowLayout = new QVBoxLayout(flowToolsGroup);
    flowLayout->setContentsMargins(0, 0, 0, 0);
    flowLayout->setSpacing(5);

    // 创建流程图工具标题按钮
    flowTitleBtn = new QPushButton("Flow Chart Tools", this);
    flowTitleBtn->setCheckable(true);
    flowTitleBtn->setChecked(true);  // 默认展开
    flowToolsExpanded = true;
    flowTitleBtn->setMinimumHeight(40);
    flowTitleBtn->setIcon(QIcon(":/image/arrow_down.png"));  // 默认显示下三角
    flowTitleBtn->setIconSize(QSize(24, 24));
    vLayout->addWidget(flowTitleBtn);

    // 创建流程图工具按钮容器
    QWidget* flowButtonsContainer = new QWidget(this);
    QGridLayout* flowGrid = new QGridLayout(flowButtonsContainer);
    flowGrid->setContentsMargins(0, 0, 0, 0);
    flowGrid->setSpacing(5);

    // 创建流程图工具按钮
    startEndTool = new QToolButton(this);
    startEndTool->setIcon(QIcon(":/image/start_end.png"));
    startEndTool->setToolTip("开始/结束");
    startEndTool->setCheckable(true);
    startEndTool->setMinimumSize(80, 80);
    startEndTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    startEndTool->installEventFilter(this);  // 安装事件过滤器
    flowGrid->addWidget(startEndTool, 0, 0);

    diamondTool = new QToolButton(this);
    diamondTool->setIcon(QIcon(":/image/diamond.png"));
    diamondTool->setToolTip("菱形");
    diamondTool->setCheckable(true);
    diamondTool->setMinimumSize(80, 80);
    diamondTool->setIconSize(QSize(48, 48));  // 增大图标尺寸
    diamondTool->installEventFilter(this);  // 安装事件过滤器
    flowGrid->addWidget(diamondTool, 0, 1);

    flowLayout->addWidget(flowButtonsContainer);
    vLayout->addWidget(flowToolsGroup);

    // 添加弹性空间
    vLayout->addStretch();

    // 设置工具栏内容
    toolbarContent->setLayout(vLayout);
    leftToolbar->addWidget(toolbarContent);

    // 连接信号槽
    connect(rectangleTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);
    connect(circleTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);
    connect(triangleTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);
    connect(arrowTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);
    connect(startEndTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);
    connect(diamondTool, &QToolButton::clicked, this, &MainWindow::onShapeToolClicked);

    // 连接折叠按钮的信号槽
    connect(basicTitleBtn, &QPushButton::clicked, [=](bool checked) {
        basicToolsGroup->setVisible(checked);
        basicToolsExpanded = checked;
        basicTitleBtn->setIcon(QIcon(checked ? ":/image/arrow_down.png" : ":/image/arrow_right.png"));
        });

    connect(flowTitleBtn, &QPushButton::clicked, [=](bool checked) {
        flowToolsGroup->setVisible(checked);
        flowToolsExpanded = checked;
        flowTitleBtn->setIcon(QIcon(checked ? ":/image/arrow_down.png" : ":/image/arrow_right.png"));
        });
}

void MainWindow::onShapeToolClicked()
{
    // 取消其他按钮的选中状态
    QToolButton* sender = qobject_cast<QToolButton*>(QObject::sender());
    if (!sender) return;

    rectangleTool->setChecked(sender == rectangleTool);
    circleTool->setChecked(sender == circleTool);
    triangleTool->setChecked(sender == triangleTool);
    arrowTool->setChecked(sender == arrowTool);
    startEndTool->setChecked(sender == startEndTool);
    diamondTool->setChecked(sender == diamondTool);

    // 清除选择状态
    clearSelection();
    canvasWidget->update();

    // 设置当前工具
    if (sender == rectangleTool) currentTool = "Rectangle";
    else if (sender == circleTool) currentTool = "Circle";
    else if (sender == triangleTool) currentTool = "Triangle";
    else if (sender == arrowTool) currentTool = "Arrow";
    else if (sender == startEndTool) currentTool = "StartEnd";
    else if (sender == diamondTool) currentTool = "Diamond";
}

void MainWindow::centerCanvas()
{
    // 获取画布区域的大小
    QSize scrollAreaSize = scrollArea->viewport()->size();

    // 计算画布应该的位置，使其居中
    int x = (scrollAreaSize.width() - pageWidth * scaleFactor) / 2;
    int y = (scrollAreaSize.height() - pageHeight * scaleFactor) / 2;

    // 设置画布位置
    canvasPosition = QPoint(x, y);
    canvasWidget->update();
}

void MainWindow::zoomIn()
{
    scaleFactor *= 1.2;  // 放大20%
    canvasWidget->update();
    centerCanvas();
    emit scaleFactorChanged(scaleFactor);  // 发送缩放因子改变信号

    // 更新Page菜单栏中的宽度和高度值
    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

    if (widthSpin) {
        widthSpin->blockSignals(true);  // 阻止信号循环
        widthSpin->setValue(pageWidth * scaleFactor);
        widthSpin->blockSignals(false);
    }

    if (heightSpin) {
        heightSpin->blockSignals(true);  // 阻止信号循环
        heightSpin->setValue(pageHeight * scaleFactor);
        heightSpin->blockSignals(false);
    }
}

void MainWindow::zoomOut()
{
    scaleFactor /= 1.2;  // 缩小20%
    canvasWidget->update();
    centerCanvas();
    emit scaleFactorChanged(scaleFactor);  // 发送缩放因子改变信号

    // 更新Page菜单栏中的宽度和高度值
    QSpinBox* widthSpin = pageToolbar->findChild<QSpinBox*>();
    QSpinBox* heightSpin = pageToolbar->findChildren<QSpinBox*>().at(1);

    if (widthSpin) {
        widthSpin->blockSignals(true);  // 阻止信号循环
        widthSpin->setValue(pageWidth * scaleFactor);
        widthSpin->blockSignals(false);
    }

    if (heightSpin) {
        heightSpin->blockSignals(true);  // 阻止信号循环
        heightSpin->setValue(pageHeight * scaleFactor);
        heightSpin->blockSignals(false);
    }
}

void MainWindow::wheelEvent(QWheelEvent* event)
{
    QMainWindow::wheelEvent(event);
}

void MainWindow::onDeleteButtonClicked()
{
    // 检查是否有多个图形被选中
    if (!selectedShapeIndices.isEmpty()) {
        // 保存状态用于撤销
        saveState();

        // 从后向前删除选中的图形，避免索引变化问题
        QList<int> indicesToDelete = selectedShapeIndices;
        std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<int>());

        for (int index : indicesToDelete) {
            if (index >= 0 && index < shapes.size()) {
                shapes.removeAt(index);
            }
        }

        // 清空选择
        selectedShapeIndex = -1;
        selectedShapeIndices.clear();

        canvasWidget->update();
    }
    else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        // 保存状态用于撤销
        saveState();

        // 删除选中的形状
        shapes.removeAt(selectedShapeIndex);
        selectedShapeIndex = -1;

        canvasWidget->update();
    }
    else {
        // 提示用户需要先选择一个形状
        QMessageBox::information(this, "Select Shape", "请先选择一个形状。");
    }
}

void MainWindow::onClearCanvasClicked()
{
    if (!shapes.isEmpty()) {
        // 保存当前状态用于撤销
        saveState();

        // 清空所有形状
        shapes.clear();
        selectedShapeIndex = -1;

        // 更新画布
        canvasWidget->update();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        onDeleteButtonClicked();
    }
    else if (event->key() == Qt::Key_Z && event->modifiers() & Qt::ControlModifier) {
        onUndoClicked();
    }
    else if (event->key() == Qt::Key_Y && event->modifiers() & Qt::ControlModifier) {
        onRedoClicked();
    }
    else if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        // 复制选中的形状
        copySelectedShapes();
    }
    else if (event->key() == Qt::Key_X && event->modifiers() & Qt::ControlModifier) {
        // 剪切选中的形状
        cutSelectedShapes();
    }
    else if (event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier) {
        // 粘贴形状
        pasteShapes();
    }
    else if (event->key() == Qt::Key_F2 && selectedShapeIndex != -1) {
        QMouseEvent* mouseEvent = new QMouseEvent(
            QEvent::MouseButtonDblClick,
            shapes[selectedShapeIndex].rect.center(),
            Qt::LeftButton,
            Qt::LeftButton,
            Qt::NoModifier
        );
        mouseDoubleClickEvent(mouseEvent);
        delete mouseEvent;
        return;
    }
    else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::showLineStyleMenu() {
    QMenu* menu = new QMenu(this);

    // 创建实线菜单项
    QAction* solidLine = menu->addAction(QIcon(":/image/solid_line.png"), "实线");
    solidLine->setData(QVariant(static_cast<int>(Qt::SolidLine)));

    // 创建虚线菜单项
    QAction* dashLine = menu->addAction(QIcon(":/image/dash_line.png"), "虚线");
    dashLine->setData(QVariant(static_cast<int>(Qt::DashLine)));

    // 创建点线菜单项
    QAction* dotLine = menu->addAction(QIcon(":/image/dot_line.png"), "点线");
    dotLine->setData(QVariant(static_cast<int>(Qt::DotLine)));

    // 创建点划线菜单项
    QAction* dashDotLine = menu->addAction(QIcon(":/image/dash_dot_line.png"), "点划线");
    dashDotLine->setData(QVariant(static_cast<int>(Qt::DashDotLine)));

    // 创建双点划线菜单项
    QAction* dashDotDotLine = menu->addAction(QIcon(":/image/dash_dot_dot_line.png"), "双点划线");
    dashDotDotLine->setData(QVariant(static_cast<int>(Qt::DashDotDotLine)));

    // 设置菜单样式
    menu->setStyleSheet(
        "QMenu {"
        "   background-color: #f0f0f0;"
        "   border: 1px solid #c0c0c0;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "}"
        "QMenu::item {"
        "   padding: 5px 20px 5px 20px;"  // 增加内边距，使图标和文字有更好的间距
        "   border-radius: 3px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #e0e0e0;"
        "}"
    );

    QAction* selectedAction = menu->exec(QCursor::pos());
    if (selectedAction) {
        Qt::PenStyle style = static_cast<Qt::PenStyle>(selectedAction->data().toInt());
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            // Save state for undo
            saveState();
            shapes[selectedShapeIndex].penStyle = style;
            canvasWidget->update();
        }
    }

    delete menu;
}

void MainWindow::createPropertyPanel() {
    // 创建属性面板
    propertyPanel = new QWidget(this);
    propertyPanel->setMinimumWidth(350);  // 设置最小宽度
    propertyPanel->setMaximumWidth(500);  // 设置最大宽度
    propertyPanel->setStyleSheet(
        "QWidget {"
        "   background-color: #e6e6e6;"  // 莫兰迪浅灰色背景
        "   border-left: 1px solid #d8e1e9;"
        "}"
        "QLabel {"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 20px;"  // 增大字体
        "   font-weight: bold;"
        "   padding: 5px;"
        "}"
        "QGroupBox {"
        "   background-color: #f0f0f0;"  // 更浅的灰色
        "   border: 1px solid #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 8px;"
        "   margin-top: 20px;"  // 增加组之间的间距
        "   padding: 15px;"  // 增加内边距
        "}"
        "QGroupBox::title {"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top center;"
        "   padding: 0 5px;"
        "   font-size: 18px;"  // 增大字体
        "   font-weight: bold;"
        "}"
        "QLineEdit, QSpinBox, QComboBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 4px;"
        "   padding: 5px;"
        "   margin: 2px;"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 16px;"  // 增大字体
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "   border: 2px solid #a8b8c8;"  // 莫兰迪深蓝色
        "}"
        "QPushButton {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border: 1px solid #a8b8c8;"  // 莫兰迪深蓝色
        "   border-radius: 4px;"
        "   padding: 5px;"
        "   color: #5c4b51;"  // 莫兰迪深色
        "   font-size: 16px;"  // 增大字体
        "   text-align: left;"
        "   padding-left: 10px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c8d1d9;"  // 莫兰迪深蓝色
        "}"
        "QPushButton:pressed {"
        "   background-color: #b8c1c9;"  // 莫兰迪更深的蓝色
        "}"
        "QSlider::groove:horizontal {"
        "   border: 1px solid #d8e1e9;"
        "   height: 10px;"
        "   background: #e6e6e6;"
        "   margin: 2px 0;"
        "   border-radius: 5px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #8b7d6b;"  // 莫兰迪棕色
        "   border: 1px solid #d8e1e9;"
        "   width: 20px;"
        "   margin: -2px 0;"
        "   border-radius: 10px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background: #9b8d7b;"  // 更浅的莫兰迪棕色
        "}"
    );

    // 创建布局
    propertyLayout = new QVBoxLayout(propertyPanel);
    propertyLayout->setContentsMargins(15, 15, 15, 15);
    propertyLayout->setSpacing(15);  // 增加组件之间的间距

    // 添加标题
    QLabel* titleLabel = new QLabel("属性", propertyPanel);
    titleLabel->setStyleSheet(
        "font-size: 24px;"  // 增大标题字体
        "font-weight: bold;"
        "color: #5c4b51;"  // 莫兰迪深色
        "padding: 5px;"
        "border-bottom: 2px solid #d8e1e9;"  // 莫兰迪蓝色
    );
    propertyLayout->addWidget(titleLabel);

    // 添加分隔线
    QFrame* line = new QFrame(propertyPanel);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #d8e1e9;");  // 莫兰迪蓝色
    propertyLayout->addWidget(line);

    // 添加属性组
    QGroupBox* shapeGroup = new QGroupBox("形状属性", propertyPanel);
    shapeGroup->setStyleSheet(
        "QGroupBox {"
        "   background-color: #f5f5f5;"  // 莫兰迪米白色背景
        "   border: 1px solid #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 12px;"  // 增大圆角
        "   margin-top: 25px;"  // 增加组之间的间距
        "   padding: 20px;"  // 增加内边距
        "}"
        "QGroupBox::title {"
        "   color: #8b7d6b;"  // 莫兰迪棕色
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top center;"
        "   padding: 0 15px;"
        "   font-size: 24px;"  // 增大字体
        "   font-weight: bold;"
        "   background-color: #f5f5f5;"  // 与GroupBox背景色相同
        "}"
    );
    QVBoxLayout* shapeLayout = new QVBoxLayout(shapeGroup);
    shapeLayout->setContentsMargins(15, 25, 15, 15);  // 增加内边距
    shapeLayout->setSpacing(15);  // 增加组件间距

    // 添加位置控制布局
    QHBoxLayout* positionLayout = new QHBoxLayout();
    positionLayout->setSpacing(15);

    // X坐标控制
    QHBoxLayout* xLayout = new QHBoxLayout();
    QLabel* xLabel = new QLabel("X:", shapeGroup);
    xLabel->setStyleSheet(
        "QLabel {"
        "   color: #8b7d6b;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    );
    QSpinBox* xSpinBox = new QSpinBox(shapeGroup);
    xSpinBox->setRange(0, 10000);
    xSpinBox->setStyleSheet(
        "QSpinBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   padding: 5px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "}"
    );
    xLayout->addWidget(xLabel);
    xLayout->addWidget(xSpinBox);
    positionLayout->addLayout(xLayout);

    // Y坐标控制
    QHBoxLayout* yLayout = new QHBoxLayout();
    QLabel* yLabel = new QLabel("Y:", shapeGroup);
    yLabel->setStyleSheet(
        "QLabel {"
        "   color: #8b7d6b;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    );
    QSpinBox* ySpinBox = new QSpinBox(shapeGroup);
    ySpinBox->setRange(0, 10000);
    ySpinBox->setStyleSheet(
        "QSpinBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   padding: 5px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "}"
    );
    yLayout->addWidget(yLabel);
    yLayout->addWidget(ySpinBox);
    positionLayout->addLayout(yLayout);
    shapeLayout->addLayout(positionLayout);

    // 添加大小控制布局
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(15);

    // 宽度控制
    QHBoxLayout* widthLayout = new QHBoxLayout();
    QLabel* widthLabel = new QLabel("W:", shapeGroup);
    widthLabel->setStyleSheet(
        "QLabel {"
        "   color: #8b7d6b;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    );
    QSpinBox* widthSpinBox = new QSpinBox(shapeGroup);
    widthSpinBox->setRange(0, 10000);
    widthSpinBox->setStyleSheet(
        "QSpinBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   padding: 5px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "}"
    );
    widthLayout->addWidget(widthLabel);
    widthLayout->addWidget(widthSpinBox);
    sizeLayout->addLayout(widthLayout);

    // 高度控制
    QHBoxLayout* heightLayout = new QHBoxLayout();
    QLabel* heightLabel = new QLabel("H:", shapeGroup);
    heightLabel->setStyleSheet(
        "QLabel {"
        "   color: #8b7d6b;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    );
    QSpinBox* heightSpinBox = new QSpinBox(shapeGroup);
    heightSpinBox->setRange(0, 10000);
    heightSpinBox->setStyleSheet(
        "QSpinBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   padding: 5px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "}"
    );
    heightLayout->addWidget(heightLabel);
    heightLayout->addWidget(heightSpinBox);
    sizeLayout->addLayout(heightLayout);
    shapeLayout->addLayout(sizeLayout);

    propertyLayout->addWidget(shapeGroup);

    // 添加样式组
    QGroupBox* styleGroup = new QGroupBox("格式属性", propertyPanel);
    styleGroup->setStyleSheet(
        "QGroupBox {"
        "   background-color: #f5f5f5;"  // 莫兰迪米白色背景
        "   border: 1px solid #d8e1e9;"  // 莫兰迪蓝色
        "   border-radius: 12px;"  // 增大圆角
        "   margin-top: 25px;"  // 增加组之间的间距
        "   padding: 20px;"  // 增加内边距
        "}"
        "QGroupBox::title {"
        "   color: #8b7d6b;"  // 莫兰迪棕色
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top center;"
        "   padding: 0 15px;"
        "   font-size: 24px;"  // 增大字体
        "   font-weight: bold;"
        "   background-color: #f5f5f5;"  // 与GroupBox背景色相同
        "}"
    );
    QVBoxLayout* styleLayout = new QVBoxLayout(styleGroup);
    styleLayout->setContentsMargins(15, 25, 15, 15);  // 增加内边距
    styleLayout->setSpacing(15);  // 增加组件间距

    // 添加填充颜色按钮
    QPushButton* fillColorBtn = new QPushButton(styleGroup);
    fillColorBtn->setIcon(QIcon(":/image/fill_color.png"));
    fillColorBtn->setText("填充颜色");
    fillColorBtn->setIconSize(QSize(28, 28));  // 增大图标
    fillColorBtn->setFixedHeight(40);  // 增加按钮高度
    fillColorBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e6d5c3;"  // 莫兰迪浅棕色
        "   border: 1px solid #d8c4b0;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d8c4b0;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(fillColorBtn);

    // 添加填充颜色预览
    QLabel* fillColorPreview = new QLabel(styleGroup);
    fillColorPreview->setObjectName("fillColorPreview");
    fillColorPreview->setFixedHeight(25);
    fillColorPreview->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   background-color: #ffffff;"
        "}"
    );
    styleLayout->addWidget(fillColorPreview);

    // 添加边框颜色按钮
    QPushButton* borderColorBtn = new QPushButton(styleGroup);
    borderColorBtn->setIcon(QIcon(":/image/border_color.png"));
    borderColorBtn->setText("线条颜色");
    borderColorBtn->setIconSize(QSize(28, 28));
    borderColorBtn->setFixedHeight(40);
    borderColorBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #d8e1e9;"  // 莫兰迪蓝色
        "   border: 1px solid #c8d1d9;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c8d1d9;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(borderColorBtn);

    // 添加边框颜色预览
    QLabel* borderColorPreview = new QLabel(styleGroup);
    borderColorPreview->setObjectName("borderColorPreview");
    borderColorPreview->setFixedHeight(25);
    borderColorPreview->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   background-color: #ffffff;"
        "}"
    );
    styleLayout->addWidget(borderColorPreview);

    // 添加线条样式按钮
    QPushButton* lineStyleBtn = new QPushButton(styleGroup);
    lineStyleBtn->setIcon(QIcon(":/image/line_style.png"));
    lineStyleBtn->setText("线条样式");
    lineStyleBtn->setIconSize(QSize(28, 28));
    lineStyleBtn->setFixedHeight(40);
    lineStyleBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e6e6e6;"  // 莫兰迪灰色
        "   border: 1px solid #d8d8d8;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d8d8d8;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(lineStyleBtn);

    // 添加线条样式预览
    QLabel* lineStylePreview = new QLabel(styleGroup);
    lineStylePreview->setObjectName("lineStylePreview");
    lineStylePreview->setFixedHeight(25);
    lineStylePreview->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   background-color: #ffffff;"
        "}"
    );
    styleLayout->addWidget(lineStylePreview);

    // 添加线条粗细按钮
    QPushButton* lineWidthBtn = new QPushButton(styleGroup);
    lineWidthBtn->setIcon(QIcon(":/image/line_width.png"));
    lineWidthBtn->setText("线条粗细");
    lineWidthBtn->setIconSize(QSize(28, 28));
    lineWidthBtn->setFixedHeight(40);
    lineWidthBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e6e6e6;"  // 莫兰迪灰色
        "   border: 1px solid #d8d8d8;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d8d8d8;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(lineWidthBtn);

    // 添加线条粗细预览
    QLabel* lineWidthPreview = new QLabel(styleGroup);
    lineWidthPreview->setObjectName("lineWidthPreview");
    lineWidthPreview->setFixedHeight(25);
    lineWidthPreview->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   background-color: #ffffff;"
        "}"
    );
    styleLayout->addWidget(lineWidthPreview);

    // 添加不透明度按钮
    QPushButton* opacityBtn = new QPushButton(styleGroup);
    opacityBtn->setIcon(QIcon(":/image/opacity.png"));
    opacityBtn->setText("不透明度");
    opacityBtn->setIconSize(QSize(28, 28));
    opacityBtn->setFixedHeight(40);
    opacityBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e60d4;"  // 莫兰迪米色
        "   border: 1px solid #d8d2c6;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d8d2c6;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(opacityBtn);

    // 创建不透明度控制的水平布局
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    opacityLayout->setSpacing(15);

    // 添加不透明度滑块
    QSlider* opacitySlider = new QSlider(Qt::Horizontal, styleGroup);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    opacitySlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   border: 1px solid #d8e1e9;"
        "   height: 10px;"
        "   background: #e6e6e6;"
        "   margin: 2px 0;"
        "   border-radius: 5px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #8b7d6b;"
        "   border: 1px solid #d8e1e9;"
        "   width: 20px;"
        "   margin: -2px 0;"
        "   border-radius: 10px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background: #9b8d7b;"
        "}"
    );
    opacityLayout->addWidget(opacitySlider);

    // 添加不透明度数值显示
    QLabel* opacityValueLabel = new QLabel("100%", styleGroup);
    opacityValueLabel->setStyleSheet(
        "QLabel {"
        "   color: #8b7d6b;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   min-width: 50px;"
        "}"
    );
    opacityLayout->addWidget(opacityValueLabel);
    styleLayout->addLayout(opacityLayout);

    // 添加箭头样式按钮
    QPushButton* arrowStyleBtn = new QPushButton(styleGroup);
    arrowStyleBtn->setIcon(QIcon(":/image/arrow.png"));
    arrowStyleBtn->setText("箭头样式");
    arrowStyleBtn->setIconSize(QSize(28, 28));
    arrowStyleBtn->setFixedHeight(40);
    arrowStyleBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #c4a3a0;"  // 莫兰迪粉色
        "   border: 1px solid #d8d8d8;"
        "   border-radius: 8px;"
        "   padding: 5px 15px;"
        "   color: #5c4b51;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d8d8d8;"
        "   border: 2px solid #8b7d6b;"
        "}"
    );
    styleLayout->addWidget(arrowStyleBtn);

    // 添加箭头样式预览
    QLabel* arrowStylePreview = new QLabel(styleGroup);
    arrowStylePreview->setObjectName("arrowStylePreview");
    arrowStylePreview->setFixedHeight(25);
    arrowStylePreview->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #d8e1e9;"
        "   border-radius: 6px;"
        "   background-color: #ffffff;"
        "}"
    );
    styleLayout->addWidget(arrowStylePreview);

    propertyLayout->addWidget(styleGroup);

    // 添加弹性空间
    propertyLayout->addStretch();

    // 连接信号槽
    connect(fillColorBtn, &QPushButton::clicked, this, &MainWindow::onFillColorButtonClicked);
    connect(borderColorBtn, &QPushButton::clicked, this, &MainWindow::onBorderColorButtonClicked);
    connect(lineStyleBtn, &QPushButton::clicked, this, &MainWindow::showLineStyleMenu);
    connect(lineWidthBtn, &QPushButton::clicked, this, &MainWindow::showLineWidthMenu);
    connect(arrowStyleBtn, &QPushButton::clicked, this, &MainWindow::showArrowStyleMenu);

    // 连接不透明度滑块的信号
    connect(opacitySlider, &QSlider::valueChanged, [=](int value) {
        // 保存状态用于撤销
        saveState();

        // 检查是否有多个图形被选中
        if (!selectedShapeIndices.isEmpty()) {
            // 批量修改所有选中图形的不透明度
            for (int index : selectedShapeIndices) {
                if (index >= 0 && index < shapes.size()) {
                    shapes[index].opacity = value;
                }
            }

            // 更新数值显示
            opacityValueLabel->setText(QString("%1%").arg(value));

            // 如果有选中的图形，使用第一个选中的图形更新预览
            if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
                // 更新填充颜色预览
                QLabel* fillColorPreview = propertyPanel->findChild<QLabel*>("fillColorPreview");
                if (fillColorPreview) {
                    QColor previewColor = shapes[selectedShapeIndex].color;
                    previewColor.setAlpha(value * 255 / 100);
                    QString colorStyle = QString("QLabel { "
                        "border: 1px solid #d8e1e9; "
                        "border-radius: 6px; "
                        "background-color: %1; "
                        "}")
                        .arg(previewColor.name(QColor::HexArgb));
                    fillColorPreview->setStyleSheet(colorStyle);
                }

                // 更新边框颜色预览
                QLabel* borderColorPreview = propertyPanel->findChild<QLabel*>("borderColorPreview");
                if (borderColorPreview) {
                    QColor borderColor = shapes[selectedShapeIndex].borderColor.isValid() ?
                        shapes[selectedShapeIndex].borderColor :
                        shapes[selectedShapeIndex].color.darker(150);
                    borderColor.setAlpha(value * 255 / 100);
                    QString colorStyle = QString("QLabel { "
                        "border: 1px solid #d8e1e9; "
                        "border-radius: 6px; "
                        "background-color: %1; "
                        "}")
                        .arg(borderColor.name(QColor::HexArgb));
                    borderColorPreview->setStyleSheet(colorStyle);
                }
            }
        }
        // 如果只有单个图形被选中
        else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            // 更新形状的不透明度
            shapes[selectedShapeIndex].opacity = value;

            // 更新数值显示
            opacityValueLabel->setText(QString("%1%").arg(value));

            // 更新填充颜色预览
            QLabel* fillColorPreview = propertyPanel->findChild<QLabel*>("fillColorPreview");
            if (fillColorPreview) {
                QColor previewColor = shapes[selectedShapeIndex].color;
                previewColor.setAlpha(value * 255 / 100);
                QString colorStyle = QString("QLabel { "
                    "border: 1px solid #d8e1e9; "
                    "border-radius: 6px; "
                    "background-color: %1; "
                    "}")
                    .arg(previewColor.name(QColor::HexArgb));
                fillColorPreview->setStyleSheet(colorStyle);
            }

            // 更新边框颜色预览
            QLabel* borderColorPreview = propertyPanel->findChild<QLabel*>("borderColorPreview");
            if (borderColorPreview) {
                QColor borderColor = shapes[selectedShapeIndex].borderColor.isValid() ?
                    shapes[selectedShapeIndex].borderColor :
                    shapes[selectedShapeIndex].color.darker(150);
                borderColor.setAlpha(value * 255 / 100);
                QString colorStyle = QString("QLabel { "
                    "border: 1px solid #d8e1e9; "
                    "border-radius: 6px; "
                    "background-color: %1; "
                    "}")
                    .arg(borderColor.name(QColor::HexArgb));
                borderColorPreview->setStyleSheet(colorStyle);
            }
        }

        // 更新画布
        canvasWidget->update();
        });

    // 初始隐藏属性面板
    propertyPanel->hide();

    // 连接XYWH的SpinBox信号
    connect(xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算偏移量
            int delta = value - shapes[selectedShapeIndex].rect.x();

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        QRect rect = shapes[index].rect;
                        rect.moveLeft(rect.x() + delta);
                        shapes[index].rect = rect;

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.moveLeft(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            canvasWidget->update();
        }
        });

    connect(ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算偏移量
            int delta = value - shapes[selectedShapeIndex].rect.y();

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        QRect rect = shapes[index].rect;
                        rect.moveTop(rect.y() + delta);
                        shapes[index].rect = rect;

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.moveTop(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            canvasWidget->update();
        }
        });

    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算缩放比例
            double ratio = 1.0;
            if (shapes[selectedShapeIndex].rect.width() > 0) {
                ratio = (double)value / shapes[selectedShapeIndex].rect.width();
            }

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        if (index == selectedShapeIndex) {
                            // 对当前选中的图形直接设置值
                            QRect rect = shapes[index].rect;
                            rect.setWidth(value);
                            shapes[index].rect = rect;
                        }
                        else {
                            // 对其他选中的图形按比例缩放
                            QRect rect = shapes[index].rect;
                            rect.setWidth(qRound(rect.width() * ratio));
                            shapes[index].rect = rect;
                        }

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.setWidth(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            canvasWidget->update();
        }
        });

    connect(heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();

            // 计算缩放比例
            double ratio = 1.0;
            if (shapes[selectedShapeIndex].rect.height() > 0) {
                ratio = (double)value / shapes[selectedShapeIndex].rect.height();
            }

            // 修改多选图形
            if (!selectedShapeIndices.isEmpty()) {
                for (int index : selectedShapeIndices) {
                    if (index >= 0 && index < shapes.size()) {
                        if (index == selectedShapeIndex) {
                            // 对当前选中的图形直接设置值
                            QRect rect = shapes[index].rect;
                            rect.setHeight(value);
                            shapes[index].rect = rect;
                        }
                        else {
                            // 对其他选中的图形按比例缩放
                            QRect rect = shapes[index].rect;
                            rect.setHeight(qRound(rect.height() * ratio));
                            shapes[index].rect = rect;
                        }

                        // 更新连接的箭头
                        updateConnectedArrows(index);
                    }
                }
            }
            else {
                // 单选情况
                QRect rect = shapes[selectedShapeIndex].rect;
                rect.setHeight(value);
                shapes[selectedShapeIndex].rect = rect;

                // 更新连接的箭头
                updateConnectedArrows(selectedShapeIndex);
            }

            canvasWidget->update();
        }
        });

    // 将SpinBox保存为类成员变量，以便在其他地方访问
    this->xSpinBox = xSpinBox;
    this->ySpinBox = ySpinBox;
    this->widthSpinBox = widthSpinBox;
    this->heightSpinBox = heightSpinBox;

    // 添加箭头样式选择（仅当选中箭头时显示）
    QComboBox* arrowStyleCombo = new QComboBox(propertyPanel);
    arrowStyleCombo->addItem("无箭头", 0);
    arrowStyleCombo->addItem("单箭头", 1);
    arrowStyleCombo->addItem("双箭头", 2);
    arrowStyleCombo->setObjectName("arrowStyleCombo");
    arrowStyleCombo->hide();  // 默认隐藏
    shapeLayout->addWidget(arrowStyleCombo);

    // 连接箭头样式改变信号
    connect(arrowStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            saveState();
            shapes[selectedShapeIndex].arrowStyle = index;
            canvasWidget->update();
        }
    });
}

void MainWindow::updatePropertyPanel() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        const Shape& shape = shapes[selectedShapeIndex];
        showPropertyPanel();

        // 更新箭头样式选择框的显示状态和值
        QComboBox* arrowStyleCombo = propertyPanel->findChild<QComboBox*>("arrowStyleCombo");
        if (arrowStyleCombo) {
            if (shape.type == "Arrow") {
                arrowStyleCombo->show();
                arrowStyleCombo->setCurrentIndex(shape.arrowStyle);
            } else {
                arrowStyleCombo->hide();
            }
        }

        // 更新填充颜色预览
        QLabel* fillColorPreview = propertyPanel->findChild<QLabel*>("fillColorPreview");
        if (fillColorPreview) {
            QColor previewColor = shape.color;
            previewColor.setAlpha(shape.opacity * 255 / 100);
            QString colorStyle = QString("QLabel { "
                "border: 1px solid #d8e1e9; "
                "border-radius: 6px; "
                "background-color: %1; "
                "}")
                .arg(previewColor.name(QColor::HexArgb));
            fillColorPreview->setStyleSheet(colorStyle);
        }

        // 更新边框颜色预览
        QLabel* borderColorPreview = propertyPanel->findChild<QLabel*>("borderColorPreview");
        if (borderColorPreview) {
            QColor borderColor = shape.borderColor.isValid() ? shape.borderColor : shape.color.darker(150);
            borderColor.setAlpha(shape.opacity * 255 / 100);
            QString colorStyle = QString("QLabel { "
                "border: 1px solid #d8e1e9; "
                "border-radius: 6px; "
                "background-color: %1; "
                "}")
                .arg(borderColor.name(QColor::HexArgb));
            borderColorPreview->setStyleSheet(colorStyle);
        }

        // 更新线条样式预览
        QLabel* lineStylePreview = propertyPanel->findChild<QLabel*>("lineStylePreview");
        if (lineStylePreview) {
            lineStylePreview->setPixmap(createLineStyleIcon(shape.penStyle).pixmap(100, 20));
        }

        // 更新线条粗细预览
        QLabel* lineWidthPreview = propertyPanel->findChild<QLabel*>("lineWidthPreview");
        if (lineWidthPreview) {
            QPixmap pixmap(100, 20);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QPen pen(Qt::black, shape.lineWidth);
            painter.setPen(pen);
            painter.drawLine(10, 10, 90, 10);
            lineWidthPreview->setPixmap(pixmap);
        }


        // 更新不透明度滑块和数值显示
        QSlider* opacitySlider = propertyPanel->findChild<QSlider*>();
        QLabel* opacityValueLabel = propertyPanel->findChildren<QLabel*>().last();
        if (opacitySlider && opacityValueLabel) {
            opacitySlider->setValue(shape.opacity);
            opacityValueLabel->setText(QString("%1%").arg(shape.opacity));
        }


        // 更新XYWH的值
        if (xSpinBox) xSpinBox->setValue(shape.rect.x());
        if (ySpinBox) ySpinBox->setValue(shape.rect.y());
        if (widthSpinBox) widthSpinBox->setValue(shape.rect.width());
        if (heightSpinBox) heightSpinBox->setValue(shape.rect.height());

        // 更新工具栏中的XYWH值
        if (arrangeXSpinBox) {
            arrangeXSpinBox->blockSignals(true);
            arrangeXSpinBox->setValue(shape.rect.x());
            arrangeXSpinBox->blockSignals(false);
        }
        if (arrangeYSpinBox) {
            arrangeYSpinBox->blockSignals(true);
            arrangeYSpinBox->setValue(shape.rect.y());
            arrangeYSpinBox->blockSignals(false);
        }
        if (arrangeWidthSpinBox) {
            arrangeWidthSpinBox->blockSignals(true);
            arrangeWidthSpinBox->setValue(shape.rect.width());
            arrangeWidthSpinBox->blockSignals(false);
        }
        if (arrangeHeightSpinBox) {
            arrangeHeightSpinBox->blockSignals(true);
            arrangeHeightSpinBox->setValue(shape.rect.height());
            arrangeHeightSpinBox->blockSignals(false);
        }

        // 更新箭头样式预览
        QLabel* arrowStylePreview = propertyPanel->findChild<QLabel*>("arrowStylePreview");
        if (arrowStylePreview) {
            if (shape.type == "Arrow") {
                arrowStylePreview->show();
                QPixmap pixmap(100, 20);
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QPen pen(Qt::black, 2);
                painter.setPen(pen);
                
                // 根据箭头样式绘制预览
                switch (shape.arrowStyle) {
                    case 0: // 无箭头
                        painter.drawLine(10, 10, 90, 10);
                        break;
                    case 1: // 单箭头
                        painter.drawLine(10, 10, 90, 10);
                        painter.drawLine(70, 5, 90, 10);
                        painter.drawLine(70, 15, 90, 10);
                        break;
                    case 2: // 双箭头
                        painter.drawLine(10, 10, 90, 10);
                        painter.drawLine(70, 5, 90, 10);
                        painter.drawLine(70, 15, 90, 10);
                        painter.drawLine(30, 5, 10, 10);
                        painter.drawLine(30, 15, 10, 10);
                        break;
                }
                arrowStylePreview->setPixmap(pixmap);
            } else {
                arrowStylePreview->hide();
            }
        }
    }
    else {
        hidePropertyPanel();
    }

}

QIcon MainWindow::createLineStyleIcon(Qt::PenStyle style) {
    QPixmap pixmap(100, 20);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, 2);
    pen.setStyle(style);
    painter.setPen(pen);

    painter.drawLine(10, 10, 90, 10);

    return QIcon(pixmap);
}

void MainWindow::showPropertyPanel() {
    if (propertyPanel) {
        propertyPanel->show();
    }
}

void MainWindow::hidePropertyPanel() {
    if (propertyPanel) {
        propertyPanel->hide();
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    updatePropertyPanel();

    // 如果正在编辑文本，且点击位置不在文本编辑器内，则完成编辑
    if (isEditingText && inlineTextEdit->isVisible()) {
        QPoint globalPos = event->globalPos();
        QRect editorRect = inlineTextEdit->geometry();
        if (!editorRect.contains(inlineTextEdit->mapFromGlobal(globalPos))) {
            finishInlineTextEdit();
        }
    }

}

// 添加Export按钮点击事件处理函数
void MainWindow::onExportClicked()
{
    hideAllSecondaryToolbars();
    exportToolbar->show();
    // 更新按钮状态
    startBtn->setChecked(false);
    arrangeBtn->setChecked(false);
    pageBtn->setChecked(false);
    exportBtn->setChecked(true);
    update();
}

void MainWindow::onFileClicked()
{
    QMenu* fileMenu = new QMenu(this);

    QAction* newAction = fileMenu->addAction("新建");
    QAction* openAction = fileMenu->addAction("打开");
    QAction* saveAction = fileMenu->addAction("保存");

    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);

    fileMenu->exec(fileBtn->mapToGlobal(QPoint(0, fileBtn->height())));
}

void MainWindow::onNewFile()
{
    // Clear current canvas
    shapes.clear();
    undoStack.clear();
    redoStack.clear();
    update();
}

void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开文件", "", "SVG文件 (*.svg);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "错误", "无法打开文件：" + fileName);
            return;
        }

        // 保存当前状态用于撤销
        saveState();

        // 清空当前画布
        shapes.clear();
        selectedShapeIndex = -1;
        emit selectionChanged();

        // 读取SVG文件内容
        QString content = file.readAll();
        file.close();

        // 使用正则表达式匹配各种图形元素
        QRegExp rectPattern("<rect[^>]*x=\"([^\"]*)\"[^>]*y=\"([^\"]*)\"[^>]*width=\"([^\"]*)\"[^>]*height=\"([^\"]*)\"[^>]*/>");
        QRegExp circlePattern("<circle[^>]*cx=\"([^\"]*)\"[^>]*cy=\"([^\"]*)\"[^>]*r=\"([^\"]*)\"[^>]*/>");
        QRegExp linePattern("<line[^>]*x1=\"([^\"]*)\"[^>]*y1=\"([^\"]*)\"[^>]*x2=\"([^\"]*)\"[^>]*y2=\"([^\"]*)\"[^>]*/>");
        QRegExp textPattern("<text[^>]*x=\"([^\"]*)\"[^>]*y=\"([^\"]*)\"[^>]*>([^<]*)</text>");

        // 解析矩形
        int pos = 0;
        while ((pos = rectPattern.indexIn(content, pos)) != -1) {
            Shape shape;
            shape.type = "Rectangle";
            shape.selected = false;
            shape.opacity = 100;
            shape.lineWidth = 2;
            shape.penStyle = Qt::SolidLine;
            shape.color = Qt::white;
            shape.borderColor = Qt::black;

            int x = rectPattern.cap(1).toInt();
            int y = rectPattern.cap(2).toInt();
            int width = rectPattern.cap(3).toInt();
            int height = rectPattern.cap(4).toInt();
            shape.rect = QRect(x, y, width, height);

            // 解析颜色和样式
            QRegExp fillPattern("fill=\"([^\"]*)\"");
            QRegExp strokePattern("stroke=\"([^\"]*)\"");
            QRegExp strokeWidthPattern("stroke-width=\"([^\"]*)\"");

            if (fillPattern.indexIn(content, pos) != -1) {
                shape.color = QColor(fillPattern.cap(1));
            }
            if (strokePattern.indexIn(content, pos) != -1) {
                shape.borderColor = QColor(strokePattern.cap(1));
            }
            if (strokeWidthPattern.indexIn(content, pos) != -1) {
                shape.lineWidth = strokeWidthPattern.cap(1).toInt();
            }

            shapes.append(shape);
            pos += rectPattern.matchedLength();
        }

        // 解析圆形
        pos = 0;
        while ((pos = circlePattern.indexIn(content, pos)) != -1) {
            Shape shape;
            shape.type = "Circle";
            shape.selected = false;
            shape.opacity = 100;
            shape.lineWidth = 2;
            shape.penStyle = Qt::SolidLine;
            shape.color = Qt::white;
            shape.borderColor = Qt::black;

            int cx = circlePattern.cap(1).toInt();
            int cy = circlePattern.cap(2).toInt();
            int r = circlePattern.cap(3).toInt();
            shape.rect = QRect(cx - r, cy - r, r * 2, r * 2);

            // 解析颜色和样式
            QRegExp fillPattern("fill=\"([^\"]*)\"");
            QRegExp strokePattern("stroke=\"([^\"]*)\"");
            QRegExp strokeWidthPattern("stroke-width=\"([^\"]*)\"");

            if (fillPattern.indexIn(content, pos) != -1) {
                shape.color = QColor(fillPattern.cap(1));
            }
            if (strokePattern.indexIn(content, pos) != -1) {
                shape.borderColor = QColor(strokePattern.cap(1));
            }
            if (strokeWidthPattern.indexIn(content, pos) != -1) {
                shape.lineWidth = strokeWidthPattern.cap(1).toInt();
            }

            shapes.append(shape);
            pos += circlePattern.matchedLength();
        }

        // 解析线条
        pos = 0;
        while ((pos = linePattern.indexIn(content, pos)) != -1) {
            Shape shape;
            shape.type = "Arrow";
            shape.selected = false;
            shape.opacity = 100;
            shape.lineWidth = 2;
            shape.penStyle = Qt::SolidLine;
            shape.color = Qt::white;
            shape.borderColor = Qt::black;
            shape.arrowStyle = 1;  // 默认单箭头

            int x1 = linePattern.cap(1).toInt();
            int y1 = linePattern.cap(2).toInt();
            int x2 = linePattern.cap(3).toInt();
            int y2 = linePattern.cap(4).toInt();
            shape.rect = QRect(x1, y1, x2 - x1, y2 - y1);

            // 检查是否有箭头标记
            QRegExp markerEndPattern("marker-end=\"([^\"]*)\"");
            QRegExp markerStartPattern("marker-start=\"([^\"]*)\"");
            if (markerEndPattern.indexIn(content, pos) != -1 && 
                markerStartPattern.indexIn(content, pos) != -1) {
                shape.arrowStyle = 2;  // 双箭头
            }
            else if (markerEndPattern.indexIn(content, pos) != -1) {
                shape.arrowStyle = 1;  // 单箭头
            }
            else {
                shape.arrowStyle = 0;  // 无箭头
            }

            shapes.append(shape);
            pos += linePattern.matchedLength();
        }

        // 解析文本
        pos = 0;
        while ((pos = textPattern.indexIn(content, pos)) != -1) {
            Shape shape;
            shape.type = "Text";
            shape.selected = false;
            shape.opacity = 100;
            shape.textColor = Qt::black;
            shape.textFont = QFont("Arial", 12);
            shape.textAlignment = Qt::AlignCenter;

            int x = textPattern.cap(1).toInt();
            int y = textPattern.cap(2).toInt();
            shape.text = textPattern.cap(3);

            // 解析字体属性
            QRegExp fontFamilyPattern("font-family=\"([^\"]*)\"");
            QRegExp fontSizePattern("font-size=\"([^\"]*)\"");
            QRegExp fillPattern("fill=\"([^\"]*)\"");

            if (fontFamilyPattern.indexIn(content, pos) != -1) {
                shape.textFont.setFamily(fontFamilyPattern.cap(1));
            }
            if (fontSizePattern.indexIn(content, pos) != -1) {
                shape.textFont.setPointSize(fontSizePattern.cap(1).toInt());
            }
            if (fillPattern.indexIn(content, pos) != -1) {
                shape.textColor = QColor(fillPattern.cap(1));
            }

            // 设置文本区域
            QFontMetrics fm(shape.textFont);
            int textWidth = fm.horizontalAdvance(shape.text);
            int textHeight = fm.height();
            shape.rect = QRect(x - textWidth/2, y - textHeight/2, textWidth, textHeight);

            shapes.append(shape);
            pos += textPattern.matchedLength();
        }

        // 更新画布
        canvasWidget->update();
        updatePropertyPanel();
    }
}

void MainWindow::onSaveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save File", "", "Flowchart Files (*.flow);;All Files (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    // Save all shape information
    for (const Shape& shape : shapes) {
        out << shape.type << ","
            << shape.rect.x() << ","
            << shape.rect.y() << ","
            << shape.rect.width() << ","
            << shape.rect.height() << ","
            << shape.color.name() << ","
            << shape.borderColor.name() << ","
            << static_cast<int>(shape.penStyle) << ","
            << shape.rotation << ","
            << shape.opacity << ","
            << shape.text << ","
            << shape.textFont.toString() << ","
            << shape.textColor.name() << "\n";
    }

    file.close();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    // 将全局坐标转换为画布坐标
    QPoint canvasPos = event->pos() - canvasPosition;

    // 检查是否双击在某个图形上
    for (int i = 0; i < shapes.size(); ++i) {
        if (shapeContainsPoint(shapes[i], canvasPos)) {
            // 如果已经在编辑文本，先完成之前的编辑
            if (isEditingText) {
                finishInlineTextEdit();
                // 强制更新画布
                canvasWidget->update();
            }
            // 开始新的文本编辑
            startInlineTextEdit(i);
            return;
        }
    }

    // 如果没有点击到图形，调用父类的处理函数
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::startInlineTextEdit(int shapeIndex)
{
    if (shapeIndex < 0 || shapeIndex >= shapes.size()) return;

    // 强制重置之前的编辑状态
    if (isEditingText) {
        if (inlineTextEdit) {
            inlineTextEdit->hide();
            inlineTextEdit->deleteLater();
        }
        isEditingText = false;
        editingShapeIndex = -1;
    }

    // 保存当前状态
    saveState();

    // 设置编辑状态
    editingShapeIndex = shapeIndex;
    isEditingText = true;

    // 创建新的文本编辑器
    inlineTextEdit = new QTextEdit(canvasWidget);
    inlineTextEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: white;"
        "   border: 2px solid #0078D7;"
        "   border-radius: 4px;"
        "   padding: 2px;"
        "}"
    );

    // 配置文本编辑器
    const Shape& shape = shapes[shapeIndex];
    inlineTextEdit->setText(shape.text);
    inlineTextEdit->setFont(shape.textFont);
    inlineTextEdit->setStyleSheet(
        QString("QTextEdit {"
            "   background-color: white;"
            "   border: 2px solid #0078D7;"
            "   border-radius: 4px;"
            "   padding: 2px;"
            "   color: %1;"
            "}")
        .arg(shape.textColor.name())
    );

    // 计算文本编辑器的位置和大小
    QRect editRect = shape.rect;

    // 根据图形类型调整编辑区域的大小
    int margin = 10;  // 边距
    if (shape.type == "Circle") {
        // 圆形使用较小的编辑区域
        int size = qMin(editRect.width(), editRect.height()) * 0.9;
        editRect.setWidth(size);
        editRect.setHeight(size);
    }
    else if (shape.type == "Triangle") {
        // 三角形使用较小的编辑区域
        editRect.setWidth(editRect.width() * 0.8);  // 使用80%的宽度
        editRect.setHeight(editRect.height() * 0.3);  // 使用30%的高度
    }
    else {
        // 其他图形使用适中的编辑区域
        editRect.adjust(margin, margin, -margin, -margin);
    }

    // 居中显示
    editRect.moveCenter(shape.rect.center());

    // 计算缩放后的位置
    QPoint center = editRect.center();
    int scaledWidth = editRect.width() * scaleFactor;
    int scaledHeight = editRect.height() * scaleFactor;

    // 创建最终的矩形，保持中心点不变
    QRect finalRect(
        center.x() - scaledWidth / 2 + canvasPosition.x(),
        center.y() - scaledHeight / 2 + canvasPosition.y(),
        scaledWidth,
        scaledHeight
    );

    // 设置文本编辑器位置和大小
    inlineTextEdit->setGeometry(finalRect);

    // 设置文本自适应和换行
    inlineTextEdit->setAlignment(Qt::AlignCenter);  // 文本居中对齐
    inlineTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);  // 启用自动换行
    inlineTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);  // 在单词边界或任意位置换行

    // 设置文本编辑器的边距
    inlineTextEdit->document()->setDocumentMargin(2);

    // 根据文本长度自动调整字体大小
    QFont font = shape.textFont;
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(shape.text);
    int availableWidth = finalRect.width() - 4;  // 减去边框宽度

    if (textWidth > availableWidth) {
        // 如果文本宽度超过可用宽度，计算合适的字体大小
        int fontSize = (availableWidth * font.pointSize()) / textWidth;
        fontSize = qMax(8, fontSize);  // 设置最小字体大小为8
        font.setPointSize(fontSize);
        inlineTextEdit->setFont(font);
    }

    // 连接文本变化信号，用于动态调整编辑器大小和字体
    connect(inlineTextEdit, &QTextEdit::textChanged, [this, finalRect, font]() {
        QTextDocument* doc = inlineTextEdit->document();
        QSizeF size = doc->size();

        // 计算新的高度，不限制最大高度
        int newHeight = size.height() + 4;

        // 计算新的位置，保持水平居中
        QRect newGeometry = finalRect;
        newGeometry.setHeight(newHeight);

        // 垂直居中
        int verticalOffset = (newHeight - finalRect.height()) / 2;
        newGeometry.translate(0, -verticalOffset);

        // 动态调整字体大小
        QFont currentFont = inlineTextEdit->font();
        QFontMetrics fm(currentFont);
        QString text = inlineTextEdit->toPlainText();
        int textWidth = fm.horizontalAdvance(text);

        if (textWidth > newGeometry.width() - 4) {
            int fontSize = ((newGeometry.width() - 4) * currentFont.pointSize()) / textWidth;
            fontSize = qMax(8, fontSize);
            currentFont.setPointSize(fontSize);
            inlineTextEdit->setFont(currentFont);
        }

        inlineTextEdit->setGeometry(newGeometry);
        });

    // 添加事件过滤器来处理ESC键和回车键
    inlineTextEdit->installEventFilter(this);

    // 确保编辑器可见并获取焦点
    inlineTextEdit->show();
    inlineTextEdit->setFocus();
    inlineTextEdit->selectAll();  // 选中所有文本

    // 强制更新画布
    canvasWidget->update();
}
void MainWindow::finishInlineTextEdit()
{
    if (!isEditingText || editingShapeIndex < 0) return;

    // 更新形状的文本
    shapes[editingShapeIndex].text = inlineTextEdit->toPlainText();

    // 重置编辑状态
    inlineTextEdit->hide();
    inlineTextEdit->deleteLater();
    inlineTextEdit = nullptr;
    isEditingText = false;
    editingShapeIndex = -1;

    // 更新画布
    canvasWidget->update();
}

void MainWindow::cancelInlineTextEdit()
{
    if (!isEditingText) return;

    // 重置编辑状态
    inlineTextEdit->hide();
    inlineTextEdit->deleteLater();
    inlineTextEdit = nullptr;
    isEditingText = false;
    editingShapeIndex = -1;

    // 更新画布
    canvasWidget->update();
}

// 获取特殊控制点的位置
QPoint MainWindow::getSpecialHandlePosition(const QRect& rect, int handleIndex, const QString& shapeType) {
    QPoint center = rect.center();

    if (shapeType == "Triangle") {
        // 计算三角形的四个特殊控制点位置
        QPoint top = QPoint(rect.left() + rect.width() / 2, rect.top());  // 顶点
        QPoint bottomLeft = QPoint(rect.left(), rect.bottom());  // 左下角
        QPoint bottomRight = QPoint(rect.right(), rect.bottom());  // 右下角
        QPoint bottomCenter = QPoint(rect.left() + rect.width() / 2, rect.bottom());  // 底边中点

        // 计算两个腰的中点
        QPoint leftMid = QPoint((top.x() + bottomLeft.x()) / 2, (top.y() + bottomLeft.y()) / 2);
        QPoint rightMid = QPoint((top.x() + bottomRight.x()) / 2, (top.y() + bottomRight.y()) / 2);

        switch (handleIndex) {
        case 0: return top;           // 顶点
        case 1: return leftMid;       // 左腰中点
        case 2: return rightMid;      // 右腰中点
        case 3: return bottomCenter;  // 底边中点
        default: return top;
        }
    }
    else {
        // 其他图形（矩形、圆形、菱形、开始结束框）的高亮点位置
        switch (handleIndex) {
        case 0: return QPoint(center.x(), rect.top());     // 上中点
        case 1: return QPoint(rect.right(), center.y());   // 右中点
        case 2: return QPoint(center.x(), rect.bottom());  // 下中点
        case 3: return QPoint(rect.left(), center.y());    // 左中点
        default: return QPoint(center.x(), rect.top());
        }
    }
}

// 获取特殊控制点的索引
int MainWindow::getSpecialHandleIndex(const QPoint& point, const QRect& rect, const QString& shapeType) {
    if (shapeType == "Triangle") {
        // 计算三角形的四个特殊控制点位置
        QPoint top = QPoint(rect.left() + rect.width() / 2, rect.top());  // 顶点
        QPoint bottomLeft = QPoint(rect.left(), rect.bottom());  // 左下角
        QPoint bottomRight = QPoint(rect.right(), rect.bottom());  // 右下角
        QPoint bottomCenter = QPoint(rect.left() + rect.width() / 2, rect.bottom());  // 底边中点

        // 计算两个腰的中点
        QPoint leftMid = QPoint((top.x() + bottomLeft.x()) / 2, (top.y() + bottomLeft.y()) / 2);
        QPoint rightMid = QPoint((top.x() + bottomRight.x()) / 2, (top.y() + bottomRight.y()) / 2);

        // 检查四个特殊控制点
        QPoint handles[4] = {
            top,           // 顶点 (0)
            leftMid,       // 左腰中点 (1)
            rightMid,      // 右腰中点 (2)
            bottomCenter   // 底边中点 (3)
        };

        for (int i = 0; i < 4; ++i) {
            if (QLineF(point, handles[i]).length() < SPECIAL_HANDLE_SIZE) {
                return i;
            }
        }
    }
    else {
        // 其他图形（矩形、圆形、菱形、开始结束框）的高亮点位置
        QPoint center = rect.center();
        QPoint handles[4] = {
            QPoint(center.x(), rect.top()),     // 上中点 (0)
            QPoint(rect.right(), center.y()),   // 右中点 (1)
            QPoint(center.x(), rect.bottom()),  // 下中点 (2)
            QPoint(rect.left(), center.y())     // 左中点 (3)
        };

        for (int i = 0; i < 4; ++i) {
            if (QLineF(point, handles[i]).length() < SPECIAL_HANDLE_SIZE) {
                return i;
            }
        }
    }
    return -1;  // 如果不在任何高亮点范围内，返回-1
}

void MainWindow::startDrawingFromHandle(const QPoint& pos, const Shape& shape)
{
    isDrawing = true;
    startPoint = pos;  // 使用特殊控制点作为起点
    endPoint = pos;

    currentShape = new Shape;
    currentShape->type = "Arrow";  // 设置为箭头类型
    currentShape->rect = QRect(startPoint, endPoint);
    currentShape->selected = false;

    // 设置箭头的初始样式
    currentShape->color = Qt::transparent;  // 箭头不需要填充
    currentShape->borderColor = Qt::black;  // 黑色边框
    currentShape->penStyle = Qt::SolidLine;  // 实线
    currentShape->opacity = 100;  // 不透明度100%
    currentShape->lineWidth = 3;  // 添加线条宽度设置
    currentShape->arrowStyle = 1;  // 默认使用单箭头样式

    // 记录箭头的起点来自哪个图形的哪个特殊控制点
    currentShape->sourceShapeIndex = shapes.indexOf(shape);
    currentShape->sourceHandleIndex = getSpecialHandleIndex(pos, shape.rect, shape.type);

    canvasWidget->update();
}


void MainWindow::showLineWidthMenu() {
    QMenu* menu = new QMenu(this);

    // 创建细线菜单项
    QAction* thinLine = menu->addAction(QIcon(":/image/thin_line.png"), "细");
    thinLine->setData(QVariant(1));  // 细线宽度为1

    // 创建中等线菜单项
    QAction* mediumLine = menu->addAction(QIcon(":/image/medium_line.png"), "中");
    mediumLine->setData(QVariant(3));  // 中等线宽度为3

    // 创建粗线菜单项
    QAction* thickLine = menu->addAction(QIcon(":/image/thick_line.png"), "粗");
    thickLine->setData(QVariant(6));  // 粗线宽度为5

    // 设置菜单样式
    menu->setStyleSheet(
        "QMenu {"
        "   background-color: #f0f0f0;"
        "   border: 1px solid #c0c0c0;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "}"
        "QMenu::item {"
        "   padding: 5px 20px 5px 20px;"
        "   border-radius: 3px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #e0e0e0;"
        "}"
    );

    QAction* selectedAction = menu->exec(QCursor::pos());
    if (selectedAction) {
        int width = selectedAction->data().toInt();

        // 保存状态用于撤销
        saveState();

        // 检查是否有多个图形被选中
        if (!selectedShapeIndices.isEmpty()) {
            // 批量修改所有选中图形的线条粗细
            for (int index : selectedShapeIndices) {
                if (index >= 0 && index < shapes.size()) {
                    shapes[index].lineWidth = width;
                }
            }
        }
        // 如果只有单个图形被选中
        else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            shapes[selectedShapeIndex].lineWidth = width;
        }

        // 更新线条粗细预览
        QLabel* lineWidthPreview = propertyPanel->findChild<QLabel*>("lineWidthPreview");
        if (lineWidthPreview && selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
            QPixmap pixmap(100, 20);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QPen pen(Qt::black, width);
            painter.setPen(pen);
            painter.drawLine(10, 10, 90, 10);
            lineWidthPreview->setPixmap(pixmap);
        }

        // 强制更新画布
        canvasWidget->update();
        update();  // 更新整个窗口
    }

    delete menu;
}

void MainWindow::startDraggingShape(const QString& shapeType, const QPoint& pos)
{
    isDraggingShape = true;
    draggedShapeType = shapeType;
    dragStartPoint = pos;
    dragCurrentPoint = pos;
}

void MainWindow::updateDraggingShape(const QPoint& pos)
{
    if (isDraggingShape) {
        dragCurrentPoint = pos;
        canvasWidget->update();
    }
}
// 完成拖拽，创建当前图形
void MainWindow::finishDraggingShape(const QPoint& pos)
{
    if (isDraggingShape) {
        // 检查是否在画布范围内
        QPoint tmpdragOffset = QPoint(250, 0);
        QPoint canvasPos = pos - canvasPosition - tmpdragOffset;
        if (canvasPos.x() >= 0 && canvasPos.x() <= pageWidth &&
            canvasPos.y() >= 0 && canvasPos.y() <= pageHeight) {

            // 创建新形状
            Shape newShape;
            newShape.type = draggedShapeType;
            newShape.rect = QRect(canvasPos.x() - 50, canvasPos.y() - 30, 100, 60); // 默认大小
            newShape.color = Qt::white;
            newShape.borderColor = Qt::black;
            newShape.penStyle = Qt::SolidLine;
            newShape.lineWidth = 3;
            newShape.selected = true;
            newShape.rotation = 0;
            newShape.opacity = 100;
            newShape.text = "";
            newShape.textFont = QFont("Arial", 12);
            newShape.textColor = Qt::black;

            // 清除其他形状的选中状态
            clearSelection();

            // 添加新形状
            shapes.append(newShape);
            selectedShapeIndex = shapes.size() - 1;

            // 保存状态用于撤销/重做
            saveState();

            // 更新画布
            canvasWidget->update();
        }
    }

    isDraggingShape = false;
    draggedShapeType = "";
}

void MainWindow::updateConnectedArrows(int shapeIndex) {
    // 检查是否在画布范围内
    if (shapeIndex >= 0 && shapeIndex < shapes.size()) {
        // 遍历所有图形，检查是否有箭头连接到该图形
        for (int i = 0; i < shapes.size(); ++i) {
            if (shapes[i].type == "Arrow") {
                // 更新起点连接的箭头
                if (shapes[i].sourceShapeIndex == shapeIndex) {
                    // 获取新的起点位置（特殊控制点的位置）
                    QPoint newStartPoint = getSpecialHandlePosition(shapes[shapeIndex].rect,
                        shapes[i].sourceHandleIndex, shapes[shapeIndex].type);
                    // 更新箭头的矩形
                    shapes[i].rect = QRect(newStartPoint, shapes[i].rect.bottomRight());
                }
                // 更新终点连接的箭头
                if (shapes[i].targetShapeIndex == shapeIndex) {
                    // 获取新的终点位置（特殊控制点的位置）
                    QPoint newEndPoint = getSpecialHandlePosition(shapes[shapeIndex].rect,
                        shapes[i].targetHandleIndex, shapes[shapeIndex].type);
                    // 更新箭头的矩形
                    shapes[i].rect = QRect(shapes[i].rect.topLeft(), newEndPoint);
                }
            }
        }
    }
}

// 复制选中的图形
void MainWindow::copySelectedShapes()
{
    // 清空之前的剪贴板内容
    clipboardShapes.clear();
    if (clipboardShape) {
        delete clipboardShape;
        clipboardShape = nullptr;
    }

    // 如果有多个图形被选中，复制所有选中的图形
    if (!selectedShapeIndices.isEmpty()) {
        for (int index : selectedShapeIndices) {
            if (index >= 0 && index < shapes.size()) {
                clipboardShapes.append(shapes[index]);
            }
        }
    }
    // 如果只有一个图形被选中，也添加到clipboardShapes，并保持向后兼容
    else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        clipboardShapes.append(shapes[selectedShapeIndex]);
        clipboardShape = new Shape(shapes[selectedShapeIndex]); // 向后兼容
    }
}

// 粘贴图形
void MainWindow::pasteShapes()
{
    if (!clipboardShapes.isEmpty() || clipboardShape) {
        saveState();

        // 计算偏移量，使新粘贴的图形错开一定距离
        QPoint offset(20, 20);

        // 粘贴多个图形
        if (!clipboardShapes.isEmpty()) {
            // 清除当前选择
            clearSelection();

            // 粘贴所有复制的图形
            for (const Shape& shape : clipboardShapes) {
                Shape newShape = shape;
                newShape.rect.translate(offset);
                newShape.selected = true;

                shapes.append(newShape);
                addToSelection(shapes.size() - 1);
            }
        }
        // 兼容旧版单图形粘贴
        else if (clipboardShape) {
            Shape newShape = *clipboardShape;
            newShape.rect.translate(offset);
            newShape.selected = true;

            // 清除当前选择并选中新粘贴的图形
            clearSelection();
            shapes.append(newShape);
            selectedShapeIndex = shapes.size() - 1;
        }

        canvasWidget->update();
    }
}

// 剪切选中的图形
void MainWindow::cutSelectedShapes()
{
    // 先复制
    copySelectedShapes();

    // 再删除
    if (!selectedShapeIndices.isEmpty()) {
        // 保存状态用于撤销
        saveState();

        // 从后向前删除选中的图形，避免索引变化问题
        QList<int> indicesToDelete = selectedShapeIndices;
        std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<int>());

        for (int index : indicesToDelete) {
            if (index >= 0 && index < shapes.size()) {
                shapes.removeAt(index);
            }
        }

        // 清空选择
        selectedShapeIndex = -1;
        selectedShapeIndices.clear();

        canvasWidget->update();
    }
    else if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        saveState();
        shapes.removeAt(selectedShapeIndex);
        selectedShapeIndex = -1;
        canvasWidget->update();
    }
}

void MainWindow::onBoldButtonClicked() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        saveState();
        QFont font = shapes[selectedShapeIndex].textFont;
        font.setBold(!font.bold());  // 切换加粗状态
        shapes[selectedShapeIndex].textFont = font;
        canvasWidget->update();
    }
}

void MainWindow::onItalicButtonClicked() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        saveState();
        QFont font = shapes[selectedShapeIndex].textFont;
        font.setItalic(!font.italic());  // 切换斜体状态
        shapes[selectedShapeIndex].textFont = font;
        canvasWidget->update();
    }
}

void MainWindow::onUnderlineButtonClicked() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        saveState();
        QFont font = shapes[selectedShapeIndex].textFont;
        font.setUnderline(!font.underline());  // 切换下划线状态
        shapes[selectedShapeIndex].textFont = font;
        canvasWidget->update();
    }
}

void MainWindow::onTextColorButtonClicked()
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        QColor currentColor = shapes[selectedShapeIndex].textColor;
        QColor color = QColorDialog::getColor(currentColor, this, "选择文本颜色");

        if (color.isValid()) {
            saveState();
            shapes[selectedShapeIndex].textColor = color;
            canvasWidget->update();
        }
    }
    else {
        QMessageBox::information(this, "选择形状", "请先选择一个形状。");
    }
}

void MainWindow::showArrowStyleMenu() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() && shapes[selectedShapeIndex].type == "Arrow") {
        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu {"
            "   background-color: #f5f5f5;"
            "   border: 1px solid #d8e1e9;"
            "   border-radius: 8px;"
            "   padding: 5px;"
            "}"
            "QMenu::item {"
            "   padding: 8px 25px 8px 20px;"
            "   border-radius: 4px;"
            "   margin: 2px 2px;"
            "}"
            "QMenu::item:selected {"
            "   background-color: #d8e1e9;"
            "}"
        );

        QAction* noArrowAction = menu.addAction("无箭头");
        QAction* singleArrowAction = menu.addAction("单箭头");
        QAction* doubleArrowAction = menu.addAction("双箭头");

        QAction* selectedAction = menu.exec(QCursor::pos());
        if (selectedAction) {
            saveState();
            if (selectedAction == noArrowAction) {
                shapes[selectedShapeIndex].arrowStyle = 0;
            } else if (selectedAction == singleArrowAction) {
                shapes[selectedShapeIndex].arrowStyle = 1;
            } else if (selectedAction == doubleArrowAction) {
                shapes[selectedShapeIndex].arrowStyle = 2;
            }
            updatePropertyPanel();
            canvasWidget->update();
        }
    }
}