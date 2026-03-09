#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include <QTextEdit>
#include <QObject>
#include <QMainWindow>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QToolBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QColorDialog>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QScrollArea>
#include <QSplitter>
#include <QMouseEvent>
#include <QToolButton>
#include <QIcon>
#include <QPainterPath>
#include <QDockWidget>
#include <QGroupBox>
#include <iostream>
#include <QWheelEvent>
#include <QtMath>
#include <QLineEdit>
#include <QTextLayout>
using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void selectionChanged();  // 选择状态改变信号
    void scaleFactorChanged(qreal factor);  // 缩放因子改变信号

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;  // 键盘事件
    void mousePressEvent(QMouseEvent* event) override;  // 鼠标按下事件
    void mouseDoubleClickEvent(QMouseEvent* event) override;  // 鼠标双击事件

private:
    QToolBar* toolbar;
    QPushButton* fileBtn;  // 添加File按钮
    QPushButton* startBtn;
    QPushButton* arrangeBtn;
    QPushButton* pageBtn;
    QPushButton* exportBtn;

    // 左侧工具栏
    QToolBar* leftToolbar;
    QToolButton* rectangleTool;
    QToolButton* circleTool;
    QToolButton* triangleTool;
    QToolButton* arrowTool;
    QToolButton* startEndTool;  // 新增
    QToolButton* diamondTool;   // 新增

    // 形状工具按钮
    QToolButton* rectButton;
    QToolButton* circleButton;
    QToolButton* triangleButton;
    QToolButton* arrowButton;

    // Secondary toolbars container
    QWidget* secondaryToolbarWidget;
    QVBoxLayout* secondaryToolbarLayout;

    // Second toolbars for each button
    QToolBar* startToolbar;
    QToolBar* arrangeToolbar;
    QToolBar* pageToolbar;
    QToolBar* exportToolbar;  // 添加Export工具栏

    // Canvas widget and scroll area
    QWidget* canvasWidget;
    QScrollArea* scrollArea;

    // Page settings
    QColor backgroundColor;
    int pageWidth;
    int pageHeight;
    bool showGrid;
    enum GridSize { None, Small, Medium, Large };
    GridSize currentGridSize;

    // Canvas position and dragging
    QPoint canvasPosition;
    bool isDragging;
    QPoint lastMousePos;

    // Start settings
    QString currentTool;

    // Arrange settings
    QString alignmentOption;

    // 绘图相关变量
    struct Shape {
        QString type;
        QRect rect;
        QColor color;      // 填充颜色
        QColor borderColor; // 边框颜色
        Qt::PenStyle penStyle;  // 添加线条样式属性
        int lineWidth;     // 添加线条粗细属性
        bool selected;     // 添加选中状态
        double rotation;   // 旋转角度
        int opacity;       // 添加不透明度属性，范围0-100
        QString text;      // 添加文本属性
        QFont textFont;    // 文本字体
        QColor textColor;  // 文本颜色
        Qt::Alignment textAlignment;  // 添加文本对齐方式属性
        int arrowStyle;    // 箭头样式：0-无箭头，1-单箭头，2-双箭头

        // 箭头连接信息
        int sourceShapeIndex;  // 箭头起点连接的图形索引
        int sourceHandleIndex; // 箭头起点连接的图形控制点索引
        int targetShapeIndex;  // 箭头终点连接的图形索引
        int targetHandleIndex; // 箭头终点连接的图形控制点索引

        // 添加相等运算符
        bool operator==(const Shape& other) const {
            return type == other.type &&
                rect == other.rect &&
                color == other.color &&
                borderColor == other.borderColor &&
                penStyle == other.penStyle &&
                selected == other.selected &&
                rotation == other.rotation &&
                opacity == other.opacity &&
                text == other.text &&
                textFont == other.textFont &&
                textColor == other.textColor &&
                textAlignment == other.textAlignment &&
                arrowStyle == other.arrowStyle &&
                sourceShapeIndex == other.sourceShapeIndex &&
                sourceHandleIndex == other.sourceHandleIndex &&
                targetShapeIndex == other.targetShapeIndex &&
                targetHandleIndex == other.targetHandleIndex;
        }
    };
    QList<Shape> shapes;
    Shape* currentShape;
    Shape* clipboardShape;  // 用于存储复制的形状
    QList<Shape> clipboardShapes;   // 用于存储复制的多个形状
    bool isDrawing;
    QPoint startPoint;
    QPoint endPoint;
    int selectedShapeIndex;  // 添加选中形状的索引

    // 撤销和重做功能相关
    QList<QList<Shape>> undoStack;  // 存储所有操作的历史记录
    QList<QList<Shape>> redoStack;  // 存储重做的历史记录
    void saveState();  // 保存当前状态
    void undo();  // 撤销操作
    void redo();  // 重做操作

    // 画布缩放相关
    double scaleFactor;  // 缩放因子
    void zoomIn();      // 放大
    void zoomOut();     // 缩小
    void centerCanvas(); // 居中画布

    //  菜单栏的子功能栏
    void createPageToolbar();
    void createStartToolbar();
    void createArrangeToolbar();
    void createExportToolbar();

    void createLeftToolbar();
    void hideAllSecondaryToolbars();
    void drawShape(QPainter& painter, const Shape& shape);
    void startDrawing(const QPoint& pos);
    void updateDrawing(const QPoint& pos);
    void finishDrawing();
    bool shapeContainsPoint(const Shape& shape, const QPoint& point);  // 检查点是否在形状内
    void clearSelection();  // 清除所有选择

    // 移动相关变量
    bool isMoving;
    QPoint moveStartPoint;
    int moveShapeIndex;

    // 调整大小相关变量
    bool isResizing;
    QPoint resizeStartPoint;
    int resizeHandle;  // 0-7 表示8个控制点
    static const int HANDLE_SIZE = 8;  // 图形形状控制点

    // 旋转相关变量
    bool isRotating;
    QPoint rotateStartPoint;
    double currentRotation;  // 当前旋转角度
    static const int ROTATE_HANDLE_DISTANCE = 30;  // 旋转控制点距离图形中心的距离

    // 左侧工具栏
    bool basicToolsExpanded;  // 基础工具组展开状态
    bool flowToolsExpanded;   // 流程图工具组展开状态
    QPushButton* basicTitleBtn;  // 基础工具标题按钮
    QPushButton* flowTitleBtn;   // 流程图工具标题按钮
    QWidget* basicToolsGroup;    // 基础工具组容器
    QWidget* flowToolsGroup;     // 流程图工具组容器

    // 属性面板相关
    QWidget* propertyPanel;  // 属性面板容器
    QVBoxLayout* propertyLayout;  // 属性面板布局
    QSpinBox* xSpinBox;      // X坐标SpinBox
    QSpinBox* ySpinBox;      // Y坐标SpinBox
    QSpinBox* widthSpinBox;  // 宽度SpinBox
    QSpinBox* heightSpinBox; // 高度SpinBox
    QSpinBox* arrangeXSpinBox;      // 工具栏中的X坐标SpinBox
    QSpinBox* arrangeYSpinBox;      // 工具栏中的Y坐标SpinBox
    QSpinBox* arrangeWidthSpinBox;  // 工具栏中的宽度SpinBox
    QSpinBox* arrangeHeightSpinBox; // 工具栏中的高度SpinBox
    void createPropertyPanel();  // 创建属性面板
    void updatePropertyPanel();  // 更新属性面板显示
    void showPropertyPanel();    // 显示属性面板
    void hidePropertyPanel();    // 隐藏属性面板
    QIcon createLineStyleIcon(Qt::PenStyle style);  // 添加线条样式图标创建函数声明

    // 形状控制点*8相关函数
    int getHandleAtPosition(const QPoint& pos, const Shape& shape);  // 获取鼠标位置对应的控制点索引
    void updateCursorForHandle(int handleIndex);  // 根据控制点更新鼠标样式

    // 高亮箭头绘制点*4
    static const int SPECIAL_HANDLE_SIZE = 6;   // 大小
    static const int SPECIAL_HANDLE_RADIUS = 3; // 半径
    QColor specialHandleColor = Qt::red;        // 颜色
    bool isHoveringOverShape;  // 是否悬停在图形上
    int hoveredShapeIndex;     // 当前悬停的图形索引

    // 绘制箭头绘制点
    void drawSpecialHandles(QPainter& painter, const Shape& shape);
    bool isPointNearSpecialHandle(const QPoint& point, const Shape& shape);  // 检查点是否在箭头绘制点附近
    // 从箭头绘制点开始绘制箭头
    void startDrawingFromHandle(const QPoint& pos, const Shape& shape);
    QPoint getSpecialHandlePosition(const QRect& rect, int handleIndex, const QString& shapeType);  // 获取位置
    int getSpecialHandleIndex(const QPoint& point, const QRect& rect, const QString& shapeType);    // 获取索引
    void updateConnectedArrows(int shapeIndex);  // 更新连接到图形的所有箭头

    void showLineStyleMenu();  // 显示线条样式菜单
    void showLineWidthMenu();  // 显示线条粗细菜单
    void showArrowStyleMenu();
    void onFillColorButtonClicked();

    QString getDashArray(Qt::PenStyle style);

    // 拖拽相关变量
    bool isDraggingShape;        // 是否正在拖拽形状
    QString draggedShapeType;    // 被拖拽的形状类型
    QPoint dragStartPoint;       // 拖拽开始点
    QPoint dragCurrentPoint;     // 拖拽当前点
    QToolButton* draggedButton;  // 当前被拖拽的按钮
    QPixmap draggedPixmap;       // 拖拽时显示的图标
    QPoint dragOffset;           // 鼠标相对于按钮的偏移量

    // 多选相关变量
    QList<int> selectedShapeIndices;  // 批量选中的图形索引列表
    bool isMultiSelecting;           // 是否处于多选模式（按住Ctrl键）

    // 框选相关变量
    bool isRectSelecting;           // 是否正在框选
    QRect selectionRect;            // 框选矩形
    QPoint selectStartPoint;        // 框选起始点

    // 拖拽相关函数
    void startDraggingShape(const QString& shapeType, const QPoint& pos);
    void updateDraggingShape(const QPoint& pos);
    void finishDraggingShape(const QPoint& pos);

    // 内嵌文本编辑相关
    QTextEdit* inlineTextEdit;  // 内嵌文本编辑控件
    int editingShapeIndex;      // 当前正在编辑的形状索引
    bool isEditingText;         // 是否正在编辑文本
    void startInlineTextEdit(int shapeIndex);  // 开始内嵌文本编辑
    void finishInlineTextEdit();  // 完成内嵌文本编辑
    void cancelInlineTextEdit();  // 取消内嵌文本编辑

