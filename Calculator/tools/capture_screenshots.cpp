// 生成实验报告所需的界面截图。
// 用法（在仓库根目录执行，建议使用离屏平台）：
//   QT_QPA_PLATFORM=offscreen build/Calculator/capture_screenshots.exe
#include "calculator.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QPushButton>

namespace {

void loadStyle(QApplication &app)
{
    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
}

void click(Calculator &calculator, const QString &buttonName)
{
    if (auto *button = calculator.findChild<QPushButton *>(buttonName))
        button->click();
}

void press(Calculator &calculator, int key)
{
    QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
    QCoreApplication::sendEvent(&calculator, &event);
}

void saveShot(Calculator &calculator, const QString &fileName)
{
    QCoreApplication::processEvents();
    calculator.grab().save(fileName);
}

QString shotPath(const QString &fileName)
{
    return QStringLiteral("docs/screenshots/") + fileName;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    loadStyle(app);

    QDir().mkpath(QStringLiteral("docs/screenshots"));

    Calculator calculator;
    calculator.resize(360, 540);
    calculator.show();
    QCoreApplication::processEvents();

    saveShot(calculator, shotPath(QStringLiteral("01-main.png")));

    // 鼠标完成带小数的运算：1.5 + 2.5 = 4
    click(calculator, QStringLiteral("btn1"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    click(calculator, QStringLiteral("btnEquals"));
    saveShot(calculator, shotPath(QStringLiteral("02-decimal-result.png")));

    // 键盘完成运算：3 * 4 = 12
    press(calculator, Qt::Key_Escape);
    press(calculator, Qt::Key_3);
    press(calculator, Qt::Key_Asterisk);
    press(calculator, Qt::Key_4);
    press(calculator, Qt::Key_Return);
    saveShot(calculator, shotPath(QStringLiteral("03-keyboard-result.png")));

    // 退格：输入 123 后退格得到 12
    press(calculator, Qt::Key_Escape);
    press(calculator, Qt::Key_1);
    press(calculator, Qt::Key_2);
    press(calculator, Qt::Key_3);
    press(calculator, Qt::Key_Backspace);
    saveShot(calculator, shotPath(QStringLiteral("04-backspace.png")));

    // 除零：8 / 0 =
    press(calculator, Qt::Key_Escape);
    press(calculator, Qt::Key_8);
    press(calculator, Qt::Key_Slash);
    press(calculator, Qt::Key_0);
    press(calculator, Qt::Key_Return);
    saveShot(calculator, shotPath(QStringLiteral("05-divide-by-zero.png")));

    // 连续运算：2 + 3 = 5，再 + 4 = 9
    press(calculator, Qt::Key_Escape);
    press(calculator, Qt::Key_2);
    press(calculator, Qt::Key_Plus);
    press(calculator, Qt::Key_3);
    press(calculator, Qt::Key_Return);
    press(calculator, Qt::Key_Plus);
    press(calculator, Qt::Key_4);
    press(calculator, Qt::Key_Return);
    saveShot(calculator, shotPath(QStringLiteral("06-continuous.png")));

    return 0;
}