private slots:

    // 右上角File
    void onFileClicked();
    void onNewFile();      // 新建文件
    void onOpenFile();     // 打开文件
    void onSaveFile();     // 保存文件

    // 文本格式化按钮槽函数
    void onBoldButtonClicked();
    void onItalicButtonClicked();
    void onUnderlineButtonClicked();

    //菜单栏四个属性点击
    void onStartClicked();
    void onArrangeClicked();
    void onPageClicked();
    void onExportClicked();

    void onBackgroundColorClicked(); //背景颜色
    void onPageSizeChanged(int width, int height); // 网页大小
    void onGridToggled(bool checked); // 网格显示
    void onToolSelected(const QString& tool);
    void onAlignmentChanged(const QString& alignment);
    void onShapeToolClicked();
    // 画布鼠标点击
    void onCanvasMousePress(QMouseEvent* event);
    void onCanvasMouseMove(QMouseEvent* event);
    void onCanvasMouseRelease(QMouseEvent* event);
    void onUndoClicked();  // 撤销按钮
    void onRedoClicked();  // 重做按钮
    void onBorderColorButtonClicked();  // 线条颜色按钮
    void onDeleteButtonClicked();  // 删除按钮
    void onClearCanvasClicked();  // 清空画布按钮

    // 复制、粘贴和剪切功能
    void copySelectedShapes();    // 复制选中的图形
    void pasteShapes();           // 粘贴图形
    void cutSelectedShapes();     // 剪切选中的图形

    void addToSelection(int shapeIndex); // 将图形添加到选择中
    void removeFromSelection(int shapeIndex); // 从选择中移除图形
    void toggleSelection(int shapeIndex); // 切换图形的选择状态
    void selectShapesInRect(const QRect& rect); // 选择矩形区域内的图形

    void onTextColorButtonClicked();
};
#endif // MAINWINDOW_H